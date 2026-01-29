

#include "LIB/stdtypes.h"
#include "MCAL/GPIO_Driver/gpio_int.h"

#include "MCAL/SPI_Driver/spi_priv.h"
#include "MCAL/SPI_Driver/spi.h"

// for each spi number , for each flag , store its callback
// 0 > RXNE_COMPLETED
// 1 > TXE_COMPLETED
// 2 > NOT USED *****************
// 3 > UNDERFLOW
// 4 > OVERRUN
// 5 > CRC_ERROR
// 6 > MODE_FAULT
// 7 > BUSY_FLAG
SPI_Callback_t SPI_Tx_Callbacks[SPI_NUMBER][NUMBER_OF_FLAGS] = {
    {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL},
    {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL},
    {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL},
    {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL}
};

static SPI_State_t SPI_State[] = {
    SPI_NOT_BUSY,
    SPI_NOT_BUSY,
    SPI_NOT_BUSY,
    SPI_NOT_BUSY
};

static uint16_t SPI_MaskData[SPI_NUMBER] = {
    0x00FF, // For SPI1
    0x00FF, // For SPI2
    0x00FF, // For SPI3
    0x00FF  // For SPI4
};


static const volatile SPI_Registers_t* SPI_Instances[] = {
    SPI1_BASE_ADDRESS,
    SPI2_BASE_ADDRESS,
    SPI3_BASE_ADDRESS,
    SPI4_BASE_ADDRESS
};

static uint16_t *SPIReceivedData[SPI_NUMBER] = {NULL,NULL,NULL,NULL};

static SPI_Status_t Init_SPI_Pins(SPI_Config_t* config);

SPI_Status_t SPI_enuInit(SPI_Config_t* SpiConfig){
    SPI_Status_t retStatus = SPI_NOT_OK;

    if(SpiConfig == NULL){
        retStatus = SPI_NOT_OK;
    } else if(SpiConfig->spiNumber > SPI_NUMBER_MASK){
        retStatus = SPI_WRONG_SPI_NUMBER;
    }else if((SpiConfig->communicationMode & SPI_COMMUNICATION_MODE_MASK) != 0){
        retStatus = SPI_WRONG_COMMUNICATION_MODE;
    }else if((SpiConfig->mode & SPI_MODE_MASK) != 0){
        retStatus = SPI_WRONG_MODE;
    }else if((SpiConfig->crcState & SPI_CRC_MASK) != 0){
        retStatus = SPI_WRONG_CRC_STATE;
    }else if((SpiConfig->dataLength & SPI_DATA_LENGTH_MASK) != 0){
        retStatus = SPI_WRONG_DATA_LENGTH;
    }else if((SpiConfig->dataOrder & SPI_DATA_ORDER_MASK) != 0){
        retStatus = SPI_WRONG_DATA_ORDER;
    }else if((SpiConfig->baudRate & SPI_BAUDRATE_MASK) != 0){
        retStatus = SPI_WRONG_BAUDRATE;
    }else if((SpiConfig->polarityPhase & SPI_POLARITY_PHASE_MASK) != 0){
        retStatus = SPI_WRONG_POLARITY_PHASE;
    }else if((SpiConfig->frameFormat & SPI_FRAME_FORMAT_MASK) != 0){
        retStatus = SPI_WRONG_FRAME_FORMAT;
    }else if((SpiConfig->dmaState & SPI_DMA_MASK) != 0){
        retStatus = SPI_WRONG_DMA_STATE;
    }else if((SpiConfig->nssManagement & SPI_NSS_MASK) != 0){
        retStatus = SPI_WRONG_NSS_MANAGEMENT;
    }else{

        retStatus = Init_SPI_Pins(SpiConfig);
        if(retStatus == SPI_OK){
            
            volatile SPI_Registers_t* SPIx = (volatile SPI_Registers_t*)SPI_Instances[SpiConfig->spiNumber];
            
            // Disable SPI before configuration
            SPIx->CR1 &= DISABLE_SPI;
            
            // Configure SPI according to SpiConfig
            SPIx->CR1 |= SpiConfig->communicationMode;
            SPIx->CR1 |= SpiConfig->mode;
            SPIx->CR1 |= SpiConfig->crcState;
            SPIx->CR1 |= SpiConfig->dataLength;
            SPIx->CR1 |= SpiConfig->dataOrder;
            SPIx->CR1 |= SpiConfig->baudRate;
            SPIx->CR1 |= SpiConfig->polarityPhase;
            SPIx->CR2 |= SpiConfig->frameFormat;
            SPIx->CR2 |= SpiConfig->dmaState;

            SPIx->CR1 |= (SpiConfig->nssManagement & SPI_NSS_CR1_SELECTION_MASK);
            SPIx->CR2 |= (SpiConfig->nssManagement & SPI_NSS_CR2_SELECTION_MASK);
            
            // Configure CRC Polynomial
            SPIx->CRCPR = SpiConfig->crcPolynomial;
            
            // Enable SPI after configuration
            SPIx->CR1 |= ENABLE_SPI;

            if(SpiConfig->dataLength == SPI_16_BIT_DATA){
                SPI_MaskData[SpiConfig->spiNumber] = 0xFFFF;
            } else {
                SPI_MaskData[SpiConfig->spiNumber] = 0x00FF;
            }
            retStatus = SPI_OK;
        } else {
            retStatus = SPI_GPIO_NOT_INITIALIZED;
        }
    }
    return retStatus;
}


SPI_Status_t SPI_enuMasterSyncTransmitReceive(SPI_Number_t spiNumber, uint16_t TxData, uint16_t* RxData){
    SPI_Status_t retStatus = SPI_NOT_OK;

    if(RxData == NULL){
        retStatus = SPI_NULL_POINTER;
    } else if(spiNumber > SPI_NUMBER_MASK){
        retStatus = SPI_WRONG_SPI_NUMBER;
    } else {
        volatile SPI_Registers_t* SPIx = (volatile SPI_Registers_t*)SPI_Instances[spiNumber];
        
        // Wait until TXE (Transmit buffer empty) flag is set
        while(((SPIx->SR >> SPI_FLAG_TXE)& SPI_GET_FIRST_BIT_MASK) == 0);
        
        // Send data
        SPIx->DR = TxData;
        
        // Wait until RXNE (Receive buffer not empty) flag is set
        while(((SPIx->SR >> SPI_FLAG_RXNE)& SPI_GET_FIRST_BIT_MASK) == 0);
        
        // Read received data
        if(SPI_MaskData[spiNumber] == 0x00FF){
            *(uint8_t*)RxData = SPIx->DR & 0x00FF;
        } else {
            *(uint16_t*)RxData = SPIx->DR;
        }

        while (((SPIx->SR >> SPI_FLAG_BUSY) & SPI_GET_FIRST_BIT_MASK) == 1); // Wait until SPI is not busy
        
        retStatus = SPI_OK;
    }

    return retStatus;
}

SPI_Status_t SPI_enuMasterSyncTransmit(SPI_Number_t spiNumber, uint16_t TxData){
    uint16_t dummyRxData; // Variable to hold dummy received data
    return SPI_enuMasterSyncTransmitReceive(spiNumber, TxData, &dummyRxData);
}

SPI_Status_t SPI_enuMasterSyncReceive(SPI_Number_t spiNumber ,uint16_t *RxData){
    uint16_t dummyTxData = SPI_MaskData[spiNumber]; // Dummy data to initiate clock
    return SPI_enuMasterSyncTransmitReceive(spiNumber, dummyTxData, RxData);
}

SPI_Status_t SPI_enuSlaveSyncTransmit(SPI_Number_t spiNumber,uint16_t TxData){
    SPI_Status_t retStatus = SPI_NOT_OK;

    if(spiNumber > SPI_NUMBER_MASK){
        retStatus = SPI_WRONG_SPI_NUMBER;
    } else {
        volatile SPI_Registers_t* SPIx = (volatile SPI_Registers_t*)SPI_Instances[spiNumber];
        
        // Wait until TXE (Transmit buffer empty) flag is set
        while(((SPIx->SR >> SPI_FLAG_TXE)& SPI_GET_FIRST_BIT_MASK) == 0);
        
        // Send data
        SPIx->DR = TxData;
        
        retStatus = SPI_OK;
    }

    return retStatus;
}

SPI_Status_t SPI_enuSlaveSyncReceive(SPI_Number_t spiNumber,uint16_t* RxData){
    SPI_Status_t retStatus = SPI_NOT_OK;

    if(RxData == NULL){
        retStatus = SPI_NULL_POINTER;
    } else if(spiNumber > SPI_NUMBER_MASK){
        retStatus = SPI_WRONG_SPI_NUMBER;
    } else {
        volatile SPI_Registers_t* SPIx = (volatile SPI_Registers_t*)SPI_Instances[spiNumber];
        
        // Wait until RXNE (Receive buffer not empty) flag is set
        while(((SPIx->SR >> SPI_FLAG_RXNE)& SPI_GET_FIRST_BIT_MASK) == 0);
        
        // Read received data
        if(SPI_MaskData[spiNumber] == 0x00FF){
            *(uint8_t*)RxData = SPIx->DR & 0x00FF;
        } else {
            *(uint16_t*)RxData = SPIx->DR;
        }
        
        retStatus = SPI_OK;
    }

    return retStatus;
}

SPI_Status_t SPI_enuMasterSelectSlave(SPI_Number_t spiNumber,SPI_SlaveCS_t slave){
    SPI_Status_t retStatus = SPI_NOT_OK;

    if(spiNumber > SPI_NUMBER_MASK){
        retStatus = SPI_WRONG_SPI_NUMBER;
    } else {
        
        // Set NSS pin low to select the slave
        // Assuming a function GPIO_enuSetPinValue exists
        GPIO_Status_t status = GPIO_enuSetPinVal(slave.port,
                                                  slave.pin,
                                                  GPIO_HIGH); 
        if(status != GPIO_OK){
            retStatus = SPI_ERROR_SELECTING_SLAVE;
        }else{
            retStatus = SPI_OK;
        }       
    }

    return retStatus;
}


SPI_Status_t SPI_enuMasterAsynTransmit(SPI_Number_t spiNumber, uint16_t TxData, SPI_Callback_t callback){
    SPI_Status_t retStatus = SPI_NOT_OK;

    // Implementation of asynchronous transmit would go here
    // This typically involves setting up interrupts and using a state machine
    // For brevity, this function is left unimplemented

    if(spiNumber > SPI_NUMBER_MASK){
        retStatus = SPI_WRONG_SPI_NUMBER;
    } else {
        // Check if SPI is busy
        if((SPI_State[spiNumber] | SPI_u8ReadFlag(spiNumber,SPI_FLAG_BUSY)) == 1){
            retStatus = SPI_STATUS_IS_BUSY; // Indicate busy error
        } else if(callback == NULL){
            retStatus = SPI_NULL_POINTER; // Indicate null pointer error
        } else {
            SPI_State[spiNumber] = SPI_BUSY;
            // Setup interrupt and callback here
            // ...

            volatile SPI_Registers_t* SPIx = (volatile SPI_Registers_t*)SPI_Instances[spiNumber];
            SPI_enuRegisterCallback(spiNumber, SPI_FLAG_TXE, callback);
            SPI_enuClearFlag(spiNumber, SPI_FLAG_TXE);
            SPI_enuEnableInterrupt(spiNumber, SPI_FLAG_TXE);
            // Start transmission
            SPIx->DR = TxData;
            
            retStatus = SPI_OK;
        }
    }
    return retStatus;
}


SPI_Status_t SPI_enuSlaveAsynTransmit(SPI_Number_t spiNumber, uint16_t TxData, SPI_Callback_t callback){
    SPI_Status_t retStatus = SPI_NOT_OK;

    // Implementation of asynchronous transmit would go here
    // This typically involves setting up interrupts and using a state machine
    // For brevity, this function is left unimplemented

    if(spiNumber > SPI_NUMBER_MASK){
        retStatus = SPI_WRONG_SPI_NUMBER;
    } else {
        // Check if SPI is busy
        if((SPI_State[spiNumber] | SPI_u8ReadFlag(spiNumber,SPI_FLAG_BUSY)) == 1){
            retStatus = SPI_STATUS_IS_BUSY; // Indicate busy error
        } else if(callback == NULL){
            retStatus = SPI_NULL_POINTER; // Indicate null pointer error
        } else {
            SPI_State[spiNumber] = SPI_BUSY;
            // Setup interrupt and callback here
            // ...

            SPI_enuRegisterCallback(spiNumber, SPI_FLAG_TXE, callback);
            SPI_enuClearFlag(spiNumber, SPI_FLAG_TXE);
            SPI_enuEnableInterrupt(spiNumber, SPI_FLAG_TXE);
            // Start transmission
            volatile SPI_Registers_t* SPIx = (volatile SPI_Registers_t*)SPI_Instances[spiNumber];
            SPIx->DR = TxData;
            
            retStatus = SPI_OK;
        }
    }
    return retStatus;
}



SPI_Status_t SPI_enuMasterAsynReceive(SPI_Number_t spiNumber, uint16_t* RxData,SPI_Callback_t callback){
    SPI_Status_t retStatus = SPI_NOT_OK;

    // Implementation of asynchronous receive would go here
    // This typically involves setting up interrupts and using a state machine
    // For brevity, this function is left unimplemented

    if(spiNumber > SPI_NUMBER_MASK){
        retStatus = SPI_WRONG_SPI_NUMBER;
    } else {
        // Check if SPI is busy
        if((SPI_State[spiNumber] | SPI_u8ReadFlag(spiNumber,SPI_FLAG_BUSY)) == 1){
            retStatus = SPI_STATUS_IS_BUSY; // Indicate busy error
        } else if((callback == NULL) || (RxData == NULL)){
            retStatus = SPI_NULL_POINTER; // Indicate null pointer error
        } else {
            SPI_State[spiNumber] = SPI_BUSY;
            // Setup interrupt and callback here
            // ...

            SPIReceivedData[spiNumber] = RxData;

            SPI_enuRegisterCallback(spiNumber, SPI_FLAG_RXNE, callback);
            SPI_enuClearFlag(spiNumber, SPI_FLAG_RXNE);
            SPI_enuEnableInterrupt(spiNumber, SPI_FLAG_RXNE);

            // Start reception by sending dummy data
            volatile SPI_Registers_t* SPIx = (volatile SPI_Registers_t*)SPI_Instances[spiNumber];
            SPIx->DR = SPI_MaskData[spiNumber]; // Send dummy data
            
            retStatus = SPI_OK;
        }
    }
    return retStatus;
}

SPI_Status_t SPI_enuSlaveAsynReceive(SPI_Number_t spiNumber, uint16_t* RxData,SPI_Callback_t callback){
    SPI_Status_t retStatus = SPI_NOT_OK;

    // Implementation of asynchronous receive would go here
    // This typically involves setting up interrupts and using a state machine
    // For brevity, this function is left unimplemented

    if(spiNumber > SPI_NUMBER_MASK){
        retStatus = SPI_WRONG_SPI_NUMBER;
    } else {
        // Check if SPI is busy
        if((SPI_State[spiNumber] | SPI_u8ReadFlag(spiNumber,SPI_FLAG_BUSY)) == 1){
            retStatus = SPI_STATUS_IS_BUSY; // Indicate busy error
        } else if((callback == NULL) || (RxData == NULL)){
            retStatus = SPI_NULL_POINTER; // Indicate null pointer error
        } else {
            SPI_State[spiNumber] = SPI_BUSY;
            // Setup interrupt and callback here
            // ...

            SPIReceivedData[spiNumber] = RxData;

            SPI_enuRegisterCallback(spiNumber, SPI_FLAG_RXNE, callback);
            SPI_enuClearFlag(spiNumber, SPI_FLAG_RXNE);
            SPI_enuEnableInterrupt(spiNumber, SPI_FLAG_RXNE);

            retStatus = SPI_OK;
        }
    }
    return retStatus;
}

SPI_Status_t SPI_enuMasterDeselectSlave(SPI_Number_t spiNumber,SPI_SlaveCS_t slave){
    SPI_Status_t retStatus = SPI_NOT_OK;

    if(spiNumber > SPI_NUMBER_MASK){
        retStatus = SPI_WRONG_SPI_NUMBER;
    } else {
        
        // Set NSS pin high to deselect the slave
        // Assuming a function GPIO_enuSetPinValue exists
        GPIO_Status_t status = GPIO_enuSetPinVal(slave.port,
                                                  slave.pin,
                                                  GPIO_LOW); 
        if(status != GPIO_OK){
            retStatus = SPI_ERROR_SELECTING_SLAVE;
        }else{
            retStatus = SPI_OK;
        }       
    }

    return retStatus;
}

uint8_t SPI_u8ReadFlag(SPI_Number_t spiNumber,SPI_Flag_t flag){
    uint8_t flagStatus = 0;

    if((spiNumber > SPI_NUMBER_MASK)||(flag > SPI_FLAG_FRAME_FORMAT_ERROR)||(flag == SPI_NOT_USED_FLAG)){
        flagStatus = 0xFF; // Indicate error for invalid SPI number
    }else{
        volatile SPI_Registers_t* SPIx = (volatile SPI_Registers_t*)SPI_Instances[spiNumber];
        flagStatus = (SPIx->SR >> flag) & SPI_GET_FIRST_BIT_MASK;
    }
    return flagStatus;
}

SPI_Status_t SPI_enuEnableInterrupt(SPI_Number_t spiNumber, SPI_Flag_t flag){
    SPI_Status_t retStatus = SPI_NOT_OK;

    if(spiNumber > SPI_NUMBER_MASK){
        retStatus = SPI_WRONG_SPI_NUMBER; // Indicate error for invalid SPI number
    }else if ((flag > SPI_FLAG_FRAME_FORMAT_ERROR)||(flag == SPI_NOT_USED_FLAG)){
        retStatus = SPI_WRONG_FLAG_VALUE; // Indicate error for invalid flag
    }else{
        volatile SPI_Registers_t* SPIx = (volatile SPI_Registers_t*)SPI_Instances[spiNumber];
        // Enable the corresponding interrupt
        switch(flag){
            case SPI_FLAG_RXNE:
                SPIx->CR2 |= SPI_INTERRUPT_ENABLE_RXNE_COMPLETED;
                break;
            case SPI_FLAG_TXE:
                SPIx->CR2 |= SPI_INTERRUPT_ENABLE_TXE_COMPLETED;
                break;
            case SPI_FLAG_UNDERRUN_ERROR:
            case SPI_FLAG_CRC_ERROR:
            case SPI_FLAG_MODE_FAULT:
            case SPI_FLAG_OVERRUN_ERROR:
                SPIx->CR2 |= SPI_INTERRUPT_ENABLE_ERR_OCCURRED;
                break;
            default:
                // do nothing >> case not possible
                break;
        }
        retStatus = SPI_OK;
    }
    return retStatus;
}

SPI_Status_t SPI_enuDisableInterrupt(SPI_Number_t spiNumber, SPI_Flag_t flag){
    SPI_Status_t retStatus = SPI_NOT_OK;

    if(spiNumber > SPI_NUMBER_MASK){
        retStatus = SPI_WRONG_SPI_NUMBER; // Indicate error for invalid SPI number
    }else if ((flag > SPI_FLAG_FRAME_FORMAT_ERROR)||(flag == SPI_NOT_USED_FLAG)){
        retStatus = SPI_WRONG_FLAG_VALUE; // Indicate error for invalid flag
    }else{
        volatile SPI_Registers_t* SPIx = (volatile SPI_Registers_t*)SPI_Instances[spiNumber];
        // Disable the corresponding interrupt
        switch(flag){
            case SPI_FLAG_RXNE:
                SPIx->CR2 &= SPI_INTERRUPT_DISABLE_RXNE_COMPLETED;
                break;
            case SPI_FLAG_TXE:
                SPIx->CR2 &= SPI_INTERRUPT_DISABLE_TXE_COMPLETED;
                break;
            case SPI_FLAG_UNDERRUN_ERROR:
            case SPI_FLAG_CRC_ERROR:
            case SPI_FLAG_MODE_FAULT:
            case SPI_FLAG_OVERRUN_ERROR:
                SPIx->CR2 &= SPI_INTERRUPT_DISABLE_ERR_OCCURRED;
                break;
            default:
                // do nothing >> case not possible
                break;
        }
        retStatus = SPI_OK;
    }
    return retStatus;
}

SPI_Status_t SPI_enuClearFlag(SPI_Number_t spiNumber,SPI_Flag_t flag){
    SPI_Status_t retStatus = SPI_NOT_OK;

    if(spiNumber > SPI_NUMBER_MASK){
        retStatus = SPI_WRONG_SPI_NUMBER; // Indicate error for invalid SPI number
    }else if ((flag > SPI_FLAG_FRAME_FORMAT_ERROR)||(flag == SPI_NOT_USED_FLAG)){
        retStatus = SPI_WRONG_FLAG_VALUE; // Indicate error for invalid flag
    }else{
        volatile SPI_Registers_t* SPIx = (volatile SPI_Registers_t*)SPI_Instances[spiNumber];
        // Clear the corresponding flag by writing 1 to it
        SPIx->SR |= (1 << flag);
        retStatus = SPI_OK;
    }
    return retStatus;
}


SPI_Status_t SPI_enuRegisterCallback(SPI_Number_t spiNumber, SPI_Flag_t flag, SPI_Callback_t callback){
    SPI_Status_t retStatus = SPI_NOT_OK;

    if(spiNumber > SPI_NUMBER_MASK){
        retStatus = SPI_WRONG_SPI_NUMBER; // Indicate error for invalid SPI number
    }else if ((flag > SPI_FLAG_FRAME_FORMAT_ERROR)||(flag == SPI_NOT_USED_FLAG)){
        retStatus = SPI_WRONG_FLAG_VALUE; // Indicate error for invalid flag
    }else if(callback == NULL){
        retStatus = SPI_NULL_POINTER; // Indicate null pointer error
    }else{
        SPI_Tx_Callbacks[spiNumber][flag] = callback;
        retStatus = SPI_OK;
    }
    return retStatus;
}




static SPI_Status_t Init_SPI_Pins(SPI_Config_t* config) {
    SPI_Status_t status = SPI_NOT_OK;
    SPI_PinsConfig_t pinsConfig;
    GPIO_cfg_t pinConfig;
    GPIO_AlternateFunction_t altFunction;
    uint8_t i;

    if(config == NULL){
        return SPI_NULL_POINTER;
    }

    // Initialize all pins as unused
    pinsConfig.MISO.usedOrNot = 0;
    pinsConfig.MOSI.usedOrNot = 0;
    pinsConfig.SCK.usedOrNot  = 0;
    pinsConfig.NSS.usedOrNot  = 0;

    // Determine alternate function based on SPI number
    switch(config->spiNumber){
        case SPI1:
            altFunction = GPIO_AF5;
            pinsConfig.MISO.port = GPIO_PORT_A;
            pinsConfig.MISO.pin  = GPIO_PIN_6;
            pinsConfig.MOSI.port = GPIO_PORT_A;
            pinsConfig.MOSI.pin  = GPIO_PIN_7;
            pinsConfig.SCK.port  = GPIO_PORT_A;
            pinsConfig.SCK.pin   = GPIO_PIN_5;
            pinsConfig.NSS.port  = GPIO_PORT_A;
            pinsConfig.NSS.pin   = GPIO_PIN_4;
            break;
        case SPI2:
            altFunction = GPIO_AF5;
            pinsConfig.MISO.port = GPIO_PORT_B;
            pinsConfig.MISO.pin  = GPIO_PIN_14;
            pinsConfig.MOSI.port = GPIO_PORT_B;
            pinsConfig.MOSI.pin  = GPIO_PIN_15;
            pinsConfig.SCK.port  = GPIO_PORT_B;
            pinsConfig.SCK.pin   = GPIO_PIN_13;
            pinsConfig.NSS.port  = GPIO_PORT_B;
            pinsConfig.NSS.pin   = GPIO_PIN_12;
            break;
        case SPI3:
            altFunction = GPIO_AF6;
            pinsConfig.MISO.port = GPIO_PORT_C;
            pinsConfig.MISO.pin  = GPIO_PIN_11;
            pinsConfig.MOSI.port = GPIO_PORT_C;
            pinsConfig.MOSI.pin  = GPIO_PIN_12;
            pinsConfig.SCK.port  = GPIO_PORT_C;
            pinsConfig.SCK.pin   = GPIO_PIN_10;
            pinsConfig.NSS.port  = GPIO_PORT_A;
            pinsConfig.NSS.pin   = GPIO_PIN_15;
            break;
        case SPI4:
            altFunction = GPIO_AF5;
            pinsConfig.MISO.port = GPIO_PORT_E;
            pinsConfig.MISO.pin  = GPIO_PIN_14;
            pinsConfig.MOSI.port = GPIO_PORT_E;
            pinsConfig.MOSI.pin  = GPIO_PIN_13;
            pinsConfig.SCK.port  = GPIO_PORT_E;
            pinsConfig.SCK.pin   = GPIO_PIN_12;
            pinsConfig.NSS.port  = GPIO_PORT_E;
            pinsConfig.NSS.pin   = GPIO_PIN_11;
            break;
        default:
            return SPI_WRONG_SPI_NUMBER;
    }

    // Determine which pins to use based on mode and communication mode
    if(config->mode == SPI_MASTER) {
        // SCK is always OUTPUT for master
        pinsConfig.SCK.usedOrNot = 1;

        // SPI_FULL_DUPLEX and SPI_HALF_DUPLEX_2LINES_TX_ONLY have same value, so use if-else
        if(config->communicationMode == SPI_FULL_DUPLEX) {
            // Master Full Duplex or TX-Only 2-Lines: SCK=OUT, MOSI=OUT
            // For Full Duplex: MISO=IN also used
            // For TX-Only: MISO=FREE (not configured)
            pinsConfig.MOSI.usedOrNot = 1;
            
            // In hardware, Full Duplex and TX-Only 2-Lines are same config (BIDIMODE=0, RXONLY=0)
            // But in Full Duplex, MISO is connected; in TX-Only it may be left unconnected
            // We'll configure MISO for Full Duplex to allow receiving
            pinsConfig.MISO.usedOrNot = 1;
            
        } else if(config->communicationMode == SPI_HALF_DUPLEX_2LINES_RX_ONLY) {
            // Master RX-Only 2-Lines: SCK=OUT, MISO=IN, MOSI=FREE
            pinsConfig.MISO.usedOrNot = 1;
            
        } else if(config->communicationMode == SPI_HALF_DUPLEX_1LINE_TX_ONLY) {
            // Master 1-Line BIDI TX: SCK=OUT, MOSI=OUT (bidirectional), MISO=FREE
            pinsConfig.MOSI.usedOrNot = 1;
            
        } else if(config->communicationMode == SPI_HALF_DUPLEX_1LINE_RX_ONLY) {
            // Master 1-Line BIDI RX: SCK=OUT, MOSI=IN (bidirectional), MISO=FREE
            pinsConfig.MOSI.usedOrNot = 1;
            
        } else {
            return SPI_WRONG_COMMUNICATION_MODE;
        }

        // Check if NSS hardware management is used
        if(config->nssManagement == SPI_NSS_MASTER_HW_OUTPUT || 
           config->nssManagement == SPI_NSS_MASTER_HW_INPUT) {
            pinsConfig.NSS.usedOrNot = 1;
            config->slavesConfig.numberOfSlaves = 0; // no external pins needed for slaves in HW NSS mode
        }else if(config->nssManagement == SPI_NSS_MASTER_SW) {
            // NSS managed by software, no pin configuration needed
        }else{
            return SPI_WRONG_NSS_MANAGEMENT;
        }

    } else if(config->mode == SPI_SLAVE) {
        // SCK is always INPUT for slave
        pinsConfig.SCK.usedOrNot = 1;

        // SPI_FULL_DUPLEX and SPI_HALF_DUPLEX_2LINES_TX_ONLY have same value, so use if-else
        if(config->communicationMode == SPI_FULL_DUPLEX) {
            // Slave Full Duplex or TX-Only 2-Lines: SCK=IN, MOSI=IN
            // For Full Duplex: MISO=OUT also used
            // For TX-Only: MISO=FREE (not configured)
            pinsConfig.MOSI.usedOrNot = 1;
            
            // Configure MISO for Full Duplex transmission
            pinsConfig.MISO.usedOrNot = 1;
            
        } else if(config->communicationMode == SPI_HALF_DUPLEX_2LINES_RX_ONLY) {
            // Slave RX-Only 2-Lines: SCK=IN, MISO=OUT, MOSI=N/A
            pinsConfig.MISO.usedOrNot = 1;
            
        } else if(config->communicationMode == SPI_HALF_DUPLEX_1LINE_TX_ONLY) {
            // Slave 1-Line BIDI TX: SCK=IN, MISO=OUT (bidirectional), MOSI=FREE
            pinsConfig.MISO.usedOrNot = 1;
            
        } else if(config->communicationMode == SPI_HALF_DUPLEX_1LINE_RX_ONLY) {
            // Slave 1-Line BIDI RX: SCK=IN, MISO=IN (bidirectional), MOSI=FREE
            pinsConfig.MISO.usedOrNot = 1;
            
        } else {
            return SPI_WRONG_COMMUNICATION_MODE;
        }

        // Check if NSS hardware management is used
        if(config->nssManagement == SPI_NSS_SLAVE_HW) {
            pinsConfig.NSS.usedOrNot = 1;
        }else if(config->nssManagement == SPI_NSS_SLAVE_SW) {
            // NSS always selected in software mode, no pin configuration needed
        }else {
            return SPI_WRONG_NSS_MANAGEMENT;
        }

    } else {
        return SPI_WRONG_MODE;
    }

    // Configure common pin settings
    pinConfig.mode              = GPIO_MODE_ALTERNATE_FUNCTION;
    pinConfig.outputType        = GPIO_OUTPUT_TYPE_PUSH_PULL;
    pinConfig.speed             = GPIO_SPEED_VERY_HIGH;
    pinConfig.pull              = GPIO_NO_PULL;
    pinConfig.alternateFunction = altFunction;

    // Loop through and configure each used pin
    SPI_Pinout_t* pins[] = {&pinsConfig.MISO, &pinsConfig.MOSI, &pinsConfig.SCK, &pinsConfig.NSS};
    
    for(i = 0; i < 4; i++) {
        if(pins[i]->usedOrNot == 1) {
            pinConfig.port = pins[i]->port;
            pinConfig.pin = pins[i]->pin;
            
            if(GPIO_enuInit(&pinConfig) != GPIO_OK) {
                return SPI_GPIO_NOT_INITIALIZED;
            }
        }
    }

    for (i=0; i<config->slavesConfig.numberOfSlaves;i++){
        pinConfig.port       = config->slavesConfig.slaves[i].port;
        pinConfig.pin        = config->slavesConfig.slaves[i].pin;
        pinConfig.mode       = GPIO_MODE_OUTPUT; // NSS pins for slaves are outputs from master perspective
        pinConfig.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
        pinConfig.speed      = GPIO_SPEED_DEFAULT;
        pinConfig.pull       = GPIO_NO_PULL;

        if(GPIO_enuInit(&pinConfig) != GPIO_OK) {
            return SPI_GPIO_NOT_INITIALIZED;
        }
    }

    status = SPI_OK;
    return status;
}


static void SPI_Private_IRQHandler(SPI_Number_t spiNumber){
    if(SPI_u8ReadFlag(spiNumber,SPI_FLAG_RXNE) == 1){
        // Disable RXNE interrupt
        SPI_enuDisableInterrupt(spiNumber, SPI_FLAG_RXNE);
        // Call the registered callback for RXNE
        if(SPI_Tx_Callbacks[spiNumber][SPI_FLAG_RXNE] != NULL){
            // Read received data
            volatile SPI_Registers_t* SPIx = (volatile SPI_Registers_t*)SPI_Instances[spiNumber];

            if(SPI_MaskData[spiNumber] == 0x00FF){
                *(uint8_t*)(SPIReceivedData[spiNumber]) = SPIx->DR & 0x00FF;
            } else {
                *(uint16_t*)(SPIReceivedData[spiNumber]) = SPIx->DR& 0xFFFF;
            }

            SPI_Tx_Callbacks[spiNumber][SPI_FLAG_RXNE]();
            SPI_State[spiNumber] = SPI_NOT_BUSY;
        }
    }

    if(SPI_u8ReadFlag(spiNumber,SPI_FLAG_TXE) == 1){
        // Call the registered callback for TXE
        SPI_enuDisableInterrupt(spiNumber, SPI_FLAG_TXE);
        if(SPI_Tx_Callbacks[spiNumber][SPI_FLAG_TXE] != NULL){
            SPI_Tx_Callbacks[spiNumber][SPI_FLAG_TXE]();
            SPI_State[spiNumber] = SPI_NOT_BUSY;
        }
    }

    if(SPI_u8ReadFlag(spiNumber,SPI_FLAG_OVERRUN_ERROR) == 1){
        // Call the registered callback for OVERRUN_ERROR
        if(SPI_Tx_Callbacks[spiNumber][SPI_FLAG_OVERRUN_ERROR] != NULL){
            SPI_Tx_Callbacks[spiNumber][SPI_FLAG_OVERRUN_ERROR]();
        }
        SPI_enuClearFlag(spiNumber,SPI_FLAG_OVERRUN_ERROR);
    }

    if(SPI_u8ReadFlag(spiNumber,SPI_FLAG_UNDERRUN_ERROR) == 1){
        // Call the registered callback for UNDERRUN_ERROR
        if(SPI_Tx_Callbacks[spiNumber][SPI_FLAG_UNDERRUN_ERROR] != NULL){
            SPI_Tx_Callbacks[spiNumber][SPI_FLAG_UNDERRUN_ERROR]();
        }
        SPI_enuClearFlag(spiNumber,SPI_FLAG_UNDERRUN_ERROR);
    }

    if(SPI_u8ReadFlag(spiNumber,SPI_FLAG_CRC_ERROR) == 1){
        // Call the registered callback for CRC_ERROR
        if(SPI_Tx_Callbacks[spiNumber][SPI_FLAG_CRC_ERROR] != NULL){
            SPI_Tx_Callbacks[spiNumber][SPI_FLAG_CRC_ERROR]();
        }
        SPI_enuClearFlag(spiNumber,SPI_FLAG_CRC_ERROR);
    }

    if(SPI_u8ReadFlag(spiNumber,SPI_FLAG_MODE_FAULT) == 1){
        // Call the registered callback for MODE_FAULT
        if(SPI_Tx_Callbacks[spiNumber][SPI_FLAG_MODE_FAULT] != NULL){
            SPI_Tx_Callbacks[spiNumber][SPI_FLAG_MODE_FAULT]();
        }
        SPI_enuClearFlag(spiNumber,SPI_FLAG_MODE_FAULT);
    }

    if(SPI_u8ReadFlag(spiNumber,SPI_FLAG_FRAME_FORMAT_ERROR) == 1){
        // Call the registered callback for FRAME_FORMAT_ERROR
        if(SPI_Tx_Callbacks[spiNumber][SPI_FLAG_FRAME_FORMAT_ERROR] != NULL){
            SPI_Tx_Callbacks[spiNumber][SPI_FLAG_FRAME_FORMAT_ERROR]();
        }
        SPI_enuClearFlag(spiNumber,SPI_FLAG_FRAME_FORMAT_ERROR);
    }
    
}



void SPI1_IRQHandler(void){
    SPI_Private_IRQHandler(SPI1);
}

void SPI2_IRQHandler(void){
    SPI_Private_IRQHandler(SPI2);
}

void SPI3_IRQHandler(void){
    SPI_Private_IRQHandler(SPI3);
}

void SPI4_IRQHandler(void){
    SPI_Private_IRQHandler(SPI4);
}

