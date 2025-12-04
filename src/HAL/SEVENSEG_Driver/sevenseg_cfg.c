#include "./HAL/SEVENSEG_Driver/sevenseg.h"

/*
 * Seven-Segment Display Configuration Structure
 * Defines the complete hardware configuration for a single 7-segment display
 * 
 * Hardware Configuration:
 * - All segments connected to GPIO Port A
 * - Sequential pin assignment (PA0 through PA6)
 * - Common anode configuration (active low)
 * - Push-pull output for strong drive capability
 * 
 * Pin Mapping:
 * PA0 → Segment A (top horizontal)
 * PA1 → Segment B (top-right vertical)
 * PA2 → Segment C (bottom-right vertical)
 * PA3 → Segment D (bottom horizontal)
 * PA4 → Segment E (bottom-left vertical)
 * PA5 → Segment F (top-left vertical)
 * PA6 → Segment G (middle horizontal)
 * 
 * Display Layout:
 *      AAA        (PA0)
 *     F   B       (PA5) (PA1)
 *      GGG        (PA6)
 *     E   C       (PA4) (PA2)
 *      DDD        (PA3)
 * 
 * Electrical Configuration:
 * - Active State: ACTIVE_LOW (common anode display)
 *   - Segment ON:  Pin = LOW (0V)
 *   - Segment OFF: Pin = HIGH (3.3V)
 *   - Common anode connected to VCC
 *   - Each segment cathode connected to GPIO pin
 * 
 * - Output Type: PUSH_PULL
 *   - Can actively drive both HIGH and LOW
 *   - No external pull resistors needed
 *   - Suitable for directly driving LED segments with current-limiting resistors
 * 
 * Usage Notes:
 * - Requires 7 current-limiting resistors (typically 220Ω - 470Ω per segment)
 * - Total current depends on number of segments lit (max 7 segments)
 * - Ensure GPIO port can handle total current draw
 * - Common anode should be connected to appropriate voltage (3.3V or 5V)
 */
const SEVSEG_cfg_t SevSegConfigration = {
    /* Segment A - Top horizontal bar */
    .PinA = {.port = SEVSEG_PORT_A,.pin = SEVSEG_PIN_0},
    
    /* Segment B - Top-right vertical bar */
    .PinB = {.port = SEVSEG_PORT_A,.pin = SEVSEG_PIN_1},
    
    /* Segment C - Bottom-right vertical bar */
    .PinC = {.port = SEVSEG_PORT_A,.pin = SEVSEG_PIN_2},
    
    /* Segment D - Bottom horizontal bar */
    .PinD = {.port = SEVSEG_PORT_A,.pin = SEVSEG_PIN_3},
    
    /* Segment E - Bottom-left vertical bar */
    .PinE = {.port = SEVSEG_PORT_A,.pin = SEVSEG_PIN_4},
    
    /* Segment F - Top-left vertical bar */
    .PinF = {.port = SEVSEG_PORT_A,.pin = SEVSEG_PIN_5},
    
    /* Segment G - Middle horizontal bar */
    .PinG = {.port = SEVSEG_PORT_A,.pin = SEVSEG_PIN_6},
    
    /* Push-pull output mode for active drive capability */
    .outputType  = SEVSEG_OUTPUT_TYPE_PUSH_PULL,
    
    /* Active low for common anode display (segment ON when pin is LOW) */
    .activeState = SEVSEG_ACTIVE_LOW,
};