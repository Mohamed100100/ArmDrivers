
#include "./HAL/SWITCH_Driver/switch.h"

/*
 * Switch Configuration Array
 * Contains hardware configuration for all switches defined in SWITCH_Name_t enumeration
 * Each entry specifies the GPIO port, pin, and connection type (pull-up/pull-down/floating)
 * Array is indexed using SWITCH_Name_t enum values for type-safe access
 * 
 * Configuration details:
 * - SWITCH1_ON_KIT: External switch on kit, uses internal pull-up resistor
 *   When switch is pressed, pin reads LOW; when released, pin reads HIGH
 */
const SWITCH_cfg_t SWITCHConfigArr[SWITCH_LEN] = {
    /* External Kit Switch 1 - Port B, Pin 4, Internal Pull-Up */
    [SWITCH1_ON_KIT] = {
        .port = SWITCH_PORT_B,
        .pin  = SWITCH_PIN_4,
        .connection = SWITCH_INTERNAL_PULLUP
    }
};