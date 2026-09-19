/******************************************************************************
 * @file    led.h
 * @author  Eng.Gemy
 * @brief   LED Driver Interface Header File
 *          Abstracts on-board BlackPill LED (PC13 active-low) and Kit LEDs
 *          (active-high) over GPIO_Driver. Configuration comes from led_cfg.h.
 * @date    2024
 * @version 1.0
 * @note    LEDs are indexed by LED_Name_t; LED_LEN is sentinel for array size.
 *          Active state inversion handled in TurnON/OFF via LedPinVAl lookup.
 ******************************************************************************/
#ifndef LED_H_
#define LED_H_

#include "./LIB/stdtypes.h" 
#include "led_cfg.h"

/******************************************************************************
 * @brief LED Status Enumeration
 * @details Mirrors GPIO error codes for direct casting. Values 2/4/5/6 align
 *          with GPIO_NULL_PTR/WRONG_PORT/PIN/OUTPUT_TYPE.
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum{
    LED_OK = 0,                         /**< Operation completed successfully */
    LED_NOT_OK,                         /**< General error or operation failed */
    LED_NULL_PTR = 2,                   /**< Null pointer (maps to GPIO_NULL_PTR) */
    LED_WRONG_PORT = 4,                 /**< Invalid port (maps to GPIO_WRONG_PORT) */
    LED_WRONG_PIN = 5,                  /**< Invalid pin (maps to GPIO_WRONG_PIN) */
    LED_WRONG_OUTPUT_TYPE = 6,          /**< Invalid output type (maps to GPIO_WRONG_OUTPUT_TYPE) */
    LED_WRONG_LED_NAME                  /**< Invalid LED name/identifier provided */
}LED_Status_t;


/******************************************************************************
 * @brief GPIO Port Enumeration for LED
 * @details Physical GPIO port where LED anode/cathode is tied.
 *          Mirrors STM32 port naming (A/B/C/D/E/H).
 ******************************************************************************/
typedef enum{
    PORT_A,     /**< GPIO Port A */
    PORT_B,     /**< GPIO Port B */
    PORT_C,     /**< GPIO Port C */
    PORT_D,     /**< GPIO Port D */
    PORT_E,     /**< GPIO Port E */
    PORT_H      /**< GPIO Port H */
}LED_Port_t;

/******************************************************************************
 * @brief GPIO Pin Enumeration for LED
 * @details Pin 0..15 within selected port. Each port has 16 pins.
 ******************************************************************************/
typedef enum{
    PIN_0,      /**< Pin 0 */
    PIN_1,      /**< Pin 1 */
    PIN_2,      /**< Pin 2 */
    PIN_3,      /**< Pin 3 */
    PIN_4,      /**< Pin 4 */
    PIN_5,      /**< Pin 5 */
    PIN_6,      /**< Pin 6 */
    PIN_7,      /**< Pin 7 */
    PIN_8,      /**< Pin 8 */
    PIN_9,      /**< Pin 9 */
    PIN_10,     /**< Pin 10 */
    PIN_11,     /**< Pin 11 */
    PIN_12,     /**< Pin 12 */
    PIN_13,     /**< Pin 13 */
    PIN_14,     /**< Pin 14 */
    PIN_15      /**< Pin 15 */
}LED_Pin_t;

/******************************************************************************
 * @brief LED Active State Enumeration
 * @details Logic level that illuminates the LED. Depends on wiring:
 *          ACTIVE_LOW = common-anode (LOW → ON), HIGH = common-cathode.
 ******************************************************************************/
typedef enum{
    LED_ACTIVE_LOW,     /**< ON at LOW (0V) — common anode (e.g., BlackPill PC13) */
    LED_ACTIVE_HIGH,    /**< ON at HIGH (3.3V) — common cathode (Kit LEDs) */
}LED_ActiveState_t;

/******************************************************************************
 * @brief GPIO Output Type Enumeration for LED
 * @details Push-pull drives both levels; open-drain needs external pull-up.
 ******************************************************************************/
typedef enum{
    LED_OUTPUT_TYPE_PUSH_PULL = 0,  /**< Push-pull — strong high & low */
    LED_OUTPUT_TYPE_OPEN_DRAIN      /**< Open-drain — requires pull-up for high */
}LED_OutputType_t;

/******************************************************************************
 * @brief LED Configuration Structure
 * @details One entry per physical LED in led_cfg.c (LedConfigArr[LED_LEN]).
 ******************************************************************************/
typedef struct {
    LED_Port_t port;                    /**< GPIO port (e.g., PORT_C) */
    LED_Pin_t  pin;                     /**< GPIO pin (e.g., PIN_13) */
    LED_ActiveState_t activeState;      /**< Level that turns LED ON */
    LED_OutputType_t  outputType;       /**< Push-pull / open-drain */
}LED_cfg_t;



/******************************************************************************
 * @brief Initialize all configured LEDs
 * @details Reads LedConfigArr, configures each GPIO as OUTPUT (push-pull/
 *          open-drain, no pull, default speed) via GPIO_enuInit. Leaves LEDs
 *          in hardware reset (OFF until TurnON).
 * @param None (uses led_cfg.c table)
 * @return LED_Status_t LED_OK if all succeed, else first GPIO error mapped.
 * @note  Must be called once after MCU/RCC init before any other LED API.
 * @author Eng.Gemy
 ******************************************************************************/
LED_Status_t LED_vdInit();

/******************************************************************************
 * @brief Turn ON the specified LED
 * @details Sets GPIO to active level: if ACTIVE_HIGH → GPIO_HIGH else LOW.
 *          Uses LedPinVAl[!(HIGH ^ activeState)].
 * @param[in] ledName LED identifier from led_cfg.h (BLACK_PILL_LED, KIT_LED_1_LED…)
 * @return LED_Status_t LED_OK on success, LED_WRONG_LED_NAME if out of range
 * @author Eng.Gemy
 ******************************************************************************/
LED_Status_t LED_vdTurnON(LED_Name_t);

/******************************************************************************
 * @brief Turn OFF the specified LED
 * @details Sets GPIO to inactive level (!(LOW ^ activeState)).
 * @param[in] ledName LED identifier
 * @return LED_Status_t LED_OK / LED_WRONG_LED_NAME
 * @author Eng.Gemy
 ******************************************************************************/
LED_Status_t LED_vdTurnOFF(LED_Name_t);

/******************************************************************************
 * @brief Toggle the specified LED (ON→OFF, OFF→ON)
 * @details Reads ODR via GPIO_enuFlipPinVal (XOR). Physical toggle regardless
 *          of activeState.
 * @param[in] ledName LED identifier
 * @return LED_Status_t LED_OK / LED_WRONG_LED_NAME
 * @note  Useful for blink; keep period > a few ms for visibility.
 * @author Eng.Gemy
 ******************************************************************************/
LED_Status_t LED_vdToggle(LED_Name_t);



#endif
