#ifndef UART_PRIV_H
#define UART_PRIV_H

#include <LIB/stdtypes.h>



#define UART1_BASE_ADDR     (( UARTRegs_t*)0x40011000UL)
#define UART2_BASE_ADDR     (( UARTRegs_t*)0x40004400UL)
#define UART6_BASE_ADDR     (( UARTRegs_t*)0x40011400UL)

#define UART_ENABLE_MASK       (0b1111111111111111110011)
#define UART_INTERRUPT_MASK    (0b1111111111111000011110)
#define UART_CR1_FLAGS_MASK    (0b0000000000000111100000)
#define UART_CR3_FLAGS_MASK    (0b0000000000000000000001)
#define UART_PARITY_MASK       (0b1111111111100111111111)
#define UART_OVERSAMPLING_MASK (0b1111110111111111111111)
#define UART_WORDLENGTH_MASK   (0b1111111110111111111111)
#define UART_SAMPLE_MASK       (0b1111111111011111111111)
#define UART_DMA_MASK          (0b1111111111111100111111)  // DMA bits mask

//                              0b1098765432109876543210
#define UART_ENABLE            (0b0000000010000000000000)  // UART Enable

#define UART_INTERRUPT_TXE_LOCAL_ENABLE   (0b0000000000000010000000)  // Transmit data register empty interrupt enable
#define UART_INTERRUPT_TXE_LOCAL_DISABLE  (0b1111111111111101111111)  // Transmit data register empty interrupt disable
#define UART_INTERRUPT_RXNE_LOCAL_ENABLE  (0b0000000000000000100000)  // Receive data register not empty interrupt enable
#define UART_INTERRUPT_RXNE_LOCAL_DISABLE (0b1111111111111111011111)
#define UART_INTERRUPT_TC_LOCAL_ENABLE    (0b0000000000000001000000)  // Transmission complete interrupt enable
#define UART_INTERRUPT_TC_LOCAL_DISABLE   (0b1111111111111110111111)  // Transmission complete interrupt disable




#define UART_TXE_FLAG_POSITION      (7UL)
#define UART_TC_FLAG_POSITION       (6UL)
#define UART_RXNE_FLAG_POSITION     (5UL)
#define UART_ORE_FLAG_POSITION      (3UL)
#define UART_NOISE_FLAG_POSITION    (2UL)
#define UART_FE_FLAG_POSITION       (1UL)
#define UART_PE_FLAG_POSITION       (0UL)

typedef enum {
    UART_BUSY,
    UART_READY
}UARTY_State_t;

typedef struct {
    uint8_t ParityErrorFlag    : 1;
    uint8_t FramingErrorFlag   : 1;
    uint8_t NoiseErrorFlag     : 1;
    uint8_t OverrunErrorFlag   : 1;
    uint8_t TC_Flag            : 1;
    uint8_t TXE_Flag           : 1;
    uint8_t RXNE_Flag          : 1;
}LocalFlags_t ;


typedef struct {
    volatile uint32_t SR;   // Status register
    volatile uint32_t DR;   // Data register
    volatile uint32_t BRR;  // Baud rate register
    volatile uint32_t CR1;  // Control register 1
    volatile uint32_t CR2;  // Control register 2
    volatile uint32_t CR3;  // Control register 3
    volatile uint32_t GTPR; // Guard time and prescaler register
} UARTRegs_t;


#endif // UART_PRIV_H