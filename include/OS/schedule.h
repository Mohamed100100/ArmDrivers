#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "LIB/stdtypes.h"

/*
 * Function pointer type definition for runnable callback functions
 * Runnables are periodic tasks that get executed by the scheduler
 * Takes a void pointer parameter to allow passing custom arguments to the task
 * Returns void
 */
typedef void(*RunnableCallback_t)(void*);

/*
 * Enumeration of possible return status codes for scheduler functions
 * Used to indicate success, failure, or specific error conditions
 */
typedef enum {
    SCHED_NOT_OK,                               /* General error or operation failed */
    SCHED_OK,                                   /* Operation completed successfully */
    SCHED_SYSTICK_ERROR,                        /* Error occurred in SysTick timer initialization or configuration */
    SCHED_SYSTICK_WRONG_START_VALUE,            /* Invalid SysTick start/reload value provided */
    SCHED_SYSTICK_FAILED_TO_SET_CALLBACK,       /* Failed to register SysTick callback function */
    SCHED_NULL_PTR,                             /* Null pointer passed as parameter */
    SCHED_ERROR_RUNNABLE_STORED_BEFORE,         /* Attempted to register a runnable that already exists in scheduler */
}SCHED_Status_t;

/*
 * Structure containing complete configuration for a schedulable runnable task
 * Defines all parameters needed for the scheduler to manage and execute periodic tasks
 */
typedef struct {
    RunnableCallback_t CBF;         /* Callback function pointer - the task to be executed periodically */
    uint32_t Periodicity_ms;        /* Execution period in milliseconds - how often the task should run */
    uint32_t FirstDalay_ms;         /* Initial delay in milliseconds before first execution (typo: should be FirstDelay_ms) */
    void * Args;                    /* Pointer to arguments to pass to the callback function */
    uint32_t Priority;              /* Task priority - higher values indicate higher priority (used for execution ordering) */
}SCHED_Runnable_t;

/*
 * Function: SCHED_enuInit
 * Description: Initializes the scheduler system and underlying SysTick timer
 *              Sets up the time base for task scheduling and prepares data structures
 *              Must be called before registering any runnables or starting the scheduler
 * Parameters:
 *   - uint32_t: System clock frequency in Hz (used to calculate SysTick timing)
 *   - uint32_t: Scheduler tick period in milliseconds (time quantum for task scheduling)
 * Returns: SCHED_Status_t indicating success or specific error condition
 * Note: This function configures the SysTick timer as the time base for the scheduler
 *       All runnable periodicities must be multiples of the tick period
 */
SCHED_Status_t SCHED_enuInit(uint32_t ,uint32_t);

/*
 * Function: SCHED_enuRegisterRunnable
 * Description: Registers a new runnable task with the scheduler
 *              Adds the task to the scheduler's internal task list
 *              Task will be executed periodically according to its configuration once scheduler starts
 * Parameters:
 *   - SCHED_Runnable_t*: Pointer to runnable configuration structure containing:
 *                        - Callback function
 *                        - Periodicity (how often to execute)
 *                        - Initial delay (when to start)
 *                        - Arguments (data to pass to callback)
 *                        - Priority (execution ordering)
 * Returns: SCHED_Status_t indicating success or error (e.g., null pointer, duplicate runnable)
 * Note: Runnable must not already be registered
 *       Scheduler prioritizes tasks based on priority value when multiple tasks are ready
 *       Higher priority tasks execute before lower priority tasks in the same tick
 */
SCHED_Status_t SCHED_enuRegisterRunnable(SCHED_Runnable_t *);

/*
 * Function: SCHED_enuRemoveRunnable
 * Description: Removes a previously registered runnable task from the scheduler
 *              Task will no longer be executed after removal
 *              Useful for dynamically enabling/disabling tasks at runtime
 * Parameters:
 *   - SCHED_Runnable_t*: Pointer to runnable configuration structure to remove
 * Returns: SCHED_Status_t indicating success or error (e.g., null pointer, runnable not found)
 * Note: Pointer must match the address of a previously registered runnable
 *       Removal does not affect other registered runnables
 */
SCHED_Status_t SCHED_enuRemoveRunnable(SCHED_Runnable_t *);

/*
 * Function: SCHED_enuStart
 * Description: Starts the scheduler and begins executing registered runnable tasks
 *              This function typically does not return (runs scheduler main loop indefinitely)
 *              Manages task execution timing, prioritization, and callback invocation
 * Parameters: None
 * Returns: None (infinite loop - does not return under normal operation)
 * Note: Must call SCHED_enuInit() before calling this function
 *       At least one runnable should be registered before starting
 *       Scheduler handles:
 *       - Tracking elapsed time for each task
 *       - Determining which tasks are ready to execute
 *       - Executing ready tasks in priority order
 *       - Managing first delay and periodic execution
 */
void SCHED_enuStart();

#endif /* SCHEDULE_H */