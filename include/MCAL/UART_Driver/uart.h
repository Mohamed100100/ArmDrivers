#ifndef UART_H
#define UART_H

#include <LIB/stdtypes.h>


//                              0b1098765432109876543210
#define UART_ENABLE_TRANSMITE       (0b0000000000000000001000)  // Transmit data enable 
#define UART_ENABLE_RECEIVE         (0b0000000000000000000100)  // Receive data enable
#define UART_DISABLE                (0b0000000000000000000000)  // UART Disable

// i found that enable txe interrupt in init will cause problem in async transmit
// so i will put it 0 and inside the async transmit function i will enable it using local define
//                                   0b1098765432109876543210
#define UART_INTERRUPT_TXE          (0b0000000000000000000000)  // Transmit data register empty interrupt enable
#define UART_INTERRUPT_ERROR        (0b0000000000000000000001)  // Error interrupt enable
#define UART_INTERRUPT_RXNE         (0b0000000000000000100000)  // Receive data register not empty interrupt enable
#define UART_INTERRUPT_TC           (0b0000000000000001000000)  // Transmission complete interrupt enable
#define UART_INTERRUPT_PE           (0b0000000000000100000000)  // Parity error interrupt enable

//                                   0b1098765432109876543210
#define UART_FLAG_TXE               (0b0000000000000010000000)  // Transmit data register empty flag
#define UART_FLAG_TC                (0b0000000000000001000000)  // Transmission complete flag
#define UART_FLAG_RXNE              (0b0000000000000000100000)  // Receive data register not empty flag
#define UART_FLAG_ORE               (0b0000000000000000001000)  // Overrun error flag
#define UART_FLAG_NOISE             (0b0000000000000000000100)  // Noise error flag
#define UART_FLAG_FE                (0b0000000000000000000010)  // Framing error flag
#define UART_FLAG_PE                (0b0000000000000000000001)  // Parity error flag

//                                   0b1098765432109876543210
#define UART_DMA_TRANSMIT_ENABLE    (0b0000000000000010000000)  // DMA transmit enable
#define UART_DMA_RECEIVE_ENABLE     (0b0000000000000001000000)  // DMA receive enable

typedef void (*UART_Callback_t)(void);

typedef struct {
    UART_Callback_t callback;
    uint16_t size;
    uint16_t index;
    uint8_t *buffer;
}UART_AsynBuffer_t;

typedef struct {
    UART_Callback_t ParityErrorCallback;
    UART_Callback_t FramingErrorCallback;
    UART_Callback_t NoiseErrorCallback;
    UART_Callback_t OverrunErrorCallback;
    UART_Callback_t TC_Callback;
} UART_Callbacks_t;

typedef enum {
    UART_NOT_OK,
    UART_OK,
    UART_NULL_PTR,
    UART_WRONG_UART_NUMBER,
    UART_WRONG_UART_ENABLE,
    UART_WRONG_PARITY,
    UART_WRONG_OVERSAMPLING,
    UART_WRONG_STOPBITS,
    UART_WRONG_WORDLENGTH,
    UART_WRONG_SAMPLE,
    UART_WRONG_INTERRUPT_FLAGS,
    UART_NOT_INIT_SUCCESSFULLY,
    UART_GPIO_ERROR,
    UART_TX_BUSY,
    UART_WRONG_DMA_ENABLE,
} UART_Status_t;

typedef enum {
    UART_1,
    UART_2,
    UART_6
}UART_Number_t;

typedef enum {        //0b1098765432109876543210
    UART_PARITY_NONE  = 0b0000000000000000000000,
    UART_PARITY_EVEN  = 0b0000000000010000000000,
    UART_PARITY_ODD   = 0b0000000000011000000000
} UART_Parity_t;       

                            //        * 
typedef enum {              //0b1098765432109876543210
    UART_OVERSAMPLING_16    = 0b0000000000000000000000,
    UART_OVERSAMPLING_8     = 0b0000001000000000000000
} UART_OverSampling_t;

                      //          **
typedef enum {        //0b1098765432109876543210
    UART_STOPBITS_1   = 0b0000000000000000000000,
    UART_STOPBITS_0_5 = 0b0000000001000000000000,
    UART_STOPBITS_2   = 0b0000000010000000000000,
    UART_STOPBITS_1_5 = 0b0000000011000000000000 
}UART_StopBit_t;
#define UART_STOPBITS_MASK   (0b1111111100111111111111)

                        //           *
typedef enum {          //0b1098765432109876543210
    UART_WORDLENGTH_8B =  0b0000000000000000000000,
    UART_WORDLENGTH_9B =  0b0000000001000000000000
}UART_WordLength_t;
                      //            *
typedef enum {        //0b1098765432109876543210
    UART_THREE_SAMPLE = 0b0000000000000000000000,
    UART_ONE_SAMPLE   = 0b0000000000100000000000
}UART_Sample_t;

typedef enum {
    UART_INIT,
    UART_NOT_INIT
}UART_InitState_t;

typedef struct {
    uint32_t PeripheralClock;          // Peripheral clock frequency in Hz
    UART_Number_t UART_Number;            // Select UART peripheral
    // baude rate in bps
    uint32_t BaudRate;                  // Set baud rate
    // you have to choose one of the following parity configurations
    // NONE, EVEN, ODD
    UART_Parity_t Parity;               // Set parity configuration
    // you have to choose one of the following oversampling modes
    // OVERSAMPLING_16, OVERSAMPLING_8
    UART_OverSampling_t OverSampling;   // Set oversampling mode
    // you have to choose one of the following stop bits configurations
    // STOPBITS_1, STOPBITS_0_5, STOPBITS_2,
    UART_StopBit_t StopBits;            // Set number of stop bits
    // you have to choose one of the following word length configurations
    // WORDLENGTH_8B, WORDLENGTH_9B
    UART_WordLength_t WordLength;       // Set word length
    UART_Sample_t Sample;               // Set sampling method
    // you have masks to enable or disable the UART peripheral
    uint32_t UartEnabled;                // Enable or disable UART (transmitter and receiver)
    uint32_t InterruptFlags;             // Enable or disable interrupt flags
} UART_Config_t;

UART_Status_t UART_enuInit(UART_Config_t* config);

UART_Status_t UART_enuSynTransmitBuffer(UART_Number_t uartNumber, const uint8_t* txBuffer, uint16_t size);
UART_Status_t UART_enuSynReceiveBuffer(UART_Number_t uartNumber, uint8_t* rxBuffer, uint16_t size);
UART_Status_t UART_enuAsynTransmitBuffer(UART_Number_t uartNumber, UART_AsynBuffer_t* txBuffer);
UART_Status_t UART_enuAsynReceiveBuffer(UART_Number_t uartNumber, UART_AsynBuffer_t* rxBuffer);

UART_Status_t UART_enuActivateDMA(UART_Number_t uartNumber, uint32_t enableDmaFlag);

uint8_t UART_u8ReadTXEFlag(UART_Number_t uartNumber);
uint8_t UART_u8ReadTCFlag(UART_Number_t uartNumber);
uint8_t UART_u8ReadRXNEFlag(UART_Number_t uartNumber);
uint8_t UART_u8ReadOREFlag(UART_Number_t uartNumber);
uint8_t UART_u8ReadNoiseFlag(UART_Number_t uartNumber);
uint8_t UART_u8ReadFEFlag(UART_Number_t uartNumber);
uint8_t UART_u8ReadPEFlag(UART_Number_t uartNumber);

UART_Status_t UART_enuEnableInterrupts(UART_Number_t uartNumber, uint32_t interruptFlags);
UART_Status_t UART_enuDisableInterrupts(UART_Number_t uartNumber, uint32_t interruptFlags);
UART_Status_t UART_enuClearFlags(UART_Number_t uartNumber,uint32_t interruptFlags);

UART_Status_t UART_enuRegisterCallbacks(UART_Number_t uartNumber, UART_Callbacks_t* callbacks);


#endif // UART_H