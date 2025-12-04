#ifndef SYSTICK_H
#define SYSTICK_H

#include "LIB/stdtypes.h"

/* 
 * Function pointer type definition for SysTick callback function
 * This callback will be invoked when the SysTick interrupt occurs
 * Takes no parameters and returns void
 */
typedef void (*SYSTICK_Callback_t)(void);

/* 
 * Enumeration of possible return status codes for SysTick driver functions
 * Used to indicate success, failure, or specific error conditions
 */
typedef enum {
    SYSTICK_NOT_OK,              /* General error or operation failed */
    SYSTICK_OK,                  /* Operation completed successfully */
    SYSTICK_WRONG_PRESCALLER,    /* Invalid prescaler value provided */
    SYSTICK_WRONG_STARTVALUE,    /* Invalid start value provided (e.g., exceeds 24-bit limit) */
    SYSTICK_OFF,                 /* SysTick timer is currently disabled */
    SYSTICK_EXCEPTION_OFF,       /* SysTick exception/interrupt is disabled */
    SYSTICK_ZERO_STARTVALUE,     /* Start value is zero (invalid for timer operation) */
    SYSTICK_NULL_PTR,            /* Null pointer passed as parameter */
}SYSTICK_Status_t;


/* 
 * Enumeration of available SysTick clock prescaler options
 * Determines the clock source and division factor for the SysTick timer
 */
typedef enum {
    SYSTICK_NO_PRESCALLER = 0b100,  /* No prescaler - uses processor clock directly (bit 2 set) */
    SYSTICK_PRESCALLER_8  = 0b000,  /* Divide by 8 prescaler - uses processor clock / 8 (bit 2 clear) */
}SYSTICK_Prescaller_t;


/* 
 * Initializes the SysTick timer with specified clock frequency and prescaler
 * Parameters:
 *   - ClockValue: The system clock frequency in Hz
 *   - Prescaler: Clock source selection (no prescaler or divide by 8)
 * Returns: SYSTICK_Status_t indicating success or failure reason
 */
SYSTICK_Status_t SYSTICK_Init(uint32_t ClockValue,SYSTICK_Prescaller_t);

/* 
 * Registers a callback function to be executed on SysTick interrupt
 * Parameters:
 *   - Callback function pointer of type SYSTICK_Callback_t
 * Returns: SYSTICK_Status_t indicating success or error (e.g., NULL pointer)
 */
SYSTICK_Status_t SYSTICK_SetCallBack(SYSTICK_Callback_t);

/* 
 * Sets the reload value for the SysTick timer
 * Parameters:
 *   - Start value (24-bit value, 0x000000 to 0xFFFFFF)
 * Returns: SYSTICK_Status_t indicating success or error (e.g., invalid value)
 */
SYSTICK_Status_t SYSTICK_SetStartValue(uint32_t);

/* 
 * Starts the SysTick counter
 * Enables counting by setting the ENABLE bit in the control register
 * No parameters, no return value
 */
void SYSTICK_StartCount();

/* 
 * Stops the SysTick counter
 * Disables counting by clearing the ENABLE bit in the control register
 * No parameters, no return value
 */
void SYSTICK_StopCount();

/* 
 * Blocking delay function using SysTick timer
 * Parameters:
 *   - Delay time in milliseconds
 * Returns: SYSTICK_Status_t indicating success or error condition
 * Note: This is a blocking function that halts program execution for the specified time
 */
SYSTICK_Status_t SYSTICK_Wait_ms(uint32_t);

/* 
 * Retrieves the current counter value from the SysTick timer
 * Parameters:
 *   - Pointer to uint32_t where the current count value will be stored
 * Returns: SYSTICK_Status_t indicating success or error (e.g., NULL pointer)
 */
SYSTICK_Status_t SYSTICK_GetCurrentCount(uint32_t *);

#endif /* SYSTICK_H */