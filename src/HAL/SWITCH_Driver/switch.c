#include "./LIB/stdtypes.h"

#include "./MCAL/GPIO_Driver/gpio_int.h"

#include "HAL/SWITCH_Driver/switch_cfg.h"
#include "./HAL/SWITCH_Driver/switch.h"

/*
 * Debouncing time in microseconds
 * Mechanical switches can "bounce" when pressed/released, creating multiple transitions
 * This delay filters out bouncing to ensure clean, single state transitions
 * Value of 1000 µs (1 ms) is typically sufficient for most mechanical switches
 */
#define DEBOUNCING_TIME     (1000UL)

/* 
 * External declaration of the switch configuration array
 * Defined in switch_cfg.c, contains hardware configuration for all switches
 */
extern const SWITCH_cfg_t SWITCHConfigArr[SWITCH_LEN];

/* 
 * Forward declaration of private delay function
 * Used internally for debouncing - not exposed in public API
 */
static void SWITCH_delay(volatile uint32_t iterations);

/*
 * Function: SWITCH_enuInit
 * Description: Initializes all configured switches by setting up their GPIO pins as inputs
 *              Iterates through all switches in configuration array and configures each one
 *              Sets GPIO mode to input with appropriate pull resistor configuration
 * Parameters: None (uses configuration from SWITCHConfigArr)
 * Returns: SWITCH_Status_t indicating success or first error encountered
 * 
 * Implementation details:
 * - Configures each GPIO pin as input mode
 * - Extracts pull resistor configuration from switch connection type (bits 0-1)
 * - Sets default values for unused GPIO parameters (speed, output type, alternate function)
 * - Stops initialization on first error and returns error status
 */
SWITCH_Status_t SWITCH_enuInit(){
    /* Initialize return status as successful */
    SWITCH_Status_t retStatus = SWITCH_OK;
    
    /* Variable to store GPIO operation status */
    GPIO_Status_t gpioStatus;
    
    /* Temporary GPIO configuration structure */
    GPIO_cfg_t cfg;
    
    /* Iterate through all configured switches */
    for(uint8_t i = 0;i<SWITCH_LEN;i++){
        /* Set GPIO mode to input (switch reads require input mode) */
        cfg.mode = GPIO_MODE_INPUT;
        
        /* Copy port configuration from switch configuration array */
        cfg.port = SWITCHConfigArr[i].port;
        
        /* Extract pull resistor configuration from switch connection type
         * Mask with 0b11 to get only bits 0-1 which represent pull-up/pull-down settings
         * SWITCH_INTERNAL_PULLUP   (0b0001) → GPIO_PULLUP   (0b01)
         * SWITCH_INTERNAL_PULLDOWN (0b0010) → GPIO_PULLDOWN (0b10)
         * SWITCH_EXTERNAL_PULLUP   (0b0100) → GPIO_FLOATING (0b00)
         * SWITCH_EXTERNAL_PULLDOWN (0b1000) → GPIO_FLOATING (0b00) */
        cfg.pull = (GPIO_Pull_t)(SWITCHConfigArr[i].connection & (0b11));
        
        /* Copy pin configuration from switch configuration array */
        cfg.pin  = SWITCHConfigArr[i].pin;
        
        /* Set alternate function to AF0 (not used for basic GPIO input, but required by structure) */
        cfg.alternateFunction = GPIO_AF0;
        
        /* Set output type to push-pull (not used for input mode, but required by structure) */
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

    /* Return final status (OK if all succeeded, or first error encountered) */
    return retStatus;
}


/*
 * Function: SWITCH_enuReadVAl
 * Description: Reads the current debounced state of the specified switch
 *              Implements software debouncing by reading twice with delay between reads
 *              Translates GPIO logic level to switch state based on pull resistor configuration
 * Parameters:
 *   - switchName: Switch identifier from SWITCH_Name_t enum (index into configuration array)
 *   - retStat: Pointer to SWITCH_State_t where result will be stored (PUSHED or RELEASED)
 * Returns: SWITCH_Status_t indicating success or specific error condition
 * 
 * Debouncing algorithm:
 * 1. Read pin value twice with delay between reads
 * 2. If both reads match, value is stable (switch has settled)
 * 3. If readings differ, repeat until stable value is found
 * 4. Translate stable GPIO level to switch state based on pull resistor configuration
 */
SWITCH_Status_t SWITCH_enuReadVAl(SWITCH_Name_t switchName,SWITCH_State_t* retStat){

    /* Initialize status as not OK (pessimistic approach) */
    SWITCH_Status_t status = SWITCH_NOT_OK;

    /* Validate output pointer parameter */
    if(NULL == retStat){
        status = SWITCH_NULL_PTR;
    }else{
        /* Validate switch name is within valid range */
        if(SWITCH_LEN < switchName){
            status = SWITCH_WRONG_NAME;
        }else{

            /* Variables to store GPIO pin readings for debouncing */
            uint8_t readVal = 0;        /* Current pin reading */
            uint8_t tempReadVAl= 1;     /* Previous pin reading (initialized differently to enter loop) */

            /* Variable to store GPIO operation status */
            GPIO_Status_t gpioStatus =  GPIO_NOT_OK;

            /* 
             * Debouncing loop: Continue reading until two consecutive readings match
             * This filters out mechanical bounce where pin rapidly changes state
             * Loop ensures stable reading before proceeding
             */
            while(readVal != tempReadVAl){
                /* First reading - store in temporary variable */
                gpioStatus = GPIO_enuReadPinVal(SWITCHConfigArr[switchName].port,SWITCHConfigArr[switchName].pin,&tempReadVAl);
                
                /* Delay to allow switch to stabilize (debouncing delay) */
                SWITCH_delay(DEBOUNCING_TIME);
                
                /* Second reading - store in main variable */
                gpioStatus = GPIO_enuReadPinVal(SWITCHConfigArr[switchName].port,SWITCHConfigArr[switchName].pin,&readVal);
                
                /* If readVal != tempReadVAl, switch is still bouncing - loop continues */
                /* If readVal == tempReadVAl, switch has stabilized - loop exits */
            }

            /* Check if GPIO read operation was successful */
            if(GPIO_OK != gpioStatus){
                status = SWITCH_ERROR_READ;
            }else{

                /* 
                 * Translate GPIO logic level to switch state based on pull resistor configuration
                 * Pull-up configuration: LOW = pressed (switch connects pin to ground)
                 * Pull-down configuration: HIGH = pressed (switch connects pin to VCC)
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
 * Function: SWITCH_delay
 * Description: Simple busy-wait delay function for debouncing
 *              Creates a software delay by executing empty loop iterations
 * Parameters:
 *   - iterations: Number of loop iterations to execute (volatile prevents optimization)
 * Returns: None
 * 
 * Implementation notes:
 * - Uses volatile parameter to prevent compiler from optimizing away the loop
 * - Delay duration depends on CPU clock speed and compiler optimization level
 * - Not a precise timing mechanism - suitable for debouncing but not accurate delays
 * - Private function (static) - only used internally by this module
 */
static void SWITCH_delay(volatile uint32_t iterations){
        /* Decrement counter until it reaches zero */
        while (iterations--) {
        /* Empty loop body - volatile keyword ensures loop is not optimized away */
    }
}