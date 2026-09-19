/******************************************************************************
 * @file    led.c
 * @author  Eng.Gemy
 * @brief   LED Driver Implementation File
 *          This file contains the implementation of all LED driver functions
 *          for STM32 BlackPill + Kit shield. It maps LED_Name_t to GPIO pins
 *          via LedConfigArr and handles active-high/low inversion.
 * @date    2024
 * @version 1.0
 * @note    Requires GPIO_Driver and RCC clock already enabled for ports.
 *          Active-low LED (e.g., PC13) is driven by GPIO_LOW to turn ON.
 ******************************************************************************/

#include "./LIB/stdtypes.h"
#include "./MCAL/GPIO_Driver/gpio_int.h"
#include "./HAL/LED_Driver/led_cfg.h"
#include "./HAL/LED_Driver/led.h"

/* Logical level constants for XOR inversion logic */
#define HIGH 1  /**< Logic HIGH (1) */
#define LOW  0  /**< Logic LOW (0) */

/**
 * @brief External LED configuration table defined in led_cfg.c
 * @note  Size is LED_LEN (BLACK_PILL_LED + 8 Kit LEDs)
 */
extern const LED_cfg_t LedConfigArr[LED_LEN];

/**
 * @brief Lookup table: index → GPIO_Val_t used by TurnON/OFF
 * @details Index 0 = GPIO_LOW, Index 1 = GPIO_HIGH.
 *          Expression !(HIGH ^ activeState) selects correct level.
 */
const GPIO_Val_t LedPinVAl[]={
    GPIO_LOW,
    GPIO_HIGH,
};

/******************************************************************************
 * @brief Initialize all configured LEDs
 * @details Iterates over LedConfigArr[LED_LEN], configures each pin as
 *          GPIO_MODE_OUTPUT, PUSH_PULL/OPEN_DRAIN per config, no pull,
 *          default speed. Sets pins to OFF state (driver leaves OFF; caller
 *          may turn ON). Validates each GPIO_enuInit return.
 * @param None (uses LedConfigArr)
 * @return LED_Status_t LED_OK if all succeed, else first GPIO error mapped
 *         to LED_Status_t, or LED_WRONG_* if config invalid.
 * @note  Must be called once after MCU/RCC init before any TurnON/OFF/Toggle.
 *        Enables RCC port clock internally via GPIO init path.
 * @author Eng.Gemy
 ******************************************************************************/
LED_Status_t LED_vdInit(){
    LED_Status_t retStatus = LED_OK;
    GPIO_Status_t gpioStatus;
    GPIO_cfg_t cfg;

    for(uint8_t i = 0;i<LED_LEN;i++){
        cfg.mode = GPIO_MODE_OUTPUT;
        cfg.port = LedConfigArr[i].port;
        cfg.outputType = LedConfigArr[i].outputType;
        cfg.pin = LedConfigArr[i].pin;
        cfg.alternateFunction = GPIO_AF0;
        cfg.pull = GPIO_NO_PULL;
        cfg.speed = GPIO_SPEED_DEFAULT;
        gpioStatus = GPIO_enuInit(&cfg);

        if(GPIO_OK != gpioStatus)
        {
            retStatus = (LED_Status_t)gpioStatus;
            break;
        }else{
            // continue to next LED
        }
    }

    return retStatus;
}

/******************************************************************************
 * @brief Turn ON the specified LED
 * @details Translates logical ON to electrical level:
 *          activeState == HIGH → GPIO_HIGH, else LOW. Uses LedPinVAl[!(HIGH^activeState)].
 * @param[in] ledName LED identifier from LED_Name_t (BLACK_PILL_LED, KIT_LED_1_LED …)
 * @return LED_Status_t LED_OK on success,
 *         LED_WRONG_LED_NAME if ledName >= LED_LEN,
 *         otherwise GPIO mapped error.
 * @note  Requires LED_vdInit() already called.
 * @author Eng.Gemy
 ******************************************************************************/
LED_Status_t LED_vdTurnON(LED_Name_t ledName){

    LED_Status_t status = LED_NOT_OK;
    if(ledName >= LED_LEN){
        status = LED_WRONG_LED_NAME;
    }else{
        status = (LED_Status_t)GPIO_enuSetPinVal(LedConfigArr[ledName].port,
                                                 LedConfigArr[ledName].pin,
                                                 LedPinVAl[!(HIGH^LedConfigArr[ledName].activeState)]);
    }

    return status;
}

/******************************************************************************
 * @brief Turn OFF the specified LED
 * @details Opposite of TurnON: uses !(LOW ^ activeState) → inactive level.
 * @param[in] ledName LED identifier from LED_Name_t
 * @return LED_Status_t LED_OK on success, LED_WRONG_LED_NAME if out of range
 * @author Eng.Gemy
 ******************************************************************************/
LED_Status_t LED_vdTurnOFF(LED_Name_t ledName){

    LED_Status_t status = LED_NOT_OK;
    if(ledName >= LED_LEN){
        status = LED_WRONG_LED_NAME;
    }else{
        status = (LED_Status_t) GPIO_enuSetPinVal(LedConfigArr[ledName].port,
                                                  LedConfigArr[ledName].pin,
                                                  LedPinVAl[!(LOW^LedConfigArr[ledName].activeState)]);
    }

    return status;
}

/******************************************************************************
 * @brief Toggle the specified LED (ON→OFF, OFF→ON)
 * @details Delegates to GPIO_enuFlipPinVal which XORs ODR.
 *          Works regardless of activeState (physical toggle).
 * @param[in] ledName LED identifier
 * @return LED_Status_t LED_OK / LED_WRONG_LED_NAME / GPIO mapped error
 * @note  Useful for blink riddles; ensure scheduler period > GPIO settle time.
 * @author Eng.Gemy
 ******************************************************************************/
LED_Status_t LED_vdToggle(LED_Name_t ledName){
    
    LED_Status_t status = LED_NOT_OK;
    if(ledName >= LED_LEN){
        status = LED_WRONG_LED_NAME;
    }else{
        status = (LED_Status_t) GPIO_enuFlipPinVal(LedConfigArr[ledName].port,
                                                  LedConfigArr[ledName].pin);
    }

    return status;
}
