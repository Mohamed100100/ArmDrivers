#ifndef HSERIAL_H
#define HSERIAL_H

#include "LIB/stdtypes.h"
#include "HAL/HSERIAL_Driver/hserial_cfg.h"

typedef void (*HSERIAL_Callback_t)(void);


typedef enum {
    HSERIAL_NOT_OK,
    HSERIAL_OK,
    HSERIAL_ERROR_INIT_UART,
    HSERIAL_FAILED_TRANSMIT,
    HSERIAL_WRONG_CHANNEL,
    HSERIAL_NULL_POINTER,
    HSERIAL_INVALID_SIZE,
    HSERIAL_ERROR_NVIC,
    HSERIAL_ERROR_INIT_DMA,
    HSERIAL_ERROR_INIT_SPI,
} HSERIAL_Status_t;

typedef enum {
    HSERIAL_MODE_UART_SYNC = 0,
    HSERIAL_MODE_UART_ASYNC,
    HSERIAL_MODE_UART_DMA,
    HSERIAL_MODE_SPI_SYNC,
    HSERIAL_MODE_SPI_ASYNC,
    HSERIAL_MODE_SPI_DMA
} HSERIAL_Mode_t;


typedef enum {
    HSERIAL_UART_1 = 0,
    HSERIAL_UART_2,
    HSERIAL_UART_6
}HSERIAL_Uart_Number_t;


typedef enum {
    HSERIAL_UART_PARITY_NONE = 0b0000000000000000000000,
    HSERIAL_UART_PARITY_EVEN = 0b0000000000010000000000,
    HSERIAL_UART_PARITY_ODD  = 0b0000000000011000000000
}HSERIAL_Uart_Parity_t;

typedef enum {
    HSERIAL_UART_OVERSAMPLING_16 = 0b0000000000000000000000,
    HSERIAL_UART_OVERSAMPLING_8  = 0b0000001000000000000000
}HSERIAL_Uart_OverSampling_t; 

typedef enum {
    HSERIAL_UART_STOPBITS_1   = 0b0000000000000000000000,
    HSERIAL_UART_STOPBITS_0_5 = 0b0000000001000000000000,
    HSERIAL_UART_STOPBITS_2   = 0b0000000010000000000000,
    HSERIAL_UART_STOPBITS_1_5 = 0b0000000011000000000000 
}HSERIAL_Uart_StopBit_t;

typedef enum {
    HSERIAL_UART_WORDLENGTH_8B =  0b0000000000000000000000,
    HSERIAL_UART_WORDLENGTH_9B =  0b0000000001000000000000
}HSERIAL_Uart_WordLength_t;

typedef enum {
    HSERIAL_UART_THREE_SAMPLE = 0b0000000000000000000000,
    HSERIAL_UART_ONE_SAMPLE   = 0b0000000000100000000000
}HSERIAL_Uart_Sample_t;


typedef enum {
    HSERIAL_DISABLE_UART          = 0b0000000000000000000000UL,
    HSERIAL_ENABLE_UART_TRANSMITE = 0b0000000000000000001000UL,
    HSERIAL_ENABLE_UART_RECEIVE   = 0b0000000000000000000100UL,
    HSERIAL_ENABLE_UART_BOTH      = 0b0000000000000000001100UL
}HSERIAL_Uart_Enable_t; 

typedef enum {
    HSERIAL_PRIORITY_0  = 0x00,  /**< Priority 0 (Highest) - 0000 0000 */
    HSERIAL_PRIORITY_1  = 0x10,  /**< Priority 1 - 0001 0000 */
    HSERIAL_PRIORITY_2  = 0x20,  /**< Priority 2 - 0010 0000 */
    HSERIAL_PRIORITY_3  = 0x30,  /**< Priority 3 - 0011 0000 */
    HSERIAL_PRIORITY_4  = 0x40,  /**< Priority 4 - 0100 0000 */
    HSERIAL_PRIORITY_5  = 0x50,  /**< Priority 5 - 0101 0000 */
    HSERIAL_PRIORITY_6  = 0x60,  /**< Priority 6 - 0110 0000 */
    HSERIAL_PRIORITY_7  = 0x70,  /**< Priority 7 - 0111 0000 */
    HSERIAL_PRIORITY_8  = 0x80,  /**< Priority 8 - 1000 0000 */
    HSERIAL_PRIORITY_9  = 0x90,  /**< Priority 9 - 1001 0000 */
    HSERIAL_PRIORITY_10 = 0xA0,  /**< Priority 10 - 1010 0000 */
    HSERIAL_PRIORITY_11 = 0xB0,  /**< Priority 11 - 1011 0000 */
    HSERIAL_PRIORITY_12 = 0xC0,  /**< Priority 12 - 1100 0000 */
    HSERIAL_PRIORITY_13 = 0xD0,  /**< Priority 13 - 1101 0000 */
    HSERIAL_PRIORITY_14 = 0xE0,  /**< Priority 14 - 1110 0000 */
    HSERIAL_PRIORITY_15 = 0xF0   /**< Priority 15 (Lowest) - 1111 0000 (lowest) */
}HSERIAL_Priority_t;


typedef enum {
    HSERIAL_SPI1 = 0,
    HSERIAL_SPI2,
    HSERIAL_SPI3,
    HSERIAL_SPI4,
}HSERIAL_Spi_Number_t;

typedef enum {
    HSERIAL_COMMINICATION_SPI_FULL_DUPLEX                = 0b00000000000000000000000000000000,
    HSERIAL_COMMINICATION_SPI_HALF_DUPLEX_2LINES_TX_ONLY = 0b00000000000000000000000000000000,
    HSERIAL_COMMINICATION_SPI_HALF_DUPLEX_2LINES_RX_ONLY = 0b00000000000000000000010000000000,
    HSERIAL_COMMINICATION_SPI_HALF_DUPLEX_1LINE_RX_ONLY  = 0b00000000000000001000000000000000,
    HSERIAL_COMMINICATION_SPI_HALF_DUPLEX_1LINE_TX_ONLY  = 0b00000000000000001100000000000000,
}HSERIAL_Spi_Communication_Mode_t;

typedef enum {
    HSERIAL_SPI_MASTER        = 0b00000000000000000000000000000000,
    HSERIAL_SPI_SLAVE         = 0b00000000000000000000000000000100
}HSERIAL_Spi_Mode_t;

typedef enum {
    HSERIAL_SPI_CRC_DISABLED = 0b00000000000000000000000000000000,
    HSERIAL_SPI_CRC_ENABLED  = 0b00000000000000000010000000000000
}HSERIAL_Spi_Crc_t;

typedef enum {
    HSERIAL_SPI_8_BIT_DATA   = 0b00000000000000000000000000000000,
    HSERIAL_SPI_16_BIT_DATA  = 0b00000000000000000000100000000000
}HSERIAL_Spi_DataLength_t;

typedef enum {
    HSERIAL_SPI_MSB_FIRST    = 0b00000000000000000000000000000000,
    HSERIAL_SPI_LSB_FIRST    = 0b00000000000000000000000010000000
}HSERIAL_Spi_DataOrder_t;

typedef enum {
    HSERIAL_SPI_BAUDERATE_DIV2     = 0b00000000000000000000000000000000,
    HSERIAL_SPI_BAUDERATE_DIV4     = 0b00000000000000000000000000001000,
    HSERIAL_SPI_BAUDERATE_DIV8     = 0b00000000000000000000000000010000,
    HSERIAL_SPI_BAUDERATE_DIV16    = 0b00000000000000000000000000011000,
    HSERIAL_SPI_BAUDERATE_DIV32    = 0b00000000000000000000000000100000,
    HSERIAL_SPI_BAUDERATE_DIV64    = 0b00000000000000000000000000101000,
    HSERIAL_SPI_BAUDERATE_DIV128   = 0b00000000000000000000000000110000,
    HSERIAL_SPI_BAUDERATE_DIV256   = 0b00000000000000000000000000111000
}HSERIAL_Spi_BaudRate_t;


typedef enum {
    HSERIAL_SPI_MOTOROLA = 0b00000000000000000000000000000000,
    HSERIAL_SPI_TI       = 0b00000000000000000000000100000000
}HSERIAL_Spi_FrameFormat_t;

// typedef enum {
//     HSERIAL_SPI_DISABLE_DMA         = 0b00000000000000000000000000000000,
//     HSERIAL_SPI_DMA_TX_ENABLE       = 0b00000000000000000000000000000010,
//     HSERIAL_SPI_DMA_RX_ENABLE       = 0b00000000000000000000000000000001,
//     HSERIAL_SPI_DMA_TX_RX_ENABLE    = 0b00000000000000000000000000000011
// }HSERIAL_Spi_Dma_t;

typedef enum {
    HSERIAL_SPI_ZERO_IDLE_FIRST_EDGE  = 0b00000000000000000000000000000000,
    HSERIAL_SPI_ZERO_IDLE_SECOND_EDGE = 0b00000000000000000000000000000001,
    HSERIAL_SPI_ONE_IDLE_FIRST_EDGE   = 0b00000000000000000000000000000010,
    HSERIAL_SPI_ONE_IDLE_SECOND_EDGE  = 0b00000000000000000000000000000011
}HSERIAL_Spi_PolarityPhase_t;

typedef enum {
    HSERIAL_SPI_NSS_MASTER_HW_OUTPUT       = 0b00000000000000000000000000000100,
    HSERIAL_SPI_NSS_MASTER_HW_INPUT        = 0b00000000000000000000000000000000,
    HSERIAL_SPI_NSS_MASTER_SW              = 0b00000000000000000000001100000000,
    HSERIAL_SPI_NSS_SLAVE_HW               = 0b00000000000000000000000000000000,
    HSERIAL_SPI_NSS_SLAVE_SW               = 0b00000000000000000000001000000000     
}HSERIAL_Spi_Nss_t;

typedef struct {
    uint32_t                     HSERIAL_UartPeripheralClock;
    HSERIAL_Uart_Number_t        HSERIAL_UartChannel;
    uint32_t                     HSERIAL_UartBaudRate;
    HSERIAL_Uart_Parity_t        HSERIAL_UartParity;
    HSERIAL_Uart_OverSampling_t  HSERIAL_UartOverSampling;
    HSERIAL_Uart_StopBit_t       HSERIAL_UartStopBits;
    HSERIAL_Uart_WordLength_t    HSERIAL_UartWordLength;
    HSERIAL_Uart_Sample_t        HSERIAL_UartSample;
    HSERIAL_Uart_Enable_t        HSERIAL_UartEnable;
} H_UART_Sync_Config_t;

typedef struct {
    uint32_t                     HSERIAL_UartPeripheralClock;
    HSERIAL_Uart_Number_t        HSERIAL_UartChannel;
    uint32_t                     HSERIAL_UartBaudRate;
    HSERIAL_Uart_Parity_t        HSERIAL_UartParity;
    HSERIAL_Uart_OverSampling_t  HSERIAL_UartOverSampling;
    HSERIAL_Uart_StopBit_t       HSERIAL_UartStopBits;
    HSERIAL_Uart_WordLength_t    HSERIAL_UartWordLength;
    HSERIAL_Uart_Sample_t        HSERIAL_UartSample;
    HSERIAL_Uart_Enable_t        HSERIAL_UartEnable;
    HSERIAL_Callback_t           HSERIAL_UartTxCompleteCallback;
    HSERIAL_Callback_t           HSERIAL_UartRxCompleteCallback;
    HSERIAL_Priority_t           HSERIAL_UartInterruptPriority;
} H_UART_Async_Config_t;
    

typedef struct {
    uint32_t                     HSERIAL_UartPeripheralClock;
    HSERIAL_Uart_Number_t        HSERIAL_UartChannel;
    uint32_t                     HSERIAL_UartBaudRate;
    HSERIAL_Uart_Parity_t        HSERIAL_UartParity;
    HSERIAL_Uart_OverSampling_t  HSERIAL_UartOverSampling;
    HSERIAL_Uart_StopBit_t       HSERIAL_UartStopBits;
    HSERIAL_Uart_WordLength_t    HSERIAL_UartWordLength;
    HSERIAL_Uart_Sample_t        HSERIAL_UartSample;
    HSERIAL_Uart_Enable_t        HSERIAL_UartEnable;
    HSERIAL_Callback_t           HSERIAL_UartTxCompleteCallback;
    HSERIAL_Callback_t           HSERIAL_UartRxCompleteCallback;
    HSERIAL_Priority_t           HSERIAL_UartInterruptPriority;

}H_UART_Dma_Config_t;

 typedef struct {
    HSERIAL_Spi_Number_t              HSERIAL_SpiChannel;
    HSERIAL_Spi_Communication_Mode_t  HSERIAL_SpiCommunicationMode;
    HSERIAL_Spi_Mode_t                HSERIAL_SpiMode;
    HSERIAL_Spi_Crc_t                 HSERIAL_SpiCrc;
    HSERIAL_Spi_DataOrder_t           HSERIAL_SpiDataOrder;
    HSERIAL_Spi_BaudRate_t            HSERIAL_SpiBaudRate;
    HSERIAL_Spi_FrameFormat_t         HSERIAL_SpiFrameFormat;
    HSERIAL_Spi_PolarityPhase_t       HSERIAL_SpiPolarityPhase;
    HSERIAL_Spi_Nss_t                 HSERIAL_SpiNss;
    HSERIAL_Spi_DataLength_t          HSERIAL_SpiDataLength;
}H_SPI_Sync_Config_t;

 typedef struct {
    HSERIAL_Spi_Number_t              HSERIAL_SpiChannel;
    HSERIAL_Spi_Communication_Mode_t  HSERIAL_SpiCommunicationMode;
    HSERIAL_Spi_Mode_t                HSERIAL_SpiMode;
    HSERIAL_Spi_Crc_t                 HSERIAL_SpiCrc;
    HSERIAL_Spi_DataOrder_t           HSERIAL_SpiDataOrder;
    HSERIAL_Spi_BaudRate_t            HSERIAL_SpiBaudRate;
    HSERIAL_Spi_FrameFormat_t         HSERIAL_SpiFrameFormat;
    HSERIAL_Spi_PolarityPhase_t       HSERIAL_SpiPolarityPhase;
    HSERIAL_Spi_Nss_t                 HSERIAL_SpiNss;
    HSERIAL_Spi_DataLength_t          HSERIAL_SpiDataLength;
    HSERIAL_Callback_t                HSERIAL_SpiTxCompleteCallback;
    HSERIAL_Callback_t                HSERIAL_SpiRxCompleteCallback;
    HSERIAL_Priority_t                HSERIAL_SpiInterruptPriority;
}H_SPI_Async_Config_t;



typedef struct {
    HSERIAL_Mode_t     HSERIAL_Mode;
    union {
        H_UART_Sync_Config_t     UART_Sync_Config;
        H_UART_Async_Config_t    UART_Async_Config;
        H_UART_Dma_Config_t      UART_Dma_Config;
        H_SPI_Sync_Config_t      SPI_Sync_Config;
        H_SPI_Async_Config_t     SPI_Async_Config;
    }; 
}HSERIAL_Config_t;


HSERIAL_Status_t HSERIAL_enuInit(void);
HSERIAL_Status_t HSERIAL_enuTransmitBuffer(HSERIAL_Channel_t channel, const uint8_t* dataBuffer, uint16_t size);
HSERIAL_Status_t HSERIAL_enuReceiveBuffer(HSERIAL_Channel_t channel,uint8_t* dataBuffer, uint16_t size);


#endif // HSERIAL_H