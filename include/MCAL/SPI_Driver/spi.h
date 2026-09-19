/******************************************************************************
 * @file    spi.h
 * @author  Eng.Gemy
 * @brief   SPI Driver Interface Header File (STM32F401 SPI1/2/3/4)
 *          Supports master/slave, full/half duplex, 8/16-bit, CPOL/CPHA,
 *          TI/Motorola, SW/HW NSS (8 slaves), CRC, DMA markers, and
 *          async interrupt mode with per-flag callbacks.
 * @date    2024
 * @version 1.0
 * @note    For master with multiple slaves use SPI_NSS_MASTER_SW (0x0300)
 *          and manage CS via GPIO (Select/Deselect). See quick guide.
 ******************************************************************************/
#ifndef SPI_H_
#define SPI_H_

#include "LIB/stdtypes.h"

#define SPI_MAX_SLAVES      8   /**< Maximum slaves addressable by one master */

/** @brief SPI callback type for async/IRQ (void(void)) */
typedef void (*SPI_Callback_t)(void);

/** @brief SPI port enumeration — mirrors GPIO ports for CS handling */
typedef enum {
    SPI_PORT_A = 0, /**< Port A */
    SPI_PORT_B,      /**< Port B */
    SPI_PORT_C,      /**< Port C */
    SPI_PORT_D,      /**< Port D */
    SPI_PORT_E,      /**< Port E */
    SPI_PORT_H,      /**< Port H */
}SPI_Port_t;

/** @brief SPI pin enumeration (0..15) within port */
typedef enum {
    SPI_PIN_0 = 0, SPI_PIN_1, SPI_PIN_2, SPI_PIN_3, SPI_PIN_4, SPI_PIN_5,
    SPI_PIN_6, SPI_PIN_7, SPI_PIN_8, SPI_PIN_9, SPI_PIN_10, SPI_PIN_11,
    SPI_PIN_12, SPI_PIN_13, SPI_PIN_14, SPI_PIN_15,
}SPI_Pin_t;

/** @brief SPI busy state per instance (async guard) */
typedef enum {
    SPI_NOT_BUSY=0, /**< Ready */
    SPI_BUSY       /**< Transaction in progress */
}SPI_State_t;

/******************************************************************************
 * @brief SPI Status Enumeration
 * @details Detailed validation errors for Init and transfer APIs.
 ******************************************************************************/
typedef enum {
    SPI_NOT_OK=0,              /**< General failure */
    SPI_OK,                    /**< Success */
    SPI_WRONG_SPI_NUMBER,      /**< spiNumber > SPI4 */
    SPI_WRONG_COMMUNICATION_MODE, /**< BIDIMODE/RXONLY mask invalid */
    SPI_WRONG_MODE,            /**< MSTR mask invalid */
    SPI_WRONG_CRC_STATE,       /**< CRCEN mask invalid */
    SPI_WRONG_DATA_LENGTH,     /**< DFF mask invalid */
    SPI_WRONG_DATA_ORDER,      /**< LSBFIRST mask invalid */
    SPI_WRONG_BAUDRATE,        /**< BR[2:0] mask invalid */
    SPI_WRONG_POLARITY_PHASE,  /**< CPOL/CPHA mask invalid */
    SPI_WRONG_FRAME_FORMAT,    /**< FRF mask invalid */
    SPI_WRONG_DMA_STATE,       /**< TXDMAEN/RXDMAEN mask invalid */
    SPI_WRONG_NSS_MANAGEMENT,  /**< SSM/SSI/SSOE combo invalid */
    SPI_NULL_POINTER,          /**< NULL buffer/callback */
    SPI_GPIO_NOT_INITIALIZED,  /**< GPIO AF init failed for SCK/MOSI/MISO/NSS */
    SPI_ERROR_SELECTING_SLAVE, /**< GPIO set for CS failed */
    SPI_STATUS_IS_BUSY,        /**< Instance BUSY (poll BUSY flag or async state) */
    SPI_WRONG_FLAG_VALUE,      /**< Flag enum out of range */
} SPI_Status_t;

/** @brief SPI instance enumeration — maps to 0x40013000/0x40003800/0x40003C00/0x40013400 */
typedef enum {
    SPI1 = 0, /**< SPI1 (APB2) */
    SPI2,     /**< SPI2 (APB1) */
    SPI3,     /**< SPI3 (APB1) */
    SPI4      /**< SPI4 (APB2) */
}SPI_Number_t;

/******************************************************************************
 * @brief SPI Communication Mode Enumeration (CR1 BIDIMODE/RXONLY/BIDIOE)
 ******************************************************************************/
typedef enum {
    // (BIDIMODE > 15 ,RXONLY > 10, BIDIOE > 14)        **   *    
    // BIDIMODE=0, RXONLY=0,BIDIOE=x >0b10987654321098765432109876543210
    SPI_FULL_DUPLEX                 = 0b00000000000000000000000000000000, /**< FD: BIDIMODE=0,RXONLY=0 */
    // BIDIMODE=0, RXONLY=0,BIDIOE=x >0b10987654321098765432109876543210
    SPI_HALF_DUPLEX_2LINES_TX_ONLY  = 0b00000000000000000000000000000000, /**< HD 2-lines TX (same bits as FD; TX-only by sw) */
    // BIDIMODE=0, RXONLY=1,BIDIOE=x >0b10987654321098765432109876543210
    SPI_HALF_DUPLEX_2LINES_RX_ONLY  = 0b00000000000000000000010000000000, /**< HD 2-lines RX ONLY (RXONLY=1) */
    // BIDIMODE=1, RXONLY=x,BIDIOE=0 >0b10987654321098765432109876543210
    SPI_HALF_DUPLEX_1LINE_RX_ONLY   = 0b00000000000000001000000000000000, /**< HD 1-line RX (BIDIMODE=1,BIDIOE=0) */
    // BIDIMODE=1, RXONLY=x,BIDIOE=1 >0b10987654321098765432109876543210
    SPI_HALF_DUPLEX_1LINE_TX_ONLY   = 0b00000000000000001100000000000000, /**< HD 1-line TX (BIDIMODE=1,BIDIOE=1) */
}SPI_Communication_t;



/******************************************************************************
 * @brief SPI Master/Slave Mode Enumeration (CR1.MSTR bit2)
 ******************************************************************************/
typedef enum {
    // MSTR > 2                                       *      
    //                 0b10987654321098765432109876543210
    SPI_SLAVE        = 0b00000000000000000000000000000000, /**< Slave (MSTR=0) */
    SPI_MASTER       = 0b00000000000000000000000000000100  /**< Master (MSTR=1) */
}SPI_Mode_t;


/******************************************************************************
 * @brief SPI CRC Enable Enumeration (CR1.CRCEN bit13)
 ******************************************************************************/
typedef enum {
    // CRCEN > 13                          *              
    //                 0b10987654321098765432109876543210
    SPI_CRC_DISABLED = 0b00000000000000000000000000000000, /**< CRC off */
    SPI_CRC_ENABLED  = 0b00000000000000000010000000000000  /**< CRC on (poly in CRCPR) */
}SPI_Crc_t;

/******************************************************************************
 * @brief SPI Data Length Enumeration (CR1.DFF bit11)
 ******************************************************************************/
typedef enum {
    // DFF > 11                              *          
    //                 0b10987654321098765432109876543210
    SPI_8_BIT_DATA   = 0b00000000000000000000000000000000, /**< 8-bit (mask 0x00FF) */
    SPI_16_BIT_DATA  = 0b00000000000000000000100000000000  /**< 16-bit (mask 0xFFFF) */
}SPI_DataLength_t;

/******************************************************************************
 * @brief SPI Data Order Enumeration (CR1.LSBFIRST bit7)
 ******************************************************************************/
typedef enum {
    // LSBFIRST > 7                              *      
    //                 0b10987654321098765432109876543210
    SPI_MSB_FIRST    = 0b00000000000000000000000000000000, /**< MSB first (default) */
    SPI_LSB_FIRST    = 0b00000000000000000000000010000000  /**< LSB first */
}SPI_DataOrder_t;

/******************************************************************************
 * @brief SPI Baud Rate Enumeration (CR1.BR[5:3])
 ******************************************************************************/
typedef enum {
    // BR[2:0] > 5:3                                    ***      
    //                      0b10987654321098765432109876543210
    SPI_BAUDRATE_DIV2     = 0b00000000000000000000000000000000, /**< fPCLK/2 */
    SPI_BAUDRATE_DIV4     = 0b00000000000000000000000000001000, /**< fPCLK/4 */
    SPI_BAUDRATE_DIV8     = 0b00000000000000000000000000010000, /**< fPCLK/8 */
    SPI_BAUDRATE_DIV16    = 0b00000000000000000000000000011000, /**< fPCLK/16 */
    SPI_BAUDRATE_DIV32    = 0b00000000000000000000000000100000, /**< fPCLK/32 */
    SPI_BAUDRATE_DIV64    = 0b00000000000000000000000000101000, /**< fPCLK/64 */
    SPI_BAUDRATE_DIV128   = 0b00000000000000000000000000110000, /**< fPCLK/128 */
    SPI_BAUDRATE_DIV256   = 0b00000000000000000000000000111000  /**< fPCLK/256 */
}SPI_BaudRate_t;

/******************************************************************************
 * @brief SPI Clock Polarity/Phase Enumeration (CR1.CPOL bit1, CPHA bit0)
 ******************************************************************************/
typedef enum {
    // CPOL > 1 , CPHA > 0                                      **
    //                          0b10987654321098765432109876543210
    SPI_ZERO_IDLE_FIRST_EDGE  = 0b00000000000000000000000000000000, /**< CPOL=0,CPHA=0 */
    SPI_ZERO_IDLE_SECOND_EDGE = 0b00000000000000000000000000000001, /**< CPOL=0,CPHA=1 */
    SPI_ONE_IDLE_FIRST_EDGE   = 0b00000000000000000000000000000010, /**< CPOL=1,CPHA=0 */
    SPI_ONE_IDLE_SECOND_EDGE  = 0b00000000000000000000000000000011  /**< CPOL=1,CPHA=1 */
}SPI_PolarityPhase_t;


/******************************************************************************
 * @brief SPI Frame Format Enumeration (CR2.FRF bit4)
 ******************************************************************************/
typedef enum {
    // FRF > 4                                      *          
    //                 0b10987654321098765432109876543210
    SPI_MOTOROLA     = 0b00000000000000000000000000000000, /**< Motorola (default) */
    SPI_TI_MODE      = 0b00000000000000000000000000010000  /**< TI mode */
}SPI_FrameFormat_t;

/******************************************************************************
 * @brief SPI DMA State Enumeration (CR2.TXDMAEN bit1, RXDMAEN bit0)
 ******************************************************************************/
typedef enum {
    // RXDMAEN > 0 , TXDMAEN > 1                            **    
    //                      0b10987654321098765432109876543210
    SPI_DISABLE_DMA       = 0b00000000000000000000000000000000, /**< DMA off */
    SPI_DMA_TX_ENABLE     = 0b00000000000000000000000000000010, /**< TX DMA */
    SPI_DMA_RX_ENABLE     = 0b00000000000000000000000000000001, /**< RX DMA */
    SPI_DMA_TX_RX_ENABLE  = 0b00000000000000000000000000000011  /**< TX+RX DMA */
}SPI_Dma_t;

/*
╔════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
║                                    QUICK SELECTION GUIDE                                                           ║
╠════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣
║                                                         |                                                          ║
║   QUESTION                                              │ USE THIS ENUM                                            ║
║   ──────────────────────────────────────────────────────┼──────────────────────────────────────────────────────────║
║                                                         │                                                          ║
║   Master with 1 slave, want simple auto NSS?            │ SPI_NSS_MASTER_HW_OUTPUT (0x0004)                        ║
║                                                         │                                                          ║
║   ──────────────────────────────────────────────────────┼──────────────────────────────────────────────────────────║
║                                                         │                                                          ║
║   Master with multiple slaves?                          │ SPI_NSS_MASTER_SW (0x0300)                               ║
║                                                         │                                                          ║
║   ──────────────────────────────────────────────────────┼──────────────────────────────────────────────────────────║
║                                                         │                                                          ║
║   Multiple masters on same bus?                         │ SPI_NSS_MASTER_HW_INPUT (0x0000)                         ║
║                                                         │                                                          ║
║   ──────────────────────────────────────────────────────┼──────────────────────────────────────────────────────────║
║                                                         │                                                          ║
║   Normal slave, master controls selection?              │ SPI_NSS_SLAVE_HW (0x0000)                                ║
║                                                         │                                                          ║
║   ──────────────────────────────────────────────────────┼──────────────────────────────────────────────────────────║
║                                                         │                                                          ║
║   Single slave, want to save NSS wire/pin?              │ SPI_NSS_SLAVE_SW (0x0200)                                ║
║                                                         │                                                          ║
╚════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝
*/
/******************************************************************************
 * @brief SPI NSS Management Enumeration (CR1.SSM/SSI + CR2.SSOE)
 * @details Encodes SSM (bit9), SSI (bit8), SSOE (bit2). See quick guide above.
 ******************************************************************************/
typedef enum {
    // SSM > 9 , SSI > 8  ,SSOE > 2                          **     *
    //                               0b10987654321098765432109876543210
    SPI_NSS_MASTER_HW_OUTPUT       = 0b00000000000000000000000000000100, /**< Single master/single slave — HW drives NSS */
    SPI_NSS_MASTER_HW_INPUT        = 0b00000000000000000000000000000000, /**< Multi-master — NSS input detects conflict */
    SPI_NSS_MASTER_SW              = 0b00000000000000000000001100000000, /**< Multi-slave — CS via GPIO array */
    
    SPI_NSS_SLAVE_HW               = 0b00000000000000000000000000000000, /**< Normal slave — master drives NSS */
    SPI_NSS_SLAVE_SW               = 0b00000000000000000000001000000000  /**< Lone slave — always selected (SSI=1) */
}SPI_Nss_t;



/******************************************************************************
 * @brief SPI flag positions in SR register
 ******************************************************************************/
typedef enum{
    SPI_FLAG_RXNE               = 0, /**< Receive buffer not empty */
    SPI_FLAG_TXE                = 1, /**< Transmit buffer empty */
    SPI_FLAG_UNDERRUN_ERROR     = 3, /**< Underrun (slave) */
    SPI_FLAG_CRC_ERROR          = 4, /**< CRC error */
    SPI_FLAG_MODE_FAULT         = 5, /**< Mode fault (NSS conflict) */
    SPI_FLAG_OVERRUN_ERROR      = 6, /**< Overrun */
    SPI_FLAG_BUSY               = 7, /**< Busy */
    SPI_FLAG_FRAME_FORMAT_ERROR = 8, /**< Frame format error (TI) */
}SPI_Flag_t;



/*******************************************************************************
 * SLAVE CHIP SELECT PIN DEFINITION
 ******************************************************************************/
/** @brief Single slave CS pin descriptor (used with MASTER_SW) */
typedef struct {
    SPI_Port_t     port; /**< GPIO port of CS */
    SPI_Pin_t      pin;  /**< GPIO pin of CS */
} SPI_SlaveCS_t;

/*******************************************************************************
 * SLAVES CONFIGURATION (for Master mode)
 ******************************************************************************/
/** @brief Master's slave table — up to 8 CS pins managed via GPIO */
typedef struct {
    SPI_SlaveCS_t   slaves[SPI_MAX_SLAVES]; /**< CS pin array */
    uint8_t         numberOfSlaves;         /**< How many entries valid */
} SPI_SlavesConfig_t;

/** @brief Complete SPI configuration passed to SPI_enuInit */
typedef struct {
    SPI_Number_t          spiNumber;       /**< Instance */
    SPI_Communication_t   communicationMode; /**< FD/HD mode */
    SPI_Mode_t            mode;            /**< MASTER/SLAVE */
    SPI_Crc_t             crcState;        /**< CRC on/off */
    SPI_DataLength_t      dataLength;      /**< 8/16-bit */
    SPI_DataOrder_t       dataOrder;       /**< MSB/LSB */
    SPI_BaudRate_t        baudRate;        /**< DIV2..256 */
    SPI_PolarityPhase_t   polarityPhase;   /**< CPOL/CPHA */
    SPI_FrameFormat_t     frameFormat;     /**< Motorola/TI */
    SPI_Dma_t             dmaState;        /**< DMA mask */
    SPI_Nss_t             nssManagement;   /**< NSS combo */
    uint16_t              crcPolynomial;   /**< CRC polynomial 0..0xFFFF (if enabled) */
    SPI_SlavesConfig_t    slavesConfig;    /**< CS table (master SW NSS only) */
}SPI_Config_t;


/******************************************************************************
 * @name SPI Public APIs
 * @{
 ******************************************************************************/
/**
 * @brief Initialize SPI with full config
 * @details Validates all bit-masks, inits GPIO AF for SCK/MISO/MOSI/NSS per
 *          mode/comm/nss combo, configures CR1/CR2 (baud/CPHA/LSB/DFF/DMA/NSS),
 *          sets CRCPR, enables SPE. Sets SPI_MaskData for 8/16-bit.
 * @param[in] SpiConfig Pointer to SPI_Config_t (must not be NULL)
 * @return SPI_Status_t SPI_OK or WRONG_* / NULL_POINTER / GPIO_NOT_INITIALIZED
 */
SPI_Status_t SPI_enuInit(SPI_Config_t* SpiConfig);
/**
 * @brief Master sync transmit+receive (blocking, full-duplex)
 * @details Polls TXE, writes DR, polls RXNE, reads DR; waits BUSY clear.
 *          Handles 8 vs 16-bit mask for DR access.
 * @param[in] spiNumber Instance
 * @param[in] TxData Word to send (low byte for 8-bit)
 * @param[out] RxData Pointer to receive word
 * @return SPI_Status_t SPI_OK or NULL_POINTER/WRONG_SPI_NUMBER
 */
SPI_Status_t SPI_enuMasterSyncTransmitReceive(SPI_Number_t spiNumber, uint16_t TxData, uint16_t *RxData);
/** @brief Master sync transmit (dummy RX discard) */
SPI_Status_t SPI_enuMasterSyncTransmit(SPI_Number_t spiNumber, uint16_t TxData);
/** @brief Master sync receive (sends dummy Tx = mask) */
SPI_Status_t SPI_enuMasterSyncReceive(SPI_Number_t spiNumber ,uint16_t *RxData);
/**
 * @brief Master async transmit — TXE interrupt
 * @details Checks BUSY (state | SR.BSY), registers callback on TXE flag,
 *          enables TXEIE, writes first DR. ISR will clear BUSY and callback.
 * @param[in] spiNumber Instance
 * @param[in] TxData Word to start
 * @param[in] callback TXE completion callback (invoked from ISR)
 * @return SPI_Status_t SPI_OK, STATUS_IS_BUSY, NULL_POINTER
 */
SPI_Status_t SPI_enuMasterAsynTransmit(SPI_Number_t spiNumber, uint16_t TxData, SPI_Callback_t callback);
/** @brief Master async receive — RXNE interrupt (sends dummy, callback on RXNE) */
SPI_Status_t SPI_enuMasterAsynReceive(SPI_Number_t spiNumber, uint16_t* RxData,SPI_Callback_t callback);
/**
 * @brief Assert slave CS (GPIO_HIGH — design drives HIGH to select due to inversion)
 * @param[in] spiNumber Instance (validated)
 * @param[in] slave CS pin descriptor from slavesConfig
 * @return SPI_Status_t SPI_OK or ERROR_SELECTING_SLAVE if GPIO fail
 * @note  Current impl uses GPIO_HIGH for select; verify board wiring (may need LOW).
 */
SPI_Status_t SPI_enuMasterSelectSlave(SPI_Number_t spiNumber,SPI_SlaveCS_t slave);
/** @brief Deassert slave CS (GPIO_LOW) */
SPI_Status_t SPI_enuMasterDeselectSlave(SPI_Number_t spiNumber,SPI_SlaveCS_t slave);

/** @brief Slave sync transmit — polls TXE, writes DR */
SPI_Status_t SPI_enuSlaveSyncTransmit(SPI_Number_t spiNumber,uint16_t TxData);
/** @brief Slave sync receive — polls RXNE, reads DR */
SPI_Status_t SPI_enuSlaveSyncReceive(SPI_Number_t spiNumber,uint16_t* RxData);
/** @brief Slave async transmit — TXE interrupt */
SPI_Status_t SPI_enuSlaveAsynTransmit(SPI_Number_t spiNumber, uint16_t TxData, SPI_Callback_t callback);
/** @brief Slave async receive — RXNE interrupt */
SPI_Status_t SPI_enuSlaveAsynReceive(SPI_Number_t spiNumber, uint16_t* RxData,SPI_Callback_t callback);

/** @brief Clear flag by writing 1 to SR bit position (driver does SR|=1<<flag) */
SPI_Status_t SPI_enuClearFlag(SPI_Number_t spiNumber,SPI_Flag_t flag);
/** @brief Register per-flag callback (Tx_Callbacks[spi][flag]) */
SPI_Status_t SPI_enuRegisterCallback(SPI_Number_t spiNumber, SPI_Flag_t flag, SPI_Callback_t callback);
/** @brief Enable interrupt in CR2 (RXNEIE/TXEIE/ERRIE) per flag */
SPI_Status_t SPI_enuEnableInterrupt(SPI_Number_t spiNumber, SPI_Flag_t flag);
/** @brief Disable interrupt in CR2 */
SPI_Status_t SPI_enuDisableInterrupt(SPI_Number_t spiNumber, SPI_Flag_t flag);
/** @brief Register per-flag callback (duplicate declaration kept for compat) */
SPI_Status_t SPI_enuRegisterCallback(SPI_Number_t spiNumber, SPI_Flag_t flag, SPI_Callback_t callback);
/** @brief Read SR flag → 0/1 or 0xFF on error */
uint8_t SPI_u8ReadFlag(SPI_Number_t spiNumber,SPI_Flag_t flag);

/** @} */

#endif // SPI_H_
