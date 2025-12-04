#include "./LIB/stdtypes.h"

#include "./MCAL/GPIO_Driver/gpio_int.h"

#include "./HAL/SEVENSEG_Driver/sevenseg_cfg.h"
#include "./HAL/SEVENSEG_Driver/sevenseg.h"

/* Logical high level constant for readability */
#define HIGH    (1UL)

/* Logical low level constant for readability */
#define LOW     (0UL)

/* 
 * External declaration of the seven-segment display configuration structure
 * Defined in sevenseg_cfg.c, contains hardware pin mapping and electrical settings
 */
extern const SEVSEG_cfg_t SevSegConfigration;

/*
 * Lookup table mapping logical levels (LOW/HIGH) to GPIO driver values
 * Used for converting between application logic and GPIO driver enums
 * Index 0 (LOW)  → GPIO_LOW
 * Index 1 (HIGH) → GPIO_HIGH
 */
const GPIO_Val_t SevenSegLevel[2] = {
    GPIO_LOW,
    GPIO_HIGH,
};

/*
 * Segment pattern lookup table for decimal digits 0-9
 * Each byte represents which segments should be illuminated for that digit
 * Bit position corresponds to segment: bit 0=A, bit 1=B, ..., bit 6=G
 * 
 * Bit Pattern: [G F E D C B A]
 * 
 * Segment Layout Reference:
 *      AAA        (bit 0)
 *     F   B       (bit 5) (bit 1)
 *      GGG        (bit 6)
 *     E   C       (bit 4) (bit 2)
 *      DDD        (bit 3)
 * 
 * Example for digit '0':
 * - Binary: 0b0111111 = segments A,B,C,D,E,F ON, segment G OFF
 * - Display: Complete outer ring, no middle bar
 * 
 * Index corresponds to the decimal value to display (0-9)
 */
const uint8_t SevenSegValues[10] = {
    0b0111111,   // 0: A,B,C,D,E,F    (all except G - forms '0')
    0b0000110,   // 1: B,C            (right side only - forms '1')
    0b1011011,   // 2: A,B,D,E,G      (top, right-top, middle, left-bottom, bottom - forms '2')
    0b1001111,   // 3: A,B,C,D,G      (top, right side, middle, bottom - forms '3')
    0b1100110,   // 4: B,C,F,G        (left-top, right side, middle - forms '4')
    0b1101101,   // 5: A,C,D,F,G      (top, right-bottom, bottom, left-top, middle - forms '5')
    0b1111101,   // 6: A,C,D,E,F,G    (all except B - forms '6')
    0b0000111,   // 7: A,B,C          (top and right side - forms '7')
    0b1111111,   // 8: A,B,C,D,E,F,G  (all segments - forms '8')
    0b1101111    // 9: A,B,C,D,F,G    (all except E - forms '9')
};


/*
 * Function: SEVSEG_enuInit
 * Description: Initializes the seven-segment display by configuring all segment GPIO pins
 *              Sets up 7 GPIO pins (A through G) as outputs and turns all segments OFF
 * Parameters: None (uses configuration from SevSegConfigration)
 * Returns: SEVSEG_Status_t indicating success or first error encountered
 * 
 * Implementation details:
 * 1. Configure common GPIO settings (output mode, push-pull, no pull, default speed)
 * 2. Iterate through all 7 segment pins (A-G)
 * 3. Initialize each pin as GPIO output
 * 4. Set each pin to OFF state (respecting active high/low configuration)
 * 5. Stop and return error if any GPIO operation fails
 * 
 * The function uses pointer arithmetic to iterate through segment pins in the
 * configuration structure (PinA through PinG are sequential in memory)
 */
SEVSEG_Status_t SEVSEG_enuInit(){
    /* Initialize return status as successful */
    SEVSEG_Status_t retStatus = SEVSEG_OK;
    
    /* Variable to store GPIO operation status */
    GPIO_Status_t gpioStatus;
    
    /* GPIO configuration structure for segment pins */
    GPIO_cfg_t cfg;

    /* Configure common GPIO settings for all segment pins */
    cfg.mode = GPIO_MODE_OUTPUT;                            /* Output mode for driving segments */
    cfg.outputType = SevSegConfigration.outputType;         /* Push-pull or open-drain from config */
    cfg.alternateFunction = GPIO_AF0;                       /* No alternate function (basic GPIO) */
    cfg.pull = GPIO_NO_PULL;                                /* No pull resistors needed for outputs */
    cfg.speed = GPIO_SPEED_DEFAULT;                         /* Default speed sufficient for static display */

    /*
     * Pointer to first segment pin configuration (PinA)
     * Used for pointer arithmetic to iterate through all 7 segment pins
     * Assumes PinA through PinG are sequential in memory (guaranteed by struct layout)
     */
    const SEVSEG_Pinout_t *ptr = &SevSegConfigration.PinA;
    
    /* Iterate through all 7 segments (A through G) */
    for(uint8_t i = 0;i<7;i++){
        /* Set port and pin for current segment using pointer arithmetic */
        cfg.port = (ptr+i)->port;
        cfg.pin = (ptr+i)->pin;
        
        /* Initialize GPIO pin with configured settings */
        gpioStatus = GPIO_enuInit(&cfg);

        /* Check if GPIO initialization was successful */
        if(GPIO_OK != gpioStatus)
        {
            /* Cast GPIO error status to SEVSEG status and exit loop */
            retStatus = (SEVSEG_Status_t)gpioStatus;
            break;
        }else{
            /*
             * Set segment to OFF state
             * 
             * Logic explanation:
             * - LOW = 0, HIGH = 1
             * - If activeState is ACTIVE_HIGH (0): LOW^0 = 0 (GPIO_LOW = OFF)
             * - If activeState is ACTIVE_LOW (1):  LOW^1 = 1 (GPIO_HIGH = OFF)
             * 
             * XOR operation automatically inverts the level for active-low displays
             * This ensures segments are OFF regardless of active state configuration
             */
            gpioStatus = GPIO_enuSetPinVal(cfg.port,cfg.pin,SevenSegLevel[(LOW^SevSegConfigration.activeState)]);

            /* Check if setting pin value was successful */
            if(GPIO_OK != gpioStatus)
            {
                /* Cast GPIO error status to SEVSEG status and exit loop */
                retStatus = (SEVSEG_Status_t)gpioStatus;
                break;
            }else{
                /* Continue to next segment */
            }
        }
    }

    /* Return final initialization status */
    return retStatus;
}

/*
 * Function: SEVSEG_enuDisplayValue
 * Description: Displays a decimal digit (0-9) on the seven-segment display
 *              Looks up segment pattern and applies it to all segment pins
 *              Automatically handles active high/low logic
 * Parameters:
 *   - Displayedvalue: Decimal digit to display (0-9)
 * Returns: SEVSEG_Status_t indicating success or error
 * 
 * Implementation algorithm:
 * 1. Look up segment pattern from SevenSegValues array
 * 2. For each segment (A through G):
 *    a. Extract bit value for current segment (LSB)
 *    b. XOR with activeState to handle active high/low
 *    c. Set GPIO pin to calculated value
 *    d. Right-shift pattern to process next segment
 * 3. Return error if any GPIO operation fails
 * 
 * The XOR logic ensures correct segment control for both common anode and common cathode:
 * - Common Cathode (ACTIVE_HIGH): bit=1, activeState=0 → 1^0=1 → HIGH (ON)
 * - Common Anode (ACTIVE_LOW):    bit=1, activeState=1 → 1^1=0 → LOW (ON)
 */
SEVSEG_Status_t SEVSEG_enuDisplayValue(uint8_t Displayedvalue){
    /* Initialize return status as successful */
    SEVSEG_Status_t retStatus = SEVSEG_OK;
    
    /* Variable to store GPIO operation status */
    GPIO_Status_t gpioStatus;
    
    /*
     * Pointer to first segment pin configuration (PinA)
     * Used for pointer arithmetic to iterate through all 7 segment pins
     */
    const SEVSEG_Pinout_t *ptr = &SevSegConfigration.PinA;
    
    /*
     * Look up segment pattern for the requested digit
     * ActualDisplayedValue holds the bit pattern where:
     * - bit 0 = Segment A state
     * - bit 1 = Segment B state
     * - ...
     * - bit 6 = Segment G state
     * Value of 1 means segment should be ON, 0 means OFF
     */
    uint8_t ActualDisplayedValue = SevenSegValues[Displayedvalue];
    
    /* Iterate through all 7 segments (A through G) */
    for(uint8_t i = 0;i<7;i++){
        /*
         * Set GPIO pin value for current segment
         * 
         * Logic breakdown:
         * 1. (ActualDisplayedValue&0b1): Extract LSB (current segment state, 0 or 1)
         * 2. XOR with activeState: Invert if active-low configuration
         *    - If bit=1 (segment should be ON):
         *      * Active-high: 1^0=1 → HIGH (ON)
         *      * Active-low:  1^1=0 → LOW (ON)
         *    - If bit=0 (segment should be OFF):
         *      * Active-high: 0^0=0 → LOW (OFF)
         *      * Active-low:  0^1=1 → HIGH (OFF)
         * 3. Use result as index into SevenSegLevel[] to get GPIO_Val_t
         * 4. Set GPIO pin to calculated value
         */
        gpioStatus = GPIO_enuSetPinVal((ptr+i)->port,(ptr+i)->pin,SevenSegLevel[(ActualDisplayedValue&0b1)^(SevSegConfigration.activeState)]);
        
        /* Check if GPIO set operation was successful */
        if(GPIO_OK != gpioStatus)
        {
            /* Cast GPIO error status to SEVSEG status and exit loop */
            retStatus = (SEVSEG_Status_t)gpioStatus;
            break;
        }else{
            /* Continue to next segment */
        }
        
        /*
         * Right-shift the pattern by 1 bit to process next segment
         * This moves bit 1 (Segment B) to bit 0 position for next iteration
         * Example:
         * - Initial:      0b1011011 (digit '2')
         * - After shift:  0b0101101 (now bit 0 = Segment B state)
         * - After shift:  0b0010110 (now bit 0 = Segment C state)
         * - etc.
         */
        ActualDisplayedValue = ActualDisplayedValue>>1;
    }

    /* Return final operation status */
    return retStatus;
}