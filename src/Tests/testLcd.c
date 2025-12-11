#include "LIB/stdtypes.h"
#include "HAL/MCU_Driver/mcu.h"
#include "MCAL/SYSTICK_TIMER_Driver/systick.h"
#include "HAL/LCD_Driver/lcd.h"

#include "test.h"


void LcdTest()
{
    MCU_Status_t mcuStatus = MCU_enuInit(NULL);

    SYSTICK_Status_t SistickStatus;
    SistickStatus = SYSTICK_Init(16000000UL, SYSTICK_NO_PRESCALLER);
    SistickStatus = SYSTICK_SetStartValue(10000);
    SYSTICK_StartCount();


    
    LCD_Status_t lcdStatus = LCD_enuSynInit();
    
    if (LCD_OK == lcdStatus)
    {
        // Write "Hello World!"
        LCD_enuSyncWriteCharacter('H');
        LCD_enuSyncWriteCharacter('e');
        LCD_enuSyncWriteCharacter('l');
        LCD_enuSyncWriteCharacter('l');
        LCD_enuSyncWriteCharacter('o');
        LCD_enuSyncWriteCharacter(' ');
        LCD_enuSyncWriteCharacter('W');
        LCD_enuSyncWriteCharacter('o');
        LCD_enuSyncWriteCharacter('r');
        LCD_enuSyncWriteCharacter('l');
        LCD_enuSyncWriteCharacter('d');
        LCD_enuSyncWriteCharacter('!');
    //     LCD_enuSyncWriteCharacter('W');
    //     LCD_enuSyncWriteCharacter('o');
    //     LCD_enuSyncWriteCharacter('r');
    //     LCD_enuSyncWriteCharacter('l');
    //     LCD_enuSyncWriteCharacter('d');
    //     LCD_enuSyncWriteCharacter('d');
    //     LCD_enuSyncWriteCharacter('d');
    //     LCD_enuSyncWriteCharacter('d');
    //     LCD_enuSyncWriteCharacter('d');
    }

    // LCD_enuClearDisplay();
    // LCD_enuSetCursor(LCD_CURSOR_OFF);
    // LCD_enuSetBlink(LCD_BLINK_OFF);
    // LCD_enuSetDisplay(LCD_DISPLAY_OFF);
    // LCD_enuSetDisplay(LCD_DISPLAY_ON);
    // LCD_enuReturnHome();
    // LCD_enuSetCursorPosition(1,15);

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

    LCD_enuSyncCreateCustomChar(0,heart);
    LCD_enuSyncWriteCustomChar(7);

    while(1)
    {
        // Main loop
    }
}