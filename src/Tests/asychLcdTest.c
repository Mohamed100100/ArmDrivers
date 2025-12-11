#include "LIB/stdtypes.h"
#include "HAL/MCU_Driver/mcu.h"
#include "HAL/LCD_Driver/lcd.h"
#include "OS/schedule.h"

#include "test.h"


void asynLcdTestCBF();
static SCHED_Runnable_t testAsynLcdRunnable ={
    .CBF = asynLcdTestCBF,
    .Periodicity_ms = 1000,
    .FirstDalay_ms = 0,
    .Args = NULL,
    .Priority = 8
};

uint8_t counter = 0;
uint8_t heart[8] = {
    0b00000,
    0b01010,
    0b11111,
    0b11111,
    0b01110,
    0b00100,
    0b00000,
    0b00000
};
void AsynchLcdTest()
{
    MCU_Status_t mcuStatus = MCU_enuInit(NULL);

    SCHED_enuInit(1,16000000UL);

    LCD_Status_t lcdStat = LCD_enuAsynInit();

    SCHED_enuRegisterRunnable(&testAsynLcdRunnable);
    
    SCHED_enuStart();
    
}

void asynLcdTestCBF(){
    LCD_Status_t lcdStat = LCD_BUSY;

    if(counter == 0){
        lcdStat = LCD_enuAsynCreateCustomChar(7, heart);
        if(lcdStat == LCD_OK){
            counter++;
        }
    }
    // }

    if(counter == 1){
        lcdStat = LCD_enuAsynWriteString("Hello");
        if(lcdStat == LCD_OK){
            counter++;
        }
    }
    if(counter == 2){
        // lcdStat = LCD_AsynDisplayCustomChar(7);
        lcdStat = LCD_AsynDisplayCustomCharAtPosition(7,1,10);

        if(lcdStat == LCD_OK){
            counter++;
        }
    }
    
}