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


    
    LCD_Status_t lcdStatus = LCD_enuInit();
    
    if (LCD_OK == lcdStatus)
    {
        // Write "Hello World!"
        LCD_enuWriteCharacter('H');
        LCD_enuWriteCharacter('e');
        LCD_enuWriteCharacter('l');
        LCD_enuWriteCharacter('l');
        LCD_enuWriteCharacter('o');
        LCD_enuWriteCharacter(' ');
        LCD_enuWriteCharacter('W');
        LCD_enuWriteCharacter('o');
        LCD_enuWriteCharacter('r');
        LCD_enuWriteCharacter('l');
        LCD_enuWriteCharacter('d');
        LCD_enuWriteCharacter('!');
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

    LCD_enuCreateCustomChar(7,heart);
    LCD_enuWriteCustomChar(7);

    while(1)
    {
        // Main loop
    }
}