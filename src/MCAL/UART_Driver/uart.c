#include "LIB/stdtypes.h"
#include <string.h>
#include "MCAL/GPIO_Driver/gpio_int.h"
#include "MCAL/UART_Driver/uart_priv.h"
#include "MCAL/UART_Driver/uart.h"


void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART6_IRQHandler(void);
static uint16_t CalculateBaudRate(uint32_t peripheralClock, uint32_t baudRate, UART_OverSampling_t oversampling) ;
static UART_Status_t Init_UART_Pins(UART_Config_t* config);
static void USART_LocalHandler(UART_Number_t uartNumber);

static LocalFlags_t LocalFlags = {0};
static UART_InitState_t UART_InitState = UART_NOT_INIT;
static UARTRegs_t* UART_Registers[] = {
    UART1_BASE_ADDR,
    UART2_BASE_ADDR,
    UART6_BASE_ADDR
};
static UARTY_State_t UART_Tx_State[] = {
    UART_READY,
    UART_READY,
    UART_READY
};
static UARTY_State_t UART_Rx_State[] = {
    UART_READY,
    UART_READY,
    UART_READY
};
static UART_Callbacks_t UartCallbacks[3] = {
    { .ParityErrorCallback = NULL, .FramingErrorCallback = NULL, .NoiseErrorCallback = NULL, .OverrunErrorCallback = NULL ,.TC_Callback = NULL},
    { .ParityErrorCallback = NULL, .FramingErrorCallback = NULL, .NoiseErrorCallback = NULL, .OverrunErrorCallback = NULL ,.TC_Callback = NULL},
    { .ParityErrorCallback = NULL, .FramingErrorCallback = NULL, .NoiseErrorCallback = NULL, .OverrunErrorCallback = NULL ,.TC_Callback = NULL}
};
static UART_AsynBuffer_t *TxBuffers[3] = {
    NULL,
    NULL,
    NULL
};
static UART_AsynBuffer_t *RxBuffers[3] = {
    NULL,
    NULL,
    NULL
};

UART_Status_t UART_enuInit(UART_Config_t* config) {
    
    UART_Status_t status = UART_NOT_OK;

    if (config == NULL) {
        status = UART_NULL_PTR;
    }else{
        if(config->UART_Number > UART_6){
            status = UART_WRONG_UART_NUMBER;
        }else{
            if((config->UartEnabled & UART_ENABLE_MASK) != 0){
                status = UART_WRONG_UART_ENABLE;
            }else{
                if((config->Parity & UART_PARITY_MASK) != 0){
                    status = UART_WRONG_PARITY;
                }else{
                    if((config->OverSampling & UART_OVERSAMPLING_MASK) != 0){
                        status = UART_WRONG_OVERSAMPLING;
                    }else{
                        if((config->StopBits & UART_STOPBITS_MASK) != 0){
                            status = UART_WRONG_STOPBITS;
                        }else{
                            if((config->WordLength & UART_WORDLENGTH_MASK) != 0){
                                status = UART_WRONG_WORDLENGTH;
                            }else{
                                if((config->Sample & UART_SAMPLE_MASK) != 0){
                                    status = UART_WRONG_SAMPLE;
                                }else{
                                    if((config->InterruptFlags & UART_INTERRUPT_MASK) != 0){
                                        status = UART_WRONG_INTERRUPT_FLAGS;
                                    }else{
                                        // Initialize UART pins
                                        status = Init_UART_Pins(config);
                                        if(status == UART_OK){

                                            volatile UARTRegs_t* uart = UART_Registers[config->UART_Number];

                                            uart->CR1 |= (config->OverSampling | config->Parity | config->WordLength |config->UartEnabled);
                                            
                                            uart->CR1 |= (config->InterruptFlags & UART_CR1_FLAGS_MASK);
                                            
                                            uart->CR2 |= config->StopBits;

                                            uart->CR3 |= config->Sample;

                                            uart->CR3 |= (config->InterruptFlags & UART_CR3_FLAGS_MASK);

                                            // Calculate and set baud rate
                                            uart->BRR = CalculateBaudRate(config->PeripheralClock, config->BaudRate, config->OverSampling);

                                            uart->CR1 |= UART_ENABLE; // Enable UART

                                            UART_InitState = UART_INIT;
                                            status = UART_OK;
                                        }else{
                                            // GPIO error
                                            status = UART_GPIO_ERROR;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return status;
}


UART_Status_t UART_enuSynTransmitBuffer(UART_Number_t uartNumber, const uint8_t* txBuffer, uint16_t size) {
    UART_Status_t status = UART_NOT_OK;

    if (txBuffer == NULL) {
        status = UART_NULL_PTR;
    } else {
        if(uartNumber > UART_6){
            status = UART_WRONG_UART_NUMBER;
        }else{
            if(UART_InitState != UART_INIT) {
                status = UART_NOT_INIT_SUCCESSFULLY;
            }else{

                UARTRegs_t* uart = UART_Registers[uartNumber];
                for (uint16_t i = 0; i < size; i++) {
                    // Wait until transmit data register is empty
                    while (UART_u8ReadTXEFlag(uartNumber) == 0); // TXE flag
                
                    // Write data to data register
                    uart->DR = txBuffer[i];
                    // Wait until transmission is complete
                    while (UART_u8ReadTCFlag(uartNumber) == 0); // TC flag
                }            
                status = UART_OK;
            }
        }
    }
    return status;
}

UART_Status_t UART_enuAsynTransmitBuffer(UART_Number_t uartNumber, UART_AsynBuffer_t* txBuffer) {
    UART_Status_t status = UART_NOT_OK;

    if(txBuffer == NULL) {
        status = UART_NULL_PTR;
    } else {
        if(uartNumber > UART_6){
            status = UART_WRONG_UART_NUMBER;
        }else{
            if(UART_InitState != UART_INIT) {
                status = UART_NOT_INIT_SUCCESSFULLY;
            }else{
                if(UART_Tx_State[uartNumber] == UART_BUSY) {
                    status = UART_TX_BUSY; // UART is busy
                } else {
                    UART_Tx_State[uartNumber] = UART_BUSY;
                    
                    // Store the transmission buffer
                    TxBuffers[uartNumber] = txBuffer;
                    TxBuffers[uartNumber]->index = 0;

                    // Start transmission by sending the first byte
                    UARTRegs_t* uart = UART_Registers[uartNumber];

                    // Wait until transmit data register is empty
                    // i am sure that it is empty here but just to be safe
                    while (UART_u8ReadTXEFlag(uartNumber) == 0); // TXE flag

                    // Write data to data register
                    uart->DR = TxBuffers[uartNumber]->buffer[TxBuffers[uartNumber]->index++];

                    // Enable TXE interrupt
                    uart->CR1 |= UART_INTERRUPT_TXE_LOCAL_ENABLE;

                    status = UART_OK;
                }
            }
        }
    }
    return status;
}




UART_Status_t UART_enuSynReceiveBuffer(UART_Number_t uartNumber, uint8_t* rxBuffer, uint16_t size) {
    UART_Status_t status = UART_NOT_OK;

    if (rxBuffer == NULL) {
        status = UART_NULL_PTR;
    } else {
        if(uartNumber > UART_6){
            status = UART_WRONG_UART_NUMBER;
        }else{
            if(UART_InitState != UART_INIT) {
                status = UART_NOT_INIT_SUCCESSFULLY;
            }else{

                UARTRegs_t* uart = UART_Registers[uartNumber];
                for (uint16_t i = 0; i < size; i++) {
                    // Wait until receive data register is not empty
                    while (UART_u8ReadRXNEFlag(uartNumber) == 0); // RXNE flag
                
                    // Read data from data register
                    rxBuffer[i] = (uint8_t)(uart->DR & 0xFF);
                }            
                status = UART_OK;
            }
        }
    }
    return status;
}


UART_Status_t UART_enuAsynReceiveBuffer(UART_Number_t uartNumber, UART_AsynBuffer_t* rxBuffer) {

    UART_Status_t status = UART_NOT_OK;

    if(rxBuffer == NULL) {
        status = UART_NULL_PTR;
    } else {
        if(uartNumber > UART_6){
            status = UART_WRONG_UART_NUMBER;
        }else{
            if(UART_InitState != UART_INIT) {
                status = UART_NOT_INIT_SUCCESSFULLY;
            }else{
                if(UART_Rx_State[uartNumber] == UART_BUSY) {
                    status = UART_TX_BUSY; // UART is busy
                } else {
                    UART_Rx_State[uartNumber] = UART_BUSY;
                    
                    // Store the reception buffer
                    RxBuffers[uartNumber] = rxBuffer;
                    RxBuffers[uartNumber]->index = 0;

                    // Start reception by enabling RXNE interrupt
                    UARTRegs_t* uart = UART_Registers[uartNumber];

                    // Enable RXNE interrupt
                    uart->CR1 |= UART_INTERRUPT_RXNE; // RXNE interrupt enable

                    status = UART_OK;
                }
                
            }
        }
    }

    return status;
}

UART_Status_t UART_enuActivateDMA(UART_Number_t uartNumber, uint32_t enableDmaFlag){
    UART_Status_t status = UART_NOT_OK;

    if(uartNumber > UART_6){
        status = UART_WRONG_UART_NUMBER;
    }else{
        if(UART_InitState != UART_INIT) {
            status = UART_NOT_INIT_SUCCESSFULLY;
        }else{
            UARTRegs_t* uart = UART_Registers[uartNumber];
            if((enableDmaFlag & UART_DMA_MASK) != 0){
                status  = UART_WRONG_DMA_ENABLE;
            }else{ 
                // Enable DMA for transmission and reception
                uart->CR3 |= enableDmaFlag;

                status = UART_OK;
            }
        }
    }
    return status;
}

UART_Status_t UART_enuEnableInterrupts(UART_Number_t uartNumber, uint32_t interruptFlags){
    UART_Status_t status = UART_NOT_OK;

    if(uartNumber > UART_6){
        status = UART_WRONG_UART_NUMBER;
    }else{
        if(UART_InitState != UART_INIT) {
            status = UART_NOT_INIT_SUCCESSFULLY;
        }else{
            if((interruptFlags & UART_INTERRUPT_MASK) != 0){
                status  = UART_WRONG_INTERRUPT_FLAGS;
            }else{ 
                // Enable specified interrupts
                UARTRegs_t* uart = UART_Registers[uartNumber];
                uart->CR1 |= (interruptFlags & UART_CR1_FLAGS_MASK);
                uart->CR3 |= (interruptFlags & UART_CR3_FLAGS_MASK);
                status = UART_OK;
            }
        }
    }
    return status;
}

UART_Status_t UART_enuDisableInterrupts(UART_Number_t uartNumber, uint32_t interruptFlags){
    UART_Status_t status = UART_NOT_OK;

    if(uartNumber > UART_6){
        status = UART_WRONG_UART_NUMBER;
    }else{
        if(UART_InitState != UART_INIT) {
            status = UART_NOT_INIT_SUCCESSFULLY;
        }else{
            if((interruptFlags & UART_INTERRUPT_MASK) != 0){
                status  = UART_WRONG_INTERRUPT_FLAGS;
            }else{ 
                // Disable specified interrupts
                UARTRegs_t* uart = UART_Registers[uartNumber];
                uart->CR1 &= ~ (interruptFlags & UART_CR1_FLAGS_MASK);
                uart->CR3 &= ~ (interruptFlags & UART_CR3_FLAGS_MASK);
                status = UART_OK;
            }
        }
    }
    return status;
}


UART_Status_t UART_enuRegisterCallbacks(UART_Number_t uartNumber, UART_Callbacks_t* callbacks) {
    UART_Status_t status = UART_NOT_OK;

    if(callbacks == NULL) {
        status = UART_NULL_PTR;
    } else {
        if(uartNumber > UART_6){
            status = UART_WRONG_UART_NUMBER;
        }else{
            UartCallbacks[uartNumber].FramingErrorCallback = callbacks->FramingErrorCallback;
            UartCallbacks[uartNumber].ParityErrorCallback = callbacks->ParityErrorCallback;
            UartCallbacks[uartNumber].NoiseErrorCallback = callbacks->NoiseErrorCallback;
            UartCallbacks[uartNumber].OverrunErrorCallback = callbacks->OverrunErrorCallback;
            UartCallbacks[uartNumber].TC_Callback = callbacks->TC_Callback;
            status = UART_OK;
        }
    }
    return status;
}

UART_Status_t UART_enuClearFlags(UART_Number_t uartNumber,uint32_t interruptFlags) {
    UART_Status_t status = UART_NOT_OK;

    if(uartNumber > UART_6){
        status = UART_WRONG_UART_NUMBER;
    }else{
        if(UART_InitState != UART_INIT) {
            status = UART_NOT_INIT_SUCCESSFULLY;
        }else{
            UARTRegs_t* uart = UART_Registers[uartNumber];
            // Clear flags by writing 0 to them
            uart->SR &= ~interruptFlags;
            status = UART_OK;
        }
    }
    return status;
}

uint8_t UART_u8ReadTXEFlag(UART_Number_t uartNumber) {
    UARTRegs_t* uart = UART_Registers[uartNumber];
    return ((uart->SR >> UART_TXE_FLAG_POSITION) & 1); 
}


uint8_t UART_u8ReadTCFlag(UART_Number_t uartNumber) {
    UARTRegs_t* uart = UART_Registers[uartNumber];
    return ((uart->SR >> UART_TC_FLAG_POSITION) & 1);
}

uint8_t UART_u8ReadRXNEFlag(UART_Number_t uartNumber) {
    UARTRegs_t* uart = UART_Registers[uartNumber];
    return ((uart->SR >> UART_RXNE_FLAG_POSITION) & 1);
}

uint8_t UART_u8ReadOREFlag(UART_Number_t uartNumber) {
    UARTRegs_t* uart = UART_Registers[uartNumber];
    return ((uart->SR >> UART_ORE_FLAG_POSITION) & 1);
}

uint8_t UART_u8ReadNoiseFlag(UART_Number_t uartNumber) {
    UARTRegs_t* uart = UART_Registers[uartNumber];
    return ((uart->SR >> UART_NOISE_FLAG_POSITION) & 1);
}

uint8_t UART_u8ReadFEFlag(UART_Number_t uartNumber) {
    UARTRegs_t* uart = UART_Registers[uartNumber];
    return ((uart->SR >> UART_FE_FLAG_POSITION) & 1);
}

uint8_t UART_u8ReadPEFlag(UART_Number_t uartNumber) {
    UARTRegs_t* uart = UART_Registers[uartNumber];
    return ((uart->SR >> UART_PE_FLAG_POSITION) & 1);
}

static uint16_t CalculateBaudRate(uint32_t peripheralClock, uint32_t baudRate, UART_OverSampling_t oversampling) {
    uint16_t brr;
    
    if (oversampling == 0)
    {
        // // 16x oversampling: BRR = fck / baud (naturally correct format)
        // brr = (uint16_t)((peripheralClock + (baudRate >> 1U)) / baudRate);

        uint16_t usartdiv =(peripheralClock) /(16 * baudRate);   // oversampling by 16(OVER8 = 0)
        uint16_t fraction =((peripheralClock %(16 * baudRate)) * 16 +(16 * baudRate)/2) /(16 * baudRate);  // get rem*16 "to be from 0:15" then rounding Rounding
        brr =(usartdiv << 4) | fraction;
    }
    else
    {
        // 8x oversampling
        uint32_t usartdiv_x8 = (peripheralClock + (baudRate >> 1U)) / baudRate;
        uint16_t mantissa = (uint16_t)(usartdiv_x8 >> 3U);
        uint16_t fraction = (uint16_t)(usartdiv_x8 & 0x07U);
        brr = (mantissa << 4U) | fraction;
    }
    
    return brr;
}

static UART_Status_t Init_UART_Pins(UART_Config_t* config) {
    UART_Status_t status = UART_NOT_OK;
    GPIO_Status_t gpioStatus = GPIO_OK;

    GPIO_cfg_t gpioConfig = {
        .mode = GPIO_MODE_ALTERNATE_FUNCTION,
        .outputType = GPIO_OUTPUT_TYPE_PUSH_PULL,
        .speed = GPIO_SPEED_DEFAULT,
        .pull = GPIO_NO_PULL,
    };
    
    switch (config->UART_Number) {
        case UART_1:
        if((config->UartEnabled & UART_ENABLE_TRANSMITE) != 0){
            // Initialize GPIO pins for UART1 (PA9 - TX, PA10 - RX)
            gpioConfig.port = GPIO_PORT_A;
            gpioConfig.pin = GPIO_PIN_9;
            gpioConfig.alternateFunction = GPIO_AF7;
            gpioStatus = GPIO_enuInit(&gpioConfig);  
        }else{
            // continue
        }
        if(gpioStatus != GPIO_OK){
            status = UART_GPIO_ERROR;
        }else{
            if((config->UartEnabled & UART_ENABLE_RECEIVE) != 0){
                gpioConfig.port = GPIO_PORT_A;
                gpioConfig.pin = GPIO_PIN_10;
                gpioConfig.alternateFunction = GPIO_AF7;
                gpioStatus = GPIO_enuInit(&gpioConfig);
            }else{
                // continue
            }
            if(gpioStatus != GPIO_OK){
                status = UART_GPIO_ERROR;
            }else{
                status = UART_OK;
            }
        }
            break;
        case UART_2:
        if((config->UartEnabled & UART_ENABLE_TRANSMITE) != 0){
            // Initialize GPIO pins for UART2 (PA2 - TX, PA3 - RX)
            gpioConfig.port = GPIO_PORT_A;
            gpioConfig.pin = GPIO_PIN_2;
            gpioConfig.alternateFunction = GPIO_AF7;
            gpioStatus = GPIO_enuInit(&gpioConfig);  
        }else{
            // continue
        }
        if(gpioStatus != GPIO_OK){
            status = UART_GPIO_ERROR;
        }else{
            if((config->UartEnabled & UART_ENABLE_RECEIVE) != 0){
                gpioConfig.port = GPIO_PORT_A;
                gpioConfig.pin = GPIO_PIN_3;
                gpioConfig.alternateFunction = GPIO_AF7;
                gpioStatus = GPIO_enuInit(&gpioConfig);
            }else{
                // continue
            }
            if(gpioStatus != GPIO_OK){
                status = UART_GPIO_ERROR;
            }else{
                status = UART_OK;
            }
        }
            break;
        case UART_6:
        if((config->UartEnabled & UART_ENABLE_TRANSMITE) != 0){
            // Initialize GPIO pins for UART6 (PC6 - TX, PC7 - RX)
            gpioConfig.port = GPIO_PORT_C;
            gpioConfig.pin = GPIO_PIN_6;
            gpioConfig.alternateFunction = GPIO_AF8;
            gpioStatus = GPIO_enuInit(&gpioConfig);  
        }else{
            // continue
        }
        if(gpioStatus != GPIO_OK){  
            status = UART_GPIO_ERROR;
        }else{
            if((config->UartEnabled & UART_ENABLE_RECEIVE) != 0){
                gpioConfig.port = GPIO_PORT_C;
                gpioConfig.pin = GPIO_PIN_7;
                gpioConfig.alternateFunction = GPIO_AF8;
                gpioStatus = GPIO_enuInit(&gpioConfig);
            }else{
                // continue
            }
            if(gpioStatus != GPIO_OK){  
                status = UART_GPIO_ERROR;
            }else{
                status = UART_OK;
            }
        }
            break;
        default:
            status = UART_WRONG_UART_NUMBER;
            break;
    }

    return status;
}



static void USART_LocalHandler(UART_Number_t uartNumber) {
    UARTRegs_t* uart = UART_Registers[uartNumber];

        if(LocalFlags.RXNE_Flag== 1) {
        if(RxBuffers[uartNumber] != NULL) {
            if(RxBuffers[uartNumber]->index < RxBuffers[uartNumber]->size) {
                // Read received byte
                RxBuffers[uartNumber]->buffer[RxBuffers[uartNumber]->index++] = (uint8_t)(uart->DR & 0xFF);
            } 
            if(RxBuffers[uartNumber]->index >= RxBuffers[uartNumber]->size) {
                // Reception complete
                // disable RXNE interrupt
                uart->CR1 &= UART_INTERRUPT_RXNE_LOCAL_DISABLE;

                UART_Rx_State[uartNumber] = UART_READY;

                // Call the callback function if set
                if(RxBuffers[uartNumber]->callback != NULL) {
                    RxBuffers[uartNumber]->callback(); 
                }
            }
        }
    }

    // Check if TXE flag is set
    if(LocalFlags.TXE_Flag == 1) {
        if(TxBuffers[uartNumber] != NULL) {
            if(TxBuffers[uartNumber]->index < TxBuffers[uartNumber]->size) {
                // Send next byte
                uart->DR = TxBuffers[uartNumber]->buffer[TxBuffers[uartNumber]->index++];
            } else {
                // disable TXE interrupt
                uart->CR1 &= UART_INTERRUPT_TXE_LOCAL_DISABLE;

                // Transmission complete
                UART_Tx_State[uartNumber] = UART_READY;

                // Call the callback function if set
                if(TxBuffers[uartNumber]->callback != NULL) {
                    TxBuffers[uartNumber]->callback(); 
                }
            }
        }
    }

    if(LocalFlags.ParityErrorFlag == 1) {
        // Parity Error
        if(UartCallbacks[uartNumber].ParityErrorCallback != NULL) {
            UartCallbacks[uartNumber].ParityErrorCallback();
        }
    }

    if(LocalFlags.FramingErrorFlag == 1) {
        // Framing Error
        if(UartCallbacks[uartNumber].FramingErrorCallback != NULL) {
            UartCallbacks[uartNumber].FramingErrorCallback();
        }
    }

    if(LocalFlags.NoiseErrorFlag == 1) {
        // Noise Error
        if(UartCallbacks[uartNumber].NoiseErrorCallback != NULL) {
            UartCallbacks[uartNumber].NoiseErrorCallback();
        }
    }

    if(LocalFlags.OverrunErrorFlag == 1) {
        // Overrun Error
        if(UartCallbacks[uartNumber].OverrunErrorCallback != NULL) {
            UartCallbacks[uartNumber].OverrunErrorCallback();
        }
    }

    if(LocalFlags.TC_Flag == 1) {
        // Transmission Complete
        if(UartCallbacks[uartNumber].TC_Callback != NULL) {
            UartCallbacks[uartNumber].TC_Callback();
        }
    }
}


void USART1_IRQHandler(void) {

    // i put this part here because the debugger when read the Dr register it will clear some flags
    // so i need to read the flags first before calling the local handler
    // this step isn't necessary if i am not using debugger
    LocalFlags.FramingErrorFlag  = UART_u8ReadFEFlag(UART_1);
    LocalFlags.NoiseErrorFlag    = UART_u8ReadNoiseFlag(UART_1);
    LocalFlags.OverrunErrorFlag  = UART_u8ReadOREFlag(UART_1);
    LocalFlags.ParityErrorFlag   = UART_u8ReadPEFlag(UART_1);
    LocalFlags.TC_Flag           = UART_u8ReadTCFlag(UART_1);
    LocalFlags.TXE_Flag          = UART_u8ReadTXEFlag(UART_1);
    LocalFlags.RXNE_Flag         = UART_u8ReadRXNEFlag(UART_1);


    USART_LocalHandler(UART_1);
}

void USART2_IRQHandler(void) {

    // i put this part here because the debugger when read the Dr register it will clear some flags
    // so i need to read the flags first before calling the local handler
    // this step isn't necessary if i am not using debugger
    LocalFlags.FramingErrorFlag  = UART_u8ReadFEFlag(UART_2);
    LocalFlags.NoiseErrorFlag    = UART_u8ReadNoiseFlag(UART_2);
    LocalFlags.OverrunErrorFlag  = UART_u8ReadOREFlag(UART_2);
    LocalFlags.ParityErrorFlag   = UART_u8ReadPEFlag(UART_2);
    LocalFlags.TC_Flag           = UART_u8ReadTCFlag(UART_2);
    LocalFlags.TXE_Flag          = UART_u8ReadTXEFlag(UART_2);
    LocalFlags.RXNE_Flag         = UART_u8ReadRXNEFlag(UART_2);

    USART_LocalHandler(UART_2);
}

void USART6_IRQHandler(void) {
    
    // i put this part here because the debugger when read the Dr register it will clear some flags
    // so i need to read the flags first before calling the local handler
    // this step isn't necessary if i am not using debugger
    LocalFlags.FramingErrorFlag  = UART_u8ReadFEFlag(UART_6);
    LocalFlags.NoiseErrorFlag    = UART_u8ReadNoiseFlag(UART_6);
    LocalFlags.OverrunErrorFlag  = UART_u8ReadOREFlag(UART_6);
    LocalFlags.ParityErrorFlag   = UART_u8ReadPEFlag(UART_6);
    LocalFlags.TC_Flag           = UART_u8ReadTCFlag(UART_6);
    LocalFlags.TXE_Flag          = UART_u8ReadTXEFlag(UART_6);
    LocalFlags.RXNE_Flag         = UART_u8ReadRXNEFlag(UART_6);

    USART_LocalHandler(UART_6);
}


