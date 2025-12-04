#ifndef LED_H_
#define LED_H_

#include "./LIB/stdtypes.h" 
#include "led_cfg.h"

/*
 * Enumeration of possible return status codes for LED driver functions
 * Values are aligned with GPIO driver error codes for compatibility
 * Used to indicate success, failure, or specific error conditions
 */
typedef enum{
    LED_OK = 0,                         /* Operation completed successfully */
    LED_NOT_OK,                         /* General error or operation failed */
    LED_NULL_PTR = 2,                   /* Null pointer passed as parameter (maps to GPIO_NULL_PTR) */
    LED_WRONG_PORT = 4,                 /* Invalid port identifier provided (maps to GPIO_WRONG_PORT) */
    LED_WRONG_PIN = 5,                  /* Invalid pin identifier provided (maps to GPIO_WRONG_PIN) */
    LED_WRONG_OUTPUT_TYPE = 6,          /* Invalid output type configuration (maps to GPIO_WRONG_OUTPUT_TYPE) */
    LED_WRONG_LED_NAME                  /* Invalid LED name/identifier provided */
}LED_Status_t;


/*
 * Enumeration of available GPIO ports for LED connection
 * Represents the physical GPIO port where an LED can be connected
 * Corresponds to STM32 GPIO port naming convention
 */
typedef enum{
    PORT_A,     /* GPIO Port A */
    PORT_B,     /* GPIO Port B */
    PORT_C,     /* GPIO Port C */
    PORT_D,     /* GPIO Port D */
    PORT_E,     /* GPIO Port E */
    PORT_H      /* GPIO Port H */
}LED_Port_t;

/*
 * Enumeration of available GPIO pins within a port
 * Represents the specific pin number (0-15) where an LED can be connected
 * Each GPIO port has 16 pins available
 */
typedef enum{
    PIN_0,      /* Pin 0 of the selected port */
    PIN_1,      /* Pin 1 of the selected port */
    PIN_2,      /* Pin 2 of the selected port */
    PIN_3,      /* Pin 3 of the selected port */
    PIN_4,      /* Pin 4 of the selected port */
    PIN_5,      /* Pin 5 of the selected port */
    PIN_6,      /* Pin 6 of the selected port */
    PIN_7,      /* Pin 7 of the selected port */
    PIN_8,      /* Pin 8 of the selected port */
    PIN_9,      /* Pin 9 of the selected port */
    PIN_10,     /* Pin 10 of the selected port */
    PIN_11,     /* Pin 11 of the selected port */
    PIN_12,     /* Pin 12 of the selected port */
    PIN_13,     /* Pin 13 of the selected port */
    PIN_14,     /* Pin 14 of the selected port */
    PIN_15      /* Pin 15 of the selected port */
}LED_Pin_t;

/*
 * Enumeration of LED active states
 * Defines the logic level required to turn the LED ON
 * Depends on LED circuit configuration (common cathode vs common anode)
 */
typedef enum{
    LED_ACTIVE_LOW,     /* LED turns ON when pin is LOW (0V) - common anode configuration */
    LED_ACTIVE_HIGH,    /* LED turns ON when pin is HIGH (3.3V/5V) - common cathode configuration */
}LED_ActiveState_t;

/*
 * Enumeration of GPIO output types for LED control
 * Determines the electrical characteristics of the output pin
 */
typedef enum{
    LED_OUTPUT_TYPE_PUSH_PULL = 0,  /* Push-pull output - can actively drive both HIGH and LOW */
    LED_OUTPUT_TYPE_OPEN_DRAIN      /* Open-drain output - can only pull LOW, needs external pull-up for HIGH */
}LED_OutputType_t;

/*
 * Structure containing complete configuration for a single LED
 * Defines all parameters needed to control an LED through GPIO
 */
typedef struct {
    LED_Port_t port;                    /* GPIO port where LED is connected (e.g., PORT_A) */
    LED_Pin_t  pin;                     /* GPIO pin number where LED is connected (e.g., PIN_5) */
    LED_ActiveState_t activeState;      /* Logic level that turns LED ON (active high/low) */
    LED_OutputType_t  outputType;       /* GPIO output type (push-pull or open-drain) */
}LED_cfg_t;



/*
 * Function: LED_vdInit
 * Description: Initializes all configured LEDs by setting up their GPIO pins
 *              Reads LED configurations from led_cfg.c and configures each LED's
 *              GPIO port, pin, mode, and output type
 * Parameters: None (uses configuration from led_cfg.c)
 * Returns: LED_Status_t indicating success or specific error condition
 * Note: This function must be called before using any other LED functions
 *       All LEDs are initialized to OFF state
 */
LED_Status_t LED_vdInit();

/*
 * Function: LED_vdTurnON
 * Description: Turns ON the specified LED by setting its GPIO pin to the active state
 *              Automatically handles active-high and active-low LED configurations
 * Parameters:
 *   - LED_Name_t: LED identifier from led_cfg.h (e.g., LED_RED, LED_GREEN)
 * Returns: LED_Status_t indicating success or error (e.g., invalid LED name)
 * Note: LED must be initialized with LED_vdInit() before calling this function
 */
LED_Status_t LED_vdTurnON(LED_Name_t);

/*
 * Function: LED_vdTurnOFF
 * Description: Turns OFF the specified LED by setting its GPIO pin to the inactive state
 *              Automatically handles active-high and active-low LED configurations
 * Parameters:
 *   - LED_Name_t: LED identifier from led_cfg.h (e.g., LED_RED, LED_GREEN)
 * Returns: LED_Status_t indicating success or error (e.g., invalid LED name)
 * Note: LED must be initialized with LED_vdInit() before calling this function
 */
LED_Status_t LED_vdTurnOFF(LED_Name_t);

/*
 * Function: LED_vdToggle
 * Description: Toggles the state of the specified LED (ON→OFF or OFF→ON)
 *              Reads current pin state and inverts it
 * Parameters:
 *   - LED_Name_t: LED identifier from led_cfg.h (e.g., LED_RED, LED_GREEN)
 * Returns: LED_Status_t indicating success or error (e.g., invalid LED name)
 * Note: LED must be initialized with LED_vdInit() before calling this function
 *       Useful for blinking effects or state indication
 */
LED_Status_t LED_vdToggle(LED_Name_t);



#endif