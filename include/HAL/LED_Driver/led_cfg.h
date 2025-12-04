#ifndef LED_CFG_H
#define LED_CFG_H

/*
 * Enumeration of all available LED names in the system
 * Each LED has a unique identifier for easy reference in application code
 * LED_LEN must always be the last element to track the total number of LEDs
 */
typedef enum {

    BLACK_PILL_LED,     /* On-board LED on BlackPill development board (typically PC13) */
    KIT_LED_1_LED,      /* External LED #1 on development kit */
    KIT_LED_2_LED,      /* External LED #2 on development kit */
    KIT_LED_3_LED,      /* External LED #3 on development kit */
    KIT_LED_4_LED,      /* External LED #4 on development kit */
    KIT_LED_5_LED,      /* External LED #5 on development kit */
    KIT_LED_6_LED,      /* External LED #6 on development kit */
    KIT_LED_7_LED,      /* External LED #7 on development kit */
    KIT_LED_8_LED,      /* External LED #8 on development kit */
    
    LED_LEN             /* Total number of LEDs - used for array sizing and bounds checking */
}LED_Name_t;

#endif // LED_CFG_H