#include "LIB/stdtypes.h"
#include "HAL/MCU_Driver/mcu.h"
#include "HAL/SEVENSEG_Driver/sevenseg.h"
#include "MCAL/SYSTICK_TIMER_Driver/systick.h"

#include "test.h"


void sevsegTest(){

    MCU_Status_t mcuStatus = MCU_enuInit(&MCU_Configs);

    SYSTICK_Status_t systickStatus = SYSTICK_Init(16000000UL,SYSTICK_NO_PRESCALLER);
    systickStatus = SYSTICK_SetStartValue(1000UL); // random vlaue for wiat

    SYSTICK_StartCount();

    SEVSEG_Status_t status = SEVSEG_enuInit();
    int i =0;
    while(1){
        for(i = 0;i<10;i++){
            status = SEVSEG_enuDisplayValue(i);
            systickStatus = SYSTICK_Wait_ms(1000);
        }
    }


} 