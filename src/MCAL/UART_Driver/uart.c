/******************************************************************************
 * @file    uart.c
 * @author  Eng.Gemy
 * @brief   UART/USART Driver Implementation File (STM32F401 USART1/2/6)
 *          Implements init, sync/async polling vs interrupt, DMA activation,
 *          flag accessors, GPIO AF setup (PA9/10, PA2/3, PC6/7), and ISR
 *          handlers USART1/2/6_IRQHandler that dispatch LocalHandler for
 *          TXE/RXNE/TC/PE/FE/NE/ORE. Baud computed via CalculateBaudRate.
 * @date    2024
 * @version 1.0
 * @note    TXE interrupt kept disabled at init (UART_INTERRUPT_TXE=0) — enabled
 *          on demand inside AsynTransmit to avoid storm. LocalFlags caches
 *          SR before DR read to avoid debugger side-effect clearing.
 ******************************************************************************/

#include "LIB/stdtypes.h"
#include <string.h>
#include "MCAL/GPIO_Driver/gpio_int.h"
#include "MCAL/UART_Driver/uart_priv.h"
#include "MCAL/UART_Driver/uart.h"


/** @brief Forward ISR declarations (weak in startup, implemented here) */
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART6_IRQHandler(void);
/** @brief Baud BRR calculation helper (16× vs 8× oversampling) */
static uint16_t CalculateBaudRate(uint32_t peripheralClock, uint32_t baudRate, UART_OverSampling_t oversampling) ;
/** @brief Init GPIO AF for TX/RX pins per UART instance */
static UART_Status_t Init_UART_Pins(UART_Config_t* config);
/** @brief Common ISR body — dispatches flags to callbacks/buffers */
static void USART_LocalHandler(UART_Number_t uartNumber);

/** @brief Cache of SR flags read at IRQ entry (avoid DR-read clear by debugger) */
static LocalFlags_t LocalFlags = {0};
/** @brief Global init state — sync/async APIs check UART_INIT */
static UART_InitState_t UART_InitState = UART_NOT_INIT;
/** @brief Register base lookup per instance index 0:UART1,1:UART2,2:UART6 */
static UARTRegs_t* UART_Registers[] = {
    UART1_BASE_ADDR,
    UART2_BASE_ADDR,
    UART6_BASE_ADDR
};
/** @brief Per-instance TX busy tracker (READY/BUSY) */
static UARTY_State_t UART_Tx_State[] = {
    UART_READY,
    UART_READY,
    UART_READY
};
/** @brief Per-instance RX busy tracker */
static UARTY_State_t UART_Rx_State[] = {
    UART_READY,
    UART_READY,
    UART_READY
};
/** @brief Per-instance error+TC callbacks (init NULL) */
static UART_Callbacks_t UartCallbacks[3] = {
    { .ParityErrorCallback = NULL, .FramingErrorCallback = NULL, .NoiseErrorCallback = NULL, .OverrunErrorCallback = NULL ,.TC_Callback = NULL},
    { .ParityErrorCallback = NULL, .FramingErrorCallback = NULL, .NoiseErrorCallback = NULL, .OverrunErrorCallback = NULL ,.TC_Callback = NULL},
    { .ParityErrorCallback = NULL, .FramingErrorCallback = NULL, .NoiseErrorCallback = NULL, .OverrunErrorCallback = NULL ,.TC_Callback = NULL}
};
/** @brief Per-instance async TX descriptors */
static UART_AsynBuffer_t TxBuffers[3] = {0};
/** @brief Per-instance async RX descriptors */
static UART_AsynBuffer_t RxBuffers[3] = {0};
/******************************************************************************
 * @brief Initialize UART with given config (see uart.h)
 * @details Validates Number/Enable/Parity/OverSampling/StopBits/WordLength/
 *          Sample/InterruptFlags masks, calls Init_UART_Pins, programs CR1
 *          (OverSampling/Parity/WordLength/Enable + CR1 interrupt bits), CR2
 *          (StopBits), CR3 (Sample + CR3 interrupts), computes BRR, enables UE.
 * @param[in] config Pointer to UART_Config_t
 * @return UART_Status_t UART_OK or error code
 ******************************************************************************/
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


/******************************************************************************
 * @brief Sync blocking transmit buffer — polls TXE/TC per byte
 * @param[in] uartNumber Instance (UART_1/2/6)
 * @param[in] txBuffer Source bytes
 * @param[in] size Number of bytes
 * @return UART_Status_t UART_OK or error (NULL_PTR, WRONG_UART_NUMBER, NOT_INIT)
 ******************************************************************************/
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

/******************************************************************************
 * @brief Async interrupt transmit — non-blocking; TXE ISR drives bytes
 * @details Checks BUSY, stores TxBuffers[uart], sends first byte (poll TXE),
 *          enables TXEIE (UART_INTERRUPT_TXE_LOCAL_ENABLE). ISR LocalHandler
 *          will send rest and invoke callback at size reached, clearing BUSY.
 * @param[in] uartNumber Instance
 * @param[in] txBuffer Descriptor (buffer/size/callback/index)
 * @return UART_Status_t UART_OK, NULL_PTR, TX_BUSY, WRONG_UART_NUMBER, NOT_INIT
 ******************************************************************************/
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
                    TxBuffers[uartNumber].buffer = txBuffer->buffer;
                    TxBuffers[uartNumber].size = txBuffer->size;
                    TxBuffers[uartNumber].callback = txBuffer->callback;
                    TxBuffers[uartNumber].index = 0;

                    // Start transmission by sending the first byte
                    UARTRegs_t* uart = UART_Registers[uartNumber];

                    // Wait until transmit data register is empty
                    // i am sure that it is empty here but just to be safe
                    while (UART_u8ReadTXEFlag(uartNumber) == 0); // TXE flag

                    // Write data to data register
                    uart->DR = TxBuffers[uartNumber].buffer[TxBuffers[uartNumber].index++];

                    // Enable TXE interrupt
                    uart->CR1 |= UART_INTERRUPT_TXE_LOCAL_ENABLE;

                    status = UART_OK;
                }
            }
        }
    }
    return status;
}



/******************************************************************************
 * @brief Sync blocking receive — polls RXNE per byte and reads DR
 * @param[in] uartNumber Instance
 * @param[out] rxBuffer Destination buffer
 * @param[in] size Bytes to read
 * @return UART_Status_t UART_OK or error
 ******************************************************************************/
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


/******************************************************************************
 * @brief Async interrupt receive — enables RXNEIE; ISR fills buffer
 * @param[in] uartNumber Instance
 * @param[in] rxBuffer Descriptor (buffer/size/callback)
 * @return UART_Status_t UART_OK, TX_BUSY if RX busy, etc.
 ******************************************************************************/
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
                    RxBuffers[uartNumber].buffer = rxBuffer->buffer;
                    RxBuffers[uartNumber].size = rxBuffer->size;
                    RxBuffers[uartNumber].callback = rxBuffer->callback;
                    RxBuffers[uartNumber].index = 0;

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

/******************************************************************************
 * @brief Enable DMA for TX/RX (sets CR3.DMAT/DMAR)
 * @param[in] uartNumber Instance
 * @param[in] enableDmaFlag Mask UART_DMA_TRANSMIT_ENABLE/RECEIVE_ENABLE
 * @return UART_Status_t UART_OK or WRONG_DMA_ENABLE / NOT_INIT / WRONG_UART_NUMBER
 ******************************************************************************/
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

/******************************************************************************
 * @brief Enable UART interrupts (ORR into CR1/CR3)
 * @param[in] uartNumber Instance
 * @param[in] interruptFlags OR of UART_INTERRUPT_* (validated against mask)
 * @return UART_Status_t UART_OK or WRONG_INTERRUPT_FLAGS
 ******************************************************************************/
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

/******************************************************************************
 * @brief Disable UART interrupts (AND-NOT)
 * @param[in] uartNumber Instance
 * @param[in] interruptFlags Mask to clear
 * @return UART_Status_t UART_OK or error
 ******************************************************************************/
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


/******************************************************************************
 * @brief Register error and TC callbacks for ISR
 * @param[in] uartNumber Instance
 * @param[in] callbacks Pointer to UART_Callbacks_t bundle (copied)
 * @return UART_Status_t UART_OK, NULL_PTR, WRONG_UART_NUMBER
 ******************************************************************************/
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

/******************************************************************************
 * @brief Clear status flags (SR &= ~mask)
 * @param[in] uartNumber Instance
 * @param[in] interruptFlags Mask of UART_FLAG_* to clear
 * @return UART_Status_t UART_OK
 * @note  Some flags require SR read + DR read sequence per RM; this does direct clear.
 ******************************************************************************/
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

/** @brief Read TXE flag (SR TXE position) @return 0/1 */
uint8_t UART_u8ReadTXEFlag(UART_Number_t uartNumber) {
    UARTRegs_t* uart = UART_Registers[uartNumber];
    return ((uart->SR >> UART_TXE_FLAG_POSITION) & 1); 
}


/** @brief Read TC flag @return 0/1 */
uint8_t UART_u8ReadTCFlag(UART_Number_t uartNumber) {
    UARTRegs_t* uart = UART_Registers[uartNumber];
    return ((uart->SR >> UART_TC_FLAG_POSITION) & 1);
}

/** @brief Read RXNE flag @return 0/1 */
uint8_t UART_u8ReadRXNEFlag(UART_Number_t uartNumber) {
    UARTRegs_t* uart = UART_Registers[uartNumber];
    return ((uart->SR >> UART_RXNE_FLAG_POSITION) & 1);
}

/** @brief Read ORE flag @return 0/1 */
uint8_t UART_u8ReadOREFlag(UART_Number_t uartNumber) {
    UARTRegs_t* uart = UART_Registers[uartNumber];
    return ((uart->SR >> UART_ORE_FLAG_POSITION) & 1);
}

/** @brief Read NE (noise) flag @return 0/1 */
uint8_t UART_u8ReadNoiseFlag(UART_Number_t uartNumber) {
    UARTRegs_t* uart = UART_Registers[uartNumber];
    return ((uart->SR >> UART_NOISE_FLAG_POSITION) & 1);
}

/** @brief Read FE flag @return 0/1 */
uint8_t UART_u8ReadFEFlag(UART_Number_t uartNumber) {
    UARTRegs_t* uart = UART_Registers[uartNumber];
    return ((uart->SR >> UART_FE_FLAG_POSITION) & 1);
}

/** @brief Read PE flag @return 0/1 */
uint8_t UART_u8ReadPEFlag(UART_Number_t uartNumber) {
    UARTRegs_t* uart = UART_Registers[uartNumber];
    return ((uart->SR >> UART_PE_FLAG_POSITION) & 1);
}

/******************************************************************************
 * @brief Compute BRR for given peripheral clock and baud
 * @details For OVER16: USARTDIV = fCK/(16*baud) → mantissa (bits12:4) + fraction (3:0, 0..15).
 *          For OVER8: x8 variant (fraction 0..7, mantissa shift 4). Uses integer math with rounding.
 * @param[in] peripheralClock APB clock Hz
 * @param[in] baudRate Desired baud bps
 * @param[in] oversampling 16× (0) or 8× (OVER8=1)
 * @return uint16_t BRR value to write to BRR register
 ******************************************************************************/
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

/******************************************************************************
 * @brief Init GPIO AF pins for UART TX/RX
 * @details Selects AF7 (UART1/2) or AF8 (UART6) on PA9/10, PA2/3, PC6/7 per
 *          UartEnabled bits (TX/RX). Uses GPIO_MODE_AF, PUSH_PULL, NO_PULL.
 * @param[in] config UART config containing UART_Number and UartEnabled
 * @return UART_Status_t UART_OK or UART_GPIO_ERROR / WRONG_UART_NUMBER
 ******************************************************************************/
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



/******************************************************************************
 * @brief Local ISR dispatcher — handles one UART instance
 * @details Checks cached LocalFlags (RXNE, TXE, PE, FE, NE, ORE, TC) and
 *          invokes per-flag callbacks / advances TxBuffers/RxBuffers indices.
 *          Disables TXEIE/RXNEIE when buffer complete and clears BUSY.
 * @param[in] uartNumber Instance to dispatch
 * @return None
 ******************************************************************************/
static void USART_LocalHandler(UART_Number_t uartNumber) {
    UARTRegs_t* uart = UART_Registers[uartNumber];

        if(LocalFlags.RXNE_Flag== 1) {
        if(RxBuffers[uartNumber].buffer != NULL) {
            if(RxBuffers[uartNumber].index < RxBuffers[uartNumber].size) {
                // Read received byte
                RxBuffers[uartNumber].buffer[RxBuffers[uartNumber].index++] = (uint8_t)(uart->DR & 0xFF);
            } 
            if(RxBuffers[uartNumber].index >= RxBuffers[uartNumber].size) {
                // Reception complete
                // disable RXNE interrupt
                uart->CR1 &= UART_INTERRUPT_RXNE_LOCAL_DISABLE;

                UART_Rx_State[uartNumber] = UART_READY;

                // Call the callback function if set
                if(RxBuffers[uartNumber].callback != NULL) {
                    RxBuffers[uartNumber].callback(); 
                }
            }
        }
    }

    // Check if TXE flag is set
    if(LocalFlags.TXE_Flag == 1) {
        if(TxBuffers[uartNumber].buffer != NULL) {
            if(TxBuffers[uartNumber].index < TxBuffers[uartNumber].size) {
                // Send next byte
                uart->DR = TxBuffers[uartNumber].buffer[TxBuffers[uartNumber].index++];
            } else {
                // disable TXE interrupt
                uart->CR1 &= UART_INTERRUPT_TXE_LOCAL_DISABLE;

                // Transmission complete
                UART_Tx_State[uartNumber] = UART_READY;

                // Call the callback function if set
                if(TxBuffers[uartNumber].callback != NULL) {
                    TxBuffers[uartNumber].callback(); 
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
            UART_enuClearFlags(uartNumber, UART_INTERRUPT_TC_LOCAL_ENABLE);
            UartCallbacks[uartNumber].TC_Callback();
        }
    }
}


/******************************************************************************
 * @brief USART1 IRQ handler — caches flags then dispatches LocalHandler
 * @details Reads SR flags into LocalFlags before DR read (debugger DR read
 *          would otherwise clear flags). Required because debugger view of DR
 *          clears ORE etc.
 * @param None
 * @return None
 ******************************************************************************/
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

/** @brief USART2 IRQ handler — see USART1 handler */
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

/** @brief USART6 IRQ handler — see USART1 handler */
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

