/******************************************************************************
 * @file    dma.h
 * @author  Eng.Gemy
 * @brief   DMA Driver Interface Header File (STM32F401 DMA1/DMA2)
 *          Abstraction for 2 controllers × 8 streams × 8 channels, with
 *          FIFO/Direct, burst, double-buffer, priority, size, increment,
 *          circular, flow-control, and 5 interrupt sources.
 * @date    2024
 * @version 1.0
 * @note    Provide PeripheralAddress = peripheral DR address, Memory0/1 for
 *          mem, NumberOfData = NDTR (must be >0). Check flags via ReadFlag.
 ******************************************************************************/

#ifndef DMA_H
#define DMA_H

#include "LIB/stdtypes.h"

/******************************************************************************
 * @brief DMA Interrupt Enable Masks (to OR into Config.Interrupts)
 * @details Bits map to CR (TCIE=bit4 0x10, HTIE=bit3 0x08, TEIE=bit2 0x04,
 *          DMEIE=bit1 0x02) and FCR.FEIE (bit7 0x80). Driver splits into
 *          SCR_REG (low nibble + 0x10) and SFCR_REG (0x80) internally.
 ******************************************************************************/
//                                                0b10987654321098765432109876543210
#define DMA_INTERRUPT_TRANSFER_COMPLETE_ENABLE    0b00000000000000000000000000010000 /**< CR.TCIE bit4 */
#define DMA_INTERRUPT_HALF_TRANSFER_ENABLE        0b00000000000000000000000000001000 /**< CR.HTIE bit3 */
#define DMA_INTERRUPT_TRANSFER_ERROR_ENABLE       0b00000000000000000000000000000100 /**< CR.TEIE bit2 */
#define DMA_INTERRUPT_DIRECT_MODE_ERROR_ENABLE    0b00000000000000000000000000000010 /**< CR.DMEIE bit1 */
#define DMA_INTERRUPT_FIFO_ERROR_ENABLE           0b00000000000000000000000010000000 /**< FCR.FEIE bit7 */

/** @brief DMA callback type for ISR (void(void)) per interrupt */
typedef void (*DMA_CallBack_t)(void);

/******************************************************************************
 * @brief DMA Interrupt Type Enumeration (indices into callbacks[5])
 ******************************************************************************/
typedef enum {
    DMA_INTERRUPT_FIFO_ERROR            , /**< Index 0 — FCR.FEIE */
    DMA_INTERRUPT_DIRECT_MODE_ERROR     , /**< Index 1 — CR.DMEIE */
    DMA_INTERRUPT_TRANSFER_ERROR        , /**< Index 2 — CR.TEIE */
    DMA_INTERRUPT_HALF_TRANSFER         , /**< Index 3 — CR.HTIE */
    DMA_INTERRUPT_TRANSMISSION_COMPLETE , /**< Index 4 — CR.TCIE (TC) */
}DMA_Interrupts_t;



/** @brief DMA controller selection */
typedef enum {
    DMA1 = 0, /**< DMA1 base 0x40026000 */
    DMA2      /**< DMA2 base 0x40026400 */
}DMA_Controller_t;


/** @brief DMA stream index (0..7 per controller) */
typedef enum {
    DMA_STREAM0 = 0, DMA_STREAM1, DMA_STREAM2, DMA_STREAM3,
    DMA_STREAM4, DMA_STREAM5, DMA_STREAM6, DMA_STREAM7
}DMA_Stream_t;

/******************************************************************************
 * @brief DMA Channel Enumeration (CR.CHSEL bits 27:25 = channel<<25)
 ******************************************************************************/
typedef enum {
//                       ***       
//                 0b10987654321098765432109876543210
    DMA_CHANNEL0 = 0b00000000000000000000000000000000, /**< CH0 — 0x00000000 */
    DMA_CHANNEL1 = 0b00000010000000000000000000000000, /**< CH1 — 0x02000000 */
    DMA_CHANNEL2 = 0b00000100000000000000000000000000, /**< CH2 */
    DMA_CHANNEL3 = 0b00000110000000000000000000000000, /**< CH3 */
    DMA_CHANNEL4 = 0b00001000000000000000000000000000, /**< CH4 (e.g., SPI1_TX) */
    DMA_CHANNEL5 = 0b00001010000000000000000000000000, /**< CH5 */
    DMA_CHANNEL6 = 0b00001100000000000000000000000000, /**< CH6 */
    DMA_CHANNEL7 = 0b00001110000000000000000000000000  /**< CH7 */
}DMA_Channel_t;


/******************************************************************************
 * @brief DMA Memory Burst Enumeration (CR.MBURST bits 24:23)
 ******************************************************************************/
typedef enum {
//                              **
//                      0b10987654321098765432109876543210
    DMA_MBurst_SINGLE = 0b00000000000000000000000000000000, /**< Single transfer */
    DMA_MBurst_INCR4  = 0b00000001000000000000000000000000, /**< INCR4 burst */
    DMA_MBurst_INCR8  = 0b00000010000000000000000000000000, /**< INCR8 */
    DMA_MBurst_INCR16 = 0b00000011000000000000000000000000  /**< INCR16 */
}DMA_MBurst_t;

/******************************************************************************
 * @brief DMA Peripheral Burst Enumeration (CR.PBURST bits 22:21)
 ******************************************************************************/
typedef enum {
//                                 **
//                      0b10987654321098765432109876543210
    DMA_PBurst_SINGLE = 0b00000000000000000000000000000000, /**< Single */
    DMA_PBurst_INCR4  = 0b00000000001000000000000000000000, /**< INCR4 */
    DMA_PBurst_INCR8  = 0b00000000010000000000000000000000, /**< INCR8 */
    DMA_PBurst_INCR16 = 0b00000000011000000000000000000000  /**< INCR16 */
}DMA_PBurst_t;


/******************************************************************************
 * @brief DMA Double Buffer Enumeration (CR.DBM bit18 + CT bit)
 ******************************************************************************/
typedef enum {
//                                             *
//                              0b10987654321098765432109876543210
    DMA_DISABLE_DOUBLE_BUFFER = 0b00000000000000000000000000000000, /**< DBM=0 — M1AR unused */
    DMA_ENABLE_DOUBLE_BUFFER  = 0b00000000000001000000000000000100, /**< DBM=1 + reserved bit2 — uses M1AR, auto CT toggle */
}DMA_DoubleBuffer_t;


/******************************************************************************
 * @brief DMA Priority Enumeration (CR.PL bits 17:16)
 ******************************************************************************/
typedef enum {
//                                           **
//                           0b10987654321098765432109876543210
    DMA_PRIORITY_LOW       = 0b00000000000000000000000000000000, /**< Low */
    DMA_PRIORITY_MEDIUM    = 0b00000000000000010000000000000000, /**< Medium */
    DMA_PRIORITY_HIGH      = 0b00000000000000100000000000000000, /**< High */
    DMA_PRIORITY_VERY_HIGH = 0b00000000000000110000000000000000  /**< Very high */
}DMA_Priority_t;


/******************************************************************************
 * @brief DMA Memory Data Size Enumeration (CR.MSIZE bits 14:13)
 ******************************************************************************/
typedef enum {
//                                           **
//                        0b10987654321098765432109876543210
    DMA_MSIZE_BYTE      = 0b00000000000000000000000000000000, /**< Byte (8-bit) */
    DMA_MSIZE_HALFWORD  = 0b00000000000000000010000000000000, /**< Half-word (16-bit) */
    DMA_MSIZE_WORD      = 0b00000000000000000100000000000000  /**< Word (32-bit) */
}DMA_MSize_t;


/******************************************************************************
 * @brief DMA Peripheral Data Size Enumeration (CR.PSIZE bits 12:11)
 ******************************************************************************/
typedef enum {
//                                             **
//                        0b10987654321098765432109876543210
    DMA_PSIZE_BYTE      = 0b00000000000000000000000000000000, /**< Byte */
    DMA_PSIZE_HALFWORD  = 0b00000000000000000000100000000000, /**< Half-word */
    DMA_PSIZE_WORD      = 0b00000000000000000001000000000000  /**< Word */
}DMA_PSize_t;

/******************************************************************************
 * @brief DMA Memory Increment Enumeration (CR.MINC bit10)
 ******************************************************************************/
typedef enum {
//                                                    *
//                             0b10987654321098765432109876543210
    DMA_MINC_FIXED           = 0b00000000000000000000000000000000, /**< Address fixed */
    DMA_MINC_AUTO_INCREMENT  = 0b00000000000000000000010000000000, /**< Auto-increment after each beat */
}DMA_MemoryInc_t;

/******************************************************************************
 * @brief DMA Peripheral Increment Enumeration (CR.PINC bit9)
 ******************************************************************************/
typedef enum {
//                                                     *
//                             0b10987654321098765432109876543210
    DMA_PINC_FIXED           = 0b00000000000000000000000000000000, /**< Fixed */
    DMA_PINC_AUTO_INCREMENT  = 0b00000000000000000000001000000000, /**< Auto-increment */
}DMA_PerihperalInc_t;


/******************************************************************************
 * @brief DMA Circular Mode Enumeration (CR.CIRC bit8)
 ******************************************************************************/
typedef enum {
//                                                       *
//                              0b10987654321098765432109876543210
    DMA_CIRCULAR_MODE_DISABLE = 0b00000000000000000000000000000000, /**< Normal (stop at NDTR=0) */
    DMA_CIRCULAR_MODE_ENABLE  = 0b00000000000000000000000100000000, /**< Circular (auto-reload NDTR) */
}DMA_CircularMode_t;

/******************************************************************************
 * @brief DMA Data Transfer Direction Enumeration (CR.DIR bits7:6)
 ******************************************************************************/
typedef enum {
//                                                **
//                      0b10987654321098765432109876543210
    DMA_DIRECTION_P2M = 0b00000000000000000000000000000000, /**< Peripheral → Memory */
    DMA_DIRECTION_M2P = 0b00000000000000000000000001000000, /**< Memory → Peripheral */
    DMA_DIRECTION_M2M = 0b00000000000000000000000010000000, /**< Memory → Memory (software) */
}DMA_Direction_t;


/******************************************************************************
 * @brief DMA Flow Controller Enumeration (CR.PFCTRL bit5)
 ******************************************************************************/
typedef enum {
//                                                                  *
//                                      0b10987654321098765432109876543210
    DMA_FLOW_CONTROL_USING_DMA        = 0b00000000000000000000000000000000, /**< DMA controls NDTR (default) */
    DMA_FLOW_CONTROL_USING_PERIPHERAL = 0b00000000000000000000000000100000, /**< Peripheral controls flow (PFCTRL=1) */
}DMA_PeripheralFlowCtrl_t;


/******************************************************************************
 * @brief DMA Mode Enumeration (FCR.DMDIS bit2)
 * @details DIRECT (DMDIS=0) bypasses FIFO; FIFO (DMDIS=1) uses threshold.
 ******************************************************************************/
typedef enum {
// 
//                    0b10987654321098765432109876543210
    DMA_MODE_DIRECT = 0b00000000000000000000000000000000, /**< Direct mode (threshold N/A) */
    DMA_MODE_FIFO   = 0b00000000000000000000000000000100, /**< FIFO mode (needs FTH) */
}DMA_Mode_t;


/******************************************************************************
 * @brief DMA FIFO Threshold Enumeration (FCR.FTH bits1:0) — valid only in FIFO mode
 ******************************************************************************/
typedef enum {
//                                                                     **
//                                     0b10987654321098765432109876543210  
    DMA_FIFO_THRESHOLD_QUARTER       = 0b00000000000000000000000000000000, /**< 1/4 full */
    DMA_FIFO_THRESHOLD_HALF          = 0b00000000000000000000000000000001, /**< 1/2 */
    DMA_FIFO_THRESHOLD_THREEQUARTERS = 0b00000000000000000000000000000010, /**< 3/4 */
    DMA_FIFO_THRESHOLD_FULL          = 0b00000000000000000000000000000011, /**< Full */
}DMA_FifoThreshold_t;



/******************************************************************************
 * @brief DMA Configuration Structure
 * @details Passed to DMA_enuInit. NumberOfData is NDTR (16-bit, must be >0).
 *          PeripheralAddress typically &PERIPH->DR, Memory0/1 are RAM/Flash.
 ******************************************************************************/
typedef struct {
    DMA_Controller_t          DMAx;                 /**< DMA1/DMA2 */
    DMA_Stream_t              Streamx;              /**< Stream 0..7 */
    DMA_Channel_t             Channel;              /**< Channel 0..7 (CHSEL) */
    DMA_Direction_t           Direction;            /**< P2M/M2P/M2M */
    DMA_PeripheralFlowCtrl_t  PeripheralFlowCtrl;   /**< DMA vs peripheral flow */
    DMA_Mode_t                Mode;                 /**< DIRECT/FIFO */
    DMA_Priority_t            Priority;             /**< LOW..VERY_HIGH */
    DMA_MSize_t               MSize;                /**< BYTE/HALFWORD/WORD */
    DMA_PSize_t               PSize;                /**< BYTE/HALFWORD/WORD */
    DMA_MemoryInc_t           MemoryInc;            /**< Fixed vs auto */
    DMA_PerihperalInc_t       PeripheralInc;        /**< Fixed vs auto */
    DMA_CircularMode_t        CircularMode;         /**< Normal vs circular */
    DMA_MBurst_t              MBurst;               /**< MBURST */
    DMA_PBurst_t              PBurst;               /**< PBURST */
    DMA_DoubleBuffer_t        DoubleBuffer;         /**< DBM + M1AR */
    DMA_FifoThreshold_t       FifoThreshold;        /**< FTH (FIFO only) */
    uint32_t                  PeripheralAddress;    /**< PAR (peripheral data register) */
    uint32_t                  Memory0Address;       /**< M0AR (memory 0) */
    uint32_t                  Memory1Address;       /**< M1AR (memory 1, double-buffer) */
    uint32_t                  Interrupts;           /**< OR of DMA_INTERRUPT_*_ENABLE */
    uint16_t                  NumberOfData;         /**< NDTR — number of beats */
}DMA_Config_t;



/******************************************************************************
 * @brief DMA Status Enumeration
 ******************************************************************************/
typedef enum {
    DMA_NOT_OK,                   /**< General failure */
    DMA_OK,                       /**< Success */
    DMA_NULL_PTR,                 /**< ConfigPtr == NULL */
    DMA_WRONG_DMA_CONTROLLER,     /**< DMAx > DMA2 */
    DMA_WRONG_STREAM,             /**< Stream > STREAM7 */
    DMA_WRONG_CHANNEL,            /**< Channel mask has invalid bits */
    DMA_WRONG_MBURST,             /**< MBurst invalid */
    DMA_WRONG_PBURST,             /**< PBurst invalid */
    DMA_WRONG_DOUBLE_BUFFER,      /**< DBM invalid */
    DMA_WRONG_PRIORITY,           /**< Priority invalid */
    DMA_WRONG_MSIZE,              /**< MSize invalid */
    DMA_WRONG_PSIZE,              /**< PSize invalid */
    DMA_WRONG_MINC,               /**< MINC invalid */
    DMA_WRONG_PINC,               /**< PINC invalid */
    DMA_WRONG_CIRCULAR_MODE,      /**< CIRC invalid */
    DMA_WRONG_DIRECTION,          /**< DIR invalid */
    DMA_WRONG_FLOW_CONTROL,       /**< PFCTRL invalid */
    DMA_WRONG_MODE,               /**< DMDIS invalid */
    DMA_WRONG_FIFO_THRESHOLD,     /**< FTH invalid */
    DMA_WRONG_INTERRUPTS,         /**< Interrupt mask has invalid bits */
    DMA_WRONG_ZERO_NUMBER_OF_DATA /**< NDTR == 0 */
}DMA_Status_t;


/******************************************************************************
 * @name DMA Public APIs
 * @{
 ******************************************************************************/
/**
 * @brief Initialize DMA stream per config (SCR, NDTR, PAR, M0AR/M1AR, FCR)
 * @details Validates all masks and NDTR>0, disables stream (EN=0), clears
 *          flags, ORs Channel/MBurst/PBurst/DBM/Priority/MSize/PSize/MINC/PINC/
 *          CIRC/DIR/PFCTRL/Interrupts into SCR, sets NDTR/PAR/M0AR/M1AR,
 *          sets DMDIS/FTH/FEIE into FCR.
 * @param[in] ConfigPtr Pointer to DMA_Config_t (must not be NULL)
 * @return DMA_Status_t DMA_OK or WRONG_* / NULL_PTR / ZERO_NUMBER_OF_DATA
 */
DMA_Status_t DMA_enuInit(const DMA_Config_t* ConfigPtr);
/**
 * @brief Start transfer — set stream SCR.EN=1
 * @details For M2M starts immediately; for peripheral waits for request.
 * @param[in] DMAx Controller
 * @param[in] Streamx Stream index
 * @return DMA_Status_t DMA_OK or WRONG_CONTROLLER/STREAM
 */
DMA_Status_t DMA_enuStartTransfer(DMA_Controller_t DMAx, DMA_Stream_t Streamx);
/**
 * @brief Stop transfer — clear SCR.EN
 * @param[in] DMAx Controller
 * @param[in] Streamx Stream
 * @return DMA_Status_t DMA_OK or error
 */
DMA_Status_t DMA_enuStopTransfer(DMA_Controller_t DMAx, DMA_Stream_t Streamx);
/**
 * @brief Update M0AR address (e.g., next buffer) — stream must be disabled
 * @param[in] DMAx Controller
 * @param[in] Streamx Stream
 * @param[in] MemoryAddress New M0AR value
 * @return DMA_Status_t DMA_OK or error
 */
DMA_Status_t DMA_enuSetMemoryAddress(DMA_Controller_t DMAx, DMA_Stream_t Streamx, uint32_t MemoryAddress);
/**
 * @brief Update NDTR count — stream must be disabled, count>0
 * @param[in] DMAx Controller
 * @param[in] Streamx Stream
 * @param[in] NumberOfData New NDTR (1..65535)
 * @return DMA_Status_t DMA_OK or WRONG_ZERO_NUMBER_OF_DATA
 */
DMA_Status_t DMA_enuSetNumberOfData(DMA_Controller_t DMAx, DMA_Stream_t Streamx, uint16_t NumberOfData);
/**
 * @brief Register callback for a specific interrupt on a stream
 * @details Stores in dmaCallbacks[DMAx][Streamx][Interrupt]. ISR will invoke.
 * @param[in] DMAx Controller
 * @param[in] Streamx Stream
 * @param[in] Interrupt Type (FIFO_ERROR..TRANSMISSION_COMPLETE)
 * @param[in] callback Function pointer (executed from ISR context)
 * @return DMA_Status_t DMA_OK or error
 */
DMA_Status_t DMA_enuRegisterCallback(DMA_Controller_t DMAx, DMA_Stream_t Streamx, DMA_Interrupts_t Interrupt, DMA_CallBack_t callback);
/**
 * @brief Read flag (LISR/HISR) for given stream+interrupt → 0/1 or 0xFF error
 * @param[in] DMAx Controller
 * @param[in] Streamx Stream
 * @param[in] Interrupt Type
 * @return uint8_t 0/1 if valid, 0xFF if wrong controller/stream/interrupt
 */
uint8_t DMA_u8ReadFlag(DMA_Controller_t DMAx, DMA_Stream_t Streamx, DMA_Interrupts_t Interrupt);
/**
 * @brief Clear flag — write 1 to LIFCR/HIFCR bit
 * @param[in] DMAx Controller
 * @param[in] Streamx Stream
 * @param[in] Interrupt Type to clear
 * @return DMA_Status_t DMA_OK or error
 */
DMA_Status_t DMA_enuClearFlag(DMA_Controller_t DMAx, DMA_Stream_t Streamx, DMA_Interrupts_t Interrupt);

/** @} */





#endif // DMA_H
