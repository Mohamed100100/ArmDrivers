/******************************************************************************
 * @file    systick.h
 * @author  Eng.Gemy
 * @brief   SysTick Timer Driver Interface Header File
 *          Wraps Cortex-M4 SysTick (24-bit down-counter at 0xE000E010) for
 *          scheduler tick and blocking delays. Supports ÷1 and ÷8 clock.
 * @date    2024
 * @version 1.0
 * @note    LOAD is 24-bit (0x000000..0xFFFFFF). ClockValue saved at Init is
 *          used by Wait_ms for tick math.
 ******************************************************************************/
#ifndef SYSTICK_H
#define SYSTICK_H

#include "LIB/stdtypes.h"

/******************************************************************************
 * @brief SysTick callback type
 * @details Invoked from SysTick_Handler ISR when VAL→0. Must be short,
 *          non-blocking, and ISR-safe. Registered via SYSTICK_SetCallBack.
 ******************************************************************************/
typedef void (*SYSTICK_Callback_t)(void);

/******************************************************************************
 * @brief SysTick Status Enumeration
 * @details Return codes for all SysTick APIs. Distinguishes OFF states and
 *          parameter errors.
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
    SYSTICK_NOT_OK,              /**< General error / operation failed */
    SYSTICK_OK,                  /**< Operation completed successfully */
    SYSTICK_WRONG_PRESCALLER,    /**< Invalid prescaler (not NO_PRESCALLER/8) */
    SYSTICK_WRONG_STARTVALUE,    /**< Start value exceeds 24-bit (upper 8 bits set) */
    SYSTICK_OFF,                 /**< SysTick ENABLE bit is clear (timer off) */
    SYSTICK_EXCEPTION_OFF,       /**< TICKINT bit clear (interrupt disabled) */
    SYSTICK_ZERO_STARTVALUE,     /**< LOAD is zero — no period configured */
    SYSTICK_NULL_PTR,            /**< NULL pointer passed as argument */
}SYSTICK_Status_t;


/******************************************************************************
 * @brief SysTick Prescaler / Clock Source Enumeration
 * @details Bit 2 (CLKSOURCE) of STK_CTRL:
 *          0b100 → processor clock (AHB), 0b000 → processor/8.
 *          Values are encoded to OR directly into STK_CTRL.
 ******************************************************************************/
typedef enum {
    SYSTICK_NO_PRESCALLER = 0b100,  /**< No prescaler — processor clock (bit2=1) */
    SYSTICK_PRESCALLER_8  = 0b000,  /**< ÷8 prescaler — processor/8 (bit2=0) */
}SYSTICK_Prescaller_t;


/******************************************************************************
 * @brief Initialize SysTick with clock and prescaler
 * @details Validates prescaler, ORs it into STK_CTRL.CLKSOURCE, sets
 *          TICKINT, saves ClockValue for Wait_ms. Does not start counting
 *          (ENABLE stays clear until StartCount).
 * @param[in] ClockValue System clock in Hz (e.g., 84000000) — used for delay calc
 * @param[in] Prescaler SYSTICK_NO_PRESCALLER or SYSTICK_PRESCALLER_8
 * @return SYSTICK_Status_t SYSTICK_OK or SYSTICK_WRONG_PRESCALLER
 * @author Eng.Gemy
 ******************************************************************************/
SYSTICK_Status_t SYSTICK_Init(uint32_t ClockValue,SYSTICK_Prescaller_t);

/******************************************************************************
 * @brief Register SysTick callback (ISR)
 * @details Stores pointer in static `callback`; invoked from SysTick_Handler.
 *          Overwrites previous callback (single slot). NULL is allowed to
 *          unregister.
 * @param[in] Callback Function pointer of type SYSTICK_Callback_t
 * @return SYSTICK_Status_t SYSTICK_OK (always) per current impl.
 * @note  Keep callback short; runs in interrupt context.
 * @author Eng.Gemy
 ******************************************************************************/
SYSTICK_Status_t SYSTICK_SetCallBack(SYSTICK_Callback_t);

/******************************************************************************
 * @brief Set SysTick reload value (period)
 * @details Writes STK_LOAD. Checks upper 8 bits (mask) → WRONG_STARTVALUE.
 *          Value is (ticks-1): e.g., 84M/1000-1 = 83999 for 1ms@84M.
 * @param[in] StartValue 24-bit reload (0x000000..0xFFFFFF); 0 → zero error later
 * @return SYSTICK_Status_t SYSTICK_OK or SYSTICK_WRONG_STARTVALUE
 * @author Eng.Gemy
 ******************************************************************************/
SYSTICK_Status_t SYSTICK_SetStartValue(uint32_t);

/******************************************************************************
 * @brief Start SysTick counting
 * @details Sets STK_CTRL.ENABLE (bit0) =1. TICKINT already set at Init.
 *          Counter loads VAL=LOAD on next tick.
 * @param None
 * @return None
 * @author Eng.Gemy
 ******************************************************************************/
void SYSTICK_StartCount();

/******************************************************************************
 * @brief Stop SysTick counting
 * @details Clears STK_CTRL.ENABLE (bit0)=0. VAL freezes.
 * @param None
 * @return None
 * @author Eng.Gemy
 ******************************************************************************/
void SYSTICK_StopCount();

/******************************************************************************
 * @brief Blocking millisecond delay using SysTick
 * @details Computes ticksPerMs = (clockSource/1000), requiredTicks =
 *          (delay_ms * ticksPerMs)/(LOAD+1). Busy-waits on systick_counter
 *          incremented by SysTick_Handler. Validates ENABLE, LOAD!=0, TICKINT.
 * @param[in] delay_ms Delay in milliseconds (must be >0)
 * @return SYSTICK_Status_t SYSTICK_OK,
 *         SYSTICK_OFF if not enabled,
 *         SYSTICK_ZERO_STARTVALUE if LOAD==0,
 *         SYSTICK_EXCEPTION_OFF if TICKINT clear.
 * @note  Blocking — do not call from ISR or when scheduler owns SysTick.
 * @warning Can exceed 24-bit for large delay_ms → wrap; caller must limit.
 * @author Eng.Gemy
 ******************************************************************************/
SYSTICK_Status_t SYSTICK_Wait_ms(uint32_t);

/******************************************************************************
 * @brief Get current SysTick counter value (VAL)
 * @details Reads STK_VAL (24-bit down-counter). Does not modify state.
 * @param[out] currentCount Pointer to uint32_t to receive VAL
 * @return SYSTICK_Status_t SYSTICK_OK or SYSTICK_NULL_PTR if pointer NULL
 * @author Eng.Gemy
 ******************************************************************************/
SYSTICK_Status_t SYSTICK_GetCurrentCount(uint32_t *);

#endif /* SYSTICK_H */
