#ifndef SEVSEG_H_
#define SEVSEG_H_

#include "./LIB/stdtypes.h" 
// #include "SEVSEG_cfg.h"

/*
 * Enumeration of possible return status codes for seven-segment display driver functions
 * Values are aligned with GPIO driver error codes for compatibility
 * Used to indicate success, failure, or specific error conditions
 */
typedef enum{
    SEVSEG_OK = 0,                      /* Operation completed successfully */
    SEVSEG_NOT_OK,                      /* General error or operation failed */
    SEVSEG_NULL_PTR = 2,                /* Null pointer passed as parameter (maps to GPIO_NULL_PTR) */
    SEVSEG_WRONG_PORT = 4,              /* Invalid port identifier provided (maps to GPIO_WRONG_PORT) */
    SEVSEG_WRONG_PIN = 5,               /* Invalid pin identifier provided (maps to GPIO_WRONG_PIN) */
    SEVSEG_WRONG_OUTPUT_TYPE = 6,       /* Invalid output type configuration (maps to GPIO_WRONG_OUTPUT_TYPE) */
    SEVSEG_WRONG_SEVSEG_NAME            /* Invalid seven-segment display name/identifier provided */
}SEVSEG_Status_t;


/*
 * Enumeration of available GPIO ports for seven-segment display connection
 * Represents the physical GPIO port where display segments can be connected
 * Corresponds to STM32 GPIO port naming convention
 */
typedef enum{
    SEVSEG_PORT_A,      /* GPIO Port A */
    SEVSEG_PORT_B,      /* GPIO Port B */
    SEVSEG_PORT_C,      /* GPIO Port C */
    SEVSEG_PORT_D,      /* GPIO Port D */
    SEVSEG_PORT_E,      /* GPIO Port E */
    SEVSEG_PORT_H       /* GPIO Port H */
}SEVSEG_Port_t;

/*
 * Enumeration of available GPIO pins within a port
 * Represents the specific pin number (0-15) where a segment can be connected
 * Each GPIO port has 16 pins available
 */
typedef enum{
    SEVSEG_PIN_0,       /* Pin 0 of the selected port */
    SEVSEG_PIN_1,       /* Pin 1 of the selected port */
    SEVSEG_PIN_2,       /* Pin 2 of the selected port */
    SEVSEG_PIN_3,       /* Pin 3 of the selected port */
    SEVSEG_PIN_4,       /* Pin 4 of the selected port */
    SEVSEG_PIN_5,       /* Pin 5 of the selected port */
    SEVSEG_PIN_6,       /* Pin 6 of the selected port */
    SEVSEG_PIN_7,       /* Pin 7 of the selected port */
    SEVSEG_PIN_8,       /* Pin 8 of the selected port */
    SEVSEG_PIN_9,       /* Pin 9 of the selected port */
    SEVSEG_PIN_10,      /* Pin 10 of the selected port */
    SEVSEG_PIN_11,      /* Pin 11 of the selected port */
    SEVSEG_PIN_12,      /* Pin 12 of the selected port */
    SEVSEG_PIN_13,      /* Pin 13 of the selected port */
    SEVSEG_PIN_14,      /* Pin 14 of the selected port */
    SEVSEG_PIN_15       /* Pin 15 of the selected port */
}SEVSEG_Pin_t;

/*
 * Enumeration of seven-segment display active states
 * Defines the logic level required to illuminate a segment
 * Depends on display circuit configuration (common cathode vs common anode)
 */
typedef enum{
    SEVSEG_ACTIVE_HIGH,     /* Segment illuminates when pin is HIGH (3.3V/5V) - common cathode configuration */
    SEVSEG_ACTIVE_LOW,      /* Segment illuminates when pin is LOW (0V) - common anode configuration */
}SEVSEG_ActiveState_t;

/*
 * Enumeration of GPIO output types for seven-segment display control
 * Determines the electrical characteristics of the output pins
 */
typedef enum{
    SEVSEG_OUTPUT_TYPE_PUSH_PULL = 0,   /* Push-pull output - can actively drive both HIGH and LOW */
    SEVSEG_OUTPUT_TYPE_OPEN_DRAIN       /* Open-drain output - can only pull LOW, needs external pull-up for HIGH */
}SEVSEG_OutputType_t;

/*
 * Structure defining the GPIO connection for a single segment pin
 * Specifies both the port and pin number for one segment
 * Used to build complete seven-segment display configuration
 */
typedef struct 
{
    SEVSEG_Port_t port;     /* GPIO port where this segment is connected */
    SEVSEG_Pin_t  pin;      /* GPIO pin number where this segment is connected */
}SEVSEG_Pinout_t;


/*
 * Structure containing complete configuration for a seven-segment display
 * Defines all parameters needed to control a 7-segment display through GPIO
 * 
 * Seven-segment display layout:
 *      AAA
 *     F   B
 *      GGG
 *     E   C
 *      DDD
 * 
 * Each segment (A-G) requires its own GPIO pin connection
 * All segments typically share same active state and output type
 */
typedef struct {
    SEVSEG_Pinout_t PinA;               /* Top horizontal segment connection */
    SEVSEG_Pinout_t PinB;               /* Top-right vertical segment connection */
    SEVSEG_Pinout_t PinC;               /* Bottom-right vertical segment connection */
    SEVSEG_Pinout_t PinD;               /* Bottom horizontal segment connection */
    SEVSEG_Pinout_t PinE;               /* Bottom-left vertical segment connection */
    SEVSEG_Pinout_t PinF;               /* Top-left vertical segment connection */
    SEVSEG_Pinout_t PinG;               /* Middle horizontal segment connection */
    SEVSEG_ActiveState_t activeState;   /* Logic level that illuminates segments (active high/low) */
    SEVSEG_OutputType_t  outputType;    /* GPIO output type (push-pull or open-drain) */
}SEVSEG_cfg_t;



/*
 * Function: SEVSEG_enuInit
 * Description: Initializes all configured seven-segment displays by setting up their GPIO pins
 *              Reads display configurations and configures each segment's GPIO port, pin,
 *              mode, and output type
 * Parameters: None (uses configuration from sevseg_cfg.c)
 * Returns: SEVSEG_Status_t indicating success or specific error condition
 * Note: This function must be called before using any other seven-segment functions
 *       All displays are initialized to blank/off state
 *       Configures 7 GPIO pins per display (segments A through G)
 */
SEVSEG_Status_t SEVSEG_enuInit();

/*
 * Function: SEVSEG_enuDisplayValue
 * Description: Displays a decimal digit (0-9) on the seven-segment display
 *              Automatically handles segment patterns and active state logic
 *              Controls which segments illuminate to form the desired digit
 * Parameters:
 *   - value: Decimal digit to display (typically 0-9, may support hex A-F)
 * Returns: SEVSEG_Status_t indicating success or error (e.g., invalid value)
 * Note: Display must be initialized with SEVSEG_enuInit() before calling this function
 *       Function handles both common cathode and common anode displays automatically
 *       based on activeState configuration
 * 
 * Segment patterns for digits 0-9:
 * 0: A,B,C,D,E,F    (segments lit)
 * 1: B,C            (segments lit)
 * 2: A,B,D,E,G      (segments lit)
 * 3: A,B,C,D,G      (segments lit)
 * 4: B,C,F,G        (segments lit)
 * 5: A,C,D,F,G      (segments lit)
 * 6: A,C,D,E,F,G    (segments lit)
 * 7: A,B,C          (segments lit)
 * 8: A,B,C,D,E,F,G  (all segments lit)
 * 9: A,B,C,D,F,G    (segments lit)
 */
SEVSEG_Status_t SEVSEG_enuDisplayValue(uint8_t value);




#endif