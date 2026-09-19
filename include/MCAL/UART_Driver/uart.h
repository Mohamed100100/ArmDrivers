/******************************************************************************
 * @file    uart.h
 * @author  Eng.Gemy
 * @brief   UART/USART Driver Interface Header File (STM32F401 USART1/2/6)
 *          Supports sync polling, interrupt-async (TXE/TC/RXNE), and DMA.
 *          Baud via BRR from PeripheralClock/OverSampling.
 * @date    2024
 * @version 1.0
 * @note    Provide PeripheralClock = actual APB1/APB2 Hz after RCC prescalers.
 *          TXE interrupt left disabled at init to avoid spurious IRQ (see note
 *          below) — enabled on demand by AsynTransmit.
 ******************************************************************************/
#ifndef UART_H
#define UART_H

#include <LIB/stdtypes.h>


/******************************************************************************
 * @brief UART Enable Masks (CR1_TE/RE)
 * @details Bit 3=TE, bit 2=RE. OR for both.
 ******************************************************************************/
//                              0b1098765432109876543210
#define UART_ENABLE_TRANSMITE       (0b0000000000000000001000)  /**< TE bit — enable transmitter */
#define UART_ENABLE_RECEIVE         (0b0000000000000000000100)  /**< RE bit — enable receiver */
#define UART_DISABLE                (0b0000000000000000000000)  /**< Disable both */

/******************************************************************************
 * @brief UART Interrupt Enable Masks (CR1 bits)
 * @note  INTERRUPT_TXE is 0 at init — HSERIAL/UART enables it only when
 *        starting AsynTransmit to avoid immediate TXE IRQ storm. Other bits
 *        map to CR1: bit5 RXNEIE, bit6 TCIE, bit8 PEIE, bit0? EIE.
 ******************************************************************************/
// i found that enable txe interrupt in init will cause problem in async transmit
// so i will put it 0 and inside the async transmit function i will enable it using local define
//                                   0b1098765432109876543210
#define UART_INTERRUPT_TXE          (0b0000000000000000000000)  /**< TXEIE — kept 0 at init */
#define UART_INTERRUPT_ERROR        (0b0000000000000000000001)  /**< EIE — error interrupt */
#define UART_INTERRUPT_RXNE         (0b0000000000000000100000)  /**< RXNEIE bit5 */
#define UART_INTERRUPT_TC           (0b0000000000000001000000)  /**< TCIE bit6 */
#define UART_INTERRUPT_PE           (0b0000000000000100000000)  /**< PEIE bit8 */

/******************************************************************************
 * @brief UART Status Flags (SR bits)
 * @details Used with u8Read*Flag accessors and ClearFlags.
 ******************************************************************************/
//                                   0b1098765432109876543210
#define UART_FLAG_TXE               (0b0000000000000010000000)  /**< TXE bit7 — transmit data register empty */
#define UART_FLAG_TC                (0b0000000000000001000000)  /**< TC bit6 — transmission complete */
#define UART_FLAG_RXNE              (0b0000000000000000100000)  /**< RXNE bit5 — receive register not empty */
#define UART_FLAG_ORE               (0b0000000000000000001000)  /**< ORE bit3 — overrun error */
#define UART_FLAG_NOISE             (0b0000000000000000000100)  /**< NE bit2 — noise error */
#define UART_FLAG_FE                (0b0000000000000000000010)  /**< FE bit1 — framing error */
#define UART_FLAG_PE                (0b0000000000000000000001)  /**< PE bit0 — parity error */

/******************************************************************************
 * @brief UART DMA Enable Masks (CR3 bits)
 ******************************************************************************/
//                                   0b1098765432109876543210
#define UART_DMA_TRANSMIT_ENABLE    (0b0000000000000010000000)  /**< CR3.DMAT bit7 */
#define UART_DMA_RECEIVE_ENABLE     (0b0000000000000001000000)  /**< CR3.DMAR bit6 */

/** @brief UART TXE/RX callback type (void(void)) for async completion */
typedef void (*UART_Callback_t)(void);

/**
 * @brief Async buffer descriptor — used for interrupt-driven TX/RX
 * @details Holds user buffer pointer, total size, current index, and completion
 *          callback invoked with TC/RX complete.
 */
typedef struct {
    UART_Callback_t callback; /**< Completion callback (or NULL) */
    uint16_t size;            /**< Total bytes to transfer */
    uint16_t index;           /**< Next byte index (managed by ISR) */
    uint8_t *buffer;          /**< User buffer pointer */
}UART_AsynBuffer_t;

/**
 * @brief UART error and TC callback bundle
 * @details Registered via UART_enuRegisterCallbacks for ISR dispatch.
 */
typedef struct {
    UART_Callback_t ParityErrorCallback;  /**< PE error ISR */
    UART_Callback_t FramingErrorCallback; /**< FE error ISR */
    UART_Callback_t NoiseErrorCallback;   /**< NE error ISR */
    UART_Callback_t OverrunErrorCallback; /**< ORE error ISR */
    UART_Callback_t TC_Callback;          /**< TC complete callback */
} UART_Callbacks_t;

/******************************************************************************
 * @brief UART Status Enumeration
 * @details Detailed error codes for init, validation, and busy states.
 ******************************************************************************/
typedef enum {
    UART_NOT_OK,              /**< General failure */
    UART_OK,                  /**< Success */
    UART_NULL_PTR,            /**< NULL config/buffer pointer */
    UART_WRONG_UART_NUMBER,   /**< UART_Number > UART_6 */
    UART_WRONG_UART_ENABLE,   /**< UartEnabled mask has invalid bits */
    UART_WRONG_PARITY,        /**< Parity mask invalid */
    UART_WRONG_OVERSAMPLING,  /**< OverSampling mask invalid */
    UART_WRONG_STOPBITS,      /**< StopBits mask invalid */
    UART_WRONG_WORDLENGTH,    /**< WordLength mask invalid */
    UART_WRONG_SAMPLE,        /**< Sample mask invalid */
    UART_WRONG_INTERRUPT_FLAGS, /**< InterruptFlags has invalid bits */
    UART_NOT_INIT_SUCCESSFULLY, /**< Not yet initialized via UART_enuInit */
    UART_GPIO_ERROR,          /**< GPIO AF init failed for TX/RX pins */
    UART_TX_BUSY,             /**< Async TX already in progress (UART_BUSY) */
    UART_WRONG_DMA_ENABLE,    /**< DMA flag mask invalid */
} UART_Status_t;

/** @brief UART instance selection — maps to base 0x40011000/0x40004400/0x40011400 */
typedef enum {
    UART_1,  /**< USART1 (APB2) */
    UART_2,  /**< USART2 (APB1) */
    UART_6   /**< USART6 (APB2) */
}UART_Number_t;

/******************************************************************************
 * @brief UART Parity Enumeration (CR1 bits PCE/PS)
 ******************************************************************************/
typedef enum {        //0b1098765432109876543210
    UART_PARITY_NONE  = 0b0000000000000000000000, /**< No parity (PCE=0) */
    UART_PARITY_EVEN  = 0b0000000000010000000000, /**< Even parity (PCE=1,PS=0) */
    UART_PARITY_ODD   = 0b0000000000011000000000  /**< Odd parity (PCE=1,PS=1) */
} UART_Parity_t;       

/******************************************************************************
 * @brief UART Oversampling Enumeration (CR1_OVER8 bit11)
 ******************************************************************************/
                             //        * 
typedef enum {              //0b1098765432109876543210
    UART_OVERSAMPLING_16    = 0b0000000000000000000000, /**< OVER8=0 → 16× (default) */
    UART_OVERSAMPLING_8     = 0b0000001000000000000000  /**< OVER8=1 → 8× (higher baud) */
} UART_OverSampling_t;

                      //          **
/******************************************************************************
 * @brief UART Stop Bits Enumeration (CR2 STOP[13:12])
 ******************************************************************************/
typedef enum {        //0b1098765432109876543210
    UART_STOPBITS_1   = 0b0000000000000000000000, /**< 1 stop bit */
    UART_STOPBITS_0_5 = 0b0000000001000000000000, /**< 0.5 stop bit */
    UART_STOPBITS_2   = 0b0000000010000000000000, /**< 2 stop bits */
    UART_STOPBITS_1_5 = 0b0000000011000000000000  /**< 1.5 stop bits */
}UART_StopBit_t;
#define UART_STOPBITS_MASK   (0b1111111100111111111111) /**< Mask to validate StopBits */

/******************************************************************************
 * @brief UART Word Length Enumeration (CR1_M bit12)
 ******************************************************************************/
                        //           *
typedef enum {          //0b1098765432109876543210
    UART_WORDLENGTH_8B =  0b0000000000000000000000, /**< 8 data bits (M=0) */
    UART_WORDLENGTH_9B =  0b0000000001000000000000  /**< 9 data bits (M=1) */
}UART_WordLength_t;
                      //            *
/******************************************************************************
 * @brief UART Sample Mode Enumeration (CR3_ONEBIT bit11)
 ******************************************************************************/
typedef enum {        //0b1098765432109876543210
    UART_THREE_SAMPLE = 0b0000000000000000000000, /**< 3-sample majority (default) */
    UART_ONE_SAMPLE   = 0b0000000000100000000000  /**< Single sample (noise tolerant) */
}UART_Sample_t;

/** @brief UART init state tracker — internal */
typedef enum {
    UART_INIT,     /**< Successfully initialized */
    UART_NOT_INIT  /**< Not yet init (sync/async will return NOT_INIT) */
}UART_InitState_t;

/******************************************************************************
 * @brief UART Configuration Structure
 * @details Passed to UART_enuInit. Baud computed as BRR = fCK/(8/16*baud)
 *          depending on OverSampling. InterruptFlags OR of UART_INTERRUPT_*.
 ******************************************************************************/
typedef struct {
    uint32_t PeripheralClock;          /**< APB clock Hz (e.g., 42000000 for APB1) */
    UART_Number_t UART_Number;            /**< Instance (UART_1/2/6) */
    // baude rate in bps
    uint32_t BaudRate;                  /**< Baud rate bps (e.g., 115200) */
    // you have to choose one of the following parity configurations
    // NONE, EVEN, ODD
    UART_Parity_t Parity;               /**< Parity selection */
    // you have to choose one of the following oversampling modes
    // OVERSAMPLING_16, OVERSAMPLING_8
    UART_OverSampling_t OverSampling;   /**< Oversampling 16× or 8× */
    // you have to choose one of the following stop bits configurations
    // STOPBITS_1, STOPBITS_0_5, STOPBITS_2,
    UART_StopBit_t StopBits;            /**< Stop bits count */
    // you have to choose one of the following word length configurations
    // WORDLENGTH_8B, WORDLENGTH_9B
    UART_WordLength_t WordLength;       /**< Word length 8/9 bits */
    UART_Sample_t Sample;               /**< Sample method (3-sample / 1-sample) */
    // you have masks to enable or disable the UART peripheral
    uint32_t UartEnabled;                /**< Mask UART_ENABLE_TRANSMITE/RECEIVE */
    uint32_t InterruptFlags;             /**< OR of UART_INTERRUPT_* (TXE kept 0 at init) */
} UART_Config_t;

/******************************************************************************
 * @name UART Init & Transfer APIs
 * @{
 ******************************************************************************/
/**
 * @brief Initialize UART peripheral with given config
 * @details Validates Number/Enable/Parity/OverSampling/StopBits/WordLength/
 *          Sample/InterruptFlags masks, inits GPIO AF (PA9/10 for UART1 etc.),
 *          programs CR1/CR2/CR3, computes BRR, enables UE, sets state INIT.
 * @param[in] config Pointer to UART_Config_t (must not be NULL)
 * @return UART_Status_t UART_OK or WRONG_* / NULL_PTR / GPIO_ERROR
 */
UART_Status_t UART_enuInit(UART_Config_t* config);

/**
 * @brief Sync blocking transmit buffer (poll TXE/TC)
 * @details Polls TXE per byte, writes DR, polls TC. Blocks until all bytes sent.
 * @param[in] uartNumber Instance
 * @param[in] txBuffer Pointer to TX data
 * @param[in] size Number of bytes
 * @return UART_Status_t UART_OK, NULL_PTR, WRONG_UART_NUMBER, NOT_INIT
 */
UART_Status_t UART_enuSynTransmitBuffer(UART_Number_t uartNumber, const uint8_t* txBuffer, uint16_t size);
/**
 * @brief Sync blocking receive buffer (poll RXNE)
 * @param[in] uartNumber Instance
 * @param[out] rxBuffer Destination buffer
 * @param[in] size Number of bytes
 * @return UART_Status_t UART_OK or error
 */
UART_Status_t UART_enuSynReceiveBuffer(UART_Number_t uartNumber, uint8_t* rxBuffer, uint16_t size);
/**
 * @brief Async interrupt transmit — non-blocking
 * @details Stores AsynBuffer, sends first byte, enables TXEIE; ISR sends rest
 *          and invokes callback on TC. Returns TX_BUSY if already BUSY.
 * @param[in] uartNumber Instance
 * @param[in] txBuffer Pointer to UART_AsynBuffer_t (buffer/size/callback/index)
 * @return UART_Status_t UART_OK, TX_BUSY, etc.
 */
UART_Status_t UART_enuAsynTransmitBuffer(UART_Number_t uartNumber, UART_AsynBuffer_t* txBuffer);
/**
 * @brief Async interrupt receive — non-blocking
 * @details Enables RXNEIE; ISR fills buffer and callback when size reached.
 * @param[in] uartNumber Instance
 * @param[in] rxBuffer Descriptor with buffer/size
 * @return UART_Status_t UART_OK
 */
UART_Status_t UART_enuAsynReceiveBuffer(UART_Number_t uartNumber, UART_AsynBuffer_t* rxBuffer);

/**
 * @brief Enable DMA for UART TX and/or RX
 * @details ORs flag into CR3.DMAT/DMAR. Valid flags UART_DMA_*_ENABLE.
 * @param[in] uartNumber Instance
 * @param[in] enableDmaFlag Mask UART_DMA_TRANSMIT_ENABLE / RECEIVE_ENABLE
 * @return UART_Status_t UART_OK or WRONG_DMA_ENABLE
 */
UART_Status_t UART_enuActivateDMA(UART_Number_t uartNumber, uint32_t enableDmaFlag);

/** @brief Read TXE flag (SR bit7) */
uint8_t UART_u8ReadTXEFlag(UART_Number_t uartNumber);
/** @brief Read TC flag (SR bit6) */
uint8_t UART_u8ReadTCFlag(UART_Number_t uartNumber);
/** @brief Read RXNE flag (SR bit5) */
uint8_t UART_u8ReadRXNEFlag(UART_Number_t uartNumber);
/** @brief Read ORE flag (SR bit3) */
uint8_t UART_u8ReadOREFlag(UART_Number_t uartNumber);
/** @brief Read NE (noise) flag (SR bit2) */
uint8_t UART_u8ReadNoiseFlag(UART_Number_t uartNumber);
/** @brief Read FE flag (SR bit1) */
uint8_t UART_u8ReadFEFlag(UART_Number_t uartNumber);
/** @brief Read PE flag (SR bit0) */
uint8_t UART_u8ReadPEFlag(UART_Number_t uartNumber);

/**
 * @brief Enable specific UART interrupts (CR1 bits)
 * @param[in] uartNumber Instance
 * @param[in] interruptFlags OR of UART_INTERRUPT_* masks
 * @return UART_Status_t UART_OK or WRONG_INTERRUPT_FLAGS
 */
UART_Status_t UART_enuEnableInterrupts(UART_Number_t uartNumber, uint32_t interruptFlags);
/** @brief Disable specific UART interrupts */
UART_Status_t UART_enuDisableInterrupts(UART_Number_t uartNumber, uint32_t interruptFlags);
/**
 * @brief Clear status flags (SR) by writing 0
 * @param[in] uartNumber Instance
 * @param[in] interruptFlags Mask of UART_FLAG_* to clear
 * @return UART_Status_t UART_OK
 */
UART_Status_t UART_enuClearFlags(UART_Number_t uartNumber,uint32_t interruptFlags);

/**
 * @brief Register error and TC callbacks for ISR dispatch
 * @param[in] uartNumber Instance
 * @param[in] callbacks Pointer to UART_Callbacks_t bundle
 * @return UART_Status_t UART_OK or NULL_PTR/WRONG_UART_NUMBER
 */
UART_Status_t UART_enuRegisterCallbacks(UART_Number_t uartNumber, UART_Callbacks_t* callbacks);

/** @} */


#endif // UART_H
