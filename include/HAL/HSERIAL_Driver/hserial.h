/******************************************************************************
 * @file    hserial.h
 * @author  Eng.Gemy
 * @brief   HSERIAL High-Level Serial Driver Interface (Unified UART/SPI)
 *          Multiplexes UART (1/2/6) and SPI (1-4) sync/async/DMA modes under
 *          one channel abstraction. Configurations live in hserial_cfg.h/.c.
 *          Provides single Init + Transmit/Receive APIs that dispatch by mode.
 * @date    2024
 * @version 1.0
 * @note    For async/DMA modes provide Tx/Rx callbacks and NVIC priorities.
 *          SPI async uses circular buffer with LocalTx/RxCallback chaining.
 ******************************************************************************/
#ifndef HSERIAL_H
#define HSERIAL_H

#include "LIB/stdtypes.h"
#include "HAL/HSERIAL_Driver/hserial_cfg.h"

/** @brief HSERIAL callback type (void(void)) for Tx/Rx completion */
typedef void (*HSERIAL_Callback_t)(void);


/******************************************************************************
 * @brief HSERIAL Status Enumeration
 * @details Unified errors for init, channel validation, and transfer.
 ******************************************************************************/
typedef enum {
    HSERIAL_NOT_OK,         /**< General failure */
    HSERIAL_OK,             /**< Success */
    HSERIAL_ERROR_INIT_UART,/**< UART_enuInit failed for channel */
    HSERIAL_FAILED_TRANSMIT,/**< Buffer transmit/receive failed (UART/SPI/DMA error) */
    HSERIAL_WRONG_CHANNEL,  /**< Channel >= HSERIAL_CHANNEL_LENGTH */
    HSERIAL_NULL_POINTER,   /**< dataBuffer == NULL */
    HSERIAL_INVALID_SIZE,   /**< size == 0 */
    HSERIAL_ERROR_NVIC,     /**< NVIC Enable/SetPriority failed for IRQ */
    HSERIAL_ERROR_INIT_DMA, /**< DMA_enuInit or Activate failed */
    HSERIAL_ERROR_INIT_SPI, /**< SPI_enuInit failed */
} HSERIAL_Status_t;

/******************************************************************************
 * @brief HSERIAL Mode Enumeration — selects transport + transfer style per channel
 ******************************************************************************/
typedef enum {
    HSERIAL_MODE_UART_SYNC = 0, /**< UART polling (UART_enuSyn*) */
    HSERIAL_MODE_UART_ASYNC,    /**< UART interrupt (UART_enuAsyn* + NVIC) */
    HSERIAL_MODE_UART_DMA,      /**< UART DMA (TX: UART_DMA + DMA stream, RX: DMA) */
    HSERIAL_MODE_SPI_SYNC,      /**< SPI polling (Master/Slave sync) */
    HSERIAL_MODE_SPI_ASYNC,     /**< SPI interrupt (+ NVIC) */
    HSERIAL_MODE_SPI_DMA        /**< SPI DMA (reserved, not yet impl.) */
} HSERIAL_Mode_t;


/** @brief HSERIAL UART instance alias (maps to UART_1/2/6) */
typedef enum {
    HSERIAL_UART_1 = 0, /**< USART1 */
    HSERIAL_UART_2,     /**< USART2 */
    HSERIAL_UART_6      /**< USART6 */
}HSERIAL_Uart_Number_t;


/******************************************************************************
 * @brief HSERIAL UART parity — mirrors UART_Parity_t bits (PCE/PS)
 ******************************************************************************/
typedef enum {
    HSERIAL_UART_PARITY_NONE = 0b0000000000000000000000, /**< No parity */
    HSERIAL_UART_PARITY_EVEN = 0b0000000000010000000000, /**< Even */
    HSERIAL_UART_PARITY_ODD  = 0b0000000000011000000000  /**< Odd */
}HSERIAL_Uart_Parity_t;

/** @brief HSERIAL UART oversampling (OVER8 bit) */
typedef enum {
    HSERIAL_UART_OVERSAMPLING_16 = 0b0000000000000000000000, /**< 16× */
    HSERIAL_UART_OVERSAMPLING_8  = 0b0000001000000000000000  /**< 8× */
}HSERIAL_Uart_OverSampling_t; 

/** @brief HSERIAL UART stop bits (CR2 STOP) */
typedef enum {
    HSERIAL_UART_STOPBITS_1   = 0b0000000000000000000000, /**< 1 */
    HSERIAL_UART_STOPBITS_0_5 = 0b0000000001000000000000, /**< 0.5 */
    HSERIAL_UART_STOPBITS_2   = 0b0000000010000000000000, /**< 2 */
    HSERIAL_UART_STOPBITS_1_5 = 0b0000000011000000000000  /**< 1.5 */
}HSERIAL_Uart_StopBit_t;

/** @brief HSERIAL UART word length (CR1_M) */
typedef enum {
    HSERIAL_UART_WORDLENGTH_8B =  0b0000000000000000000000, /**< 8-bit */
    HSERIAL_UART_WORDLENGTH_9B =  0b0000000001000000000000  /**< 9-bit */
}HSERIAL_Uart_WordLength_t;

/** @brief HSERIAL UART sample mode (CR3 ONE_BIT) */
typedef enum {
    HSERIAL_UART_THREE_SAMPLE = 0b0000000000000000000000, /**< 3-sample majority */
    HSERIAL_UART_ONE_SAMPLE   = 0b0000000000100000000000  /**< One sample */
}HSERIAL_Uart_Sample_t;


/******************************************************************************
 * @brief HSERIAL UART enable mask (CR1 TE/RE bits 3/2)
 ******************************************************************************/
typedef enum {
    HSERIAL_DISABLE_UART          = 0b0000000000000000000000UL, /**< Disable */
    HSERIAL_ENABLE_UART_TRANSMITE = 0b0000000000000000001000UL, /**< TE */
    HSERIAL_ENABLE_UART_RECEIVE   = 0b0000000000000000000100UL, /**< RE */
    HSERIAL_ENABLE_UART_BOTH      = 0b0000000000000000001100UL  /**< TE+RE */
}HSERIAL_Uart_Enable_t; 

/******************************************************************************
 * @brief HSERIAL NVIC priority (maps to IPR top nibble 0x00..0xF0, 0 highest)
 ******************************************************************************/
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
    HSERIAL_PRIORITY_15 = 0xF0   /**< Priority 15 (Lowest) - 1111 0000 */
}HSERIAL_Priority_t;


/** @brief HSERIAL SPI instance alias (1..4) */
typedef enum {
    HSERIAL_SPI1 = 0, /**< SPI1 */
    HSERIAL_SPI2,     /**< SPI2 */
    HSERIAL_SPI3,     /**< SPI3 */
    HSERIAL_SPI4,     /**< SPI4 */
}HSERIAL_Spi_Number_t;

/** @brief HSERIAL SPI communication mode (BIDIMODE/RXONLY) */
typedef enum {
    HSERIAL_COMMINICATION_SPI_FULL_DUPLEX                = 0b00000000000000000000000000000000, /**< FD */
    HSERIAL_COMMINICATION_SPI_HALF_DUPLEX_2LINES_TX_ONLY = 0b00000000000000000000000000000000, /**< HD 2-lines TX */
    HSERIAL_COMMINICATION_SPI_HALF_DUPLEX_2LINES_RX_ONLY = 0b00000000000000000000010000000000, /**< HD 2-lines RX */
    HSERIAL_COMMINICATION_SPI_HALF_DUPLEX_1LINE_RX_ONLY  = 0b00000000000000001000000000000000, /**< HD 1-line RX */
    HSERIAL_COMMINICATION_SPI_HALF_DUPLEX_1LINE_TX_ONLY  = 0b00000000000000001100000000000000, /**< HD 1-line TX */
}HSERIAL_Spi_Communication_Mode_t;

/** @brief HSERIAL SPI master/slave (MSTR bit2) */
typedef enum {
    HSERIAL_SPI_MASTER        = 0b00000000000000000000000000000000, /**< Slave value per original mapping */
    HSERIAL_SPI_SLAVE         = 0b00000000000000000000000000000100  /**< Master value (note inversion in hserial.h) */
}HSERIAL_Spi_Mode_t;

/** @brief HSERIAL SPI CRC (CRCEN bit13) */
typedef enum {
    HSERIAL_SPI_CRC_DISABLED = 0b00000000000000000000000000000000, /**< Off */
    HSERIAL_SPI_CRC_ENABLED  = 0b00000000000000000010000000000000  /**< On */
}HSERIAL_Spi_Crc_t;

/** @brief HSERIAL SPI data length (DFF bit11) */
typedef enum {
    HSERIAL_SPI_8_BIT_DATA   = 0b00000000000000000000000000000000, /**< 8-bit */
    HSERIAL_SPI_16_BIT_DATA  = 0b00000000000000000000100000000000  /**< 16-bit */
}HSERIAL_Spi_DataLength_t;

/** @brief HSERIAL SPI data order (LSBFIRST bit7) */
typedef enum {
    HSERIAL_SPI_MSB_FIRST    = 0b00000000000000000000000000000000, /**< MSB first */
    HSERIAL_SPI_LSB_FIRST    = 0b00000000000000000000000010000000  /**< LSB first */
}HSERIAL_Spi_DataOrder_t;

/** @brief HSERIAL SPI baud rate (BR 5:3) */
typedef enum {
    HSERIAL_SPI_BAUDERATE_DIV2     = 0b00000000000000000000000000000000, /**< /2 */
    HSERIAL_SPI_BAUDERATE_DIV4     = 0b00000000000000000000000000001000, /**< /4 */
    HSERIAL_SPI_BAUDERATE_DIV8     = 0b00000000000000000000000000010000, /**< /8 */
    HSERIAL_SPI_BAUDERATE_DIV16    = 0b00000000000000000000000000011000, /**< /16 */
    HSERIAL_SPI_BAUDERATE_DIV32    = 0b00000000000000000000000000100000, /**< /32 */
    HSERIAL_SPI_BAUDERATE_DIV64    = 0b00000000000000000000000000101000, /**< /64 */
    HSERIAL_SPI_BAUDERATE_DIV128   = 0b00000000000000000000000000110000, /**< /128 */
    HSERIAL_SPI_BAUDERATE_DIV256   = 0b00000000000000000000000000111000  /**< /256 */
}HSERIAL_Spi_BaudRate_t;


/** @brief HSERIAL SPI frame format (FRF bit4) */
typedef enum {
    HSERIAL_SPI_MOTOROLA = 0b00000000000000000000000000000000, /**< Motorola */
    HSERIAL_SPI_TI       = 0b00000000000000000000000100000000  /**< TI */
}HSERIAL_Spi_FrameFormat_t;

// typedef enum {
//     HSERIAL_SPI_DISABLE_DMA         = 0b00000000000000000000000000000000,
//     HSERIAL_SPI_DMA_TX_ENABLE       = 0b00000000000000000000000000000010,
//     HSERIAL_SPI_DMA_RX_ENABLE       = 0b00000000000000000000000000000001,
//     HSERIAL_SPI_DMA_TX_RX_ENABLE    = 0b00000000000000000000000000000011
// }HSERIAL_Spi_Dma_t;

/** @brief HSERIAL SPI CPOL/CPHA (bits1:0) */
typedef enum {
    HSERIAL_SPI_ZERO_IDLE_FIRST_EDGE  = 0b00000000000000000000000000000000, /**< CPOL0 CPHA0 */
    HSERIAL_SPI_ZERO_IDLE_SECOND_EDGE = 0b00000000000000000000000000000001, /**< CPOL0 CPHA1 */
    HSERIAL_SPI_ONE_IDLE_FIRST_EDGE   = 0b00000000000000000000000000000010, /**< CPOL1 CPHA0 */
    HSERIAL_SPI_ONE_IDLE_SECOND_EDGE  = 0b00000000000000000000000000000011  /**< CPOL1 CPHA1 */
}HSERIAL_Spi_PolarityPhase_t;

/** @brief HSERIAL SPI NSS management (SSM/SSI/SSOE) */
typedef enum {
    HSERIAL_SPI_NSS_MASTER_HW_OUTPUT       = 0b00000000000000000000000000000100, /**< HW output SSOE */
    HSERIAL_SPI_NSS_MASTER_HW_INPUT        = 0b00000000000000000000000000000000, /**< HW input */
    HSERIAL_SPI_NSS_MASTER_SW              = 0b00000000000000000000001100000000, /**< SW (GPIO CS) */
    HSERIAL_SPI_NSS_SLAVE_HW               = 0b00000000000000000000000000000000, /**< Slave HW */
    HSERIAL_SPI_NSS_SLAVE_SW               = 0b00000000000000000000001000000000  /**< Slave SW */
}HSERIAL_Spi_Nss_t;

/******************************************************************************
 * @brief HSERIAL UART sync config (polling — no callbacks/NVIC)
 ******************************************************************************/
typedef struct {
    uint32_t                     HSERIAL_UartPeripheralClock; /**< APB clock Hz */
    HSERIAL_Uart_Number_t        HSERIAL_UartChannel;         /**< USART1/2/6 */
    uint32_t                     HSERIAL_UartBaudRate;        /**< Baud bps */
    HSERIAL_Uart_Parity_t        HSERIAL_UartParity;          /**< Parity */
    HSERIAL_Uart_OverSampling_t  HSERIAL_UartOverSampling;    /**< OVER16/8 */
    HSERIAL_Uart_StopBit_t       HSERIAL_UartStopBits;        /**< Stop bits */
    HSERIAL_Uart_WordLength_t    HSERIAL_UartWordLength;      /**< 8/9-bit */
    HSERIAL_Uart_Sample_t        HSERIAL_UartSample;          /**< ONE/THREE sample */
    HSERIAL_Uart_Enable_t        HSERIAL_UartEnable;          /**< TE/RE/BOTH */
} H_UART_Sync_Config_t;

/******************************************************************************
 * @brief HSERIAL UART async config (interrupt + callbacks + priority)
 ******************************************************************************/
typedef struct {
    uint32_t                     HSERIAL_UartPeripheralClock; /**< APB clock */
    HSERIAL_Uart_Number_t        HSERIAL_UartChannel;         /**< Instance */
    uint32_t                     HSERIAL_UartBaudRate;        /**< Baud */
    HSERIAL_Uart_Parity_t        HSERIAL_UartParity;          /**< Parity */
    HSERIAL_Uart_OverSampling_t  HSERIAL_UartOverSampling;    /**< OVER sampling */
    HSERIAL_Uart_StopBit_t       HSERIAL_UartStopBits;        /**< Stop */
    HSERIAL_Uart_WordLength_t    HSERIAL_UartWordLength;      /**< Word len */
    HSERIAL_Uart_Sample_t        HSERIAL_UartSample;          /**< Sample */
    HSERIAL_Uart_Enable_t        HSERIAL_UartEnable;          /**< Enable mask */
    HSERIAL_Callback_t           HSERIAL_UartTxCompleteCallback; /**< TX done ISR callback */
    HSERIAL_Callback_t           HSERIAL_UartRxCompleteCallback; /**< RX done ISR callback */
    HSERIAL_Priority_t           HSERIAL_UartInterruptPriority;  /**< NVIC priority (0x00..0xF0) */
} H_UART_Async_Config_t;
    

/******************************************************************************
 * @brief HSERIAL UART DMA config (TX+RX DMA streams, per-direction NVIC)
 ******************************************************************************/
typedef struct {
    uint32_t                     HSERIAL_UartPeripheralClock; /**< APB clock */
    HSERIAL_Uart_Number_t        HSERIAL_UartChannel;         /**< Instance */
    uint32_t                     HSERIAL_UartBaudRate;        /**< Baud */
    HSERIAL_Uart_Parity_t        HSERIAL_UartParity;          /**< Parity */
    HSERIAL_Uart_OverSampling_t  HSERIAL_UartOverSampling;    /**< OVER */
    HSERIAL_Uart_StopBit_t       HSERIAL_UartStopBits;        /**< Stop */
    HSERIAL_Uart_WordLength_t    HSERIAL_UartWordLength;      /**< Word len */
    HSERIAL_Uart_Sample_t        HSERIAL_UartSample;          /**< Sample */
    HSERIAL_Uart_Enable_t        HSERIAL_UartEnable;          /**< Enable */
    HSERIAL_Callback_t           HSERIAL_UartTxCompleteCallback; /**< TX TC callback (via UART TC IRQ) */
    HSERIAL_Callback_t           HSERIAL_UartRxCompleteCallback; /**< RX DMA TC callback */
    HSERIAL_Priority_t           HSERIAL_UartInterruptPriority;  /**< Priority for UART TC + DMA RX IRQ */
}H_UART_Dma_Config_t;

 /**
  * @brief HSERIAL SPI sync config (polling, no callbacks)
  */
 typedef struct {
    HSERIAL_Spi_Number_t              HSERIAL_SpiChannel;            /**< SPI1..4 */
    HSERIAL_Spi_Communication_Mode_t  HSERIAL_SpiCommunicationMode;  /**< FD/HD */
    HSERIAL_Spi_Mode_t                HSERIAL_SpiMode;               /**< Master/Slave */
    HSERIAL_Spi_Crc_t                 HSERIAL_SpiCrc;                /**< CRC */
    HSERIAL_Spi_DataOrder_t           HSERIAL_SpiDataOrder;          /**< MSB/LSB */
    HSERIAL_Spi_BaudRate_t            HSERIAL_SpiBaudRate;           /**< DIV2..256 */
    HSERIAL_Spi_FrameFormat_t         HSERIAL_SpiFrameFormat;        /**< Motorola/TI */
    HSERIAL_Spi_PolarityPhase_t       HSERIAL_SpiPolarityPhase;      /**< CPOL/CPHA */
    HSERIAL_Spi_Nss_t                 HSERIAL_SpiNss;                /**< NSS */
    HSERIAL_Spi_DataLength_t          HSERIAL_SpiDataLength;         /**< 8/16-bit */
}H_SPI_Sync_Config_t;

 /**
  * @brief HSERIAL SPI async config (interrupt + callbacks + priority)
  */
 typedef struct {
    HSERIAL_Spi_Number_t              HSERIAL_SpiChannel;            /**< Instance */
    HSERIAL_Spi_Communication_Mode_t  HSERIAL_SpiCommunicationMode;  /**< Mode */
    HSERIAL_Spi_Mode_t                HSERIAL_SpiMode;               /**< Master/Slave */
    HSERIAL_Spi_Crc_t                 HSERIAL_SpiCrc;                /**< CRC */
    HSERIAL_Spi_DataOrder_t           HSERIAL_SpiDataOrder;          /**< Order */
    HSERIAL_Spi_BaudRate_t            HSERIAL_SpiBaudRate;           /**< Baud */
    HSERIAL_Spi_FrameFormat_t         HSERIAL_SpiFrameFormat;        /**< Frame */
    HSERIAL_Spi_PolarityPhase_t       HSERIAL_SpiPolarityPhase;      /**< CPOL/CPHA */
    HSERIAL_Spi_Nss_t                 HSERIAL_SpiNss;                /**< NSS */
    HSERIAL_Spi_DataLength_t          HSERIAL_SpiDataLength;         /**< Length */
    HSERIAL_Callback_t                HSERIAL_SpiTxCompleteCallback; /**< TX done */
    HSERIAL_Callback_t                HSERIAL_SpiRxCompleteCallback; /**< RX done */
    HSERIAL_Priority_t                HSERIAL_SpiInterruptPriority;  /**< NVIC priority */
}H_SPI_Async_Config_t;



/******************************************************************************
 * @brief Top-level HSERIAL channel config union — indexed by hserial_cfg.h
 * @details Outer Mode selects which union member is valid. Array
 *          HSERIAL_Configurations[HSERIAL_CHANNEL_LENGTH] defined in hserial_cfg.c.
 ******************************************************************************/
typedef struct {
    HSERIAL_Mode_t     HSERIAL_Mode; /**< Mode that selects union member */
    union {
        H_UART_Sync_Config_t     UART_Sync_Config;  /**< Valid if MODE_UART_SYNC */
        H_UART_Async_Config_t    UART_Async_Config; /**< Valid if MODE_UART_ASYNC */
        H_UART_Dma_Config_t      UART_Dma_Config;   /**< Valid if MODE_UART_DMA */
        H_SPI_Sync_Config_t      SPI_Sync_Config;   /**< Valid if MODE_SPI_SYNC */
        H_SPI_Async_Config_t     SPI_Async_Config;  /**< Valid if MODE_SPI_ASYNC */
    }; 
}HSERIAL_Config_t;


/******************************************************************************
 * @name HSERIAL Public APIs (dispatch by channel Mode)
 * @{
 ******************************************************************************/
/**
 * @brief Initialize all HSERIAL channels from HSERIAL_Configurations[]
 * @details Iterates HSERIAL_CHANNEL_LENGTH, switches on HSERIAL_Mode and calls
 *          appropriate HSERIAL_enu*InitUart/Spi (which in turn calls UART/SPI/
 *          DMA/NVIC inits). Returns first error or HSERIAL_OK.
 * @param None (uses hserial_cfg.c table)
 * @return HSERIAL_Status_t HSERIAL_OK or ERROR_INIT_UART/DMA/SPI/NVIC / NOT_OK
 */
HSERIAL_Status_t HSERIAL_enuInit(void);
/**
 * @brief Transmit buffer on given channel (mode-dependent)
 * @details Validates channel < LENGTH, buffer !=NULL, size>0, then switches
 *          on channel Mode → calls HSERIAL_enu*TransmitBuffer. Sync blocks,
 *          async/DMA non-blocking with callbacks.
 * @param[in] channel Channel index from HSERIAL_Channel_t
 * @param[in] dataBuffer Source bytes (retained until DMA/ISR complete for async)
 * @param[in] size Number of bytes (>0)
 * @return HSERIAL_Status_t HSERIAL_OK, WRONG_CHANNEL, NULL_POINTER, INVALID_SIZE,
 *         FAILED_TRANSMIT
 */
HSERIAL_Status_t HSERIAL_enuTransmitBuffer(HSERIAL_Channel_t channel, const uint8_t* dataBuffer, uint16_t size);
/**
 * @brief Receive buffer on given channel (mode-dependent)
 * @details Validates channel/buffer/size, switches on Mode →
 *          HSERIAL_enu*ReceiveBuffer. Sync polls RXNE; async/DMA via IRQ.
 * @param[in] channel Channel index
 * @param[out] dataBuffer Destination buffer (must remain valid for async)
 * @param[in] size Bytes to receive (>0)
 * @return HSERIAL_Status_t HSERIAL_OK or error
 */
HSERIAL_Status_t HSERIAL_enuReceiveBuffer(HSERIAL_Channel_t channel,uint8_t* dataBuffer, uint16_t size);

/** @} */

#endif // HSERIAL_H
