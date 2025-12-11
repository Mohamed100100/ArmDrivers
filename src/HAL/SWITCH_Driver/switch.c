#include "./LIB/stdtypes.h"

#include "./MCAL/GPIO_Driver/gpio_int.h"
#include "./OS/schedule.h"

#include "HAL/SWITCH_Driver/switch_cfg.h"
#include "./HAL/SWITCH_Driver/switch.h"

/* 
 * External declaration of the switch configuration array
 * Defined in switch_cfg.c, contains hardware configuration for all switches
 */
extern const SWITCH_cfg_t SWITCHConfigArr[SWITCH_LEN];

/*
 * Forward declaration of switch runnable callback function
 * Registered with scheduler to perform periodic switch state sampling and debouncing
 */
static void Switch_runnableCBF(void *args);

/*
 * Forward declaration of local switch reading function
 * Performs single GPIO read without debouncing delay (debouncing handled by runnable)
 */
static SWITCH_Status_t SWITCH_enuLocalReadVAl(SWITCH_Name_t switchName,SWITCH_State_t* retStat);

/*
 * Array storing the previous state of each switch from the last read
 * Used for debouncing algorithm to detect stable state transitions
 * Initialized to SWITCH_RELEASED for all switches
 */
static SWITCH_State_t prevState[SWITCH_LEN]   = {SWITCH_RELEASED};

/*
 * Array storing the current stable/debounced state of each switch
 * This is the authoritative state that applications read
 * Updated only after debouncing confirms a stable state change
 * Initialized to SWITCH_RELEASED for all switches
 */
static SWITCH_State_t SwitchState[SWITCH_LEN] = {SWITCH_RELEASED};

/*
 * Array of debouncing counters for each switch
 * Counts consecutive identical readings to confirm state is stable
 * Reset to 0 when state changes or after stable state is confirmed
 * Initialized to 0 for all switches
 */
static uint8_t SwitchCounter[SWITCH_LEN] = {0};

/*
 * Scheduler runnable configuration for switch driver
 * Registers the switch sampling task to run periodically
 * 
 * Configuration:
 * - CBF: Switch_runnableCBF - callback function for switch processing
 * - Periodicity_ms: 5ms - samples switches every 5 milliseconds
 * - FirstDelay_ms: 0 - starts immediately (no initial delay)
 * - Args: NULL - no arguments needed
 * - Priority: 0 - highest priority (switch input typically time-critical)
 */
static SCHED_Runnable_t SwitchRunnable ={
    .CBF = Switch_runnableCBF,
    .Periodicity_ms = 5,
    .FirstDalay_ms = 0,
    .Args = NULL,
    .Priority = 0
};


/*
 * Function: SWITCH_enuInit
 * Description: Initializes all configured switches and registers switch task with scheduler
 *              This is an updated version that integrates with the scheduler for debouncing
 *              instead of using blocking delays
 * Parameters: None (uses configuration from SWITCHConfigArr)
 * Returns: SWITCH_Status_t indicating success or specific error condition
 * 
 * Updated implementation:
 * 1. Configure all switch GPIO pins as inputs with pull resistors
 * 2. Register switch runnable with scheduler for periodic sampling
 * 3. Return error if GPIO initialization or scheduler registration fails
 * 
 * Advantages over previous implementation:
 * - Non-blocking debouncing (uses scheduler instead of delay loops)
 * - Better system responsiveness (no blocking delays)
 * - Consistent sampling rate (5ms periodic execution)
 * - Allows other tasks to run during debouncing
 */
SWITCH_Status_t SWITCH_enuInit(){
    /* Initialize return status as successful */
    SWITCH_Status_t retStatus = SWITCH_OK;
    
    /* Variable to store GPIO operation status */
    GPIO_Status_t gpioStatus;
    
    /* Temporary GPIO configuration structure */
    GPIO_cfg_t cfg;
    
    /* Iterate through all configured switches to initialize their GPIO pins */
    for(uint8_t i = 0;i<SWITCH_LEN;i++){
        /* Set GPIO mode to input (switches require input mode for reading) */
        cfg.mode = GPIO_MODE_INPUT;
        
        /* Copy port configuration from switch configuration array */
        cfg.port = SWITCHConfigArr[i].port;
        
        /* Extract pull resistor configuration from switch connection type
         * Mask with 0b11 to get only bits 0-1 which represent pull-up/pull-down settings */
        cfg.pull = (GPIO_Pull_t)(SWITCHConfigArr[i].connection & (0b11));
        
        /* Copy pin configuration from switch configuration array */
        cfg.pin  = SWITCHConfigArr[i].pin;
        
        /* Set alternate function to AF0 (not used for basic GPIO input) */
        cfg.alternateFunction = GPIO_AF0;
        
        /* Set output type to push-pull (not used for input mode) */
        cfg.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
        
        /* Set default speed (not critical for input mode) */
        cfg.speed = GPIO_SPEED_DEFAULT;
        
        /* Initialize the GPIO pin with the configured settings */
        gpioStatus = GPIO_enuInit(&cfg);


        /* Check if GPIO initialization was successful */
        if(GPIO_OK != gpioStatus)
        {
            /* Cast GPIO error status to SWITCH status type and store */
            retStatus = (SWITCH_Status_t)gpioStatus;
            
            /* Exit loop on first error - fail-fast approach */
            break;
        }else{
            /* Continue to next switch if initialization was successful */
        }
    }

    /* If all GPIO pins initialized successfully, register with scheduler */
    if(SWITCH_OK == retStatus){
        /* Register the switch runnable with the scheduler for periodic execution */
        SCHED_Status_t schedStat = SCHED_enuRegisterRunnable(&SwitchRunnable);
        
        /* Check if scheduler registration was successful */
        if(schedStat == SCHED_OK){
            retStatus = SWITCH_OK;
        }else{
            /* Return scheduler error if registration failed */
            retStatus = SWITCH_ERROR_SCHEDUALE;
        }
    }else{
        /* GPIO initialization failed - return that error status */
    }

    /* Return final initialization status */
    return retStatus;
}


/*
 * Function: SWITCH_enuLocalReadVAl
 * Description: Performs a single immediate GPIO read of the specified switch
 *              This is a local helper function used by the scheduler runnable
 *              Does NOT perform debouncing (debouncing handled by runnable callback)
 * Parameters:
 *   - switchName: Switch identifier from SWITCH_Name_t enum
 *   - retStat: Pointer to store the instantaneous switch state (not debounced)
 * Returns: SWITCH_Status_t indicating success or error
 * 
 * Differences from previous SWITCH_enuReadVAl:
 * - No blocking delay loops (removed SWITCH_delay calls)
 * - Single GPIO read instead of multiple reads with delay
 * - Returns instantaneous state, not debounced state
 * - Used internally by scheduler runnable for periodic sampling
 * 
 * Note: This function reads the raw GPIO state which may contain bounce
 *       The scheduler runnable (Switch_runnableCBF) handles debouncing
 */
static SWITCH_Status_t SWITCH_enuLocalReadVAl(SWITCH_Name_t switchName,SWITCH_State_t* retStat){

    /* Initialize status as not OK */
    SWITCH_Status_t status = SWITCH_NOT_OK;

    /* Validate output pointer parameter */
    if(NULL == retStat){
        status = SWITCH_NULL_PTR;
    }else{
        /* Validate switch name is within valid range */
        if(SWITCH_LEN <= switchName){
            status = SWITCH_WRONG_NAME;
        }else{

            /* Variable to store GPIO pin reading */
            uint8_t readVal = 0;

            /* Variable to store GPIO operation status */
            GPIO_Status_t gpioStatus =  GPIO_NOT_OK;

            /* 
             * OLD IMPLEMENTATION (commented out):
             * Previous version used blocking while loop with delays for debouncing
             * This blocked the entire system during debouncing
             * 
             * NEW IMPLEMENTATION:
             * Single GPIO read - debouncing handled by periodic scheduler callback
             * Non-blocking approach allows system to remain responsive
             */
            // uint8_t tempReadVAl= 1;
            // while(readVal != tempReadVAl){
            //     gpioStatus = GPIO_enuReadPinVal(SWITCHConfigArr[switchName].port,SWITCHConfigArr[switchName].pin,&tempReadVAl);
            //     SWITCH_delay(DEBOUNCING_TIME);
            //     gpioStatus = GPIO_enuReadPinVal(SWITCHConfigArr[switchName].port,SWITCHConfigArr[switchName].pin,&readVal);
            // }

            /* Perform single immediate GPIO read */
            gpioStatus = GPIO_enuReadPinVal(SWITCHConfigArr[switchName].port,SWITCHConfigArr[switchName].pin,&readVal);

            /* Check if GPIO read operation was successful */
            if(GPIO_OK != gpioStatus){
                status = SWITCH_ERROR_READ;
            }else{

                /* 
                 * Translate GPIO logic level to switch state based on pull resistor configuration
                 * Pull-up configuration: LOW = pressed, HIGH = released
                 * Pull-down configuration: HIGH = pressed, LOW = released
                 */
                switch(SWITCHConfigArr[switchName].connection){
                    /* Pull-up configurations (internal or external) */
                    case SWITCH_INTERNAL_PULLUP:
                    case SWITCH_EXTERNAL_PULLUP:
                    /* With pull-up: Pin is HIGH when released, LOW when pressed */
                    if(readVal == 0){
                        *retStat = SWITCH_PUSHED;
                    }else{
                        *retStat = SWITCH_RELEASED;
                    }
                    break;
                    
                    /* Pull-down configurations (internal or external) */
                    case SWITCH_INTERNAL_PULLDOWN:
                    case SWITCH_EXTERNAL_PULLDOWN:
                    /* With pull-down: Pin is LOW when released, HIGH when pressed */
                    if(readVal == 1){
                        *retStat = SWITCH_PUSHED;
                    }else{
                        *retStat = SWITCH_RELEASED;
                    }
                    break;
                }
                
                /* Operation completed successfully */
                status = SWITCH_OK;
            }
        }
    }
    
    /* Return final status */
    return (status);
}


/*
 * Function: Switch_runnableCBF
 * Description: Scheduler runnable callback that performs periodic switch sampling and debouncing
 *              Called by scheduler every 5ms to sample all switches and update their states
 *              Implements counter-based debouncing algorithm for noise immunity
 * Parameters:
 *   - args: Unused argument pointer (required by scheduler callback signature)
 * Returns: None
 * 
 * Debouncing Algorithm:
 * 1. Read current instantaneous state of switch
 * 2. Compare with previous reading (prevState)
 * 3. If same as previous:
 *    - Increment counter (state is stable)
 *    - If counter reaches threshold (4 consecutive identical readings = 20ms):
 *      * Update official state (SwitchState)
 *      * Reset counter
 * 4. If different from previous:
 *    - Reset counter (state is bouncing)
 * 5. Store current reading as previous for next iteration
 * 
 * Timing Analysis:
 * - Runnable period: 5ms
 * - Debounce threshold: 4 consecutive readings
 * - Total debounce time: 4 × 5ms = 20ms
 * - This filters out bounces shorter than 20ms
 * 
 * Advantages:
 * - Non-blocking (no delay loops)
 * - Predictable timing (scheduler-driven)
 * - Configurable threshold (adjust counter limit)
 * - Independent per switch (each has own counter)
 */
static void Switch_runnableCBF(void *args){
    /* Variable to store current instantaneous switch state */
    SWITCH_State_t currentStat = SWITCH_RELEASED;
    
    /* Variable to store switch read operation status */
    SWITCH_Status_t switchStat = SWITCH_NOT_OK;
    
    /* Iterate through all configured switches */
    for(uint32_t i =0;i<SWITCH_LEN;i++){
        /* Read current instantaneous state of this switch */
        switchStat = SWITCH_enuLocalReadVAl(i,&currentStat);
        
        /* Check if read operation was successful */
        if(SWITCH_OK != switchStat){
            /* If read failed, mark counter as error state */
            SwitchCounter[i] = SWITCH_ERROR;
        }else{
            /* Check if current reading matches previous reading (stable state) */
            if(currentStat == prevState[i]){
                /* State is stable - increment debounce counter */
                SwitchCounter[i]++;

                /*
                 * Check if counter has reached debounce threshold
                 * Threshold of 4 means 4 consecutive identical readings
                 * At 5ms per reading: 4 × 5ms = 20ms stable time required
                 */
                if(SwitchCounter[i] >=4){
                    /* State has been stable for 20ms - update official state */
                    SwitchState[i] = currentStat;
                    
                    /* Reset counter for next state change detection */
                    SwitchCounter[i] = 0;
                }else{
                    /* Not enough consecutive readings yet - continue counting */
                }
            }else{
                /* State changed from previous reading - reset counter (bouncing detected) */
                SwitchCounter[i] = 0;
            }
            
            /* Store current reading as previous for next iteration */
            prevState[i] = currentStat;
        }
    }

}

/*
 * Function: SWITCH_enuReadVAl
 * Description: Public API to read the debounced state of a switch
 *              Returns the stable, debounced state maintained by the scheduler runnable
 *              Non-blocking - returns immediately with last debounced state
 * Parameters:
 *   - switchName: Switch identifier from SWITCH_Name_t enum
 *   - retStat: Pointer to store the debounced switch state
 * Returns: SWITCH_Status_t indicating success or error
 * 
 * Implementation notes:
 * - Does NOT read GPIO directly
 * - Returns cached state from SwitchState array
 * - State is updated by scheduler runnable every 5ms
 * - Always returns immediately (non-blocking)
 * - State is guaranteed to be debounced (stable for 20ms)
 * 
 * Usage pattern:
 * 1. Call SWITCH_enuInit() once at startup
 * 2. Call SWITCH_enuReadVAl() anytime to get current debounced state
 * 3. No delays needed - scheduler handles everything in background
 * 
 * Comparison with old implementation:
 * - OLD: Blocking function with delay loops (could take up to 2ms per call)
 * - NEW: Non-blocking function returns immediately (microseconds)
 * - OLD: Each call performs debouncing
 * - NEW: Debouncing happens in background (scheduler runnable)
 */
SWITCH_Status_t SWITCH_enuReadVAl(SWITCH_Name_t switchName,SWITCH_State_t* retStat){
    /* Initialize status as not OK */
    SWITCH_Status_t status = SWITCH_NOT_OK;

    /* Validate output pointer parameter */
    if(NULL == retStat){
        status = SWITCH_NULL_PTR;
    }else{
        
        /* Validate switch name is within valid range */
        if(SWITCH_LEN <= switchName){
            status = SWITCH_WRONG_NAME;
        }else{
            /* Return the cached debounced state from SwitchState array */
            *retStat = SwitchState[switchName];
            status = SWITCH_OK;
        }
    }
    
    /* Return operation status */
    return status;
}