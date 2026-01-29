#ifndef SPI_H_
#define SPI_H_

#include "LIB/stdtypes.h"

#define SPI_MAX_SLAVES      8   // Maximum slaves per master

typedef void (*SPI_Callback_t)(void);

typedef enum {
    SPI_PORT_A = 0,
    SPI_PORT_B,
    SPI_PORT_C,
    SPI_PORT_D,
    SPI_PORT_E,
    SPI_PORT_H,
}SPI_Port_t;

typedef enum {
    SPI_PIN_0 = 0,
    SPI_PIN_1,
    SPI_PIN_2,
    SPI_PIN_3,
    SPI_PIN_4,
    SPI_PIN_5,
    SPI_PIN_6,
    SPI_PIN_7,
    SPI_PIN_8,
    SPI_PIN_9,
    SPI_PIN_10,
    SPI_PIN_11,
    SPI_PIN_12,
    SPI_PIN_13,
    SPI_PIN_14,
    SPI_PIN_15,
}SPI_Pin_t;

typedef enum {
    SPI_NOT_BUSY=0,
    SPI_BUSY
}SPI_State_t;

typedef enum {
    SPI_NOT_OK=0,
    SPI_OK,
    SPI_WRONG_SPI_NUMBER,
    SPI_WRONG_COMMUNICATION_MODE,
    SPI_WRONG_MODE,
    SPI_WRONG_CRC_STATE,
    SPI_WRONG_DATA_LENGTH,
    SPI_WRONG_DATA_ORDER,
    SPI_WRONG_BAUDRATE,
    SPI_WRONG_POLARITY_PHASE,
    SPI_WRONG_FRAME_FORMAT,
    SPI_WRONG_DMA_STATE,
    SPI_WRONG_NSS_MANAGEMENT,
    SPI_NULL_POINTER,
    SPI_GPIO_NOT_INITIALIZED,
    SPI_ERROR_SELECTING_SLAVE,
    SPI_STATUS_IS_BUSY,
    SPI_WRONG_FLAG_VALUE,
} SPI_Status_t;

typedef enum {
    SPI1 = 0,
    SPI2,
    SPI3,
    SPI4
}SPI_Number_t;

typedef enum {
    // (BIDIMODE > 15 ,RXONLY > 10, BIDIOE > 14)        **   *    
    // BIDIMODE=0, RXONLY=0,BIDIOE=x >0b10987654321098765432109876543210
    SPI_FULL_DUPLEX                 = 0b00000000000000000000000000000000,
    // BIDIMODE=0, RXONLY=0,BIDIOE=x >0b10987654321098765432109876543210
    SPI_HALF_DUPLEX_2LINES_TX_ONLY  = 0b00000000000000000000000000000000,
    // BIDIMODE=0, RXONLY=1,BIDIOE=x >0b10987654321098765432109876543210
    SPI_HALF_DUPLEX_2LINES_RX_ONLY  = 0b00000000000000000000010000000000,
    // BIDIMODE=1, RXONLY=x,BIDIOE=0 >0b10987654321098765432109876543210
    SPI_HALF_DUPLEX_1LINE_RX_ONLY   = 0b00000000000000001000000000000000,
    // BIDIMODE=1, RXONLY=x,BIDIOE=1 >0b10987654321098765432109876543210
    SPI_HALF_DUPLEX_1LINE_TX_ONLY   = 0b00000000000000001100000000000000,
}SPI_Communication_t;



typedef enum {
    // MSTR > 2                                       *      
    //                 0b10987654321098765432109876543210
    SPI_SLAVE        = 0b00000000000000000000000000000000,
    SPI_MASTER       = 0b00000000000000000000000000000100
}SPI_Mode_t;


typedef enum {
    // CRCEN > 13                          *              
    //                 0b10987654321098765432109876543210
    SPI_CRC_DISABLED = 0b00000000000000000000000000000000,
    SPI_CRC_ENABLED  = 0b00000000000000000010000000000000
}SPI_Crc_t;

typedef enum {
    // DFF > 11                              *          
    //                 0b10987654321098765432109876543210
    SPI_8_BIT_DATA   = 0b00000000000000000000000000000000,
    SPI_16_BIT_DATA  = 0b00000000000000000000100000000000
}SPI_DataLength_t;

typedef enum {
    // LSBFIRST > 7                              *      
    //                 0b10987654321098765432109876543210
    SPI_MSB_FIRST    = 0b00000000000000000000000000000000,
    SPI_LSB_FIRST    = 0b00000000000000000000000010000000
}SPI_DataOrder_t;

typedef enum {
    // BR[2:0] > 5:3                                    ***      
    //                      0b10987654321098765432109876543210
    SPI_BAUDRATE_DIV2     = 0b00000000000000000000000000000000,
    SPI_BAUDRATE_DIV4     = 0b00000000000000000000000000001000,
    SPI_BAUDRATE_DIV8     = 0b00000000000000000000000000010000,
    SPI_BAUDRATE_DIV16    = 0b00000000000000000000000000011000,
    SPI_BAUDRATE_DIV32    = 0b00000000000000000000000000100000,
    SPI_BAUDRATE_DIV64    = 0b00000000000000000000000000101000,
    SPI_BAUDRATE_DIV128   = 0b00000000000000000000000000110000,
    SPI_BAUDRATE_DIV256   = 0b00000000000000000000000000111000      
}SPI_BaudRate_t;

typedef enum {
    // CPOL > 1 , CPHA > 0                                      **
    //                          0b10987654321098765432109876543210
    SPI_ZERO_IDLE_FIRST_EDGE  = 0b00000000000000000000000000000000,
    SPI_ZERO_IDLE_SECOND_EDGE = 0b00000000000000000000000000000001,
    SPI_ONE_IDLE_FIRST_EDGE   = 0b00000000000000000000000000000010,
    SPI_ONE_IDLE_SECOND_EDGE  = 0b00000000000000000000000000000011
}SPI_PolarityPhase_t;


typedef enum {
    // FRF > 4                                      *          
    //                 0b10987654321098765432109876543210
    SPI_MOTOROLA     = 0b00000000000000000000000000000000,
    SPI_TI_MODE      = 0b00000000000000000000000000010000
}SPI_FrameFormat_t;

typedef enum {
    // RXDMAEN > 0 , TXDMAEN > 1                            **    
    //                      0b10987654321098765432109876543210
    SPI_DISABLE_DMA       = 0b00000000000000000000000000000000,
    SPI_DMA_TX_ENABLE     = 0b00000000000000000000000000000010,
    SPI_DMA_RX_ENABLE     = 0b00000000000000000000000000000001,
    SPI_DMA_TX_RX_ENABLE  = 0b00000000000000000000000000000011
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
typedef enum {
    // SSM > 9 , SSI > 8  ,SSOE > 2                          **     *
    //                               0b10987654321098765432109876543210
    SPI_NSS_MASTER_HW_OUTPUT       = 0b00000000000000000000000000000100, // single master single slave
    SPI_NSS_MASTER_HW_INPUT        = 0b00000000000000000000000000000000, // Multiple masters 
    SPI_NSS_MASTER_SW              = 0b00000000000000000000001100000000, // Multiple slaves (no need for NSS at master >> we will manage it using GPIOPins)
    
    SPI_NSS_SLAVE_HW               = 0b00000000000000000000000000000000, // Normal Slave (slave NSS managed by master)
    SPI_NSS_SLAVE_SW               = 0b00000000000000000000001000000000  // Single slave (no need for NSS at slave >> always slave selected)
}SPI_Nss_t;



typedef enum{
    SPI_FLAG_RXNE               = 0,
    SPI_FLAG_TXE                = 1,
    SPI_FLAG_UNDERRUN_ERROR     = 3,
    SPI_FLAG_CRC_ERROR          = 4,
    SPI_FLAG_MODE_FAULT         = 5,
    SPI_FLAG_OVERRUN_ERROR      = 6,
    SPI_FLAG_BUSY               = 7,
    SPI_FLAG_FRAME_FORMAT_ERROR = 8,
}SPI_Flag_t;



/*******************************************************************************
 * SLAVE CHIP SELECT PIN DEFINITION
 ******************************************************************************/
typedef struct {
    SPI_Port_t     port;
    SPI_Pin_t      pin;
} SPI_SlaveCS_t;

/*******************************************************************************
 * SLAVES CONFIGURATION (for Master mode)
 ******************************************************************************/
typedef struct {
    SPI_SlaveCS_t   slaves[SPI_MAX_SLAVES];
    uint8_t         numberOfSlaves;
} SPI_SlavesConfig_t;

typedef struct {
    SPI_Number_t          spiNumber;
    SPI_Communication_t   communicationMode;
    SPI_Mode_t            mode;
    SPI_Crc_t             crcState;
    SPI_DataLength_t      dataLength;
    SPI_DataOrder_t       dataOrder;
    SPI_BaudRate_t        baudRate;
    SPI_PolarityPhase_t   polarityPhase;
    SPI_FrameFormat_t     frameFormat;
    SPI_Dma_t             dmaState;
    SPI_Nss_t             nssManagement;
    uint16_t              crcPolynomial;    // Value from 0 to 0xFFFF
    SPI_SlavesConfig_t    slavesConfig;     // Used only in Master mode
}SPI_Config_t;


SPI_Status_t SPI_enuInit(SPI_Config_t* SpiConfig);
SPI_Status_t SPI_enuMasterSyncTransmitReceive(SPI_Number_t spiNumber, uint16_t TxData, uint16_t *RxData);
SPI_Status_t SPI_enuMasterSyncTransmit(SPI_Number_t spiNumber, uint16_t TxData);
SPI_Status_t SPI_enuMasterSyncReceive(SPI_Number_t spiNumber ,uint16_t *RxData);
SPI_Status_t SPI_enuMasterAsynTransmit(SPI_Number_t spiNumber, uint16_t TxData, SPI_Callback_t callback);
SPI_Status_t SPI_enuMasterAsynReceive(SPI_Number_t spiNumber, uint16_t* RxData,SPI_Callback_t callback);
SPI_Status_t SPI_enuMasterSelectSlave(SPI_Number_t spiNumber,SPI_SlaveCS_t slave);
SPI_Status_t SPI_enuMasterDeselectSlave(SPI_Number_t spiNumber,SPI_SlaveCS_t slave);

SPI_Status_t SPI_enuSlaveSyncTransmit(SPI_Number_t spiNumber,uint16_t TxData);
SPI_Status_t SPI_enuSlaveSyncReceive(SPI_Number_t spiNumber,uint16_t* RxData);
SPI_Status_t SPI_enuSlaveAsynTransmit(SPI_Number_t spiNumber, uint16_t TxData, SPI_Callback_t callback);
SPI_Status_t SPI_enuSlaveAsynReceive(SPI_Number_t spiNumber, uint16_t* RxData,SPI_Callback_t callback);

SPI_Status_t SPI_enuClearFlag(SPI_Number_t spiNumber,SPI_Flag_t flag);
SPI_Status_t SPI_enuRegisterCallback(SPI_Number_t spiNumber, SPI_Flag_t flag, SPI_Callback_t callback);
SPI_Status_t SPI_enuEnableInterrupt(SPI_Number_t spiNumber, SPI_Flag_t flag);
SPI_Status_t SPI_enuDisableInterrupt(SPI_Number_t spiNumber, SPI_Flag_t flag);
SPI_Status_t SPI_enuRegisterCallback(SPI_Number_t spiNumber, SPI_Flag_t flag, SPI_Callback_t callback);
uint8_t SPI_u8ReadFlag(SPI_Number_t spiNumber,SPI_Flag_t flag);

#endif // SPI_H_