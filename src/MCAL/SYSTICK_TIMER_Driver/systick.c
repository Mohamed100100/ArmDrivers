/******************************************************************************
 * @file    systick.c
 * @author  Eng.Gemy
 * @brief   SysTick Timer Driver Implementation File
 *          Implements 24-bit SysTick at 0xE000E010 (STK_CTRL/LOAD/VAL).
 *          Provides Init, callback, reload, start/stop, blocking Wait_ms,
 *          and ISR that drives systick_counter + user callback.
 * @date    2024
 * @version 1.0
 * @note    Registers defined in systick_priv.h (SYSTICK_Registers->STK_*).
 *          systick_counter is volatile flag for Wait_ms polling.
 ******************************************************************************/

#include "LIB/stdtypes.h"

#include "MCAL/SYSTICK_TIMER_Driver/systick_priv.h"
#include "MCAL/SYSTICK_TIMER_Driver/systick.h"

/** @brief Forward declaration of ISR (hardware vector) */
void SysTick_Handler(void);

/** @brief Counts SysTick interrupts since last Wait_ms reset */
static uint32_t systick_counter  = 0;

/** @brief User callback invoked from ISR (NULL if none) */
static SYSTICK_Callback_t callback = NULL;

/** @brief Saved system clock in Hz for delay math (from SYSTICK_Init) */
static uint32_t clockSourceValue=0;

/******************************************************************************
 * @brief Initialize SysTick with clock and prescaler
 * @details Validates prescaler (NO_PRESCALLER/8), ORs into STK_CTRL.CLKSOURCE,
 *          enables TICKINT, saves ClockValue. Leaves ENABLE clear.
 * @param[in] ClockValue System clock Hz
 * @param[in] prescaller SYSTICK_NO_PRESCALLER or SYSTICK_PRESCALLER_8
 * @return SYSTICK_Status_t SYSTICK_OK or SYSTICK_WRONG_PRESCALLER
 ******************************************************************************/
SYSTICK_Status_t SYSTICK_Init(uint32_t ClockValue,SYSTICK_Prescaller_t prescaller){
    SYSTICK_Status_t status = SYSTICK_NOT_OK;

    /* Validate prescaler value - must be either NO_PRESCALLER or PRESCALLER_8 */
    if((SYSTICK_NO_PRESCALLER!= prescaller)&&(SYSTICK_PRESCALLER_8!= prescaller)){
        status = SYSTICK_WRONG_PRESCALLER;
    }else{
        /* Set the clock source bit in the control register */
        SYSTICK_Registers->STK_CTRL |= prescaller;
        
        /* Enable SysTick exception (interrupt) by setting bit 1 */
        SYSTICK_Registers->STK_CTRL |= SYSTICK_ENABLE_EXCEPTION;
        
        /* Store the clock frequency for later calculations */
        clockSourceValue = ClockValue;
        
        status = SYSTICK_OK;
    }

    return status;
}

/******************************************************************************
 * @brief Register user callback for SysTick ISR
 * @details Stores copyCallback into static callback; SysTick_Handler will
 *          invoke it after incrementing systick_counter. Allows NULL to clear.
 * @param[in] copyCallback Function pointer (or NULL to unregister)
 * @return SYSTICK_Status_t SYSTICK_OK (always succeeds per current impl)
 * @note  Keep ISR short; do not block.
 ******************************************************************************/
SYSTICK_Status_t SYSTICK_SetCallBack(SYSTICK_Callback_t copyCallback){
    /* Store the callback function pointer for later invocation in ISR */
    callback = copyCallback;
    return SYSTICK_OK;
}

/******************************************************************************
 * @brief Set SysTick reload value (period)
 * @details Checks upper 8 bits via SYSTICK_STARTVALUE_MASK → WRONG_STARTVALUE
 *          else writes STK_LOAD.
 * @param[in] startValue 24-bit reload (0..0xFFFFFF)
 * @return SYSTICK_Status_t SYSTICK_OK or SYSTICK_WRONG_STARTVALUE
 ******************************************************************************/
SYSTICK_Status_t SYSTICK_SetStartValue(uint32_t startValue){
    SYSTICK_Status_t status = SYSTICK_NOT_OK;

    /* Check if any of the upper 8 bits are set (value exceeds 24-bit range) */
    if(0!=(startValue & SYSTICK_STARTVALUE_MASK)){
        status = SYSTICK_WRONG_STARTVALUE;
    }else{
        /* Load the reload value into the STK_LOAD register */
        SYSTICK_Registers->STK_LOAD = startValue;
        status = SYSTICK_OK;
    }
    return status;
}

/******************************************************************************
 * @brief Start SysTick counting
 * @details Sets STK_CTRL.ENABLE (bit0)=1.
 * @param None
 * @return None
 ******************************************************************************/
void SYSTICK_StartCount(){
    /* Set the ENABLE bit (bit 0) in the control register to start counting */
    SYSTICK_Registers->STK_CTRL |=SYSTICK_START_COUNTING;
}

/******************************************************************************
 * @brief Stop SysTick counting
 * @details Clears STK_CTRL.ENABLE (bit0)=0.
 * @param None
 * @return None
 ******************************************************************************/
void SYSTICK_StopCount(){
    /* Clear the ENABLE bit (bit 0) in the control register to stop counting */
    SYSTICK_Registers->STK_CTRL &=SYSTICK_STOP_COUNTING;
}

/******************************************************************************
 * @brief Blocking delay in milliseconds using SysTick interrupts
 * @details Validates ENABLE, LOAD!=0, TICKINT. Derives clockSource (÷8 if
 *          prescaler bit clear), computes reloadValue=LOAD+1, ticksPerMs=
 *          clockSource/1000, requiredTicks=(delay_ms*ticksPerMs)/reloadValue.
 *          Resets systick_counter and busy-waits until requiredTicks reached.
 * @param[in] delay_ms Milliseconds to block (1.. large; upper bound by 24-bit)
 * @return SYSTICK_Status_t SYSTICK_OK, SYSTICK_OFF, SYSTICK_ZERO_STARTVALUE,
 *         SYSTICK_EXCEPTION_OFF
 * @note  Blocking — stalls scheduler. Use scheduler delay instead when OS runs.
 ******************************************************************************/
SYSTICK_Status_t SYSTICK_Wait_ms(uint32_t delay_ms){
    
    SYSTICK_Status_t status = SYSTICK_NOT_OK;
    
    /* Check if SysTick timer is enabled (bit 0 of control register) */
    if(0 == (SYSTICK_Registers->STK_CTRL & SYSTICK_ENABLE_MASK_CHECK)){
        status = SYSTICK_OFF;
    }else{
        /* Check if a valid reload value has been set */
        if(0 == (SYSTICK_Registers->STK_LOAD)){
            status = SYSTICK_ZERO_STARTVALUE;
        }else{
            /* Check if SysTick exception/interrupt is enabled */
            if(0 == (SYSTICK_Registers->STK_CTRL &SYSTICK_EXCEPTION_MASK_CHECK)){
                status = SYSTICK_EXCEPTION_OFF;
            }else{
                /* Determine the actual clock source based on prescaler bit (bit 2) */
                uint32_t clockSource;
                if(SYSTICK_Registers->STK_CTRL & SYSTICK_PRESCALLER_MASK_CHECK){
                    /* Processor clock (no prescaler) - bit 2 is set */
                    clockSource = clockSourceValue;
                }else{
                    /* External clock (AHB/8 prescaler) - bit 2 is clear */
                    clockSource = clockSourceValue / 8;
                }
        
                /* Calculate the number of timer ticks needed for the delay */
                /* Add 1 because the timer counts from LOAD to 0 (inclusive) */
                uint32_t reloadValue = SYSTICK_Registers->STK_LOAD + 1;
                
                /* Calculate how many clock ticks occur per millisecond */
                uint32_t ticksPerMs = clockSource / 1000UL;
                
                /* Calculate how many timer overflows are needed for the requested delay */
                uint32_t requiredTicks = (delay_ms * ticksPerMs) / reloadValue;
                
                /* Handle edge case where delay is very small (less than one timer period) */
                if(requiredTicks == 0)
                {
                    requiredTicks = 1;
                }
        
                /* Reset the interrupt counter and wait for it to reach required value */
                systick_counter = 0;
                
                /* Busy-wait loop until enough SysTick interrupts have occurred */
                while(systick_counter < requiredTicks)
                {
                    /* Wait for counter to reach required value */
                }
                
                status = SYSTICK_OK;
            }
        }
    }
    return status;
}

/******************************************************************************
 * @brief Get current SysTick counter (VAL)
 * @details Reads STK_VAL (24-bit). Validates pointer.
 * @param[out] currentCount Pointer to store VAL
 * @return SYSTICK_Status_t SYSTICK_OK or SYSTICK_NULL_PTR
 ******************************************************************************/
SYSTICK_Status_t SYSTICK_GetCurrentCount(uint32_t *currentCount){
    SYSTICK_Status_t status = SYSTICK_NOT_OK;

    /* Validate the pointer parameter */
    if(NULL == currentCount){
        status = SYSTICK_NULL_PTR;
    }else{
        /* Read the current counter value from the STK_VAL register */
        *currentCount = SYSTICK_Registers->STK_VAL;
        status = SYSTICK_OK;
    }
    return status;
}

/******************************************************************************
 * @brief Get COUNTFLAG (bit16) — did timer hit 0 since last read?
 * @details Extracts STK_CTRL.COUNTFLAG (reading clears it per HW).
 * @param[out] CounterFlag Pointer to uint8_t for 0/1
 * @return SYSTICK_Status_t SYSTICK_OK or SYSTICK_NULL_PTR
 * @note  Helper not in public header in some forks — kept for debug.
 ******************************************************************************/
SYSTICK_Status_t SYSTICK_GetCounterFlag(uint8_t *CounterFlag){
    SYSTICK_Status_t status = SYSTICK_NOT_OK;

    /* Validate the pointer parameter */
    if(NULL == CounterFlag){
        status = SYSTICK_NULL_PTR;
    }else{
        /* Extract the COUNTFLAG bit (bit 16) from the control register */
        *CounterFlag = ((SYSTICK_Registers->STK_CTRL)>>SYSTICK_COUNT_FLAG_POS)&SYSTICK_COUNT_FLAG_MASK;
        status = SYSTICK_OK;
    }
    return status;
}

/******************************************************************************
 * @brief SysTick ISR — hardware vector 0x3B
 * @details Increments systick_counter (for Wait_ms) and invokes user callback
 *          if registered. Executes in interrupt context — keep short.
 * @param None
 * @return None
 * @note  Registered via SYSTICK_SetCallBack; scheduler uses this for tick.
 ******************************************************************************/
void SysTick_Handler(void){
    /* Increment the interrupt counter used by SYSTICK_Wait_ms */
    systick_counter++;

    /* If a callback function has been registered, invoke it */
    if(NULL != callback){
        callback();
    }
}
