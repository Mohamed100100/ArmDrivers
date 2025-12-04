#ifndef SWITCH_H
#define SWITCH_H

#include "switch_cfg.h"



/*
 * Enumeration of possible return status codes for switch driver functions
 * Used to indicate success, failure, or specific error conditions
 */
typedef enum{
    SWITCH_OK = 0,              /* Operation completed successfully */
    SWITCH_NOT_OK,              /* General error or operation failed */
    SWITCH_NULL_PTR,            /* Null pointer passed as parameter */
    SWITCH_ERROR,               /* Generic error condition */
    SWITCH_WRONG_NAME,          /* Invalid switch name/identifier provided */
    SWITCH_ERROR_READ,          /* Error occurred while reading switch state */
    SWITCH_ERROR_SCHEDUALE      /* Error in scheduling/timing operations (typo: should be SCHEDULE) */
}SWITCH_Status_t;

/*
 * Enumeration of available GPIO ports for switch connection
 * Represents the physical GPIO port where a switch can be connected
 * Corresponds to STM32 GPIO port naming convention
 */
typedef enum{
    SWITCH_PORT_A,      /* GPIO Port A */
    SWITCH_PORT_B,      /* GPIO Port B */
    SWITCH_PORT_C,      /* GPIO Port C */
    SWITCH_PORT_D,      /* GPIO Port D */
    SWITCH_PORT_E,      /* GPIO Port E */
    SWITCH_PORT_H       /* GPIO Port H */
}SWITCH_Port_t;

/*
 * Enumeration of available GPIO pins within a port
 * Represents the specific pin number (0-15) where a switch can be connected
 * Each GPIO port has 16 pins available
 */
typedef enum{
    SWITCH_PIN_0,       /* Pin 0 of the selected port */
    SWITCH_PIN_1,       /* Pin 1 of the selected port */
    SWITCH_PIN_2,       /* Pin 2 of the selected port */
    SWITCH_PIN_3,       /* Pin 3 of the selected port */
    SWITCH_PIN_4,       /* Pin 4 of the selected port */
    SWITCH_PIN_5,       /* Pin 5 of the selected port */
    SWITCH_PIN_6,       /* Pin 6 of the selected port */
    SWITCH_PIN_7,       /* Pin 7 of the selected port */
    SWITCH_PIN_8,       /* Pin 8 of the selected port */
    SWITCH_PIN_9,       /* Pin 9 of the selected port */
    SWITCH_PIN_10,      /* Pin 10 of the selected port */
    SWITCH_PIN_11,      /* Pin 11 of the selected port */
    SWITCH_PIN_12,      /* Pin 12 of the selected port */
    SWITCH_PIN_13,      /* Pin 13 of the selected port */
    SWITCH_PIN_14,      /* Pin 14 of the selected port */
    SWITCH_PIN_15       /* Pin 15 of the selected port */
}SWITCH_Pin_t;

/*
 * Enumeration of switch connection/pull resistor configurations
 * Determines the electrical configuration and default state of the switch input
 * Values are bit-encoded for potential combination or masking operations
 */
typedef enum{
    SWITCH_INTERNAL_PULLUP   = 0b0001,      /* Internal pull-up resistor enabled (pin HIGH when switch open, LOW when closed) */
    SWITCH_INTERNAL_PULLDOWN = 0b0010,      /* Internal pull-down resistor enabled (pin LOW when switch open, HIGH when closed) */
    SWITCH_EXTERNAL_PULLUP   = 0b0100,      /* External pull-up resistor used, pin configured as floating input */
    SWITCH_EXTERNAL_PULLDOWN = 0b1000       /* External pull-down resistor used, pin configured as floating input */
}SWITCH_Connection_t;

/*
 * Structure containing complete configuration for a single switch
 * Defines all parameters needed to read a switch through GPIO
 */
typedef struct {
    SWITCH_Port_t port;                 /* GPIO port where switch is connected (e.g., SWITCH_PORT_B) */
    SWITCH_Pin_t  pin;                  /* GPIO pin number where switch is connected (e.g., SWITCH_PIN_4) */
    SWITCH_Connection_t connection;     /* Pull resistor configuration (internal/external, pull-up/pull-down) */
}SWITCH_cfg_t;



/*
 * Enumeration of possible switch physical states
 * Represents the current mechanical state of the switch
 * Independent of electrical logic level (depends on pull-up/pull-down configuration)
 */
typedef enum{
    SWITCH_PUSHED,      /* Switch is currently pressed/closed */
    SWITCH_RELEASED,    /* Switch is currently released/open */
}SWITCH_State_t;

/*
 * Function: SWITCH_enuInit
 * Description: Initializes all configured switches by setting up their GPIO pins
 *              Reads switch configurations from switch_cfg.c and configures each switch's
 *              GPIO port, pin, mode, and pull resistor settings
 * Parameters: None (uses configuration from switch_cfg.c)
 * Returns: SWITCH_Status_t indicating success or specific error condition
 * Note: This function must be called before using any other switch functions
 *       Configures GPIO pins as inputs with appropriate pull-up/pull-down resistors
 */
SWITCH_Status_t SWITCH_enuInit();

/*
 * Function: SWITCH_enuReadVAl
 * Description: Reads the current state of the specified switch with debouncing
 *              Implements debouncing algorithm to filter out mechanical bounce
 *              Translates GPIO logic level to switch state (PUSHED/RELEASED)
 *              based on pull resistor configuration
 * Parameters:
 *   - SWITCH_Name_t: Switch identifier from switch_cfg.h (e.g., SWITCH1_ON_KIT)
 *   - SWITCH_State_t*: Pointer to store the switch state (PUSHED or RELEASED)
 * Returns: SWITCH_Status_t indicating success or error (e.g., invalid switch name, null pointer)
 * Note: Switch must be initialized with SWITCH_enuInit() before calling this function
 *       Function accounts for pull-up/pull-down configuration when determining state
 *       For pull-up: LOW = PUSHED, HIGH = RELEASED
 *       For pull-down: HIGH = PUSHED, LOW = RELEASED
 */
SWITCH_Status_t SWITCH_enuReadVAl(SWITCH_Name_t,SWITCH_State_t*);

#endif /* SWITCH_H */