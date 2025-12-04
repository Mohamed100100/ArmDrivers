#ifndef SYSTICK_PRIV_H
#define SYSTICK_PRIV_H

#include "LIB/stdtypes.h"

/* Base address of the SysTick timer peripheral in the ARM Cortex-M memory map */
#define SYSTICK_BASE_ADDRESS            (0xE000E010)
    
/* Bit mask to enable the SysTick exception (interrupt) - sets bit 1 of STK_CTRL */
#define SYSTICK_ENABLE_EXCEPTION        (0b10UL)

/* Bit mask to start the SysTick counter - sets bit 0 of STK_CTRL */
#define SYSTICK_START_COUNTING          (0b1UL)

/* Bit mask to stop the SysTick counter - clears bit 0 of STK_CTRL (all bits set except bit 0) */
#define SYSTICK_STOP_COUNTING           (0xFFFFFFFEUL)
    
/* Mask for the upper 8 bits of the 24-bit start value in STK_LOAD register */
#define SYSTICK_STARTVALUE_MASK         (0xFF000000UL)

/* Mask to check the COUNTFLAG bit (bit 16) of STK_CTRL register */
#define SYSTICK_COUNT_FLAG_MASK         (0b1UL)

/* Mask to check if SysTick is enabled - checks bit 0 of STK_CTRL */
#define SYSTICK_ENABLE_MASK_CHECK       (0b1UL)

/* Mask to check if SysTick exception is enabled - checks bit 1 of STK_CTRL */
#define SYSTICK_EXCEPTION_MASK_CHECK    (0b10UL)

/* Mask to check the clock source prescaler bit - checks bit 2 of STK_CTRL */
#define SYSTICK_PRESCALLER_MASK_CHECK   (0x4UL)

/* Bit position of the COUNTFLAG in the STK_CTRL register */
#define SYSTICK_COUNT_FLAG_POS          (16UL)


/* 
 * Structure representing the SysTick timer peripheral registers
 * Maps directly to the hardware register layout in memory
 */
typedef struct 
{
    uint32_t STK_CTRL;   /* Control and Status register - controls timer operation and reports status */
    uint32_t STK_LOAD;   /* Reload Value register - holds the value to load into STK_VAL when it reaches zero */
    uint32_t STK_VAL;    /* Current Value register - holds the current counter value */
    uint32_t STK_CALIB;  /* Calibration Value register - contains calibration data for 10ms timing */
}SYSTICK_Regs_t;


/* 
 * Pointer to the SysTick registers structure
 * Initialized to point to the SysTick base address in memory
 * Allows access to SysTick hardware registers through structure member notation
 */
SYSTICK_Regs_t *SYSTICK_Registers = (SYSTICK_Regs_t *)SYSTICK_BASE_ADDRESS;



#endif /* SYSTICK_PRIV_H */