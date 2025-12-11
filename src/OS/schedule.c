#include "MCAL/SYSTICK_TIMER_Driver/systick.h"

#include "OS/schedule_cfg.h"
#include "OS/schedule.h"

/*
 * Static variable storing the scheduler tick time in milliseconds
 * Represents the time quantum for the scheduler (how often the scheduler runs)
 * Set during SCHED_enuInit() and used for timing calculations
 */
static uint32_t TickTime = 0;

/*
 * Static flag indicating whether a SysTick interrupt has occurred
 * Set to TRUE by SysTick ISR (SCHED_vdExec)
 * Cleared by scheduler main loop after processing tick
 * Used for synchronization between ISR and main scheduler loop
 */
static bool_t Systick_triggered = FALSE;

/*
 * Static array of pointers to registered runnable tasks
 * Array is indexed by task priority (priority value used as array index)
 * NULL entries indicate no task registered at that priority level
 * Size defined by MAX_RUNNABLES constant from schedule_cfg.h
 */
static SCHED_Runnable_t* savedRunnbles[MAX_RUNNABLES];

/*
 * Forward declaration of SysTick callback function
 * Called by SysTick ISR on every timer overflow
 * Sets flag to trigger scheduler execution
 */
static void SCHED_vdExec();

/*
 * Forward declaration of runnable execution function
 * Called from scheduler main loop on every tick
 * Iterates through runnables and executes those that are ready
 */
static void localExecuteRunnables();

/*
 * Function: SCHED_enuInit
 * Description: Initializes the scheduler system and configures SysTick timer
 *              Sets up the time base for periodic task scheduling
 *              Calculates and configures SysTick reload value for desired tick period
 * Parameters:
 *   - copyTickTime_ms: Scheduler tick period in milliseconds (time quantum)
 *   - copyClockSourceVAlue_hz: System clock frequency in Hz (used for SysTick calculation)
 * Returns: SCHED_Status_t indicating success or specific error condition
 * 
 * Implementation steps:
 * 1. Store tick time for later use
 * 2. Initialize SysTick with clock frequency and no prescaler
 * 3. Calculate SysTick reload value: (TickTime_ms × Clock_Hz / 1000) - 1
 * 4. Set SysTick reload value
 * 5. Register scheduler tick callback with SysTick
 */
SCHED_Status_t SCHED_enuInit(uint32_t copyTickTime_ms,uint32_t copyClockSourceVAlue_hz){
    
    /* Initialize return status as not OK (pessimistic approach) */
    SCHED_Status_t retStatus = SCHED_NOT_OK;
    
    /* Variable to track SysTick driver operation status */
    SYSTICK_Status_t systickStatus = SYSTICK_NOT_OK;
    
    /* Store the tick time for use in runnable execution timing */
    TickTime = copyTickTime_ms;

    /* 
     * Initialize SysTick timer with system clock and no prescaler
     * Uses full processor clock frequency for maximum timing accuracy
     */
    systickStatus = SYSTICK_Init(copyClockSourceVAlue_hz,SYSTICK_NO_PRESCALLER);

    /* Check if SysTick initialization was successful */
    if(SYSTICK_OK!=systickStatus){
        /* Return error if SysTick failed to initialize */
        retStatus = SCHED_SYSTICK_ERROR;
    }else{
        /* Variable to store calculated SysTick reload value */
        uint32_t loadValue;
        
        /*
         * Calculate SysTick reload value for desired tick period
         * Formula: LoadValue = (TickTime_ms × ClockFrequency_Hz / 1000) - 1
         * 
         * Explanation:
         * - Multiply tick time by clock frequency to get total clock cycles
         * - Divide by 1000 to convert milliseconds to seconds
         * - Subtract 1 because SysTick counts from LOAD to 0 (inclusive)
         * 
         * Example: For 1ms tick at 84MHz: (1 × 84,000,000 / 1000) - 1 = 83,999
         */
        loadValue = ((copyTickTime_ms * copyClockSourceVAlue_hz) / 1000) - 1;

        /* Set the calculated reload value in SysTick timer */
        systickStatus = SYSTICK_SetStartValue(loadValue);
        
        /* Check if reload value was set successfully */
        if(SYSTICK_OK!=systickStatus){
            /* Return error if reload value is invalid (e.g., exceeds 24-bit range) */
            retStatus = SCHED_SYSTICK_WRONG_START_VALUE;
        }else{
            /*
             * Register SCHED_vdExec as the SysTick interrupt callback
             * This function will be called on every SysTick overflow (every tick)
             */
            systickStatus = SYSTICK_SetCallBack(SCHED_vdExec);
            
            /* Check if callback registration was successful */
            if(SYSTICK_OK!=systickStatus){
                /* Return error if callback registration failed */
                retStatus = SCHED_SYSTICK_FAILED_TO_SET_CALLBACK;
            }else{
                /* All initialization steps completed successfully */
                retStatus = SCHED_OK;
            }
        }
    }
    
    /* Return final initialization status */
    return retStatus;
}


/*
 * Function: SCHED_vdExec
 * Description: SysTick interrupt callback function
 *              Called automatically by SysTick ISR on every timer overflow
 *              Sets flag to notify main scheduler loop that a tick has occurred
 * Parameters: None
 * Returns: None
 * 
 * Implementation notes:
 * - Executes in interrupt context - must be fast and non-blocking
 * - Only sets a flag; actual runnable execution happens in main loop context
 * - This design separates ISR context from task execution context
 * - Prevents long-running tasks from blocking interrupts
 */
static void SCHED_vdExec(){
    /* Set flag to indicate SysTick interrupt occurred */
    Systick_triggered = TRUE;
}


/*
 * Function: SCHED_enuRegisterRunnable
 * Description: Registers a new runnable task with the scheduler
 *              Adds the task to the scheduler's priority-indexed task array
 *              Priority value is used as array index for O(1) access
 * Parameters:
 *   - runnabelPtr: Pointer to runnable configuration structure
 * Returns: SCHED_Status_t indicating success or error
 * 
 * Error conditions:
 * - SCHED_NULL_PTR: Null pointer passed
 * - SCHED_ERROR_RUNNABLE_STORED_BEFORE: Priority slot already occupied
 * - SCHED_OK: Registration successful
 * 
 * Implementation notes:
 * - Each priority level can have only one runnable
 * - Priority value must be less than MAX_RUNNABLES
 * - No validation of priority range (assumes valid enum/define values)
 */
SCHED_Status_t SCHED_enuRegisterRunnable(SCHED_Runnable_t * runnabelPtr){

    /* Initialize return status as not OK */
    SCHED_Status_t retStatus = SCHED_NOT_OK;
    
    /* Validate pointer parameter */
    if(NULL == runnabelPtr){
        retStatus = SCHED_NULL_PTR;
    }else{
        /* Check if priority slot is already occupied */
        if(NULL != savedRunnbles[runnabelPtr->Priority]){
            /* Return error if another runnable already registered at this priority */
            retStatus = SCHED_ERROR_RUNNABLE_STORED_BEFORE;
        }else{
            /* Store runnable pointer at its priority index in the array */
            savedRunnbles[runnabelPtr->Priority] = runnabelPtr;
            retStatus = SCHED_OK;
        }
    }
    
    /* Return registration status */
    return retStatus;
}

/*
 * Function: SCHED_enuRemoveRunnable
 * Description: Removes a previously registered runnable from the scheduler
 *              Clears the runnable pointer at its priority index
 *              Task will no longer be executed after removal
 * Parameters:
 *   - runnabelPtr: Pointer to runnable configuration structure to remove
 * Returns: SCHED_Status_t indicating success or error
 * 
 * Implementation notes:
 * - Simply sets the priority slot to NULL
 * - Does not validate if runnable was actually registered
 * - No memory deallocation (runnable struct remains in application memory)
 * - Runnable can be re-registered later if needed
 */
SCHED_Status_t SCHED_enuRemoveRunnable(SCHED_Runnable_t *runnabelPtr){
    /* Initialize return status as not OK */
    SCHED_Status_t retStatus = SCHED_NOT_OK;
    
    /* Validate pointer parameter */
    if(NULL == runnabelPtr){
        retStatus = SCHED_NULL_PTR;
    }else{
        /* Clear the runnable pointer at its priority index */
        savedRunnbles[runnabelPtr->Priority] = NULL;
        retStatus = SCHED_OK;
    }
    
    /* Return removal status */
    return retStatus;
}

/*
 * Function: SCHED_enuStart
 * Description: Starts the scheduler and begins executing registered runnable tasks
 *              This is the scheduler's main loop - runs indefinitely
 *              Waits for SysTick interrupts and executes runnables on each tick
 * Parameters: None
 * Returns: None (infinite loop - does not return)
 * 
 * Scheduler operation:
 * 1. Start SysTick timer to begin generating periodic interrupts
 * 2. Enter infinite main loop
 * 3. Wait for SysTick flag to be set by ISR
 * 4. Clear flag and execute all ready runnables
 * 5. Repeat forever
 * 
 * Design notes:
 * - Uses polling approach (checks flag in loop)
 * - Runnable execution happens in main context, not ISR context
 * - Allows long-running tasks without blocking interrupts
 * - Simple cooperative scheduling (no preemption within a tick)
 */
void SCHED_enuStart(){
    /* Start the SysTick timer - begins generating periodic interrupts */
    SYSTICK_StartCount();

    /* Infinite main scheduler loop */
    while(1){
        /* Check if SysTick interrupt has occurred */
        if(Systick_triggered == TRUE){
            /* Clear the flag to acknowledge this tick */
            Systick_triggered = FALSE;
            
            /* Execute all runnables that are ready to run */
            localExecuteRunnables();
        }else{
            /* No tick occurred yet - wait for next interrupt */
            /* NOP (No Operation) - could implement idle/sleep mode here */
        }
    }
    
}

/*
 * Function: localExecuteRunnables
 * Description: Iterates through all registered runnables and executes those that are ready
 *              Called from scheduler main loop on every tick
 *              Tracks elapsed time and determines which tasks should execute
 * Parameters: None
 * Returns: None
 * 
 * Scheduling algorithm:
 * 1. Maintain a tick counter (increments by TickTime each call)
 * 2. For each priority level (0 to MAX_RUNNABLES-1):
 *    a. Check if runnable is registered at this priority
 *    b. Check if callback function is valid
 *    c. Check if enough time has elapsed (tickCounters % Periodicity == 0)
 *    d. If ready, execute callback with its arguments
 * 3. Increment tick counter
 * 
 * Implementation notes:
 * - Uses static variable to maintain tick count across calls
 * - Modulo operation determines if periodicity has elapsed
 * - Does NOT handle FirstDelay_ms (bug/missing feature)
 * - Executes runnables in priority order (lower index = higher priority)
 * - All ready runnables execute within single tick (cooperative multitasking)
 */
static void localExecuteRunnables(){

    /*
     * Static tick counter maintains total elapsed time in milliseconds
     * Persists across function calls to track time for all runnables
     * Incremented by TickTime at end of each scheduler tick
     */
    static uint64_t tickCounters = 0;

    /* Iterate through all possible priority levels */
    for(uint32_t index = 0;index<MAX_RUNNABLES;index++){
        /* Check if a runnable is registered at this priority level */
        if(NULL != savedRunnbles[index]){
            /* Check if the runnable has a valid callback function */
            if(NULL != savedRunnbles[index]->CBF){
                /*
                 * Check if it's time to execute this runnable
                 * Uses modulo to check if current time is a multiple of periodicity
                 * 
                 * Example: If periodicity is 10ms and TickTime is 1ms:
                 * - tickCounters = 0, 1, 2, ..., 9, 10, 11, ...
                 * - tickCounters % 10 = 0 only at 0, 10, 20, 30, ... (every 10ms)
                 * 
                 * Note: This implementation has a bug - it doesn't account for FirstDelay_ms
                 * All runnables start executing immediately from tick 0
                 */
                if(tickCounters >= savedRunnbles[index]->FirstDalay_ms){

                    if(0 == ((tickCounters-savedRunnbles[index]->FirstDalay_ms)%savedRunnbles[index]->Periodicity_ms)){
                        /* Execute the runnable's callback function with its arguments */
                        savedRunnbles[index]->CBF(savedRunnbles[index]->Args);
                    }else{
                        /* Not yet time to execute this runnable - skip to next */
                    }
                }else{
                    // not yet the first delay
                }
            }else{
                /* Runnable structure exists but callback is NULL - skip */
                /* This is a configuration error by the user */
            }
        }else{
            /* No runnable registered at this priority level - skip */
        }
    }
    
    /*
     * Increment tick counter by the scheduler tick time
     * Tracks total elapsed time for all runnable timing calculations
     */
    tickCounters+=TickTime;
}