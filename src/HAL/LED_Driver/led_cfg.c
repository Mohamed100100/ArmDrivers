#include "./HAL/LED_Driver/led.h"

/*
 * LED Configuration Array
 * Contains hardware configuration for all LEDs defined in LED_Name_t enumeration
 * Each entry specifies the GPIO port, pin, active state, and output type
 * Array is indexed using LED_Name_t enum values for type-safe access
 * 
 * Configuration details:
 * - BLACK_PILL_LED: On-board LED, active-low (illuminates when pin is LOW)
 * - KIT_LED_1 to KIT_LED_8: External LEDs, active-high (illuminate when pin is HIGH)
 * - All LEDs use push-pull output for strong drive capability
 */
const LED_cfg_t LedConfigArr[LED_LEN] = {
    /* On-board LED on BlackPill - Port C, Pin 13, Active Low, Push-Pull */
    [BLACK_PILL_LED] = {
        .port = PORT_C,
        .pin  = PIN_13,
        .activeState = LED_ACTIVE_LOW,
        .outputType  = LED_OUTPUT_TYPE_PUSH_PULL
    },
    /* External Kit LED 1 - Port B, Pin 0, Active High, Push-Pull */
    [KIT_LED_1_LED] = {
        .port = PORT_B,
        .pin  = PIN_0,
        .activeState = LED_ACTIVE_HIGH,
        .outputType  = LED_OUTPUT_TYPE_PUSH_PULL
    },
    /* External Kit LED 2 - Port B, Pin 1, Active High, Push-Pull */
    [KIT_LED_2_LED] = {
        .port = PORT_B,
        .pin  = PIN_1,
        .activeState = LED_ACTIVE_HIGH,
        .outputType  = LED_OUTPUT_TYPE_PUSH_PULL
    },
    /* External Kit LED 3 - Port A, Pin 2, Active High, Push-Pull */
    [KIT_LED_3_LED] = {
        .port = PORT_A,
        .pin  = PIN_2,
        .activeState = LED_ACTIVE_HIGH,
        .outputType  = LED_OUTPUT_TYPE_PUSH_PULL
    },
    /* External Kit LED 4 - Port A, Pin 3, Active High, Push-Pull */
    [KIT_LED_4_LED] = {
        .port = PORT_A,
        .pin  = PIN_3,
        .activeState = LED_ACTIVE_HIGH,
        .outputType  = LED_OUTPUT_TYPE_PUSH_PULL
    },
    /* External Kit LED 5 - Port A, Pin 4, Active High, Push-Pull */
    [KIT_LED_5_LED] = {
        .port = PORT_A,
        .pin  = PIN_4,
        .activeState = LED_ACTIVE_HIGH,
        .outputType  = LED_OUTPUT_TYPE_PUSH_PULL
    },
    /* External Kit LED 6 - Port A, Pin 5, Active High, Push-Pull */
    [KIT_LED_6_LED] = {
        .port = PORT_A,
        .pin  = PIN_5,
        .activeState = LED_ACTIVE_HIGH,
        .outputType  = LED_OUTPUT_TYPE_PUSH_PULL
    },
    /* External Kit LED 7 - Port A, Pin 6, Active High, Push-Pull */
    [KIT_LED_7_LED] = {
        .port = PORT_A,
        .pin  = PIN_6,
        .activeState = LED_ACTIVE_HIGH,
        .outputType  = LED_OUTPUT_TYPE_PUSH_PULL
    },
    /* External Kit LED 8 - Port A, Pin 7, Active High, Push-Pull */
    [KIT_LED_8_LED] = {
        .port = PORT_A,
        .pin  = PIN_7,
        .activeState = LED_ACTIVE_HIGH,
        .outputType  = LED_OUTPUT_TYPE_PUSH_PULL
    }
};