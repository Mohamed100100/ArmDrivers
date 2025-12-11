/******************************************************************************
 * @file    lcd.c
 * @brief   LCD 16x2 Driver Implementation (HD44780 Compatible)
 * @details This driver supports 8-bit mode operation for character LCD displays
 *          Features: Character display, custom characters, cursor control
 * @author  Eng.Gemy    
 ******************************************************************************/

#include "LIB/stdtypes.h"
#include <string.h>
#include "OS/schedule.h"
#include "MCAL/GPIO_Driver/gpio_int.h"
#include "MCAL/SYSTICK_TIMER_Driver/systick.h"
#include "HAL/LCD_Driver/lcd_queue.h"
#include "HAL/LCD_Driver/lcd.h"

#define NUMBER_OF_BITS_IN_8_MODE (11UL)
#define COLUMN_LENGTH            (16UL)
#define RETURN_HOME_COMMAND      (0b10UL)
#define CLEAR_DISPLAY_COMMAND    (0b1UL)
#define FONT_BIT_POSITION        (2UL)
#define LINEDISPLAY_BIT_POSITION (3UL)
#define BITOPEARATION_BIT_POSITION (4UL)
#define DISPLAY_BIT_POSITION     (2UL)
#define CURSOR_BIT_POSITION      (1UL)
#define INCREMENT_BIT_POSITION    (1UL)
#define FUNCTION_SET_MASK        (0b100000UL)
#define DISPLAY_CONTROL_MASK     (0b1000UL)
#define ENTRY_MODE_MASK          (0b100UL)
#define CGRAM_ADDRESS_MASK       (0b01000000UL)
#define DDRAM_ADDRESS_MASK       (0b010000000UL)
#define ROW_0_OFFSET             (0UL)
#define ROW_1_OFFSET             (0x40UL)
#define LOCATION_MASK            (7UL)
#define SPECIAL_CHAR_LENGHT      (8UL)

/******************************************************************************
 * GLOBAL VARIABLES AND CONSTANTS
 ******************************************************************************/

// typedef enum {
//     ALL_BITS    = 0,
//     HIGH_NIBBLE = 4,
//     LOW_NIBBLE  = 0
// }Bits_t;


typedef enum {
    LCD_NO_ACTION,
    LCD_INIT,
    LCD_WRITE_STRING,
    LCD_CREATE_CUSTOM_CHAR,
}LCD_Asyn_States_t;

typedef enum {
    INIT_8BIT_START_SEQUANCE_HIGH,
    INIT_8BIT_START_SEQUANCE_LOW, 
    INIT_8BIT_FUNCTION_SET_HIGH,
    INIT_8BIT_FUNCTION_SET_LOW,
    INIT_8BIT_DISPLAY_ON_HIGH,
    INIT_8BIT_DISPLAY_ON_LOW,
    INIT_8BIT_CLEAR_DISPLAY_HIGH,
    INIT_8BIT_CLEAR_DISPLAY_LOW,
    INIT_8BIT_ENTRY_MODE_HIGH,
    INIT_8BIT_ENTRY_MODE_LOW,

    INIT_4BIT_HIGH_NIBBLE_START_SEQUANCE_HIGH,
    INIT_4BIT_HIGH_NIBBLE_START_SEQUANCE_LOW,
    INIT_4BIT_SWITCH_TO_4BIT_MODE_HIGH,
    INIT_4BIT_SWITCH_TO_4BIT_MODE_LOW,
    INIT_4BIT_HIGH_NIBBLE_FUNCTION_SET_HIGH,
    INIT_4BIT_HIGH_NIBBLE_FUNCTION_SET_LOW,
    INIT_4BIT_HIGH_NIBBLE_DISPLAY_ON_HIGH,
    INIT_4BIT_HIGH_NIBBLE_DISPLAY_ON_LOW,
    INIT_4BIT_HIGH_NIBBLE_CLEAR_DISPLAY_HIGH,
    INIT_4BIT_HIGH_NIBBLE_CLEAR_DISPLAY_LOW,
    INIT_4BIT_HIGH_NIBBLE_ENTRY_MODE_HIGH,
    INIT_4BIT_HIGH_NIBBLE_ENTRY_MODE_LOW,
    INIT_4BIT_LOW_NIBBLE_FUNCTION_SET_HIGH,
    INIT_4BIT_LOW_NIBBLE_FUNCTION_SET_LOW,
    INIT_4BIT_LOW_NIBBLE_DISPLAY_ON_HIGH,
    INIT_4BIT_LOW_NIBBLE_DISPLAY_ON_LOW,
    INIT_4BIT_LOW_NIBBLE_CLEAR_DISPLAY_HIGH,
    INIT_4BIT_LOW_NIBBLE_CLEAR_DISPLAY_LOW,
    INIT_4BIT_LOW_NIBBLE_ENTRY_MODE_HIGH,
    INIT_4BIT_LOW_NIBBLE_ENTRY_MODE_LOW,
    INIT_DONE,
    INIT_FAILED
}LCD_InitSeq_t;


// typedef enum{
//     WRITE_CHAR_8BIT_HIGH,
//     WRITE_CHAR_8BIT_LOW,
//     WRITE_CHAR_4BIT_HIGH_NIBBLE_HIGH,
//     WRITE_CHAR_4BIT_HIGH_NIBBLE_LOW,
//     WRITE_CHAR_4BIT_LOW_NIBBLE_HIGH,
//     WRITE_CHAR_4BIT_LOW_NIBBLE_LOW,
//     WRITE_CHAR_DONE
// }LCD_WriteSeq_t;

typedef enum {
    CREATE_CUSTOM_CHAR_8BIT_CURSOR_HIGH,
    CREATE_CUSTOM_CHAR_8BIT_CURSOR_LOW,
    CREATE_CUSTOM_CHAR_8BIT_HIGH,
    CREATE_CUSTOM_CHAR_8BIT_LOW,
    CREATE_CUSTOM_CHAR_8BIT_SET_CURSOR_DDRAM_HIGH,
    CREATE_CUSTOM_CHAR_8BIT_SET_CURSOR_DDRAM_LOW,

    CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_CURSOR_HIGH,
    CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_CURSOR_LOW,
    CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_CURSOR_HIGH,
    CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_CURSOR_LOW,
    CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_HIGH,
    CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_LOW,
    CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_HIGH,
    CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_LOW,
    CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_SET_CURSOR_DDRAM_HIGH,
    CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_SET_CURSOR_DDRAM_LOW,
    CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_SET_CURSOR_DDRAM_HIGH,
    CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_SET_CURSOR_DDRAM_LOW,

    CREATE_CUSTOM_CHAR_DONE
}LCD_CreateCustomCharSeq_t;

typedef enum{
    WRITE_STRING_8BIT_CURSOR_HIGH,
    WRITE_STRING_8BIT_CURSOR_LOW,
    WRITE_STRING_8_BIT_HIGH,
    WRITE_STRING_8_BIT_LOW,

    WRITE_STRING_4BIT_HIGH_NIBBLE_CURSOR_HIGH,
    WRITE_STRING_4BIT_HIGH_NIBBLE_CURSOR_LOW,
    WRITE_STRING_4BIT_LOW_NIBBLE_CURSOR_HIGH,
    WRITE_STRING_4BIT_LOW_NIBBLE_CURSOR_LOW,
    WRITE_STRING_4_BIT_HIGH_NIBBLE_HIGH,
    WRITE_STRING_4_BIT_HIGH_NIBBLE_LOW,
    WRITE_STRING_4_BIT_LOW_NIBBLE_HIGH,
    WRITE_STRING_4_BIT_LOW_NIBBLE_LOW,

    WRITE_STRING_DONE
}LCD_WriteStringSeq_t;


/**
 * @brief GPIO pin value lookup table
 * @details Maps boolean values (0,1) to GPIO states (LOW, HIGH)
 *          Index 0 = GPIO_LOW, Index 1 = GPIO_HIGH
 */
const GPIO_Val_t LCDpinVAl[] = {
    GPIO_LOW,   /* Index 0 - Logic 0 */
    GPIO_HIGH,  /* Index 1 - Logic 1 */
};

/**
 * @brief Static variables to track cursor position
 * @details These variables maintain the current cursor row and column
 *          Updated automatically by write and position functions
 *          Range: Row (0-1), Column (0-15) for 16x2 LCD
 */
static uint8_t LCD_CurrentRow = 0;  /* Current row position (0 or 1) */
static uint8_t LCD_CurrentCol = 0;  /* Current column position (0-15) */

/******************************************************************************
 * FORWARD DECLARATIONS - STATIC (PRIVATE) FUNCTIONS
 ******************************************************************************/
void static ExecuteInitSeq();
void static lcdRunnableCBF();
void static ExecuteWriteString();
static LCD_Status_t LCD_enuInitGpioPins();
static LCD_Status_t LCD_InitSequence_8BitMode(void);
static LCD_Status_t ClearDisplay(Bits_t bits);
static LCD_Status_t LCD_InitSequence_4BitMode(void);
static LCD_Status_t LCD_SetCursor_Local(uint8_t row, uint8_t col, Bits_t nibble);
static void ExecutCreateCustomChar();

static SCHED_Runnable_t lcdRunnable = {
    .CBF = lcdRunnableCBF,
    .Args = NULL,
    .FirstDalay_ms = 50,
    .Periodicity_ms = 5,
    .Priority = 2
};

static LCD_Asyn_States_t lcdState = LCD_NO_ACTION;
static LCD_InitSeq_t initSeq = INIT_DONE;
static LCD_WriteStringSeq_t writeStringSeq = WRITE_STRING_DONE;
LCD_CreateCustomCharSeq_t createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
static LCD_Callback_t Lcd_Callback = NULL;


/**
 * @brief Write a byte to LCD data pins (DB0-DB7)
 * @param byte: Data byte to write (0x00 to 0xFF)
 * @return LCD_Status_t: Operation status
 */
static LCD_Status_t LCD_WriteByte(uint8_t byte);

/**
 * @brief Send command to LCD controller
 * @param cmd: Command byte to send
 * @return LCD_Status_t: Operation status
 */
static LCD_Status_t LCD_WriteCommand(uint8_t cmd);



/**
 * @brief Set LCD function parameters (8/4-bit, lines, font)
 * @param bitOperation: 8-bit or 4-bit mode
 * @param LineDisplay: 1-line or 2-line display
 * @param FontSize: 5x8 or 5x10 dot font
 * @return LCD_Status_t: Operation status
 */
static LCD_Status_t FunctionSet(LCD_BitOperation_t bitOperation, LCD_LineDisplay_t LineDisplay,LCD_FontSize_t FontSize,Bits_t bits);
/**
 * @brief Control display, cursor, and blink settings
 * @param Display: Display ON/OFF
 * @param Cursor: Cursor ON/OFF
 * @param Blink: Blink ON/OFF
 * @return LCD_Status_t: Operation status
 */
static LCD_Status_t DisplayControl(LCD_Display_t Display,LCD_Cursor_t Cursor,LCD_Blink_t Blink,Bits_t bits);

/**
 * @brief Set entry mode (increment/decrement, shift)
 * @param DisplayShiftOperation: Auto shift ON/OFF
 * @param IncrementStatus: Increment or decrement cursor
 * @return LCD_Status_t: Operation status
 */
static LCD_Status_t EnteryModeSet(LCD_DisplayShift_t DisplayShiftOperation,LCD_IncDec_t IncrementStatus,Bits_t bits);

/**
 * @brief Set DDRAM address for cursor positioning
 * @param address: DDRAM address (0x00 to 0x7F)
 * @return LCD_Status_t: Operation status
 */
static LCD_Status_t LCD_enuSetDDRAMAddress(uint8_t address,Bits_t bits);
/**
 * @brief Set CGRAM address for custom character creation
 * @param address: CGRAM address (0x00 to 0x3F)
 * @return LCD_Status_t: Operation status
 */
static LCD_Status_t LCD_enuSetCGRAMAddress(uint8_t address,Bits_t bits);

static LCD_Status_t LCD_GenerateEnablePulse(void);

/******************************************************************************
 * EXTERNAL VARIABLES
 * @note These are defined in user application code
 ******************************************************************************/

extern LCD_Config_t LcdCong ;                    /* LCD configuration structure */
extern const LCD_Pinout_8BitMode_t LcdPinout ;   /* LCD pin mapping structure */

/******************************************************************************
 * PUBLIC FUNCTIONS IMPLEMENTATION
 ******************************************************************************/

/**
 * @brief Initialize LCD hardware and controller
 * @details Performs the following:
 *          1. Initializes GPIO pins (DB0-DB7, RS, RW, EN)
 *          2. Executes HD44780 initialization sequence
 *          3. Configures display settings from LcdCong structure
 * @return LCD_Status_t: 
 *         - LCD_OK: Initialization successful
 *         - LCD_INIT_ERROR: GPIO or initialization failure
 * @note Only 8-bit mode is currently supported
 */
LCD_Status_t LCD_enuSynInit()
{
    LCD_Status_t retStatus = LCD_NOT_OK;  /* Default return status */

    /* Initialize GPIO pins based on bit operation mode */
    retStatus = LCD_enuInitGpioPins();
    
    if(LCD_OK == retStatus){

        if(LcdCong.BitOperation == LCD_8_BIT_OPERATION){
            /* Execute HD44780 initialization sequence for 8-bit mode */
            retStatus = LCD_InitSequence_8BitMode();
        }else if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
            /* Execute HD44780 initialization sequence for 4-bit mode */
            retStatus = LCD_InitSequence_4BitMode();
        }
    }
    else {
        //retStatus = retStatus;  /* Preserve error status */
    }
    return retStatus;  /* Single exit point */
}


/**
 * @brief Execute HD44780 initialization sequence
 * @details Follows HD44780 datasheet initialization procedure:
 *          1. Wait >40ms after power-up
 *          2. Send Function Set command
 *          3. Send Display Control command
 *          4. Send Clear Display command
 *          5. Send Entry Mode Set command
 * @return LCD_Status_t: Initialization status
 * @note This function implements proper timing as per HD44780 datasheet
 */
static LCD_Status_t LCD_InitSequence_8BitMode(void)
{
    LCD_Status_t retStatus = LCD_NOT_OK;        /* Function return status */
    SYSTICK_Status_t systickStat = SYSTICK_NOT_OK;  /* Timer status */
    GPIO_Status_t gpioStatus = GPIO_NOT_OK;
    uint8_t startSeqCount = 0;
        
    /* Wait for LCD power-up (>40ms after Vcc rises to 4.5V) */
    systickStat = SYSTICK_Wait_ms(40);
    
    if (SYSTICK_OK != systickStat){
        retStatus = LCD_TIMER_ERROR;  /* Timer error occurred */
    }else{
        /* ========== HD44780 Initialization by Instruction ========== */
        
        /* Start Sequence: Send 0x30 three times to ensure LCD is in known state */
        for(startSeqCount = 0; startSeqCount < 3; startSeqCount++){
            /* Set RS=0 for command, RW=0 for write */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RS.port, LcdPinout.RS.pin, GPIO_LOW);
            if(GPIO_OK != gpioStatus){
                retStatus = LCD_GPIO_ERROR;
                break;
            }else{
                // Continue to next step
            }
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RW.port, LcdPinout.RW.pin, GPIO_LOW);
            if(GPIO_OK != gpioStatus){
                retStatus = LCD_GPIO_ERROR;
                break;
            }else{
                // Continue to next step
            }
            
            /* Send 0x30 initialization command */
            retStatus = LCD_WriteByte(0x30 >> ALL_BITS);
            if (LCD_OK != retStatus){
                break;
            }
            
            /* Generate enable pulse */
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK != retStatus){
                break;
            }
            
            /* Wait between start sequence commands */
            systickStat = SYSTICK_Wait_ms(5);
            if (SYSTICK_OK != systickStat){
                retStatus = LCD_TIMER_ERROR;
                break;
            }
        }
        
        if (LCD_OK != retStatus){
            /* Start sequence failed */
        }else{
            /* Step 1: Function Set - Configure interface, lines, and font */
            retStatus = FunctionSet(LcdCong.BitOperation,LcdCong.LineDisplay,LcdCong.FontSize,ALL_BITS);
        if (LCD_OK != retStatus){
            // retStatus = retStatus;  /* Preserve error status */
        }else{
            // generate high >> low
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK != retStatus){
                // retStatus = retStatus;  /* Preserve error status */
            }else{
                systickStat = SYSTICK_Wait_ms(1);  /* Wait >4.1ms as per datasheet */
                
                if (SYSTICK_OK != systickStat){
                    retStatus = LCD_TIMER_ERROR;
                }else{
                    /* Step 2: Display Control - Configure display, cursor, blink */
                    retStatus = DisplayControl(LcdCong.Display,LcdCong.Cursor,LcdCong.Blink,ALL_BITS);
                    
                    if (LCD_OK != retStatus){
                        // retStatus = retStatus;  /* Preserve error status */
                    }else{
                        // generate high >> low
                        retStatus = LCD_GenerateEnablePulse();
                        if (LCD_OK != retStatus){
                            // retStatus = retStatus;  /* Preserve error status */
                        }else{
                            systickStat = SYSTICK_Wait_ms(1);  /* Wait >100μs */

                            if (SYSTICK_OK != systickStat){
                                retStatus = LCD_TIMER_ERROR;
                            }else{
                                /* Step 3: Clear Display - Clears all display and returns home */
                                retStatus = ClearDisplay(ALL_BITS);  /* Clear display command */
                                
                                if (LCD_OK != retStatus){
                                    // retStatus = retStatus;  /* Preserve error status */ 
                                }else{
                                    // generate high >> low
                                    retStatus = LCD_GenerateEnablePulse();
                                    if (LCD_OK != retStatus){
                                        // retStatus = retStatus;  /* Preserve error status */
                                    }else{
                                        systickStat = SYSTICK_Wait_ms(2);  /* Wait for clear (1.64ms typical) */

                                        if (SYSTICK_OK != systickStat){
                                            retStatus = LCD_TIMER_ERROR;
                                        }else{
                                            /* Step 4: Entry Mode Set - Configure increment and shift */
                                            retStatus =EnteryModeSet(LcdCong.DisplayShiftOperation,LcdCong.IncrementStatus,ALL_BITS);
                                            // generate high >> low
                                            retStatus = LCD_GenerateEnablePulse();
                                            if (LCD_OK != retStatus){
                                                // retStatus = retStatus;  /* Preserve error status */
                                            }else{
                                                retStatus = LCD_OK;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    }
    
    return retStatus;  /* Single exit point */
}

/**
 * @brief Write a byte to LCD data lines (DB0-DB7)
 * @details Writes 8 bits to data pins, generates enable pulse
 *          Timing: Enable pulse width = 1ms (>450ns required)
 * @param byte: 8-bit data to write (0x00 to 0xFF)
 * @return LCD_Status_t:
 *         - LCD_OK: Write successful
 *         - LCD_GPIO_ERROR: GPIO operation failed
 *         - LCD_TIMER_ERROR: Timer operation failed
 * @note This function handles the low-level LCD write protocol
 */
static LCD_Status_t LCD_WriteByte(uint8_t byte){
    LCD_Status_t retStatus = LCD_OK;        /* Function return status */
    GPIO_Status_t gpioStatus = GPIO_NOT_OK; /* GPIO operation status */

    uint8_t bitIndex = 0;
    if(LcdCong.BitOperation == LCD_8_BIT_OPERATION){
        /* Write bit 0 to DB0 pin */
        gpioStatus = GPIO_enuSetPinVal(LcdPinout.DB0.port, LcdPinout.DB0.pin,LCDpinVAl[(byte >> (bitIndex++)) & 1]);

        if (gpioStatus != GPIO_OK){
            retStatus = LCD_GPIO_ERROR;
        }else{
            /* Write bit 1 to DB1 pin */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.DB1.port, LcdPinout.DB1.pin,LCDpinVAl[(byte >>  (bitIndex++)) & 1]);
            if (gpioStatus != GPIO_OK){
                retStatus = LCD_GPIO_ERROR;
            }else{
                /* Write bit 2 to DB2 pin */
                gpioStatus = GPIO_enuSetPinVal(LcdPinout.DB2.port, LcdPinout.DB2.pin,LCDpinVAl[(byte >>  (bitIndex++)) & 1]);
                if (gpioStatus != GPIO_OK){
                    retStatus = LCD_GPIO_ERROR;
                }else{
                    /* Write bit 3 to DB3 pin */
                    gpioStatus = GPIO_enuSetPinVal(LcdPinout.DB3.port, LcdPinout.DB3.pin,LCDpinVAl[(byte >>  (bitIndex++)) & 1]);

                    if (gpioStatus != GPIO_OK){
                        retStatus = LCD_GPIO_ERROR;
                    }else{
                        retStatus = LCD_OK;   /* First 4 bits written successfully */
                    }
                }
            }
        }
    }else{
        // continue with 4-bit mode
    }
    if (retStatus == LCD_OK){
        /* Continue with bits 4-7 */
        /* Write bit 4 to DB4 pin */
       gpioStatus = GPIO_enuSetPinVal(LcdPinout.DB4.port, LcdPinout.DB4.pin,LCDpinVAl[(byte >>  (bitIndex++)) & 1]);
       if (gpioStatus != GPIO_OK){
           retStatus = LCD_GPIO_ERROR;
       }else{
           /* Write bit 5 to DB5 pin */
           gpioStatus = GPIO_enuSetPinVal(LcdPinout.DB5.port, LcdPinout.DB5.pin,LCDpinVAl[(byte >>  (bitIndex++)) & 1]);
           if (gpioStatus != GPIO_OK){
               retStatus = LCD_GPIO_ERROR;
           }else{
               /* Write bit 6 to DB6 pin */
               gpioStatus = GPIO_enuSetPinVal(LcdPinout.DB6.port, LcdPinout.DB6.pin,LCDpinVAl[(byte >>  (bitIndex++)) & 1]);
               if (gpioStatus != GPIO_OK){
                   retStatus = LCD_GPIO_ERROR;
               }else{
                   /* Write bit 7 to DB7 pin */
                   gpioStatus = GPIO_enuSetPinVal(LcdPinout.DB7.port, LcdPinout.DB7.pin,LCDpinVAl[(byte >>  (bitIndex++)) & 1]);
                   if (gpioStatus != GPIO_OK){
                       retStatus = LCD_GPIO_ERROR;
                   }else{
                       retStatus = LCD_OK;   /* All operations successful */
                   }
               }
            }
        }
    }else{
        // retStatus = retStatus;  /* Preserve error status */
    }
    
    return retStatus;  /* Single exit point - MISRA C compliant */
}


/**
 * @brief Send command to LCD controller
 * @details Sets RS=0 (command mode), RW=0 (write mode), then writes byte
 *          RS=0: Instruction register (command)
 *          RW=0: Write operation
 * @param cmd: Command byte to send (see HD44780 command set)
 * @return LCD_Status_t:
 *         - LCD_OK: Command sent successfully
 *         - LCD_GPIO_ERROR: GPIO operation failed
 * @note This is used for all LCD commands (clear, home, config, etc.)
 */
static LCD_Status_t LCD_WriteCommand(uint8_t cmd)
{
    LCD_Status_t retStatus = LCD_NOT_OK;    /* Function return status */
    GPIO_Status_t gpioStatus = GPIO_NOT_OK; /* GPIO operation status */
    
    /* Set RS = 0 for command mode (instruction register) */
    gpioStatus = GPIO_enuSetPinVal(LcdPinout.RS.port, 
                                    LcdPinout.RS.pin, 
                                    GPIO_LOW);
    
    if (GPIO_OK != gpioStatus){
        retStatus = LCD_GPIO_ERROR;
    }else{
        /* Set RW = 0 for write mode */
        gpioStatus = GPIO_enuSetPinVal(LcdPinout.RW.port, 
                                        LcdPinout.RW.pin, 
                                        GPIO_LOW);
        
        if (GPIO_OK != gpioStatus){
            retStatus = LCD_GPIO_ERROR;
        }else{
            /* Write command byte to LCD */
            retStatus = LCD_WriteByte(cmd);
        }
    }
    
    return retStatus;  /* Single exit point - MISRA C compliant */
}

/**
 * @brief Write a character to LCD at current cursor position
 * @details Sets RS=1 (data mode), RW=0 (write mode), then writes character
 *          Automatically increments cursor position and handles line wrapping
 *          RS=1: Data register (character data)
 *          RW=0: Write operation
 * @param displayedChar: ASCII character code to display (0x00 to 0xFF)
 * @return LCD_Status_t:
 *         - LCD_OK: Character written successfully
 *         - LCD_GPIO_ERROR: GPIO operation failed
 * @note Cursor tracking is updated automatically
 *       Line wrap occurs at column 16 (for 16x2 LCD)
 */
LCD_Status_t LCD_enuSyncWriteCharacter(uint8_t displayedChar)
{
    LCD_Status_t retStatus = LCD_NOT_OK;    /* Function return status */
    GPIO_Status_t gpioStatus = GPIO_NOT_OK; /* GPIO operation status */
    
    /* Set RS = 1 for data mode (data register) */
    gpioStatus = GPIO_enuSetPinVal(LcdPinout.RS.port, LcdPinout.RS.pin, GPIO_HIGH);
    
    if (GPIO_OK != gpioStatus){
        retStatus = LCD_GPIO_ERROR;
    }else{
        /* Set RW = 0 for write mode */
        gpioStatus = GPIO_enuSetPinVal(LcdPinout.RW.port, LcdPinout.RW.pin, GPIO_LOW);
        
        if (GPIO_OK != gpioStatus){
            retStatus = LCD_GPIO_ERROR;
        }else{

            if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
                /* In 4-bit mode, send higher nibble first */
                uint8_t highNibble = displayedChar >> HIGH_NIBBLE;
                retStatus = LCD_WriteByte(highNibble);
                
                if (LCD_OK == retStatus){
                    retStatus = LCD_GenerateEnablePulse();
                    if (LCD_OK == retStatus){
                        // continue to send lower nibble
                        uint8_t lowNibble = displayedChar >> LOW_NIBBLE;
                        retStatus = LCD_WriteByte(lowNibble);
                        if (LCD_OK == retStatus){
                            retStatus = LCD_GenerateEnablePulse();
                            if (LCD_OK == retStatus){
                                retStatus = LCD_OK; /* All operations successful */
                            }else{
                                // retStatus = retStatus;  /* Preserve error status */
                            }
                        }else{
                            // retStatus = retStatus;  /* Preserve error status */
                        }
                    }else{
                        // retStatus = retStatus;  /* Preserve error status */
                    }
                }else{
                    // return retStatus;  /* Exit on error */
                }

            }else if(LcdCong.BitOperation == LCD_8_BIT_OPERATION){
                /* In 8-bit mode, send full byte directly */
                retStatus = LCD_WriteByte(displayedChar >> ALL_BITS);
                if (LCD_OK == retStatus){

                    retStatus = LCD_GenerateEnablePulse();
                    if (LCD_OK == retStatus){
                        retStatus = LCD_OK; /* All operations successful */
                    }else{
                        // retStatus = retStatus;  /* Preserve error status */
                    }
                }else{
                    // retStatus = retStatus;  /* Preserve error status */
                }
            }else{
                retStatus = LCD_WRONG_BIT_OPERATION;
            }
        }
        if (LCD_OK == retStatus){
            /* Update column position (auto-increment after write) */
            LCD_CurrentCol++;
            
            /* Handle line wrap for 16-column LCD */
            if (LCD_CurrentCol >= COLUMN_LENGTH){
                LCD_CurrentCol = 0;  /* Reset column to start */
                
                /* Move to next line */
                if (LCD_CurrentRow == 0){
                    LCD_CurrentRow = 1;  /* Move from row 0 to row 1 */
                }else{
                    LCD_CurrentRow = 0;  /* Wrap from row 1 to row 0 */
                }
                /* Update LCD cursor position after wrap */
                retStatus = LCD_enuSyncSetCursorPosition(LCD_CurrentRow,LCD_CurrentCol);
            }
        }else{
            // retStatus = retStatus;  /* Preserve error status */
        }
        
    }
    
    return retStatus;  /* Single exit point - MISRA C compliant */
}


/**
 * @brief Return cursor to home position (0,0)
 * @details Sends "Return Home" command (0x02)
 *          - Sets DDRAM address to 0x00
 *          - Returns display shift to original position
 *          - Does NOT clear display content
 * @return LCD_Status_t: Operation status
 * @note Execution time: ~1.64ms
 */
LCD_Status_t LCD_enuSyncReturnHome(){

    LCD_Status_t retStatus = LCD_NOT_OK;
    if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
        retStatus = LCD_WriteCommand(RETURN_HOME_COMMAND>>HIGH_NIBBLE);  /* Return Home command high nibble */
        if (LCD_OK == retStatus){
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK == retStatus){
                retStatus = LCD_WriteCommand(RETURN_HOME_COMMAND>>LOW_NIBBLE);  /* Return Home command low nibble */
                if (LCD_OK == retStatus){
                    retStatus = LCD_GenerateEnablePulse();
                    if (LCD_OK == retStatus){
                        retStatus = LCD_OK; /* All operations successful */
                    }else{
                        // retStatus = retStatus;  /* Preserve error status */
                    }
                }else{
                    // retStatus = retStatus;  /* Preserve error status */
                }
            }else{
                // retStatus = retStatus;  /* Preserve error status */
            }
        }else{
            // retStatus = retStatus;  /* Preserve error status */
        }
    }else if(LcdCong.BitOperation == LCD_8_BIT_OPERATION){
        retStatus = LCD_WriteCommand(RETURN_HOME_COMMAND>>ALL_BITS);  /* Return Home command */
        if (LCD_OK == retStatus){
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK == retStatus){
                retStatus = LCD_OK; /* All operations successful */
            }else{
                // retStatus = retStatus;  /* Preserve error status */  
            }
        }else{
            // retStatus = retStatus;  /* Preserve error status */
        }
    }else{
        retStatus = LCD_WRONG_BIT_OPERATION;
    }
    return retStatus;  /* Single exit point - MISRA C compliant */
}

/**
 * @brief Clear entire LCD display
 * @details Sends "Clear Display" command (0x01)
 *          - Clears all display data
 *          - Sets DDRAM address to 0x00
 *          - Returns cursor to home position
 * @return LCD_Status_t: Operation status
 * @note Execution time: ~1.64ms
 */
static LCD_Status_t ClearDisplay(Bits_t bits){

    LCD_Status_t retStatus =LCD_WriteCommand(CLEAR_DISPLAY_COMMAND>>bits);  /* Clear Display command (0x01) */
    return retStatus;  /* Single exit point */
}

LCD_Status_t LCD_enuSyncClearDisplay(){
    LCD_Status_t retStatus = LCD_NOT_OK;

    if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
        retStatus = ClearDisplay(HIGH_NIBBLE);
        if (LCD_OK == retStatus){
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK == retStatus){
                retStatus = ClearDisplay(LOW_NIBBLE);
                if (LCD_OK == retStatus){
                    retStatus = LCD_GenerateEnablePulse();
                    if (LCD_OK == retStatus){
                        retStatus = LCD_OK; /* All operations successful */
                    }else{
                        // retStatus = retStatus;  /* Preserve error status */
                    }
                }else{
                    // retStatus = retStatus;  /* Preserve error status */
                }
            }else{
                // retStatus = retStatus;  /* Preserve error status */
            }
        }else{
            // return retStatus;  /* Exit on error */
        }
    }else if(LcdCong.BitOperation == LCD_8_BIT_OPERATION){
        retStatus = ClearDisplay(ALL_BITS);
        if (LCD_OK == retStatus){
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK == retStatus){
                retStatus = LCD_OK; /* All operations successful */
            }else{
                // retStatus = retStatus;  /* Preserve error status */  
            }
        }else{
            // retStatus = retStatus;  /* Preserve error status */
        }
    }else{
        retStatus = LCD_WRONG_BIT_OPERATION;
    }
    return retStatus;  /* Single exit point - MISRA C compliant */
}

/**
 * @brief Set cursor visibility state
 * @details Controls cursor display ON/OFF
 *          When cursor is OFF, blink is also turned OFF
 * @param cursorState: LCD_CURSOR_ON or LCD_CURSOR_OFF
 * @return LCD_Status_t: Operation status
 * @note Updates global configuration and sends Display Control command
 */
LCD_Status_t LCD_enuSyncSetCursor(LCD_Cursor_t cursorState)
{
    LCD_Status_t retStatus = LCD_NOT_OK;
    
    /* Update configuration with new cursor state */
    LcdCong.Cursor = cursorState;
    
    /* If cursor is turned OFF, also turn OFF blink */
    if(cursorState == LCD_CURSOR_OFF){
        LcdCong.Blink = LCD_BLINK_OFF;
    }
    
    if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
        /* Send Display Control command in 4-bit mode */
        retStatus = DisplayControl(LcdCong.Display,LcdCong.Cursor,LcdCong.Blink,HIGH_NIBBLE);
        if (LCD_OK == retStatus){
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK == retStatus){
                retStatus = DisplayControl(LcdCong.Display,LcdCong.Cursor,LcdCong.Blink,LOW_NIBBLE);
                if (LCD_OK == retStatus){
                    retStatus = LCD_GenerateEnablePulse();
                    if (LCD_OK == retStatus){
                        retStatus = LCD_OK; /* All operations successful */
                    }else{
                        // retStatus = retStatus;  /* Preserve error status */
                    }
                }else{
                    // retStatus = retStatus;  /* Preserve error status */
                }
            }else{
                // retStatus = retStatus;  /* Preserve error status */
            }
        }else{
            // return retStatus;  /* Exit on error */
        }
    }else if(LcdCong.BitOperation == LCD_8_BIT_OPERATION){
        /* Send Display Control command with updated settings */
        retStatus = DisplayControl(LcdCong.Display,LcdCong.Cursor,LcdCong.Blink,ALL_BITS);
        if( LCD_OK == retStatus){
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK == retStatus){
                retStatus = LCD_OK; /* All operations successful */
            }else{
                // retStatus = retStatus;  /* Preserve error status */  
            }
        }else{
            // retStatus = retStatus;  /* Preserve error status */
        }
    }else{
        retStatus = LCD_WRONG_BIT_OPERATION;
    }
    return retStatus;  /* Single exit point - MISRA C compliant */
}

/**
 * @brief Set cursor blink state
 * @details Controls cursor blinking ON/OFF
 * @param blinkState: LCD_BLINK_ON or LCD_BLINK_OFF
 * @return LCD_Status_t: Operation status
 * @note Updates global configuration and sends Display Control command
 *       Blink only visible when cursor is ON
 */
LCD_Status_t LCD_enuSyncSetBlink(LCD_Blink_t blinkState)
{
    LCD_Status_t retStatus = LCD_NOT_OK;
    
    /* Update configuration with new blink state */
    LcdCong.Blink = blinkState;

    if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
        /* Send Display Control command in 4-bit mode */
        retStatus = DisplayControl(LcdCong.Display,LcdCong.Cursor,LcdCong.Blink,HIGH_NIBBLE);
        if (LCD_OK == retStatus){
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK == retStatus){
                retStatus = DisplayControl(LcdCong.Display,LcdCong.Cursor,LcdCong.Blink,LOW_NIBBLE);
                if (LCD_OK == retStatus){
                    retStatus = LCD_GenerateEnablePulse();
                    if (LCD_OK == retStatus){
                        retStatus = LCD_OK; /* All operations successful */
                    }else{
                        // retStatus = retStatus;  /* Preserve error status */
                    }
                }else{
                    // retStatus = retStatus;  /* Preserve error status */
                }
            }else{
                // retStatus = retStatus;  /* Preserve error status */
            }
        }else{
            // return retStatus;  /* Exit on error */
        }
    }else if(LcdCong.BitOperation == LCD_8_BIT_OPERATION){
        /* Send Display Control command with updated settings */
        retStatus = DisplayControl(LcdCong.Display,LcdCong.Cursor,LcdCong.Blink,ALL_BITS);
        if( LCD_OK == retStatus){
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK == retStatus){
                retStatus = LCD_OK; /* All operations successful */
            }else{
                // retStatus = retStatus;  /* Preserve error status */  
            }
        }else{
            // retStatus = retStatus;  /* Preserve error status */
        }
    }else{
        retStatus = LCD_WRONG_BIT_OPERATION;    
    }
    return retStatus;  /* Single exit point - MISRA C compliant */
}

/**
 * @brief Set display ON/OFF state
 * @details Controls entire display visibility
 *          - ON: Display content visible
 *          - OFF: Display content hidden (but not erased)
 * @param displayState: LCD_DISPLAY_ON or LCD_DISPLAY_OFF
 * @return LCD_Status_t: Operation status
 * @note Updates global configuration and sends Display Control command
 *       Display data is retained when turned OFF
 */
LCD_Status_t LCD_enuSyncSetDisplay(LCD_Display_t displayState)
{
    LCD_Status_t retStatus = LCD_NOT_OK;
    
    /* Update configuration with new display state */
    LcdCong.Display = displayState;
    

    if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
        /* Send Display Control command in 4-bit mode */
        retStatus = DisplayControl(LcdCong.Display,LcdCong.Cursor,LcdCong.Blink,HIGH_NIBBLE);
        if (LCD_OK == retStatus){
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK == retStatus){
                retStatus = DisplayControl(LcdCong.Display,LcdCong.Cursor,LcdCong.Blink,LOW_NIBBLE);
                if (LCD_OK == retStatus){
                    retStatus = LCD_GenerateEnablePulse();
                    if (LCD_OK == retStatus){
                        retStatus = LCD_OK; /* All operations successful */
                    }else{
                        // retStatus = retStatus;  /* Preserve error status */
                    }
                }else{
                    // retStatus = retStatus;  /* Preserve error status */
                }
            }else{
                // retStatus = retStatus;  /* Preserve error status */
            }
        }else{
            // return retStatus;  /* Exit on error */
        }
    }else if(LcdCong.BitOperation == LCD_8_BIT_OPERATION){
        /* Send Display Control command with updated settings */
        retStatus = DisplayControl(LcdCong.Display,LcdCong.Cursor,LcdCong.Blink,ALL_BITS);
        if( LCD_OK == retStatus){
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK == retStatus){
                retStatus = LCD_OK; /* All operations successful */
            }else{
                // retStatus = retStatus;  /* Preserve error status */  
            }
        }else{
            // retStatus = retStatus;  /* Preserve error status */
        }
    }else{
        retStatus = LCD_WRONG_BIT_OPERATION;    
    }

    return retStatus;  /* Single exit point - MISRA C compliant */
}

/**
 * @brief Send Function Set command to LCD
 * @details Command format: 0b001 DL N F xx
 *          DL (bit 4): Data length (1=8-bit, 0=4-bit)
 *          N  (bit 3): Number of lines (1=2-line, 0=1-line)
 *          F  (bit 2): Font size (1=5x10, 0=5x8)
 * @param bitOperation: 8-bit or 4-bit interface mode
 * @param LineDisplay: 1-line or 2-line display
 * @param FontSize: 5x8 or 5x10 dot font
 * @return LCD_Status_t: Operation status
 * @note This command can only be sent when LCD is in 8-bit mode
 */
static LCD_Status_t FunctionSet(LCD_BitOperation_t bitOperation, LCD_LineDisplay_t LineDisplay,LCD_FontSize_t FontSize,Bits_t bits){
    uint8_t command =0;  /* Command byte to build */
    
    /* Build Function Set command: 0b00100000 | DL | N | F */
    command |= ((FUNCTION_SET_MASK)|(FontSize<<FONT_BIT_POSITION)|((LineDisplay<<LINEDISPLAY_BIT_POSITION))|(bitOperation<<BITOPEARATION_BIT_POSITION));
    
    command = command >> bits;
    /* Send command to LCD */
    LCD_Status_t retStatus = LCD_WriteCommand(command);
    return retStatus;  /* Single exit point */
}

/**
 * @brief Send Display Control command to LCD
 * @details Command format: 0b00001 D C B
 *          D (bit 2): Display ON/OFF (1=ON, 0=OFF)
 *          C (bit 1): Cursor ON/OFF (1=ON, 0=OFF)
 *          B (bit 0): Blink ON/OFF (1=ON, 0=OFF)
 * @param Display: Display ON/OFF state
 * @param Cursor: Cursor ON/OFF state
 * @param Blink: Blink ON/OFF state
 * @return LCD_Status_t: Operation status
 * @note Controls visibility of display, cursor, and cursor blink
 */
static LCD_Status_t DisplayControl(LCD_Display_t Display,LCD_Cursor_t Cursor,LCD_Blink_t Blink,Bits_t bits){
    uint8_t command =0;  /* Command byte to build */
    
    /* Build Display Control command: 0b00001000 | D | C | B */
    command |=(DISPLAY_CONTROL_MASK)|((Display)<<DISPLAY_BIT_POSITION)|((Cursor<<CURSOR_BIT_POSITION)|(Blink));
    
    command = command >> bits;
    /* Send command to LCD */
    LCD_Status_t retStatus = LCD_WriteCommand(command);
    return retStatus;  /* Single exit point */
}

/**
 * @brief Send Entry Mode Set command to LCD
 * @details Command format: 0b000001 I/D S
 *          I/D (bit 1): Increment/Decrement (1=increment, 0=decrement)
 *          S   (bit 0): Shift display (1=shift ON, 0=shift OFF)
 * @param DisplayShiftOperation: Auto shift ON/OFF after character write
 * @param IncrementStatus: Cursor increment or decrement direction
 * @return LCD_Status_t: Operation status
 * @note Controls cursor movement direction and auto-shift behavior
 */
static LCD_Status_t EnteryModeSet(LCD_DisplayShift_t DisplayShiftOperation,LCD_IncDec_t IncrementStatus,Bits_t bits){
    uint8_t command =0;  /* Command byte to build */
    
    /* Build Entry Mode Set command: 0b00000100 | I/D | S */
    command |=(ENTRY_MODE_MASK)|((IncrementStatus)<<INCREMENT_BIT_POSITION)|(DisplayShiftOperation);
    
    command = command >> bits;
    /* Send command to LCD */
    LCD_Status_t retStatus = LCD_WriteCommand(command);
    return retStatus;  /* Single exit point */
}

/**
 * @brief Set number of display lines (1-line or 2-line)
 * @details Updates configuration and sends Function Set command
 * @param LineDisplay: LCD_1_LINE_DISPLAY or LCD_2_LINE_DISPLAY
 * @return LCD_Status_t: Operation status
 * @note Updates global configuration LcdCong.LineDisplay
 *       Most 16x2 LCDs require 2-line mode
 */
LCD_Status_t LCD_enuSyncSetLineDisplay(LCD_LineDisplay_t LineDisplay)
{
    LCD_Status_t retStatus = LCD_NOT_OK;
    
    /* Update configuration with new line display mode */
    LcdCong.LineDisplay = LineDisplay;
    

    if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
        /* Send Function Set command in 4-bit mode */
        retStatus = FunctionSet(LcdCong.BitOperation,LcdCong.LineDisplay,LcdCong.FontSize,HIGH_NIBBLE);
        if (LCD_OK == retStatus){
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK == retStatus){
                retStatus = FunctionSet(LcdCong.BitOperation,LcdCong.LineDisplay,LcdCong.FontSize,LOW_NIBBLE);
                if (LCD_OK == retStatus){
                    retStatus = LCD_GenerateEnablePulse();
                    if (LCD_OK == retStatus){
                        retStatus = LCD_OK; /* All operations successful */
                    }else{
                        // retStatus = retStatus;  /* Preserve error status */
                    }
                }else{
                    // retStatus = retStatus;  /* Preserve error status */
                }
            }else{
                // retStatus = retStatus;  /* Preserve error status */
            }
        }else{
            // return retStatus;  /* Exit on error */
        }
    }else if(LcdCong.BitOperation == LCD_8_BIT_OPERATION){
        /* Send Function Set command with updated configuration */
        retStatus = FunctionSet(LcdCong.BitOperation,LcdCong.LineDisplay,LcdCong.FontSize,ALL_BITS);
        if( LCD_OK == retStatus){
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK == retStatus){
                retStatus = LCD_OK; /* All operations successful */
            }else{
                // retStatus = retStatus;  /* Preserve error status */  
            }
        }else{
            // retStatus = retStatus;  /* Preserve error status */
        }
    }else{
        retStatus = LCD_WRONG_BIT_OPERATION;    
    }
    return retStatus;  /* Single exit point */
}

/**
 * @brief Set font size (5x8 or 5x10 dots)
 * @details Updates configuration and sends Function Set command
 *          5x8: Standard font (8 custom characters available)
 *          5x10: Tall font (4 custom characters available)
 * @param FontSize: LCD_5_7_DOT_FONT (5x8) or LCD_5_10_DOT_FONT (5x10)
 * @return LCD_Status_t: Operation status
 * @note Updates global configuration LcdCong.FontSize
 *       5x10 font only available in 1-line mode
 */
LCD_Status_t LCD_enuSyncSetFontSize(LCD_FontSize_t FontSize)
{
    LCD_Status_t retStatus = LCD_NOT_OK;    

    /* Update configuration with new font size */
    LcdCong.FontSize = FontSize;

    if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
        /* Send Function Set command in 4-bit mode */
        retStatus = FunctionSet(LcdCong.BitOperation,LcdCong.LineDisplay,LcdCong.FontSize,HIGH_NIBBLE);
        if (LCD_OK == retStatus){
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK == retStatus){
                retStatus = FunctionSet(LcdCong.BitOperation,LcdCong.LineDisplay,LcdCong.FontSize,LOW_NIBBLE);
                if (LCD_OK == retStatus){
                    retStatus = LCD_GenerateEnablePulse();
                    if (LCD_OK == retStatus){
                        retStatus = LCD_OK; /* All operations successful */
                    }else{
                        // retStatus = retStatus;  /* Preserve error status */
                    }
                }else{
                    // retStatus = retStatus;  /* Preserve error status */
                }
            }else{
                // retStatus = retStatus;  /* Preserve error status */
            }
        }else{
            // return retStatus;  /* Exit on error */
        }
    }else if(LcdCong.BitOperation == LCD_8_BIT_OPERATION){
    
        /* Send Function Set command with updated configuration */
        retStatus = FunctionSet(LcdCong.BitOperation,LcdCong.LineDisplay,LcdCong.FontSize,ALL_BITS);
        if( LCD_OK == retStatus){
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK == retStatus){
                retStatus = LCD_OK; /* All operations successful */
            }else{
                // retStatus = retStatus;  /* Preserve error status */  
            }
        }else{
            // retStatus = retStatus;  /* Preserve error status */
        }
    }else{
        retStatus = LCD_WRONG_BIT_OPERATION;    
    }
    return retStatus;  /* Single exit point */
}

/**
 * @brief Set display shift mode (auto-shift ON/OFF)
 * @details Controls whether display shifts automatically after write
 *          AUTO_SHIFT: Display shifts after each character write
 *          NO_SHIFT: Display remains stationary
 * @param displayShift: LCD_AUTO_SHIFT or LCD_NO_SHIFT
 * @return LCD_Status_t: Operation status
 * @note Updates global configuration LcdCong.DisplayShiftOperation
 *       Used for scrolling text effects
 */
LCD_Status_t LCD_enuSyncDisplayShift(LCD_DisplayShift_t displayShift){

    LCD_Status_t retStatus = LCD_NOT_OK;    

    /* Update configuration with new display shift mode */
    LcdCong.DisplayShiftOperation = displayShift;

    if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
        /* Send Entry Mode Set command in 4-bit mode */
        retStatus = EnteryModeSet(LcdCong.DisplayShiftOperation,LcdCong.IncrementStatus,HIGH_NIBBLE);
        if (LCD_OK == retStatus){
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK == retStatus){
                retStatus = EnteryModeSet(LcdCong.DisplayShiftOperation,LcdCong.IncrementStatus,LOW_NIBBLE);
                if (LCD_OK == retStatus){
                    retStatus = LCD_GenerateEnablePulse();
                    if (LCD_OK == retStatus){
                        retStatus = LCD_OK; /* All operations successful */
                    }else{
                        // retStatus = retStatus;  /* Preserve error status */
                    }
                }else{
                    // retStatus = retStatus;  /* Preserve error status */
                }
            }else{
                // retStatus = retStatus;  /* Preserve error status */
            }
        }else{
            // return retStatus;  /* Exit on error */
        }
    }else if(LcdCong.BitOperation == LCD_8_BIT_OPERATION){
    
        /* Send Entry Mode Set command with updated configuration */
        retStatus =EnteryModeSet(LcdCong.DisplayShiftOperation,LcdCong.IncrementStatus,ALL_BITS);
        if( LCD_OK == retStatus){
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK == retStatus){
                retStatus = LCD_OK; /* All operations successful */
            }else{
                // retStatus = retStatus;  /* Preserve error status */  
            }
        }else{
            // retStatus = retStatus;  /* Preserve error status */
        }
    }else{
        retStatus = LCD_WRONG_BIT_OPERATION;    
    }  
    return retStatus;  /* Single exit point */
}

/**
 * @brief Set cursor increment/decrement mode
 * @details Controls cursor movement direction after character write
 *          INCREMENT: Cursor moves right (left-to-right writing)
 *          DECREMENT: Cursor moves left (right-to-left writing)
 * @param incrementDecrement: LCD_INCREMENT or LCD_DECREMENT
 * @return LCD_Status_t: Operation status
 * @note Updates global configuration LcdCong.IncrementStatus
 *       Affects cursor address counter behavior
 */
LCD_Status_t LCD_enuSyncSetIncrementDecrementMode(LCD_IncDec_t incrementDecrement){

    LCD_Status_t retStatus = LCD_NOT_OK;    

    /* Update configuration with new increment/decrement mode */
    LcdCong.IncrementStatus = incrementDecrement;

    if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
        /* Send Entry Mode Set command in 4-bit mode */
        retStatus = EnteryModeSet(LcdCong.DisplayShiftOperation,LcdCong.IncrementStatus,HIGH_NIBBLE);
        if (LCD_OK == retStatus){
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK == retStatus){
                retStatus = EnteryModeSet(LcdCong.DisplayShiftOperation,LcdCong.IncrementStatus,LOW_NIBBLE);
                if (LCD_OK == retStatus){
                    retStatus = LCD_GenerateEnablePulse();
                    if (LCD_OK == retStatus){
                        retStatus = LCD_OK; /* All operations successful */
                    }else{
                        // retStatus = retStatus;  /* Preserve error status */
                    }
                }else{
                    // retStatus = retStatus;  /* Preserve error status */
                }
            }else{
                // retStatus = retStatus;  /* Preserve error status */
            }
        }else{
            // return retStatus;  /* Exit on error */
        }
    }else if(LcdCong.BitOperation == LCD_8_BIT_OPERATION){
    
        /* Send Entry Mode Set command with updated configuration */
        retStatus =EnteryModeSet(LcdCong.DisplayShiftOperation,LcdCong.IncrementStatus,ALL_BITS);
        if( LCD_OK == retStatus){
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK == retStatus){
                retStatus = LCD_OK; /* All operations successful */
            }else{
                // retStatus = retStatus;  /* Preserve error status */  
            }
        }else{
            // retStatus = retStatus;  /* Preserve error status */
        }
    }else{
        retStatus = LCD_WRONG_BIT_OPERATION;    
    } 
    return retStatus;  /* Single exit point */
}

/**
 * @brief Set CGRAM (Character Generator RAM) address
 * @details Command format: 0b01 AC5 AC4 AC3 AC2 AC1 AC0
 *          Sets address counter to CGRAM for custom character creation
 *          Address range: 0x00 to 0x3F (64 bytes)
 *          8 characters × 8 bytes each = 64 bytes total
 * @param address: CGRAM address (0x00 to 0x3F), masked to 6 bits
 * @return LCD_Status_t: Operation status
 * @note After setting CGRAM address, write 8 bytes to define character
 *       Character locations: 0-7 (location × 8 = start address)
 */
static LCD_Status_t LCD_enuSetCGRAMAddress(uint8_t address,Bits_t bits)
{
    LCD_Status_t retStatus = LCD_NOT_OK;
    uint8_t command = 0;  /* Command byte to build */
    
    /* Mask address to 6 bits (0-63) - only AC5-AC0 are valid */
    address &= 0x3F;
    
    /* Build Set CGRAM Address command: 0b01000000 | address */
    command |= ((CGRAM_ADDRESS_MASK) | (address));
    
    command = command >> bits;  /* Shift command based on bit operation */
    /* Send command to LCD */
    retStatus = LCD_WriteCommand(command);
    
    return retStatus;  /* Single exit point */
}

/**
 * @brief Set DDRAM (Display Data RAM) address
 * @details Command format: 0b1 AC6 AC5 AC4 AC3 AC2 AC1 AC0
 *          Sets address counter to DDRAM for cursor positioning
 *          Address range: 0x00 to 0x7F (128 bytes)
 *          Row 0: 0x00-0x0F (columns 0-15)
 *          Row 1: 0x40-0x4F (columns 0-15)
 * @param address: DDRAM address (0x00 to 0x7F), masked to 7 bits
 * @return LCD_Status_t: Operation status
 * @note This is used internally by LCD_enuSetCursorPosition()
 *       Updates cursor position tracking variables
 */
static LCD_Status_t LCD_enuSetDDRAMAddress(uint8_t address,Bits_t bits)
{
    LCD_Status_t retStatus = LCD_NOT_OK;
    uint8_t command = 0;  /* Command byte to build */
    
    /* Mask address to 7 bits (0-127) - only AC6-AC0 are valid */
    address &= 0x7F;
    
    /* Build Set DDRAM Address command: 0b10000000 | address */
    command |= ((DDRAM_ADDRESS_MASK) | (address));

    command = command >> bits;  /* Shift command based on bit operation */
    
    /* Send command to LCD */
    retStatus = LCD_WriteCommand(command);
    
    return retStatus;  /* Single exit point */
}

/**
 * @brief Set cursor position using row and column
 * @details Calculates DDRAM address from row/column and updates cursor
 *          Row 0: DDRAM address 0x00 + col (0x00-0x0F)
 *          Row 1: DDRAM address 0x40 + col (0x40-0x4F)
 * @param row: Row number (0 or 1 for 16x2 LCD)
 * @param col: Column number (0 to 15 for 16x2 LCD)
 * @return LCD_Status_t:
 *         - LCD_OK: Position set successfully
 *         - LCD_WRONG_ROW: Invalid row number (not 0 or 1)
 *         - LCD_WRONG_COLUMN: Invalid column number (>15)
 * @note Updates LCD_CurrentRow and LCD_CurrentCol tracking variables
 *       Only supports 2-line LCD displays (16 columns max)
 */
LCD_Status_t LCD_enuSyncSetCursorPosition(uint8_t row, uint8_t col)
{
    LCD_Status_t retStatus = LCD_NOT_OK;
    uint8_t address = 0;  /* DDRAM address to calculate */
    
    /* Validate column range (0-15 for 16-column LCD) */
    if (col > 15){
        retStatus = LCD_WRONG_COLUMN;  /* Column out of range */
    }else{
        /* Calculate DDRAM address based on row */
        if (row == 0){
            address = ROW_0_OFFSET + col;  /* First line: 0x00-0x0F */
            LCD_CurrentRow = row;  /* Update row tracking */
            LCD_CurrentCol = col;  /* Update column tracking */
        }else if (row == 1){
            address = ROW_1_OFFSET + col;  /* Second line: 0x40-0x4F */
            LCD_CurrentRow = row;  /* Update row tracking */
            LCD_CurrentCol = col;  /* Update column tracking */
        }else{
            retStatus = LCD_WRONG_ROW;  /* Row out of range (only 0 or 1 valid) */
        }
        if(LCD_WRONG_ROW != retStatus){
            if(LcdCong.BitOperation == LCD_8_BIT_OPERATION){
                retStatus = LCD_enuSetDDRAMAddress(address,ALL_BITS);  /* Set DDRAM address */
                if(retStatus == LCD_OK){
                    retStatus = LCD_GenerateEnablePulse();
                }else{
                    //retStatus = retStatus
                }
            }else{
                retStatus = LCD_enuSetDDRAMAddress(address,HIGH_NIBBLE);  /* Set DDRAM address */
                if(retStatus == LCD_OK){
                    retStatus = LCD_GenerateEnablePulse();
                    if (LCD_OK == retStatus){
                        retStatus = LCD_enuSetDDRAMAddress(address,LOW_NIBBLE);  /* Set DDRAM address */    
                        if(retStatus == LCD_OK){
                        retStatus = LCD_GenerateEnablePulse();
                        }else{
                            //retStatus = retStatus
                        }
                    }else{
                        //retStatus = retStatus
                    }
                }else{
                    //retStatus = retStatus
                }

            }
                
        }
    }
    
    return retStatus;  /* Single exit point */
}

/**
 * @brief Create custom character in CGRAM
 * @details Stores 5x8 or 5x10 custom character pattern in CGRAM
 *          Each character uses 8 bytes (one byte per row)
 *          Location 0-7: 8 custom characters available (5x8 font)
 *          Location 0-3: 4 custom characters available (5x10 font)
 * @param location: Character location (0-7 for 5x8, 0-3 for 5x10)
 * @param charmap: Array of 8 bytes defining character pattern
 *                 Each byte represents one row (5 bits used)
 *                 Bit pattern: xxxxx (x=1: pixel on, x=0: pixel off)
 * @return LCD_Status_t:
 *         - LCD_OK: Character created successfully
 *         - LCD_NULL_PTR: charmap is NULL pointer
 *         - LCD_NOT_OK: Invalid location (>9)
 *         - LCD_GPIO_ERROR: GPIO operation failed
 *         - LCD_ERROR_SPECIALCHAR: Character write failed
 * @note After creating character, cursor returns to previous position
 *       Custom characters are displayed using codes 0-7
 */
LCD_Status_t LCD_enuSyncCreateCustomChar(uint8_t location, const uint8_t charmap[SPECIAL_CHAR_LENGHT])
{
    LCD_Status_t retStatus = LCD_NOT_OK;
    
    /* Validate input parameters */
    if (NULL == charmap){
        retStatus = LCD_NULL_PTR;  /* NULL pointer passed */
    }else if (location > LOCATION_MASK){
        retStatus = LCD_WRONG_LOCATION;  /* Invalid location */
    }else{
        /* Set CGRAM address (location * 8) */
        /* Each character uses 8 bytes, so multiply location by 8 */
        if(LcdCong.BitOperation == LCD_8_BIT_OPERATION){
            retStatus = LCD_enuSetCGRAMAddress((location<<3),ALL_BITS);
            if(LCD_OK == retStatus){
                retStatus = LCD_GenerateEnablePulse();
                if( LCD_OK == retStatus){
                    retStatus = LCD_OK; /* All operations successful */
                }else{
                    // retStatus = retStatus;  /* Preserve error status */  
                }
            }else{
                // retStatus = retStatus;  /* Preserve error status */
            }
        }else {
            if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
                retStatus = LCD_enuSetCGRAMAddress((location<<3),HIGH_NIBBLE);
                if(retStatus == LCD_OK){
                    retStatus = LCD_GenerateEnablePulse();
                    if( LCD_OK == retStatus){
                        retStatus = LCD_enuSetCGRAMAddress((location<<3),LOW_NIBBLE);
                        if(retStatus == LCD_OK){
                            retStatus = LCD_GenerateEnablePulse();
                        }else{
                            // retStatus = retStatus;  /* Preserve error status */
                        }
                    }else{
                        // retStatus = retStatus;  /* Preserve error status */
                    }
                }else{
                    // retStatus = retStatus;  /* Preserve error status */  
                }
            }else{
                retStatus = LCD_WRONG_BIT_OPERATION;
            }
        }
        if (LCD_OK == retStatus){
            GPIO_Status_t gpioStatus = GPIO_NOT_OK;  /* GPIO operation status */
            
            /* Write 8 bytes of character data to CGRAM */
            for (uint8_t i = 0; i < SPECIAL_CHAR_LENGHT; i++){
    
                /* Set RS = 1 for data mode (writing character pattern) */
                gpioStatus = GPIO_enuSetPinVal(LcdPinout.RS.port, 
                                                LcdPinout.RS.pin, 
                                                GPIO_HIGH);
                
                if (GPIO_OK != gpioStatus){
                    retStatus = LCD_GPIO_ERROR;  /* GPIO operation failed */
                }else{
                    /* Set RW = 0 for write mode */
                    gpioStatus = GPIO_enuSetPinVal(LcdPinout.RW.port, 
                                                    LcdPinout.RW.pin, 
                                                    GPIO_LOW);
                    
                    if (GPIO_OK != gpioStatus){
                        retStatus = LCD_GPIO_ERROR;  /* GPIO operation failed */
                    }else{
                        if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
                            retStatus = LCD_WriteByte(charmap[i] >> HIGH_NIBBLE);
                            if (LCD_OK == retStatus){
                                retStatus = LCD_GenerateEnablePulse();
                                if (LCD_OK == retStatus){
                                    retStatus = LCD_WriteByte(charmap[i] >> LOW_NIBBLE);
                                    if (LCD_OK == retStatus){
                                        retStatus = LCD_GenerateEnablePulse();
                                    }else{
                                        retStatus = LCD_ERROR_SPECIALCHAR;  /* Character write failed */
                                    }
                                }else{
                                    retStatus = LCD_ERROR_SPECIALCHAR;  /* Character write failed */
                                }
                            }else{
                                retStatus = LCD_ERROR_SPECIALCHAR;  /* Character write failed */
                            }
                        }else if(LcdCong.BitOperation == LCD_8_BIT_OPERATION){
                            /* Write character pattern byte to CGRAM */
                            retStatus = LCD_WriteByte(charmap[i]);
                            if (LCD_OK == retStatus){
                                retStatus = LCD_GenerateEnablePulse();
                            }else{
                                retStatus = LCD_ERROR_SPECIALCHAR;  /* Character write failed */
                            }
                        }else{
                            retStatus = LCD_WRONG_BIT_OPERATION;
                        }
                    }
                }
            }
            
            /* Return cursor to previous position (back to DDRAM mode) */
            retStatus = LCD_enuSyncSetCursorPosition(LCD_CurrentRow,LCD_CurrentCol);            
        }
    }
    
    return retStatus;  /* Single exit point */
}

/**
 * @brief Display custom character at current cursor position
 * @details Writes custom character code to display custom character
 *          Custom characters are accessed using codes 0-7
 *          Must be created first using LCD_enuCreateCustomChar()
 * @param location: Custom character location (0-7)
 * @return LCD_Status_t:
 *         - LCD_OK: Character displayed successfully
 *         - LCD_NOT_OK: Invalid location (>7)
 * @note The custom character must exist in CGRAM before calling this
 *       Cursor advances automatically after displaying character
 */
LCD_Status_t LCD_enuSyncWriteCustomChar(uint8_t location)
{
    LCD_Status_t retStatus = LCD_NOT_OK;
    
    /* Validate location (only 0-7 valid for 8 custom characters) */
    if (location > LOCATION_MASK){
        retStatus = LCD_WRONG_LOCATION;  /* Invalid custom character location */
    }else{
        /* Write character code (0-7) to display custom character */
        retStatus = LCD_enuSyncWriteCharacter((location));
    }
    
    return retStatus;  /* Single exit point */
}

/**
 * @brief Generate enable pulse for LCD (HIGH -> LOW transition)
 * @details Creates falling edge on EN pin to latch data into LCD
 *          Timing: EN high for 1ms, then low with 1ms delay
 * @return LCD_Status_t:
 *         - LCD_OK: Pulse generated successfully
 *         - LCD_GPIO_ERROR: GPIO operation failed
 *         - LCD_TIMER_ERROR: Timer operation failed
 * @note Minimum EN pulse width: >450ns (we use 1ms for safety)
 *       This function is called by LCD_WriteByte() after data is set
 */
static LCD_Status_t LCD_GenerateEnablePulse(void)
{
    LCD_Status_t retStatus = LCD_NOT_OK;    /* Function return status */
    GPIO_Status_t gpioStatus = GPIO_NOT_OK; /* GPIO operation status */
    
    /* Set EN pin HIGH to start pulse */
    gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
    
    if (GPIO_OK != gpioStatus){
        retStatus = LCD_GPIO_ERROR;  /* Failed to set EN high */
    }else{
        SYSTICK_Status_t systickStat = SYSTICK_NOT_OK;  /* Timer status */
        
        /* Wait for enable pulse width (>450ns required, using 1ms) */
        systickStat = SYSTICK_Wait_ms(1);
        
        if (SYSTICK_OK != systickStat){
            retStatus = LCD_TIMER_ERROR;  /* Timer delay failed */
        }else{
            /* Set EN pin LOW to latch data into LCD (falling edge) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            
            if (GPIO_OK != gpioStatus){
                retStatus = LCD_GPIO_ERROR;  /* Failed to set EN low */
            }else{
                /* Wait for command execution time */
                systickStat = SYSTICK_Wait_ms(1);
                
                if (SYSTICK_OK != systickStat){
                    retStatus = LCD_TIMER_ERROR;  /* Timer delay failed */
                }else{
                    retStatus = LCD_OK;  /* All operations successful */
                }
            }
        }
    }
    
    return retStatus;  /* Single exit point - MISRA C compliant */
}

/******************************************************************************
 * END OF FILE
 ******************************************************************************/



/* Initialization */
LCD_Status_t LCD_enuAsynInit(){

    LCD_Status_t retStatus = LCD_NOT_OK;
    
    retStatus = LCD_enuInitGpioPins();
    if(LCD_OK!=retStatus){
        retStatus = LCD_FAILED_TO_INIT;
    }else{
        SCHED_Status_t schedStat = SCHED_enuRegisterRunnable(&lcdRunnable);

        if(SCHED_OK!=schedStat){
            retStatus = LCD_FAILED_TO_INIT;
        }else{
            Queue_Init();
            if(LCD_4_BIT_OPERATION == LcdCong.BitOperation){
                // initSeq   = INIT_4BIT_HIGH_NIBBLE_FUNCTION_SET_HIGH;
                initSeq   = INIT_4BIT_HIGH_NIBBLE_START_SEQUANCE_HIGH;
                lcdState  = LCD_INIT;
                retStatus = LCD_OK;
            }else{
                if(LCD_8_BIT_OPERATION == LcdCong.BitOperation){
                    // initSeq   = INIT_8BIT_FUNCTION_SET_HIGH;
                    initSeq   = INIT_8BIT_START_SEQUANCE_HIGH;
                    lcdState  = LCD_INIT;
                    retStatus = LCD_OK;
                }else{
                    retStatus = LCD_WRONG_BIT_OPERATION;    
                }
            }
        }
    }

    return retStatus;
}


LCD_Status_t LCD_enuAsynWriteString(uint8_t* displayedString){
    LCD_Status_t retStatus = LCD_NOT_OK;
    if(NULL == displayedString){
        retStatus = LCD_NULL_PTR;
    }else{

        if(LCD_NO_ACTION!=lcdState){
            retStatus = LCD_BUSY;
        }else{
            if(INIT_FAILED == initSeq){
                retStatus = LCD_NOT_INITIALIZED;
            }else{
                LCD_DataBuffer_t lcdBuffer = {
                    .row = LCD_CurrentRow,
                    .col = LCD_CurrentCol,
                };
                strcpy((char*)lcdBuffer.buff, (char *)displayedString);
                Queue_Push(&lcdBuffer);

                if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
                    writeStringSeq = WRITE_STRING_4_BIT_HIGH_NIBBLE_HIGH;
                }else{
                    writeStringSeq = WRITE_STRING_8_BIT_HIGH;
                }
                lcdState = LCD_WRITE_STRING;
                retStatus = LCD_OK;
            }
        }
    }
    return retStatus;
}


LCD_Status_t LCD_enuAsynWriteStringAtPosition(uint8_t* displayedString, uint8_t row, uint8_t col){
    LCD_Status_t retStatus = LCD_NOT_OK;
    if(NULL == displayedString){
        retStatus = LCD_NULL_PTR;
    }else{

        if(LCD_NO_ACTION!=lcdState){
            retStatus = LCD_BUSY;
        }else{
            if(INIT_FAILED == initSeq){
                retStatus = LCD_NOT_INITIALIZED;
            }else{
                LCD_DataBuffer_t lcdBuffer = {
                    .row = row,
                    .col = col,
                };
                strcpy((char*)lcdBuffer.buff, (char *)displayedString);
                Queue_Push(&lcdBuffer);

                if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
                    writeStringSeq = WRITE_STRING_4BIT_HIGH_NIBBLE_CURSOR_HIGH;
                }else{
                    writeStringSeq = WRITE_STRING_8BIT_CURSOR_HIGH;
                }
                lcdState = LCD_WRITE_STRING;
                retStatus = LCD_OK;
            }
        }
    }
    return retStatus;
}

typedef struct{
    uint8_t location;
    uint8_t charmap[SPECIAL_CHAR_LENGHT];
}CustomCharBuffer_t;

static CustomCharBuffer_t CustomCharBuffer;

LCD_Status_t LCD_enuAsynCreateCustomChar(uint8_t location, const uint8_t charmap[SPECIAL_CHAR_LENGHT]){
    LCD_Status_t retStatus = LCD_NOT_OK;
    if (NULL == charmap){
        retStatus = LCD_NULL_PTR;  /* NULL pointer passed */
    }else if (location > LOCATION_MASK){
        retStatus = LCD_WRONG_LOCATION;  /* Invalid location */
    }else{
        if(LCD_NO_ACTION!=lcdState){
            retStatus = LCD_BUSY;
        }else{
            if(INIT_FAILED == initSeq){
                retStatus = LCD_NOT_INITIALIZED;
            }else{
                CustomCharBuffer.location = location;
                memcpy(CustomCharBuffer.charmap, charmap, SPECIAL_CHAR_LENGHT);
                if(LcdCong.BitOperation == LCD_8_BIT_OPERATION){
                    createCustomCharSeq = CREATE_CUSTOM_CHAR_8BIT_CURSOR_HIGH;
                }else{
                    createCustomCharSeq = CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_CURSOR_HIGH;
                }
                lcdState = LCD_CREATE_CUSTOM_CHAR;
                retStatus = LCD_OK;
            }
        }
    }
    return retStatus;
}
void static lcdRunnableCBF(){
    switch(lcdState){
        case LCD_INIT         : ExecuteInitSeq();break;
        case LCD_WRITE_STRING : ExecuteWriteString();break;
        case LCD_CREATE_CUSTOM_CHAR : ExecutCreateCustomChar();break;
        case LCD_NO_ACTION    : /* Do nothing */ break;
        default               : /* Do nothing */ break;
    }

}

static uint8_t startSeq = 0;

void static ExecuteInitSeq(){
    GPIO_Status_t gpioStatus;
    LCD_Status_t retStatus;
    
    switch(initSeq){
        case INIT_8BIT_START_SEQUANCE_HIGH :
            retStatus = LCD_WriteCommand(0x30>>ALL_BITS); /* Send 0x30 to initialize */
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);
                }
                break;
            }
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_8BIT_START_SEQUANCE_LOW;
            break;
        case INIT_8BIT_START_SEQUANCE_LOW :
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            startSeq++;
            if(startSeq < 3){
                initSeq    = INIT_8BIT_START_SEQUANCE_HIGH;
            }else{
                startSeq = 0;
                initSeq    = INIT_8BIT_FUNCTION_SET_HIGH;
            }
            break;

        case INIT_8BIT_FUNCTION_SET_HIGH :
            retStatus   = FunctionSet(LcdCong.BitOperation,LcdCong.LineDisplay,LcdCong.FontSize,ALL_BITS);
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);
                }
                break;
            }
            gpioStatus  = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_8BIT_FUNCTION_SET_LOW;
            break;

        case INIT_8BIT_FUNCTION_SET_LOW :
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_8BIT_DISPLAY_ON_HIGH;
            break;

        case INIT_8BIT_DISPLAY_ON_HIGH: 
            retStatus   = DisplayControl(LcdCong.Display,LcdCong.Cursor,LcdCong.Blink,ALL_BITS);
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);
                }
                break;
            }
            gpioStatus  = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_8BIT_DISPLAY_ON_LOW;
            break;

        case INIT_8BIT_DISPLAY_ON_LOW :
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_8BIT_CLEAR_DISPLAY_HIGH;
            break;

        case INIT_8BIT_CLEAR_DISPLAY_HIGH : 
            retStatus   = ClearDisplay( ALL_BITS);
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);
                }
                break;
            }
            gpioStatus  = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_8BIT_CLEAR_DISPLAY_LOW;
            break;

        case INIT_8BIT_CLEAR_DISPLAY_LOW :
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_8BIT_ENTRY_MODE_HIGH;
            break;

        case INIT_8BIT_ENTRY_MODE_HIGH : 
            retStatus   = EnteryModeSet(LcdCong.DisplayShiftOperation,LcdCong.IncrementStatus,ALL_BITS);
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);
                }
                break;
            }
            gpioStatus  = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_8BIT_ENTRY_MODE_LOW;
            break;

        case INIT_8BIT_ENTRY_MODE_LOW :
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_DONE;
            lcdState   = LCD_NO_ACTION;
            /* Notify user that initialization completed successfully */
            if(Lcd_Callback != NULL){
                Lcd_Callback(LCD_OK);
            }
            break;

        case INIT_4BIT_HIGH_NIBBLE_START_SEQUANCE_HIGH :
            retStatus = LCD_WriteCommand(0x30>>HIGH_NIBBLE); /* Send 0x30 to initialize */
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);
                }
                break;
            }
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_4BIT_HIGH_NIBBLE_START_SEQUANCE_LOW;
            break;
        case INIT_4BIT_HIGH_NIBBLE_START_SEQUANCE_LOW :
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            startSeq++;
            if(startSeq < 3){
                initSeq    = INIT_4BIT_HIGH_NIBBLE_START_SEQUANCE_HIGH;
            }else{
                startSeq = 0;
                initSeq    = INIT_4BIT_SWITCH_TO_4BIT_MODE_HIGH;
            }
            break;
        case INIT_4BIT_SWITCH_TO_4BIT_MODE_HIGH :
            retStatus = LCD_WriteCommand(0x20>>HIGH_NIBBLE); /* Send 0x30 to initialize */
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);
                }
                break;
            }
            gpioStatus  = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_4BIT_SWITCH_TO_4BIT_MODE_LOW;
            break;
        case INIT_4BIT_SWITCH_TO_4BIT_MODE_LOW :
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_4BIT_HIGH_NIBBLE_FUNCTION_SET_HIGH;
            break;
        case INIT_4BIT_HIGH_NIBBLE_FUNCTION_SET_HIGH :
            retStatus   = FunctionSet(LcdCong.BitOperation,LcdCong.LineDisplay,LcdCong.FontSize,HIGH_NIBBLE);
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);
                }
                break;
            }
            gpioStatus  = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_4BIT_HIGH_NIBBLE_FUNCTION_SET_LOW;
            break;
        case INIT_4BIT_HIGH_NIBBLE_FUNCTION_SET_LOW :
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_4BIT_LOW_NIBBLE_FUNCTION_SET_HIGH;
            break;  
        case INIT_4BIT_LOW_NIBBLE_FUNCTION_SET_HIGH :
            retStatus   = FunctionSet(LcdCong.BitOperation,LcdCong.LineDisplay,LcdCong.FontSize,LOW_NIBBLE);
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);
                }
                break;
            }
            gpioStatus  = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_4BIT_LOW_NIBBLE_FUNCTION_SET_LOW;
            break;
        case INIT_4BIT_LOW_NIBBLE_FUNCTION_SET_LOW :
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_4BIT_HIGH_NIBBLE_DISPLAY_ON_HIGH;
            break;
        case INIT_4BIT_HIGH_NIBBLE_DISPLAY_ON_HIGH: 
            retStatus   = DisplayControl(LcdCong.Display,LcdCong.Cursor,LcdCong.Blink,HIGH_NIBBLE);
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);
                }
                break;
            }
            gpioStatus  = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_4BIT_HIGH_NIBBLE_DISPLAY_ON_LOW;
            break;
        case INIT_4BIT_HIGH_NIBBLE_DISPLAY_ON_LOW :
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_4BIT_LOW_NIBBLE_DISPLAY_ON_HIGH;
            break;  
        case INIT_4BIT_LOW_NIBBLE_DISPLAY_ON_HIGH: 
            retStatus   = DisplayControl(LcdCong.Display,LcdCong.Cursor,LcdCong.Blink,LOW_NIBBLE);
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);
                }
                break;
            }
            gpioStatus  = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_4BIT_LOW_NIBBLE_DISPLAY_ON_LOW;
            break;
        case INIT_4BIT_LOW_NIBBLE_DISPLAY_ON_LOW :
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_4BIT_HIGH_NIBBLE_CLEAR_DISPLAY_HIGH;
            break;  
        case INIT_4BIT_HIGH_NIBBLE_CLEAR_DISPLAY_HIGH : 
            retStatus   = ClearDisplay(HIGH_NIBBLE);
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);
                }
                break;
            }
            gpioStatus  = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_4BIT_HIGH_NIBBLE_CLEAR_DISPLAY_LOW;
            break;
        case INIT_4BIT_HIGH_NIBBLE_CLEAR_DISPLAY_LOW :
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_4BIT_LOW_NIBBLE_CLEAR_DISPLAY_HIGH;
            break;  
        case INIT_4BIT_LOW_NIBBLE_CLEAR_DISPLAY_HIGH : 
            retStatus   = ClearDisplay(LOW_NIBBLE);
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);
                }
                break;
            }
            gpioStatus  = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_4BIT_LOW_NIBBLE_CLEAR_DISPLAY_LOW;
            break;
        case INIT_4BIT_LOW_NIBBLE_CLEAR_DISPLAY_LOW :
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_4BIT_HIGH_NIBBLE_ENTRY_MODE_HIGH;
            break;  
        case INIT_4BIT_HIGH_NIBBLE_ENTRY_MODE_HIGH : 
            retStatus   = EnteryModeSet(LcdCong.DisplayShiftOperation,LcdCong.IncrementStatus,HIGH_NIBBLE);
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);
                }
                break;
            }
            gpioStatus  = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_4BIT_HIGH_NIBBLE_ENTRY_MODE_LOW;
            break;
        case INIT_4BIT_HIGH_NIBBLE_ENTRY_MODE_LOW :
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_4BIT_LOW_NIBBLE_ENTRY_MODE_HIGH;
            break;  
        case INIT_4BIT_LOW_NIBBLE_ENTRY_MODE_HIGH : 
            retStatus   = EnteryModeSet(LcdCong.DisplayShiftOperation,LcdCong.IncrementStatus,LOW_NIBBLE);
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);
                }
                break;
            }
            gpioStatus  = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_4BIT_LOW_NIBBLE_ENTRY_MODE_LOW;
            break;
        case INIT_4BIT_LOW_NIBBLE_ENTRY_MODE_LOW :
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_DONE;
            lcdState   = LCD_NO_ACTION;
            /* Notify user that initialization completed successfully */
            if(Lcd_Callback != NULL){
                Lcd_Callback(LCD_INIT_SUCEESSFULLY);
            }
            break;
        case INIT_DONE :
            /* Do nothing, initialization complete */
            break;
 
    }
}

static uint8_t iterator = 0;
void static ExecuteWriteString(){
    LCD_Status_t retstat;
    GPIO_Status_t gpioStatus;
    LCD_DataBuffer_t* PointerToBufferTop = Queue_Top();
    switch(writeStringSeq){
        case WRITE_STRING_8BIT_CURSOR_HIGH : 
            retstat    = LCD_SetCursor_Local(PointerToBufferTop->row,PointerToBufferTop->col,ALL_BITS);
            if(retstat != LCD_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retstat);
                }
                break;
            }
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            writeStringSeq = WRITE_STRING_8BIT_CURSOR_LOW;
            break;

        case WRITE_STRING_8BIT_CURSOR_LOW : 
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            if('\0' == PointerToBufferTop->buff[iterator]){
                Queue_Pop();
                if(Queue_IsEmpty() == true){
                    writeStringSeq = WRITE_STRING_DONE;
                    lcdState = LCD_NO_ACTION;
                    /* Notify user that string write completed successfully */
                    if(Lcd_Callback != NULL){
                        Lcd_Callback(LCD_OK);
                    }
                }else{
                    writeStringSeq = WRITE_STRING_8BIT_CURSOR_HIGH;
                    lcdState = LCD_WRITE_STRING;
                }
                iterator =0;
            }else{
                writeStringSeq = WRITE_STRING_8_BIT_HIGH;
            }
            break;
        case WRITE_STRING_8_BIT_HIGH :
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RS.port, LcdPinout.RS.pin, GPIO_HIGH);  /* RS=1 for data */
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RW.port, LcdPinout.RW.pin, GPIO_LOW);   /* RW=0 for write */
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            retstat    = LCD_WriteByte(PointerToBufferTop->buff[iterator++]);
            if(retstat != LCD_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retstat);
                }
                break;
            }
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            writeStringSeq = WRITE_STRING_8_BIT_LOW;
            break;

        case WRITE_STRING_8_BIT_LOW :
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            LCD_CurrentCol++;
                
            /* Handle line wrap for 16-column LCD */
            if (LCD_CurrentCol >= COLUMN_LENGTH){
                LCD_CurrentCol = 0;  /* Reset column to start */
                    
                /* Move to next line */
                if (LCD_CurrentRow == 0){
                    LCD_CurrentRow = 1;  /* Move from row 0 to row 1 */
                }else{
                    LCD_CurrentRow = 0;  /* Wrap from row 1 to row 0 */
                }
                PointerToBufferTop->col = LCD_CurrentCol;
                PointerToBufferTop->row = LCD_CurrentRow;
                /* Update LCD cursor position after wrap */
                writeStringSeq = WRITE_STRING_8BIT_CURSOR_HIGH;
            }else{

                if('\0' == PointerToBufferTop->buff[iterator]){
                    Queue_Pop();
                    if(Queue_IsEmpty() == true){
                        writeStringSeq = WRITE_STRING_DONE;
                        lcdState = LCD_NO_ACTION;
                        /* Notify user that string write completed successfully */
                        if(Lcd_Callback != NULL){
                            Lcd_Callback(LCD_OK);
                        }
                    }else{
                        writeStringSeq = WRITE_STRING_8BIT_CURSOR_HIGH;
                        lcdState = LCD_WRITE_STRING;
                    }
                    iterator =0;
                }else{
                    writeStringSeq = WRITE_STRING_8_BIT_HIGH;
                }
            }
            break;

        case WRITE_STRING_4BIT_HIGH_NIBBLE_CURSOR_HIGH:
            retstat    = LCD_SetCursor_Local(PointerToBufferTop->row,PointerToBufferTop->col,HIGH_NIBBLE);
            if(retstat != LCD_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retstat);
                }
                break;
            }
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            writeStringSeq = WRITE_STRING_4BIT_HIGH_NIBBLE_CURSOR_LOW;
            break;
        case WRITE_STRING_4BIT_HIGH_NIBBLE_CURSOR_LOW:
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            writeStringSeq = WRITE_STRING_4BIT_LOW_NIBBLE_CURSOR_HIGH;
            break;
        case WRITE_STRING_4BIT_LOW_NIBBLE_CURSOR_HIGH:
            retstat    = LCD_SetCursor_Local(PointerToBufferTop->row,PointerToBufferTop->col,LOW_NIBBLE);
            if(retstat != LCD_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retstat);
                }
                break;
            }
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            writeStringSeq = WRITE_STRING_4BIT_LOW_NIBBLE_CURSOR_LOW;
            break;
        case WRITE_STRING_4BIT_LOW_NIBBLE_CURSOR_LOW:
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            if('\0' == PointerToBufferTop->buff[iterator]){
                Queue_Pop();
                if(Queue_IsEmpty() == true){
                    writeStringSeq = WRITE_STRING_DONE;
                    lcdState = LCD_NO_ACTION;
                    /* Notify user that string write completed successfully */
                    if(Lcd_Callback != NULL){
                        Lcd_Callback(LCD_OK);
                    }
                }else{
                    writeStringSeq = WRITE_STRING_4BIT_HIGH_NIBBLE_CURSOR_HIGH;
                    lcdState = LCD_WRITE_STRING;
                }
                iterator =0;
            }else{
                writeStringSeq = WRITE_STRING_4_BIT_HIGH_NIBBLE_HIGH;
            }
            break;

        case WRITE_STRING_4_BIT_HIGH_NIBBLE_HIGH :
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RS.port, LcdPinout.RS.pin, GPIO_HIGH);  /* RS=1 for data */
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RW.port, LcdPinout.RW.pin, GPIO_LOW);   /* RW=0 for write */
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            retstat    = LCD_WriteByte(PointerToBufferTop->buff[iterator] >> HIGH_NIBBLE);
            if(retstat != LCD_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retstat);
                }
                break;
            }
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            writeStringSeq = WRITE_STRING_4_BIT_HIGH_NIBBLE_LOW;
            break;
        case WRITE_STRING_4_BIT_HIGH_NIBBLE_LOW :
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            writeStringSeq = WRITE_STRING_4_BIT_LOW_NIBBLE_HIGH;
            break;  
        case WRITE_STRING_4_BIT_LOW_NIBBLE_HIGH :
            retstat    = LCD_WriteByte(PointerToBufferTop->buff[iterator++] >> LOW_NIBBLE);
            if(retstat != LCD_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retstat);
                }
                break;
            }
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            writeStringSeq = WRITE_STRING_4_BIT_LOW_NIBBLE_LOW;
            break;
        case WRITE_STRING_4_BIT_LOW_NIBBLE_LOW :
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            LCD_CurrentCol++;
            /* Handle line wrap for 16-column LCD */
            if (LCD_CurrentCol >= COLUMN_LENGTH){
                LCD_CurrentCol = 0;  /* Reset column to start */
                /* Move to next line */
                if (LCD_CurrentRow == 0){
                    LCD_CurrentRow = 1;  /* Move from row 0 to row 1 */
                }else{
                    LCD_CurrentRow = 0;  /* Wrap from row 1 to row 0 */
                }
                PointerToBufferTop->col = LCD_CurrentCol;
                PointerToBufferTop->row = LCD_CurrentRow;
                /* Update LCD cursor position after wrap */
                writeStringSeq = WRITE_STRING_4BIT_HIGH_NIBBLE_CURSOR_HIGH;
            }else{
                if('\0' == PointerToBufferTop->buff[iterator]){
                    Queue_Pop();
                    if(Queue_IsEmpty() == true){
                        writeStringSeq = WRITE_STRING_DONE;
                        lcdState = LCD_NO_ACTION;
                        /* Notify user that string write completed successfully */
                        if(Lcd_Callback != NULL){
                            Lcd_Callback(LCD_WRITE_SUCCESSFULLY);
                        }
                    }else{
                        writeStringSeq = WRITE_STRING_4BIT_HIGH_NIBBLE_CURSOR_HIGH;
                        lcdState = LCD_WRITE_STRING;
                    }
                    iterator =0;
                }else{
                    writeStringSeq = WRITE_STRING_4_BIT_HIGH_NIBBLE_HIGH;
                }
            }

    }
}


static LCD_Status_t LCD_enuInitGpioPins(){
    LCD_Status_t retStatus = LCD_OK;                            /* Function return status */

    if ((LcdCong.BitOperation != LCD_4_BIT_OPERATION)&&(LcdCong.BitOperation != LCD_8_BIT_OPERATION)){
        retStatus = LCD_WRONG_BIT_OPERATION;  /* Invalid bit operation mode */
    }else{
        uint8_t totalPins = 0;
        if(LcdCong.BitOperation == LCD_8_BIT_OPERATION){
            totalPins = 11;  /* 8 data pins + RS + RW + EN */
        }else{
            totalPins = 7;   /* 4 data pins + RS + RW + EN */
        }
        /* Configure GPIO pin settings for LCD pins */
        GPIO_cfg_t gpioCfg = {
            .alternateFunction = GPIO_AF0,                      /* No alternate function */
            .mode = GPIO_MODE_OUTPUT,                           /* Output mode for LCD control */
            .outputType = GPIO_OUTPUT_TYPE_PUSH_PULL,          /* Push-pull output */
            .pull = GPIO_NO_PULL,                              /* No pull-up/down resistor */
            .speed = GPIO_SPEED_DEFAULT                        /* Default speed */
        };

        GPIO_Status_t gpioStat = GPIO_NOT_OK;                  /* GPIO operation status */
        const LCD_PinInfo_t *ptr = &(LcdPinout.DB4);          /* Pointer to first pin (DB4) */

        /* Initialize all 11 pins in sequence (DB0-DB7, RS, RW, EN) */
        for (uint8_t i = 0; i < totalPins; i++){
            gpioCfg.port = (ptr + i)->port;  /* Get port from pin structure */
            gpioCfg.pin = (ptr + i)->pin;    /* Get pin number from pin structure */
            gpioStat = GPIO_enuInit(&gpioCfg);  /* Initialize GPIO pin */

            /* Check if GPIO initialization failed */
            if (GPIO_OK != gpioStat){
                retStatus = LCD_INIT_ERROR;  /* Set error status */
                break;
            }
        }
    }
    return retStatus;  /* Single exit point - MISRA C compliant */
}

/**
 * @brief Execute HD44780 initialization sequence in 4-bit mode
 * @details Follows HD44780 datasheet initialization procedure for 4-bit operation:
 *          1. Wait >40ms after power-up
 *          2. Send Function Set command (high nibble then low nibble)
 *          3. Send Display Control command (high nibble then low nibble)
 *          4. Send Clear Display command (high nibble then low nibble)
 *          5. Send Entry Mode Set command (high nibble then low nibble)
 * @return LCD_Status_t: Initialization status
 * @note This function implements proper timing as per HD44780 datasheet for 4-bit mode
 */
static LCD_Status_t LCD_InitSequence_4BitMode(void)
{
    LCD_Status_t retStatus = LCD_NOT_OK;        /* Function return status */
    SYSTICK_Status_t systickStat = SYSTICK_NOT_OK;  /* Timer status */
    GPIO_Status_t gpioStatus = GPIO_NOT_OK;
    uint8_t startSeqCount = 0;
        
    /* Wait for LCD power-up (>40ms after Vcc rises to 4.5V) */
    systickStat = SYSTICK_Wait_ms(40);
    
    if (SYSTICK_OK != systickStat){
        retStatus = LCD_TIMER_ERROR;  /* Timer error occurred */
    }else{
        /* ========== HD44780 Initialization by Instruction (4-bit mode) ========== */
        
        /* Start Sequence: Send 0x30 three times (high nibble only) to ensure LCD is in known state */
        for(startSeqCount = 0; startSeqCount < 3; startSeqCount++){
            /* Set RS=0 for command, RW=0 for write */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RS.port, LcdPinout.RS.pin, GPIO_LOW);
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RW.port, LcdPinout.RW.pin, GPIO_LOW);
            
            /* Send 0x30 initialization command (high nibble only) */
            retStatus = LCD_WriteByte(0x30 >> HIGH_NIBBLE);
            if (LCD_OK != retStatus){
                break;
            }
            
            /* Generate enable pulse */
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK != retStatus){
                break;
            }
            
            /* Wait between start sequence commands */
            systickStat = SYSTICK_Wait_ms(5);
            if (SYSTICK_OK != systickStat){
                retStatus = LCD_TIMER_ERROR;
                break;
            }
        }
        
        if (LCD_OK != retStatus){
            /* Start sequence failed */
        }else{
            /* Switch to 4-bit mode: Send 0x20 (high nibble only) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RS.port, LcdPinout.RS.pin, GPIO_LOW);
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RW.port, LcdPinout.RW.pin, GPIO_LOW);
            retStatus = LCD_WriteByte(0x20 >> HIGH_NIBBLE);
            
            if (LCD_OK != retStatus){
                /* Switch to 4-bit mode failed */
            }else{
                retStatus = LCD_GenerateEnablePulse();
                if (LCD_OK != retStatus){
                    /* Enable pulse failed */
                }else{
                    systickStat = SYSTICK_Wait_ms(1);
                    if (SYSTICK_OK != systickStat){
                        retStatus = LCD_TIMER_ERROR;
                    }else{
                        /* Step 1: Function Set - Configure interface, lines, and font */
                        /* Send high nibble first */
                        retStatus = FunctionSet(LcdCong.BitOperation, LcdCong.LineDisplay, LcdCong.FontSize, HIGH_NIBBLE);
        if (LCD_OK != retStatus){
            // retStatus = retStatus;  /* Preserve error status */
        }else{
            retStatus = LCD_GenerateEnablePulse();
            if (LCD_OK != retStatus){
                // retStatus = retStatus;  /* Preserve error status */
            }else{
                /* Send low nibble */
                retStatus = FunctionSet(LcdCong.BitOperation, LcdCong.LineDisplay, LcdCong.FontSize, LOW_NIBBLE);
                if (LCD_OK != retStatus){
                    // retStatus = retStatus;  /* Preserve error status */
                }else{
                    retStatus = LCD_GenerateEnablePulse();
                    if (LCD_OK != retStatus){
                        // retStatus = retStatus;  /* Preserve error status */
                    }else{
                        systickStat = SYSTICK_Wait_ms(1);  /* Wait >4.1ms as per datasheet */
                        
                        if (SYSTICK_OK != systickStat){
                            retStatus = LCD_TIMER_ERROR;
                        }else{
                            /* Step 2: Display Control - Configure display, cursor, blink */
                                    /* Send high nibble first */
                                    retStatus = DisplayControl(LcdCong.Display, LcdCong.Cursor, LcdCong.Blink, HIGH_NIBBLE);
                                    
                                    if (LCD_OK != retStatus){
                                        // retStatus = retStatus;  /* Preserve error status */
                                    }else{
                                        retStatus = LCD_GenerateEnablePulse();
                                        if (LCD_OK != retStatus){
                                            // retStatus = retStatus;  /* Preserve error status */
                                        }else{
                                            /* Send low nibble */
                                            retStatus = DisplayControl(LcdCong.Display, LcdCong.Cursor, LcdCong.Blink, LOW_NIBBLE);
                                            if (LCD_OK != retStatus){
                                                // retStatus = retStatus;  /* Preserve error status */
                                            }else{
                                                retStatus = LCD_GenerateEnablePulse();
                                                if (LCD_OK != retStatus){
                                                    // retStatus = retStatus;  /* Preserve error status */
                                                }else{
                                                    systickStat = SYSTICK_Wait_ms(1);  /* Wait >100μs */

                                                    if (SYSTICK_OK != systickStat){
                                                        retStatus = LCD_TIMER_ERROR;
                                                    }else{
                                                        /* Step 3: Clear Display - Clears all display and returns home */
                                                        /* Send high nibble first */
                                                        retStatus = ClearDisplay(HIGH_NIBBLE);
                                                        
                                                        if (LCD_OK != retStatus){
                                                            // retStatus = retStatus;  /* Preserve error status */
                                                        }else{
                                                            retStatus = LCD_GenerateEnablePulse();
                                                            if (LCD_OK != retStatus){
                                                                // retStatus = retStatus;  /* Preserve error status */
                                                            }else{
                                                                /* Send low nibble */
                                                                retStatus = ClearDisplay(LOW_NIBBLE);
                                                                if (LCD_OK != retStatus){
                                                                    // retStatus = retStatus;  /* Preserve error status */
                                                                }else{
                                                                    retStatus = LCD_GenerateEnablePulse();
                                                                    if (LCD_OK != retStatus){
                                                                        // retStatus = retStatus;  /* Preserve error status */
                                                                    }else{
                                                                        systickStat = SYSTICK_Wait_ms(2);  /* Wait for clear (1.64ms typical) */

                                                                        if (SYSTICK_OK != systickStat){
                                                                            retStatus = LCD_TIMER_ERROR;
                                                                        }else{
                                                                            /* Step 4: Entry Mode Set - Configure increment and shift */
                                                                            /* Send high nibble first */
                                                                            retStatus = EnteryModeSet(LcdCong.DisplayShiftOperation, LcdCong.IncrementStatus, HIGH_NIBBLE);
                                                                            if (LCD_OK != retStatus){
                                                                                // retStatus = retStatus;  /* Preserve error status */
                                                                            }else{
                                                                                retStatus = LCD_GenerateEnablePulse();
                                                                                if (LCD_OK != retStatus){
                                                                                    // retStatus = retStatus;  /* Preserve error status */
                                                                                }else{
                                                                                    /* Send low nibble */
                                                                                    retStatus = EnteryModeSet(LcdCong.DisplayShiftOperation, LcdCong.IncrementStatus, LOW_NIBBLE);
                                                                                    if (LCD_OK != retStatus){
                                                                                        // retStatus = retStatus;  /* Preserve error status */
                                                                                    }else{
                                                                                        retStatus = LCD_GenerateEnablePulse();
                                                                                        if (LCD_OK != retStatus){
                                                                                            // retStatus = retStatus;  /* Preserve error status */
                                                                                        }else{
                                                                                            retStatus = LCD_OK;
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    }
    }
    return retStatus;  /* Single exit point */
}


/**
 * @brief Set LCD cursor position to specified row and column
 * @details Calculates DDRAM address and updates LCD cursor position
 *          Supports both 4-bit and 8-bit operation modes
 * @param row: Target row (0 or 1 for 2-line LCD)
 * @param col: Target column (0-15 for 16-column LCD)
 * @param nibble: Bits_t - ALL_BITS, HIGH_NIBBLE, or LOW_NIBBLE
 * @return LCD_Status_t: Operation status
 * @note Updates internal cursor position tracking variables
 *       Caller is responsible for generating enable pulse
 */
static LCD_Status_t LCD_SetCursor_Local(uint8_t row, uint8_t col, Bits_t nibble)
{
    LCD_Status_t retStatus = LCD_NOT_OK;
    uint8_t address = 0;  /* DDRAM address to calculate */
    
    /* Validate column range (0-15 for 16-column LCD) */
    if (col > 15){
        retStatus = LCD_WRONG_COLUMN;  /* Column out of range */
    }else{
        /* Calculate DDRAM address based on row */
        if (row == 0){
            address = ROW_0_OFFSET + col;  /* First line: 0x00-0x0F */
            LCD_CurrentRow = row;  /* Update row tracking */
            LCD_CurrentCol = col;  /* Update column tracking */
            retStatus = LCD_OK;   /* Valid row */
        }else if (row == 1){
            address = ROW_1_OFFSET + col;  /* Second line: 0x40-0x4F */
            LCD_CurrentRow = row;  /* Update row tracking */
            LCD_CurrentCol = col;  /* Update column tracking */
            retStatus = LCD_OK;   /* Valid row */
        }else{
            retStatus = LCD_WRONG_ROW;  /* Row out of range (only 0 or 1 valid) */
        }
        
        /* Set DDRAM address if row is valid */
        if (LCD_WRONG_ROW != retStatus){
            retStatus = LCD_enuSetDDRAMAddress(address, nibble);  /* Set DDRAM address with specified nibble */
        }
    }
    
    return retStatus;
}


static uint8_t customCharIterator = 0;

static void ExecutCreateCustomChar(){
    LCD_Status_t retstat;
    GPIO_Status_t gpioStatus;
    switch(createCustomCharSeq){

        case CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_CURSOR_HIGH:
            retstat   = LCD_enuSetCGRAMAddress((CustomCharBuffer.location) << 3, HIGH_NIBBLE);
            if(retstat != LCD_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retstat);
                }
                break;
            }
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            createCustomCharSeq = CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_CURSOR_LOW;
            break;
        case CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_CURSOR_LOW:
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            createCustomCharSeq = CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_CURSOR_HIGH;
            break;
        case CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_CURSOR_HIGH:
            retstat   = LCD_enuSetCGRAMAddress((CustomCharBuffer.location) << 3, LOW_NIBBLE);
            if(retstat != LCD_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retstat);
                }
                break;
            }
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            createCustomCharSeq = CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_CURSOR_LOW;
            break;
        case CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_CURSOR_LOW:
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            customCharIterator = 0;
            createCustomCharSeq = CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_HIGH;
            break;
        case CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_HIGH:
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RS.port, LcdPinout.RS.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RW.port, LcdPinout.RW.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            retstat   = LCD_WriteByte(CustomCharBuffer.charmap[customCharIterator] >> HIGH_NIBBLE);
            if(retstat != LCD_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retstat);
                }
                break;
            }
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            createCustomCharSeq = CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_LOW;
            break;
        case CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_LOW:
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            createCustomCharSeq = CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_HIGH;
            break;
        case CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_HIGH:
            retstat    = LCD_WriteByte(CustomCharBuffer.charmap[customCharIterator++] >> LOW_NIBBLE);
            if(retstat != LCD_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retstat);
                }
                break;
            }
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            createCustomCharSeq = CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_LOW;
            break;
        case CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_LOW:
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            if(customCharIterator >= SPECIAL_CHAR_LENGHT){
                customCharIterator = 0;
                createCustomCharSeq = CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_SET_CURSOR_DDRAM_HIGH;
            }else{
                createCustomCharSeq = CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_HIGH;
            }
            break;
        case CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_SET_CURSOR_DDRAM_HIGH:
                retstat    = LCD_SetCursor_Local(LCD_CurrentRow,LCD_CurrentCol,HIGH_NIBBLE);
                if(retstat != LCD_OK){
                    createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                    lcdState = LCD_NO_ACTION;
                    if(Lcd_Callback != NULL){
                        Lcd_Callback(retstat);
                    }
                    break;
                }
                gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
                if(gpioStatus != GPIO_OK){
                    createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                    lcdState = LCD_NO_ACTION;
                    if(Lcd_Callback != NULL){
                        Lcd_Callback(LCD_GPIO_ERROR);
                    }
                    break;
                }
                createCustomCharSeq = CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_SET_CURSOR_DDRAM_LOW;
                break;
        case CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_SET_CURSOR_DDRAM_LOW:
                gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
                if(gpioStatus != GPIO_OK){
                    createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                    lcdState = LCD_NO_ACTION;
                    if(Lcd_Callback != NULL){
                        Lcd_Callback(LCD_GPIO_ERROR);
                    }
                    break;
                }
                createCustomCharSeq = CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_SET_CURSOR_DDRAM_HIGH;
                break;
        case CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_SET_CURSOR_DDRAM_HIGH :
                retstat    = LCD_SetCursor_Local(LCD_CurrentRow,LCD_CurrentCol,LOW_NIBBLE);
                if(retstat != LCD_OK){
                    createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                    lcdState = LCD_NO_ACTION;
                    if(Lcd_Callback != NULL){
                        Lcd_Callback(retstat);
                    }
                    break;
                }
                gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
                if(gpioStatus != GPIO_OK){
                    createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                    lcdState = LCD_NO_ACTION;
                    if(Lcd_Callback != NULL){
                        Lcd_Callback(LCD_GPIO_ERROR);
                    }
                    break;
                }
                createCustomCharSeq = CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_SET_CURSOR_DDRAM_LOW;
                break;
        case CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_SET_CURSOR_DDRAM_LOW :
                gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
                if(gpioStatus != GPIO_OK){
                    createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                    lcdState = LCD_NO_ACTION;
                    if(Lcd_Callback != NULL){
                        Lcd_Callback(LCD_GPIO_ERROR);
                    }
                    break;
                }
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                /* Notify user that custom character creation completed successfully */
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_OK);
                }
                break;
            
        case CREATE_CUSTOM_CHAR_8BIT_CURSOR_HIGH:
            retstat   = LCD_enuSetCGRAMAddress((CustomCharBuffer.location) << 3, ALL_BITS);
            if(retstat != LCD_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retstat);
                }
                break;
            }
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            createCustomCharSeq = CREATE_CUSTOM_CHAR_8BIT_CURSOR_LOW;
            break;
        case CREATE_CUSTOM_CHAR_8BIT_CURSOR_LOW:
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            createCustomCharSeq = CREATE_CUSTOM_CHAR_8BIT_HIGH;
            break;
        case CREATE_CUSTOM_CHAR_8BIT_HIGH:
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RS.port, LcdPinout.RS.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RW.port, LcdPinout.RW.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            retstat   = LCD_WriteByte(CustomCharBuffer.charmap[customCharIterator++]);
            if(retstat != LCD_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retstat);
                }
                break;
            }
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            createCustomCharSeq = CREATE_CUSTOM_CHAR_8BIT_LOW;  
            break;
        case CREATE_CUSTOM_CHAR_8BIT_LOW:
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            if(customCharIterator >= SPECIAL_CHAR_LENGHT){
                customCharIterator = 0;
                createCustomCharSeq = CREATE_CUSTOM_CHAR_8BIT_SET_CURSOR_DDRAM_HIGH;
            }else{
                createCustomCharSeq = CREATE_CUSTOM_CHAR_8BIT_HIGH;
            }
            break;
        case CREATE_CUSTOM_CHAR_8BIT_SET_CURSOR_DDRAM_HIGH:
                retstat    = LCD_SetCursor_Local(LCD_CurrentRow,LCD_CurrentCol,ALL_BITS);
                if(retstat != LCD_OK){
                    createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                    lcdState = LCD_NO_ACTION;
                    if(Lcd_Callback != NULL){
                        Lcd_Callback(retstat);
                    }
                    break;
                }
                gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
                if(gpioStatus != GPIO_OK){
                    createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                    lcdState = LCD_NO_ACTION;
                    if(Lcd_Callback != NULL){
                        Lcd_Callback(LCD_GPIO_ERROR);
                    }
                    break;
                }
                createCustomCharSeq = CREATE_CUSTOM_CHAR_8BIT_SET_CURSOR_DDRAM_LOW;
                break;
        case CREATE_CUSTOM_CHAR_8BIT_SET_CURSOR_DDRAM_LOW:
                gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
                if(gpioStatus != GPIO_OK){
                    createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                    lcdState = LCD_NO_ACTION;
                    if(Lcd_Callback != NULL){
                        Lcd_Callback(LCD_GPIO_ERROR);
                    }
                    break;
                }
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                /* Notify user that custom character creation completed successfully */
                if(Lcd_Callback != NULL){
                Lcd_Callback(LCD_CREATE_CUSTOM_CHAR_SUCCESSFULLY);
                }
                break;

        case CREATE_CUSTOM_CHAR_DONE:
            /* Do nothing, creation complete */
            break;
            
    }
}

LCD_Status_t LCD_enuAsynWriteCharacter(uint8_t displayedChar){
    LCD_Status_t retStatus = LCD_NOT_OK;
    if(LCD_NO_ACTION!=lcdState){
        retStatus = LCD_BUSY;
    }else{
        if(INIT_FAILED == initSeq){
            retStatus = LCD_NOT_INITIALIZED;
        }else{
            LCD_DataBuffer_t lcdBuffer;
            lcdBuffer.buff[0] = displayedChar;
            lcdBuffer.buff[1] = '\0';
            Queue_Push(&lcdBuffer);
            if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
                writeStringSeq = WRITE_STRING_4_BIT_HIGH_NIBBLE_HIGH;
            }else{
                writeStringSeq = WRITE_STRING_8_BIT_HIGH;
            }
            lcdState = LCD_WRITE_STRING;
            retStatus = LCD_OK;
        }
    }
    return retStatus;
}


LCD_Status_t LCD_AsynDisplayCustomChar(uint8_t location){
    LCD_Status_t retStatus = LCD_NOT_OK;
    if(LCD_NO_ACTION!=lcdState){
        retStatus = LCD_BUSY;
    }else{
        if(INIT_FAILED == initSeq){
            retStatus = LCD_NOT_INITIALIZED;
        }else{
    
            /* Validate location (only 0-7 valid for 8 custom characters) */
            if (location > LOCATION_MASK){
                retStatus = LCD_WRONG_LOCATION;  /* Invalid custom character location */
            }else{
                LCD_DataBuffer_t lcdBuffer;
                lcdBuffer.buff[0] = location;
                lcdBuffer.buff[1] = '\0';
                Queue_Push(&lcdBuffer);
            
                if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
                    writeStringSeq = WRITE_STRING_4_BIT_HIGH_NIBBLE_HIGH;
                }else{
                    writeStringSeq = WRITE_STRING_8_BIT_HIGH;
                }
                lcdState = LCD_WRITE_STRING;
                retStatus = LCD_OK;
            }
        }
    }
    return retStatus;
}

LCD_Status_t LCD_AsynDisplayCustomCharAtPosition(uint8_t location, uint8_t row, uint8_t col){
    LCD_Status_t retStatus = LCD_NOT_OK;
    if(LCD_NO_ACTION!=lcdState){
        retStatus = LCD_BUSY;
    }else{
        if(INIT_FAILED == initSeq){
            retStatus = LCD_NOT_INITIALIZED;
        }else{
    
            /* Validate location (only 0-7 valid for 8 custom characters) */
            if (location > LOCATION_MASK){
                retStatus = LCD_WRONG_LOCATION;  /* Invalid custom character location */
            }else{
                LCD_DataBuffer_t lcdBuffer;
                lcdBuffer.buff[0] = location;
                lcdBuffer.buff[1] = '\0';
                lcdBuffer.row = row;
                lcdBuffer.col = col;
                Queue_Push(&lcdBuffer);
            
                if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
                    writeStringSeq = WRITE_STRING_4BIT_HIGH_NIBBLE_CURSOR_HIGH;
                }else{
                    writeStringSeq = WRITE_STRING_8BIT_CURSOR_HIGH;
                }
                lcdState = LCD_WRITE_STRING;
                retStatus = LCD_OK;
            }
        }
    }
    return retStatus;
}


void LCD_vdAsyncRegisterCallback(LCD_Callback_t callback){
    Lcd_Callback = callback;
}