#include "LIB/stdtypes.h"

#include "MCAL/SYSTICK_TIMER_Driver/systick_priv.h"
#include "MCAL/SYSTICK_TIMER_Driver/systick.h"

/* Forward declaration of the SysTick interrupt handler */
void SysTick_Handler(void);

/* Static variable to count the number of SysTick interrupts that have occurred */
static uint32_t systick_counter  = 0;

/* Static pointer to store the user-defined callback function */
static SYSTICK_Callback_t callback = NULL;

/* Static variable to store the system clock frequency value in Hz */
static uint32_t clockSourceValue=0;

/*
 * Function: SYSTICK_Init
 * Description: Initializes the SysTick timer with the specified clock frequency and prescaler
 * Parameters:
 *   - ClockValue: System clock frequency in Hz
 *   - prescaller: Clock source selection (processor clock or AHB/8)
 * Returns: Status code indicating success or specific error condition
 */
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

/*
 * Function: SYSTICK_SetCallBack
 * Description: Registers a user-defined callback function to be executed on SysTick interrupt
 * Parameters:
 *   - copyCallback: Function pointer to the callback function
 * Returns: SYSTICK_OK status (always succeeds)
 */
SYSTICK_Status_t SYSTICK_SetCallBack(SYSTICK_Callback_t copyCallback){
    /* Store the callback function pointer for later invocation in ISR */
    callback = copyCallback;
    return SYSTICK_OK;
}

/*
 * Function: SYSTICK_SetStartValue
 * Description: Sets the reload value for the SysTick timer counter
 * Parameters:
 *   - startValue: 24-bit reload value (0x000000 to 0xFFFFFF)
 * Returns: Status code indicating success or if value exceeds 24-bit limit
 */
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

/*
 * Function: SYSTICK_StartCount
 * Description: Starts the SysTick counter by enabling the timer
 * Parameters: None
 * Returns: None
 */
void SYSTICK_StartCount(){
    /* Set the ENABLE bit (bit 0) in the control register to start counting */
    SYSTICK_Registers->STK_CTRL |=SYSTICK_START_COUNTING;
}

/*
 * Function: SYSTICK_StopCount
 * Description: Stops the SysTick counter by disabling the timer
 * Parameters: None
 * Returns: None
 */
void SYSTICK_StopCount(){
    /* Clear the ENABLE bit (bit 0) in the control register to stop counting */
    SYSTICK_Registers->STK_CTRL &=SYSTICK_STOP_COUNTING;
}

/*
 * Function: SYSTICK_Wait_ms
 * Description: Blocking delay function that waits for specified milliseconds using SysTick
 * Parameters:
 *   - delay_ms: Delay time in milliseconds
 * Returns: Status code indicating success or specific error (timer off, no start value, etc.)
 */
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

/*
 * Function: SYSTICK_GetCurrentCount
 * Description: Reads the current value of the SysTick counter
 * Parameters:
 *   - currentCount: Pointer to store the current counter value
 * Returns: Status code indicating success or NULL pointer error
 */
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

/*
 * Function: SYSTICK_GetCounterFlag
 * Description: Reads the COUNTFLAG bit which indicates if timer counted to 0 since last read
 * Parameters:
 *   - CounterFlag: Pointer to store the flag value (0 or 1)
 * Returns: Status code indicating success or NULL pointer error
 * Note: Reading this flag automatically clears it
 */
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

/*
 * Function: SysTick_Handler
 * Description: SysTick interrupt service routine (ISR)
 *              Automatically called by hardware when SysTick counter reaches zero
 * Parameters: None
 * Returns: None
 * Note: This is the actual interrupt handler that executes on every SysTick exception
 */
void SysTick_Handler(void){
    /* Increment the interrupt counter used by SYSTICK_Wait_ms */
    systick_counter++;

    /* If a callback function has been registered, invoke it */
    if(NULL != callback){
        callback();
    }
}