
#include "LIB/stdtypes.h"
#include "HAL/MCU_Driver/mcu.h"
#include "HAL/SWITCH_Driver/switch.h"
#include "HAL/LED_Driver/led.h"
#include "OS/schedule.h"
#include "MCAL/SYSTICK_TIMER_Driver/systick.h"

#include "test.h"
void toggleLeds();
static SCHED_Runnable_t testSwitchRunnable ={
    .CBF = toggleLeds,
    .Periodicity_ms = 50,
    .FirstDalay_ms = 0,
    .Args = NULL,
    .Priority = 2
};

void SwitchTest(){
    MCU_Status_t mcuStatus = MCU_enuInit(&MCU_Configs);

    SCHED_enuInit(1,16000000UL);
    
    LED_Status_t ledStat =  LED_vdInit();

    SCHED_enuRegisterRunnable(&testSwitchRunnable);
    
    SWITCH_enuInit();

    SCHED_enuStart();

}


void toggleLeds(){
    SWITCH_State_t stat = SWITCH_RELEASED;
    volatile SWITCH_Status_t Switchstatus;
    SYSTICK_Status_t SistickStatus;
    LED_Status_t ledStat;


    Switchstatus = SWITCH_enuReadVAl(SWITCH1_ON_KIT,&stat);
    
    if(SWITCH_PUSHED == stat)
    {
        ledStat = LED_vdToggle(KIT_LED_1_LED);
            
        ledStat= LED_vdToggle(KIT_LED_2_LED);
    }

}
