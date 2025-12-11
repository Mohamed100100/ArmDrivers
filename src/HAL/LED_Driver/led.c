
#include "./LIB/stdtypes.h"

#include "./MCAL/GPIO_Driver/gpio_int.h"

#include "./HAL/LED_Driver/led_cfg.h"
#include "./HAL/LED_Driver/led.h"

#define HIGH 1
#define LOW  0

extern const LED_cfg_t LedConfigArr[LED_LEN];

const GPIO_Val_t LedPinVAl[]={
    GPIO_LOW,
    GPIO_HIGH,
};


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
            // continue
        }
    }

    return retStatus;
}

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
