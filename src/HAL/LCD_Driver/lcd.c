/******************************************************************************
 * @file    lcd.c
 * @brief   LCD 16x2 Driver Implementation (HD44780 Compatible)
 * @details This driver supports 8-bit mode operation for character LCD displays
 *          Features: Character display, custom characters, cursor control
 * @author  Eng.Gemy    
 ******************************************************************************/

/******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "LIB/stdtypes.h"
#include <string.h>
#include "OS/schedule.h"
#include "MCAL/GPIO_Driver/gpio_int.h"
#include "MCAL/SYSTICK_TIMER_Driver/systick.h"
#include "HAL/LCD_Driver/lcd_queue.h"
#include "HAL/LCD_Driver/lcd.h"

/******************************************************************************
 * PRIVATE DEFINES
 ******************************************************************************/
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
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/**
 * @brief Asynchronous LCD operation states
 * @details Tracks the current asynchronous operation being executed by the LCD state machine
 *          These states are managed by the scheduler runnable (lcdRunnableCBF)
 * @note Only one operation can be active at a time to prevent conflicts
 */
typedef enum {
    LCD_NO_ACTION,              /**< Idle state - no operation in progress */
    LCD_INIT,                   /**< Initialization sequence in progress */
    LCD_WRITE_STRING,           /**< String writing operation in progress */
    LCD_CREATE_CUSTOM_CHAR,     /**< Custom character creation in progress */
}LCD_Asyn_States_t;

/**
 * @brief LCD initialization sequence states for async operation
 * @details Tracks progress through the HD44780 initialization procedure
 *          8-bit mode: Single byte per command (HIGH/LOW = EN pulse states)
 *          4-bit mode: Two nibbles per command (HIGH_NIBBLE, then LOW_NIBBLE)
 * 
 * HD44780 Init Sequence (per datasheet):
 *   1. Power-on wait (>40ms)
 *   2. Start sequence: Send 0x30 three times (8-bit interface)
 *   3. Switch to 4-bit mode (4-bit only): Send 0x20
 *   4. Function Set: Configure interface width, lines, font
 *   5. Display Control: Turn on display, cursor, blink
 *   6. Clear Display: Clear all data and return home
 *   7. Entry Mode: Set increment direction and shift mode
 * 
 * Each state represents one step in the enable pulse (HIGH → LOW transition)
 * @note HIGH states set EN pin high, LOW states set EN pin low (latch data)
 */
typedef enum {
    /* 8-bit mode initialization states */
    INIT_8BIT_START_SEQUANCE_HIGH,          /**< Start seq: EN=1 (8-bit) */
    INIT_8BIT_START_SEQUANCE_LOW,           /**< Start seq: EN=0 (latch) */
    INIT_8BIT_FUNCTION_SET_HIGH,            /**< Function set: EN=1 */
    INIT_8BIT_FUNCTION_SET_LOW,             /**< Function set: EN=0 */
    INIT_8BIT_DISPLAY_ON_HIGH,              /**< Display ctrl: EN=1 */
    INIT_8BIT_DISPLAY_ON_LOW,               /**< Display ctrl: EN=0 */
    INIT_8BIT_CLEAR_DISPLAY_HIGH,           /**< Clear display: EN=1 */
    INIT_8BIT_CLEAR_DISPLAY_LOW,            /**< Clear display: EN=0 */
    INIT_8BIT_ENTRY_MODE_HIGH,              /**< Entry mode: EN=1 */
    INIT_8BIT_ENTRY_MODE_LOW,               /**< Entry mode: EN=0 */

    /* 4-bit mode initialization states */
    INIT_4BIT_HIGH_NIBBLE_START_SEQUANCE_HIGH,  /**< Start seq: Upper nibble EN=1 */
    INIT_4BIT_HIGH_NIBBLE_START_SEQUANCE_LOW,   /**< Start seq: Upper nibble EN=0 */
    INIT_4BIT_SWITCH_TO_4BIT_MODE_HIGH,         /**< Switch to 4-bit: EN=1 */
    INIT_4BIT_SWITCH_TO_4BIT_MODE_LOW,          /**< Switch to 4-bit: EN=0 */
    INIT_4BIT_HIGH_NIBBLE_FUNCTION_SET_HIGH,    /**< Function set: Upper nibble EN=1 */
    INIT_4BIT_HIGH_NIBBLE_FUNCTION_SET_LOW,     /**< Function set: Upper nibble EN=0 */
    INIT_4BIT_HIGH_NIBBLE_DISPLAY_ON_HIGH,      /**< Display ctrl: Upper nibble EN=1 */
    INIT_4BIT_HIGH_NIBBLE_DISPLAY_ON_LOW,       /**< Display ctrl: Upper nibble EN=0 */
    INIT_4BIT_HIGH_NIBBLE_CLEAR_DISPLAY_HIGH,   /**< Clear display: Upper nibble EN=1 */
    INIT_4BIT_HIGH_NIBBLE_CLEAR_DISPLAY_LOW,    /**< Clear display: Upper nibble EN=0 */
    INIT_4BIT_HIGH_NIBBLE_ENTRY_MODE_HIGH,      /**< Entry mode: Upper nibble EN=1 */
    INIT_4BIT_HIGH_NIBBLE_ENTRY_MODE_LOW,       /**< Entry mode: Upper nibble EN=0 */
    INIT_4BIT_LOW_NIBBLE_FUNCTION_SET_HIGH,     /**< Function set: Lower nibble EN=1 */
    INIT_4BIT_LOW_NIBBLE_FUNCTION_SET_LOW,      /**< Function set: Lower nibble EN=0 */
    INIT_4BIT_LOW_NIBBLE_DISPLAY_ON_HIGH,       /**< Display ctrl: Lower nibble EN=1 */
    INIT_4BIT_LOW_NIBBLE_DISPLAY_ON_LOW,        /**< Display ctrl: Lower nibble EN=0 */
    INIT_4BIT_LOW_NIBBLE_CLEAR_DISPLAY_HIGH,    /**< Clear display: Lower nibble EN=1 */
    INIT_4BIT_LOW_NIBBLE_CLEAR_DISPLAY_LOW,     /**< Clear display: Lower nibble EN=0 */
    INIT_4BIT_LOW_NIBBLE_ENTRY_MODE_HIGH,       /**< Entry mode: Lower nibble EN=1 */
    INIT_4BIT_LOW_NIBBLE_ENTRY_MODE_LOW,        /**< Entry mode: Lower nibble EN=0 */
    
    /* Completion states */
    INIT_DONE,                                  /**< Initialization completed successfully */
    INIT_FAILED                                 /**< Initialization failed (error occurred) */
}LCD_InitSeq_t;

/**
 * @brief Custom character creation sequence states for async operation
 * @details State machine for writing custom character data to CGRAM
 * 
 * Custom Character Creation Process:
 *   1. Set CGRAM address (location × 8) to point to character storage
 *   2. Write 8 bytes of pattern data (one byte per row, 5 bits used)
 *   3. Restore DDRAM address to return cursor to display area
 * 
 * CGRAM Memory Layout:
 *   - 64 bytes total (0x00 to 0x3F)
 *   - 8 characters × 8 bytes each (5x8 font)
 *   - OR 4 characters × 8 bytes each (5x10 font)
 * 
 * @note Each byte represents one row of pixels (bit pattern: 000xxxxx)
 *       HIGH/LOW refers to enable pulse states (HIGH=set EN, LOW=latch)
 */
typedef enum {
    /* 8-bit mode custom character states */
    CREATE_CUSTOM_CHAR_8BIT_CURSOR_HIGH,              /**< Set CGRAM addr: EN=1 */
    CREATE_CUSTOM_CHAR_8BIT_CURSOR_LOW,               /**< Set CGRAM addr: EN=0 */
    CREATE_CUSTOM_CHAR_8BIT_HIGH,                     /**< Write char data: EN=1 */
    CREATE_CUSTOM_CHAR_8BIT_LOW,                      /**< Write char data: EN=0 */
    CREATE_CUSTOM_CHAR_8BIT_SET_CURSOR_DDRAM_HIGH,    /**< Restore DDRAM addr: EN=1 */
    CREATE_CUSTOM_CHAR_8BIT_SET_CURSOR_DDRAM_LOW,     /**< Restore DDRAM addr: EN=0 */

    /* 4-bit mode custom character states */
    CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_CURSOR_HIGH,           /**< Set CGRAM: Upper nibble EN=1 */
    CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_CURSOR_LOW,            /**< Set CGRAM: Upper nibble EN=0 */
    CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_CURSOR_HIGH,            /**< Set CGRAM: Lower nibble EN=1 */
    CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_CURSOR_LOW,             /**< Set CGRAM: Lower nibble EN=0 */
    CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_HIGH,                  /**< Write data: Upper nibble EN=1 */
    CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_LOW,                   /**< Write data: Upper nibble EN=0 */
    CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_HIGH,                   /**< Write data: Lower nibble EN=1 */
    CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_LOW,                    /**< Write data: Lower nibble EN=0 */
    CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_SET_CURSOR_DDRAM_HIGH, /**< Restore DDRAM: Upper nibble EN=1 */
    CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_SET_CURSOR_DDRAM_LOW,  /**< Restore DDRAM: Upper nibble EN=0 */
    CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_SET_CURSOR_DDRAM_HIGH,  /**< Restore DDRAM: Lower nibble EN=1 */
    CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_SET_CURSOR_DDRAM_LOW,   /**< Restore DDRAM: Lower nibble EN=0 */

    CREATE_CUSTOM_CHAR_DONE                                    /**< Operation complete */
}LCD_CreateCustomCharSeq_t;

/**
 * @brief String writing sequence states for async operation
 * @details State machine for writing strings character-by-character to LCD
 * 
 * String Writing Process:
 *   1. Set cursor position (if needed) using DDRAM address
 *   2. Set RS=1 (data mode), RW=0 (write mode)
 *   3. Write character byte to data pins
 *   4. Generate enable pulse to latch data
 *   5. Repeat for each character until null terminator
 *   6. Handle line wrapping automatically (16 columns → next row)
 * 
 * Queue-based Operation:
 *   - Multiple strings can be queued for sequential display
 *   - Queue is processed one character at a time (non-blocking)
 *   - Position is tracked automatically (LCD_CurrentRow/Col)
 * 
 * @note HIGH/LOW refers to enable pulse states (HIGH=set EN, LOW=latch)
 *       CURSOR states set the starting position before writing
 */
typedef enum{
    /* 8-bit mode string writing states */
    WRITE_STRING_8BIT_CURSOR_HIGH,                    /**< Set cursor position: EN=1 */
    WRITE_STRING_8BIT_CURSOR_LOW,                     /**< Set cursor position: EN=0 */
    WRITE_STRING_8_BIT_HIGH,                          /**< Write character: EN=1 */
    WRITE_STRING_8_BIT_LOW,                           /**< Write character: EN=0 */

    /* 4-bit mode string writing states */
    WRITE_STRING_4BIT_HIGH_NIBBLE_CURSOR_HIGH,        /**< Set cursor: Upper nibble EN=1 */
    WRITE_STRING_4BIT_HIGH_NIBBLE_CURSOR_LOW,         /**< Set cursor: Upper nibble EN=0 */
    WRITE_STRING_4BIT_LOW_NIBBLE_CURSOR_HIGH,         /**< Set cursor: Lower nibble EN=1 */
    WRITE_STRING_4BIT_LOW_NIBBLE_CURSOR_LOW,          /**< Set cursor: Lower nibble EN=0 */
    WRITE_STRING_4_BIT_HIGH_NIBBLE_HIGH,              /**< Write char: Upper nibble EN=1 */
    WRITE_STRING_4_BIT_HIGH_NIBBLE_LOW,               /**< Write char: Upper nibble EN=0 */
    WRITE_STRING_4_BIT_LOW_NIBBLE_HIGH,               /**< Write char: Lower nibble EN=1 */
    WRITE_STRING_4_BIT_LOW_NIBBLE_LOW,                /**< Write char: Lower nibble EN=0 */

    WRITE_STRING_DONE                                 /**< Operation complete */
}LCD_WriteStringSeq_t;

/**
 * @brief Buffer structure for custom character data in async operation
 * @details Stores custom character information during async creation process
 *          Data is copied here to prevent issues if user buffer is modified/freed
 * @note SPECIAL_CHAR_LENGHT = 8 bytes (one byte per row for 5x8 character)
 */
typedef struct{
    uint8_t location;                        /**< Character location (0-7 for 5x8 font) */
    uint8_t charmap[SPECIAL_CHAR_LENGHT];   /**< 8-byte pattern data (5 bits per byte) */
}CustomCharBuffer_t;

/******************************************************************************
 * PRIVATE CONSTANTS
 ******************************************************************************/

/**
 * @brief GPIO pin value lookup table
 * @details Maps boolean values (0,1) to GPIO states (LOW, HIGH)
 *          Index 0 = GPIO_LOW, Index 1 = GPIO_HIGH
 */
const GPIO_Val_t LCDpinVAl[] = {
    GPIO_LOW,   /* Index 0 - Logic 0 */
    GPIO_HIGH,  /* Index 1 - Logic 1 */
};

/******************************************************************************
 * PRIVATE STATIC VARIABLES
 ******************************************************************************/

/**
 * @brief Static variables to track cursor position
 * @details These variables maintain the current cursor row and column
 *          Updated automatically by write and position functions
 *          Range: Row (0-1), Column (0-15) for 16x2 LCD
 */
static uint8_t LCD_CurrentRow = 0;  /* Current row position (0 or 1) */
static uint8_t LCD_CurrentCol = 0;  /* Current column position (0-15) */

/**
 * @brief State machine variables for asynchronous LCD operations
 * @details These static variables track the current state of each async operation
 *          Only one operation can be active at a time (enforced by lcdState)
 */
static LCD_Asyn_States_t lcdState = LCD_NO_ACTION;                  /**< Current async operation (IDLE by default) */
static LCD_InitSeq_t initSeq = INIT_DONE;                           /**< Init sequence state (DONE = not initialized) */
static LCD_WriteStringSeq_t writeStringSeq = WRITE_STRING_DONE;     /**< Write string state (DONE = idle) */
LCD_CreateCustomCharSeq_t createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE; /**< Custom char state (DONE = idle) */

/**
 * @brief Counter for initialization start sequence iterations
 * @details The HD44780 requires sending 0x30 command three times during initialization
 *          This counter tracks how many times the command has been sent (0-2)
 */
static uint8_t startSeq = 0;

/**
 * @brief Character iterator for string writing state machine
 * @details Tracks current character index being written from queued string
 *          Incremented after each character write, reset to 0 when string complete
 *          Range: 0 to string length-1
 */
static uint8_t iterator = 0;

/**
 * @brief Custom character data iterator for creation state machine
 * @details Tracks current byte index being written to CGRAM during custom character creation
 *          Each custom character consists of 8 bytes (one byte per row)
 *          Incremented after each byte write, reset to 0 when character complete
 *          Range: 0 to 7 (SPECIAL_CHAR_LENGHT-1)
 */
static uint8_t customCharIterator = 0;

/**
 * @brief Static buffer for storing custom character data during async creation
 */
static CustomCharBuffer_t CustomCharBuffer;

/**
 * @brief User callback function pointer for async operation completion/errors
 * @details Invoked when an async operation completes successfully or encounters an error
 *          Allows application to be notified of LCD events without blocking
 * @note Set to NULL by default (no callback). Register via LCD_vdAsyncRegisterCallback()
 */
static LCD_Callback_t Lcd_Callback = NULL;

/******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/

/* State machine functions */
static void lcdRunnableCBF(void);
static void ExecuteInitSeq(void);
static void ExecuteWriteString(void);
static void ExecutCreateCustomChar(void);

/* Initialization functions */
static LCD_Status_t LCD_enuInitGpioPins(void);
static LCD_Status_t LCD_InitSequence_8BitMode(void);
static LCD_Status_t LCD_InitSequence_4BitMode(void);

/* Low-level hardware functions */
static LCD_Status_t LCD_WriteByte(uint8_t byte);
static LCD_Status_t LCD_WriteCommand(uint8_t cmd);
static LCD_Status_t LCD_GenerateEnablePulse(void);

/* HD44780 command functions */
static LCD_Status_t FunctionSet(LCD_BitOperation_t bitOperation, LCD_LineDisplay_t LineDisplay, LCD_FontSize_t FontSize, Bits_t bits);
static LCD_Status_t DisplayControl(LCD_Display_t Display, LCD_Cursor_t Cursor, LCD_Blink_t Blink, Bits_t bits);
static LCD_Status_t EnteryModeSet(LCD_DisplayShift_t DisplayShiftOperation, LCD_IncDec_t IncrementStatus, Bits_t bits);
static LCD_Status_t ClearDisplay(Bits_t bits);
static LCD_Status_t LCD_enuSetDDRAMAddress(uint8_t address, Bits_t bits);
static LCD_Status_t LCD_enuSetCGRAMAddress(uint8_t address, Bits_t bits);

/* Helper functions */
static LCD_Status_t LCD_SetCursor_Local(uint8_t row, uint8_t col, Bits_t nibble);

/**
 * @brief Scheduler runnable configuration for LCD asynchronous operations
 * @details Defines how the LCD state machine is executed by the OS scheduler
 * 
 * Configuration Parameters:
 *   - CBF: Callback function (lcdRunnableCBF) called periodically by scheduler
 *   - Args: Arguments passed to callback (NULL - not used)
 *   - FirstDalay_ms: Initial delay before first execution (50ms)
 *                    Allows system to stabilize after scheduler start
 *   - Periodicity_ms: Time between executions (5ms)
 *                     Each call advances state machine by one step
 *   - Priority: Execution priority (2 = medium priority)
 *               Higher number = lower priority
 * 
 * Timing Considerations:
 *   - 5ms period chosen to balance responsiveness vs CPU usage
 *   - Too fast: Wastes CPU cycles (LCD needs time between operations)
 *   - Too slow: Sluggish display updates, poor user experience
 *   - 50ms first delay ensures LCD has time to power up
 * 
 * @note Registered with scheduler in LCD_enuAsynInit()
 *       Must not be modified during operation
 *       Scheduler must be running for async operations to work
 */
static SCHED_Runnable_t lcdRunnable = {
    .CBF = lcdRunnableCBF,      /**< Callback function executed by scheduler */
    .Args = NULL,               /**< No arguments needed */
    .FirstDalay_ms = 50,        /**< 50ms initial delay (LCD power-up time) */
    .Periodicity_ms = 5,        /**< 5ms between executions (state machine step rate) */
    .Priority = 2               /**< Medium priority (adjust based on system requirements) */
};

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



/* Asynchronous LCD Initialization */

/**
 * @brief Initialize LCD asynchronously using OS scheduler
 * @details Non-blocking initialization that performs LCD setup over multiple scheduler cycles
 *          Ideal for applications where blocking delays are unacceptable
 * 
 * Initialization Steps:
 *   1. Initialize GPIO pins for LCD interface
 *   2. Register LCD runnable with OS scheduler
 *   3. Start initialization state machine (executed by scheduler)
 * 
 * Timing:
 *   - Scheduler calls lcdRunnableCBF() every 5ms
 *   - Complete initialization takes ~50-100ms depending on mode
 *   - Application can continue running during initialization
 * 
 * @return LCD_Status_t:
 *         - LCD_OK: Async initialization started successfully
 *         - LCD_FAILED_TO_INIT: GPIO init failed or scheduler registration failed
 * 
 * @note Call LCD_vdAsyncRegisterCallback() before this function to be notified of completion
 *       Do not attempt to use LCD functions until callback indicates success (LCD_INIT_SUCEESSFULLY)
 *       Requires OS scheduler to be running (SCHED_Start() must be called)
 */
LCD_Status_t LCD_enuAsynInit(){

    LCD_Status_t retStatus = LCD_NOT_OK;
    
    /* Step 1: Initialize GPIO pins based on configuration */
    retStatus = LCD_enuInitGpioPins();
    if(LCD_OK!=retStatus){
        retStatus = LCD_FAILED_TO_INIT;  /* GPIO initialization failed */
    }else{
        /* Step 2: Register LCD runnable with OS scheduler */
        SCHED_Status_t schedStat = SCHED_enuRegisterRunnable(&lcdRunnable);

        if(SCHED_OK!=schedStat){
            retStatus = LCD_FAILED_TO_INIT;  /* Scheduler registration failed */
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


/**
 * @brief Write null-terminated string asynchronously at current cursor position
 * @details Non-blocking function that queues string for display
 *          String is written character-by-character over multiple scheduler cycles
 *          Automatic line wrapping at column 16
 * 
 * Queue-based Operation:
 *   - String is copied to internal queue buffer
 *   - Multiple strings can be queued (FIFO order)
 *   - Each character written one-per-scheduler-cycle (5ms intervals)
 *   - Safe to modify/free user buffer immediately after call returns
 * 
 * Line Wrapping:
 *   - Automatic wrap at column 16 (16x2 LCD)
 *   - Row 0 column 15 → Row 1 column 0
 *   - Row 1 column 15 → Row 0 column 0 (wraps back to top)
 * 
 * @param displayedString Pointer to null-terminated ASCII string
 * 
 * @return LCD_Status_t:
 *         - LCD_OK: String queued successfully
 *         - LCD_NULL_PTR: displayedString is NULL
 *         - LCD_BUSY: Another async operation in progress
 *         - LCD_NOT_INITIALIZED: LCD initialization not complete
 * 
 * @note String starts at current cursor position (LCD_CurrentRow, LCD_CurrentCol)
 *       Callback invoked when string fully written (if registered)
 *       String buffer copied internally - safe to modify after call
 *       Maximum string length defined by queue buffer size
 */
LCD_Status_t LCD_enuAsynWriteString(uint8_t* displayedString){
    LCD_Status_t retStatus = LCD_NOT_OK;
    
    /* Validate input parameter */
    if(NULL == displayedString){
        retStatus = LCD_NULL_PTR;  /* Null pointer provided */
    }else{
        /* Check if LCD is busy with another operation */
        if(LCD_NO_ACTION!=lcdState){
            retStatus = LCD_BUSY;  /* Operation already in progress */
        }else{
            /* Check if LCD has been initialized successfully */
            if(INIT_FAILED == initSeq){
                retStatus = LCD_NOT_INITIALIZED;  /* Init not complete or failed */
            }else{
                /* Prepare buffer with current cursor position */
                LCD_DataBuffer_t lcdBuffer = {
                    .row = LCD_CurrentRow,  /* Use current row position */
                    .col = LCD_CurrentCol,  /* Use current column position */
                };
                
                /* Copy string to internal buffer (prevents user buffer modification issues) */
                strcpy((char*)lcdBuffer.buff, (char *)displayedString);
                Queue_Push(&lcdBuffer);  /* Add to display queue */

                /* Set initial state based on bit operation mode */
                if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
                    writeStringSeq = WRITE_STRING_4_BIT_HIGH_NIBBLE_HIGH;  /* Start with data write (no cursor positioning) */
                }else{
                    writeStringSeq = WRITE_STRING_8_BIT_HIGH;  /* Start with data write (no cursor positioning) */
                }
                
                /* Activate state machine */
                lcdState = LCD_WRITE_STRING;
                retStatus = LCD_OK;
            }
        }
    }
    return retStatus;
}


/**
 * @brief Write null-terminated string asynchronously at specified position
 * @details Non-blocking function that positions cursor then writes string
 *          Combines cursor positioning and string write in one operation
 *          More efficient than separate SetCursorPosition + WriteString calls
 * 
 * Operation Sequence:
 *   1. Set cursor to specified row and column
 *   2. Write string character-by-character
 *   3. Automatic line wrapping from specified position
 * 
 * @param displayedString Pointer to null-terminated ASCII string
 * @param row Row number (0 or 1 for 16x2 LCD)
 * @param col Column number (0-15 for 16-column LCD)
 * 
 * @return LCD_Status_t:
 *         - LCD_OK: Operation queued successfully
 *         - LCD_NULL_PTR: displayedString is NULL
 *         - LCD_BUSY: Another async operation in progress
 *         - LCD_NOT_INITIALIZED: LCD initialization not complete
 * 
 * @note Position is validated during execution (not immediately)
 *       Invalid position will trigger callback with error status
 *       String buffer copied internally - safe to modify after call
 *       Callback invoked when string fully written (if registered)
 */
LCD_Status_t LCD_enuAsynWriteStringAtPosition(uint8_t* displayedString, uint8_t row, uint8_t col){
    LCD_Status_t retStatus = LCD_NOT_OK;
    
    /* Validate input parameter */
    if(NULL == displayedString){
        retStatus = LCD_NULL_PTR;  /* Null pointer provided */
    }else{
        /* Check if LCD is busy with another operation */
        if(LCD_NO_ACTION!=lcdState){
            retStatus = LCD_BUSY;  /* Operation already in progress */
        }else{
            /* Check if LCD has been initialized successfully */
            if(INIT_FAILED == initSeq){
                retStatus = LCD_NOT_INITIALIZED;  /* Init not complete or failed */
            }else{
                /* Prepare buffer with specified position */
                LCD_DataBuffer_t lcdBuffer = {
                    .row = row,  /* Target row position */
                    .col = col,  /* Target column position */
                };
                
                /* Copy string to internal buffer */
                strcpy((char*)lcdBuffer.buff, (char *)displayedString);
                Queue_Push(&lcdBuffer);  /* Add to display queue */

                /* Set initial state based on bit operation mode */
                if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
                    writeStringSeq = WRITE_STRING_4BIT_HIGH_NIBBLE_CURSOR_HIGH;  /* Start with cursor positioning */
                }else{
                    writeStringSeq = WRITE_STRING_8BIT_CURSOR_HIGH;  /* Start with cursor positioning */
                }
                
                /* Activate state machine */
                lcdState = LCD_WRITE_STRING;
                retStatus = LCD_OK;
            }
        }
    }
    return retStatus;
}

/**
 * @brief Create custom character asynchronously in CGRAM
 * @details Non-blocking function that queues custom character creation
 *          Character data is written to CGRAM over multiple scheduler cycles
 * 
 * Custom Character Format:
 *   - location: Character slot (0-7 for 5x8 font, 0-3 for 5x10 font)
 *   - charmap[8]: 8 bytes defining character pattern
 *   - Each byte = one row of 5 pixels (bit pattern: 000xxxxx)
 *   - Bit 4 (MSB) = leftmost pixel, Bit 0 (LSB) = rightmost pixel
 * 
 * @param location Custom character location (0-7)
 * @param charmap[8] Array of 8 bytes defining character pattern
 * 
 * @return LCD_Status_t:
 *         - LCD_OK: Operation started successfully
 *         - LCD_NULL_PTR: charmap is NULL
 *         - LCD_WRONG_LOCATION: Invalid location (>7)
 *         - LCD_BUSY: Another async operation in progress
 *         - LCD_NOT_INITIALIZED: LCD not initialized
 * 
 * @note Character will be accessible via LCD_AsynDisplayCustomChar(location)
 *       Cursor position is preserved (returns to previous position after creation)
 *       Completion/error reported via callback (if registered)
 */
LCD_Status_t LCD_enuAsynCreateCustomChar(uint8_t location, const uint8_t charmap[SPECIAL_CHAR_LENGHT]){
    LCD_Status_t retStatus = LCD_NOT_OK;
    
    /* Validate input parameters */
    if (NULL == charmap){
        retStatus = LCD_NULL_PTR;  /* NULL pointer passed */
    }else if (location > LOCATION_MASK){
        retStatus = LCD_WRONG_LOCATION;  /* Invalid location */
    }else{
        /* Check if LCD is busy with another operation */
        if(LCD_NO_ACTION!=lcdState){
            retStatus = LCD_BUSY;
        }else{
            /* Check if LCD has been initialized */
            if(INIT_FAILED == initSeq){
                retStatus = LCD_NOT_INITIALIZED;
            }else{
                /* Copy character data to internal buffer (prevents user buffer modification issues) */
                CustomCharBuffer.location = location;
                memcpy(CustomCharBuffer.charmap, charmap, SPECIAL_CHAR_LENGHT);
                
                /* Set initial state based on bit operation mode */
                if(LcdCong.BitOperation == LCD_8_BIT_OPERATION){
                    createCustomCharSeq = CREATE_CUSTOM_CHAR_8BIT_CURSOR_HIGH;
                }else{
                    createCustomCharSeq = CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_CURSOR_HIGH;
                }
                
                /* Activate state machine */
                lcdState = LCD_CREATE_CUSTOM_CHAR;
                retStatus = LCD_OK;
            }
        }
    }
    return retStatus;
}
/**
 * @brief LCD scheduler runnable callback function
 * @details Called periodically by the OS scheduler to advance async LCD operations
 *          Dispatches to appropriate state machine based on current operation
 * 
 * Execution Flow:
 *   - Called every 5ms (configured in lcdRunnable.Periodicity_ms)
 *   - Checks lcdState to determine which operation is active
 *   - Executes one step of the active state machine per call
 *   - Non-blocking: Returns quickly to allow other tasks to run
 * 
 * @note This function is registered with the scheduler in LCD_enuAsynInit()
 *       Do not call directly - managed automatically by scheduler
 */
static void lcdRunnableCBF(){
    /* Dispatch to appropriate state machine based on current operation */
    switch(lcdState){
        case LCD_INIT         : ExecuteInitSeq();break;           /* Initialization in progress */
        case LCD_WRITE_STRING : ExecuteWriteString();break;       /* String writing in progress */
        case LCD_CREATE_CUSTOM_CHAR : ExecutCreateCustomChar();break; /* Custom char creation in progress */
        case LCD_NO_ACTION    : /* Do nothing */ break;           /* Idle state */
        default               : /* Do nothing */ break;           /* Invalid state */
    }

}

/**
 * @brief Execute one step of the asynchronous LCD initialization sequence
 * @details State machine that performs HD44780 initialization procedure over multiple calls
 *          Each call advances the state by one step (one enable pulse)
 * 
 * Initialization Process:
 *   8-bit mode:
 *     1. Send 0x30 three times (start sequence)
 *     2. Function Set (configure interface)
 *     3. Display Control (turn on display)
 *     4. Clear Display
 *     5. Entry Mode Set (set text direction)
 * 
 *   4-bit mode (additional steps):
 *     1. Send 0x30 three times (start sequence) - 8-bit mode
 *     2. Send 0x20 to switch to 4-bit mode
 *     3. Function Set (sent as two nibbles)
 *     4. Display Control (sent as two nibbles)
 *     5. Clear Display (sent as two nibbles)
 *     6. Entry Mode Set (sent as two nibbles)
 * 
 * Error Handling:
 *   - Each GPIO operation is checked for errors
 *   - On error: Set INIT_FAILED state, invoke callback, stop execution
 *   - Prevents partially initialized LCD from being used
 * 
 * @note Called by lcdRunnableCBF() when lcdState == LCD_INIT
 *       Non-blocking: Executes one state per call (5ms intervals)
 *       Uses global variables: initSeq, startSeq, LcdCong, LcdPinout, Lcd_Callback
 */
static void ExecuteInitSeq(){
    GPIO_Status_t gpioStatus;  /* GPIO operation status */
    LCD_Status_t retStatus;    /* LCD function return status */
    
    /* State machine for initialization sequence */
    switch(initSeq){
        /********** 8-BIT MODE INIT: Send start sequence (0x30) three times - EN HIGH **********/
        /* HD44780 datasheet requires 0x30 command sent 3 times with >4.1ms delay between */
        /* This ensures LCD wakes up in 8-bit mode regardless of previous state */
        case INIT_8BIT_START_SEQUANCE_HIGH :
            /* Send 0x30 command (Function Set with 8-bit interface) */
            retStatus = LCD_WriteCommand(0x30>>ALL_BITS);
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;   /* Mark initialization as failed */
                lcdState = LCD_NO_ACTION; /* Stop state machine */
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);  /* Notify user of error */
                }
                break;  /* Exit state machine */
            }
            /* Generate enable pulse HIGH (data setup time) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_8BIT_START_SEQUANCE_LOW;  /* Next: Latch command */
            break;
        /********** 8-BIT MODE INIT: Latch start sequence - EN LOW **********/
        case INIT_8BIT_START_SEQUANCE_LOW :
            /* Generate enable pulse LOW (falling edge latches command into LCD) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            startSeq++;  /* Increment start sequence counter */
            if(startSeq < 3){  /* Need to send 0x30 three times total */
                initSeq    = INIT_8BIT_START_SEQUANCE_HIGH;  /* Repeat start sequence */
            }else{
                startSeq = 0;  /* Reset counter for potential future use */
                initSeq    = INIT_8BIT_FUNCTION_SET_HIGH;  /* Proceed to Function Set command */
            }
            break;

        /********** 8-BIT MODE INIT: Send Function Set command - EN HIGH **********/
        /* Configure LCD interface: 8-bit, 2-line, 5x8 font (or per configuration) */
        case INIT_8BIT_FUNCTION_SET_HIGH :
            /* Build and send Function Set command (0b001DLNFxx) */
            retStatus   = FunctionSet(LcdCong.BitOperation,LcdCong.LineDisplay,LcdCong.FontSize,ALL_BITS);
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);
                }
                break;
            }
            /* Generate enable pulse HIGH */
            gpioStatus  = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_8BIT_FUNCTION_SET_LOW;  /* Next: Latch command */
            break;

        /********** 8-BIT MODE INIT: Latch Function Set - EN LOW **********/
        case INIT_8BIT_FUNCTION_SET_LOW :
            /* Generate enable pulse LOW (latch Function Set configuration) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_8BIT_DISPLAY_ON_HIGH;  /* Next: Configure display control */
            break;

        /********** 8-BIT MODE INIT: Send Display Control command - EN HIGH **********/
        /* Turn on display, configure cursor and blink (per configuration) */
        case INIT_8BIT_DISPLAY_ON_HIGH: 
            /* Build and send Display Control command (0b00001DCB) */
            /* D=Display ON/OFF, C=Cursor ON/OFF, B=Blink ON/OFF */
            retStatus   = DisplayControl(LcdCong.Display,LcdCong.Cursor,LcdCong.Blink,ALL_BITS);
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);
                }
                break;
            }
            /* Generate enable pulse HIGH */
            gpioStatus  = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_8BIT_DISPLAY_ON_LOW;  /* Next: Latch command */
            break;

        /********** 8-BIT MODE INIT: Latch Display Control - EN LOW **********/
        case INIT_8BIT_DISPLAY_ON_LOW :
            /* Generate enable pulse LOW (latch Display Control settings) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_8BIT_CLEAR_DISPLAY_HIGH;  /* Next: Clear display memory */
            break;

        /********** 8-BIT MODE INIT: Send Clear Display command - EN HIGH **********/
        /* Clear all display data, reset cursor to home (0,0) */
        case INIT_8BIT_CLEAR_DISPLAY_HIGH : 
            /* Send Clear Display command (0x01) - clears DDRAM and returns home */
            retStatus   = ClearDisplay( ALL_BITS);
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);
                }
                break;
            }
            /* Generate enable pulse HIGH */
            gpioStatus  = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_8BIT_CLEAR_DISPLAY_LOW;  /* Next: Latch command */
            break;

        /********** 8-BIT MODE INIT: Latch Clear Display - EN LOW **********/
        case INIT_8BIT_CLEAR_DISPLAY_LOW :
            /* Generate enable pulse LOW (latch Clear Display command) */
            /* Note: Clear Display requires ~1.64ms execution time */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_8BIT_ENTRY_MODE_HIGH;  /* Next: Configure entry mode */
            break;

        /********** 8-BIT MODE INIT: Send Entry Mode Set command - EN HIGH **********/
        /* Configure cursor movement direction and display shift behavior */
        case INIT_8BIT_ENTRY_MODE_HIGH : 
            /* Build and send Entry Mode Set command (0b000001I/DS) */
            /* I/D=Increment/Decrement, S=Display shift ON/OFF */
            retStatus   = EnteryModeSet(LcdCong.DisplayShiftOperation,LcdCong.IncrementStatus,ALL_BITS);
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);
                }
                break;
            }
            /* Generate enable pulse HIGH */
            gpioStatus  = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_8BIT_ENTRY_MODE_LOW;  /* Next: Latch final command */
            break;

        /********** 8-BIT MODE INIT: Latch Entry Mode - INITIALIZATION COMPLETE **********/
        case INIT_8BIT_ENTRY_MODE_LOW :
            /* Generate enable pulse LOW (latch Entry Mode settings) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            /* Initialization sequence complete - LCD ready for use */
            initSeq    = INIT_DONE;          /* Mark initialization complete */
            lcdState   = LCD_NO_ACTION;      /* Return state machine to idle */
            /* Notify user that initialization completed successfully */
            if(Lcd_Callback != NULL){
                Lcd_Callback(LCD_OK);  /* Success callback - LCD ready */
            }
            break;

        /********** 4-BIT MODE INIT: Send start sequence (0x30) in 8-bit mode three times **********/
        /* 4-bit initialization begins in 8-bit mode, then switches to 4-bit */
        /* Send upper nibble only (DB7-DB4), LCD interprets as 8-bit command initially */
        case INIT_4BIT_HIGH_NIBBLE_START_SEQUANCE_HIGH :
            /* Send upper nibble of 0x30 command (0x3) on DB7-DB4 */
            retStatus = LCD_WriteCommand(0x30>>HIGH_NIBBLE);
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;   /* Mark initialization as failed */
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);
                }
                break;
            }
            /* Generate enable pulse HIGH */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_4BIT_HIGH_NIBBLE_START_SEQUANCE_LOW;  /* Next: Latch command */
            break;
        /********** 4-BIT MODE INIT: Latch start sequence **********/
        case INIT_4BIT_HIGH_NIBBLE_START_SEQUANCE_LOW :
            /* Generate enable pulse LOW (latch command) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            startSeq++;  /* Increment counter */
            if(startSeq < 3){  /* Need to send 0x30 three times */
                initSeq    = INIT_4BIT_HIGH_NIBBLE_START_SEQUANCE_HIGH;  /* Repeat */
            }else{
                startSeq = 0;  /* Reset counter */
                initSeq    = INIT_4BIT_SWITCH_TO_4BIT_MODE_HIGH;  /* Switch to 4-bit mode */
            }
            break;
        /********** 4-BIT MODE INIT: Switch from 8-bit to 4-bit mode - EN HIGH **********/
        /* Send 0x20 to configure LCD for 4-bit operation */
        case INIT_4BIT_SWITCH_TO_4BIT_MODE_HIGH :
            /* Send upper nibble of 0x20 (0x2) - switches LCD to 4-bit interface */
            retStatus = LCD_WriteCommand(0x20>>HIGH_NIBBLE);
            if(retStatus != LCD_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retStatus);
                }
                break;
            }
            /* Generate enable pulse HIGH */
            gpioStatus  = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq     = INIT_4BIT_SWITCH_TO_4BIT_MODE_LOW;  /* Next: Latch mode switch */
            break;
        /********** 4-BIT MODE INIT: Latch 4-bit mode switch **********/
        /* After this, all commands must be sent as two nibbles (upper then lower) */
        case INIT_4BIT_SWITCH_TO_4BIT_MODE_LOW :
            /* Generate enable pulse LOW (latch 4-bit mode selection) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            /* LCD now in 4-bit mode - all future commands as two nibbles */
            initSeq    = INIT_4BIT_HIGH_NIBBLE_FUNCTION_SET_HIGH;  /* Send Function Set */
            break;

        /**************************************************************************
         * 4-BIT MODE INITIALIZATION SEQUENCE
         * 
         * Pattern: Each command sent as TWO nibbles (upper 4 bits, then lower 4 bits)
         * Each nibble latched with separate enable pulse (HIGH then LOW)
         * 
         * State Pattern for Each Command:
         *   1. HIGH_NIBBLE_xxx_HIGH:  Send upper 4 bits, EN=HIGH
         *   2. HIGH_NIBBLE_xxx_LOW:   Latch upper nibble, EN=LOW
         *   3. LOW_NIBBLE_xxx_HIGH:   Send lower 4 bits, EN=HIGH
         *   4. LOW_NIBBLE_xxx_LOW:    Latch lower nibble, EN=LOW
         * 
         * Commands Sent (in order):
         *   1. Start Sequence (0x30) × 3 - upper nibble only (still in 8-bit mode)
         *   2. Switch to 4-bit (0x20) - upper nibble only
         *   3. Function Set - both nibbles (configure 4-bit, 2-line, 5x8 font)
         *   4. Display Control - both nibbles (turn on display, cursor, blink)
         *   5. Clear Display - both nibbles (clear DDRAM, home cursor)
         *   6. Entry Mode Set - both nibbles (increment, no shift)
         **************************************************************************/

        /********** 4-BIT: Function Set Upper Nibble **********/
        case INIT_4BIT_HIGH_NIBBLE_FUNCTION_SET_HIGH :
            /* Send upper 4 bits of Function Set (0b001D) where D=0 for 4-bit */
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
        /********** 4-BIT: Function Set Upper Nibble - Latch **********/
        case INIT_4BIT_HIGH_NIBBLE_FUNCTION_SET_LOW :
            /* Latch upper nibble of Function Set command */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_4BIT_LOW_NIBBLE_FUNCTION_SET_HIGH;  /* Next: Send lower nibble */
            break;  
        /********** 4-BIT: Function Set Lower Nibble - EN HIGH **********/
        case INIT_4BIT_LOW_NIBBLE_FUNCTION_SET_HIGH :
            /* Send lower 4 bits of Function Set (LNFxx) */
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
        /********** 4-BIT: Function Set Lower Nibble - Latch **********/
        case INIT_4BIT_LOW_NIBBLE_FUNCTION_SET_LOW :
            /* Latch lower nibble - Function Set command complete */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_4BIT_HIGH_NIBBLE_DISPLAY_ON_HIGH;  /* Next: Display Control */
            break;
        /********** 4-BIT: Display Control Upper Nibble - EN HIGH **********/
        case INIT_4BIT_HIGH_NIBBLE_DISPLAY_ON_HIGH: 
            /* Send upper 4 bits of Display Control (0b0000) */
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
        /********** 4-BIT: Display Control Upper Nibble - Latch **********/
        case INIT_4BIT_HIGH_NIBBLE_DISPLAY_ON_LOW :
            /* Latch upper nibble of Display Control */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_4BIT_LOW_NIBBLE_DISPLAY_ON_HIGH;  /* Next: Send lower nibble */
            break;  
        /********** 4-BIT: Display Control Lower Nibble - EN HIGH **********/
        case INIT_4BIT_LOW_NIBBLE_DISPLAY_ON_HIGH: 
            /* Send lower 4 bits of Display Control (1DCB) */
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
        /********** 4-BIT: Display Control Lower Nibble - Latch **********/
        case INIT_4BIT_LOW_NIBBLE_DISPLAY_ON_LOW :
            /* Latch lower nibble - Display Control complete */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_4BIT_HIGH_NIBBLE_CLEAR_DISPLAY_HIGH;  /* Next: Clear Display */
            break;  
        /********** 4-BIT: Clear Display Upper Nibble - EN HIGH **********/
        case INIT_4BIT_HIGH_NIBBLE_CLEAR_DISPLAY_HIGH : 
            /* Send upper 4 bits of Clear Display command (0x00) */
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
        /********** 4-BIT: Clear Display Upper Nibble - Latch **********/
        case INIT_4BIT_HIGH_NIBBLE_CLEAR_DISPLAY_LOW :
            /* Latch upper nibble of Clear Display */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_4BIT_LOW_NIBBLE_CLEAR_DISPLAY_HIGH;  /* Next: Send lower nibble */
            break;  
        /********** 4-BIT: Clear Display Lower Nibble - EN HIGH **********/
        case INIT_4BIT_LOW_NIBBLE_CLEAR_DISPLAY_HIGH : 
            /* Send lower 4 bits of Clear Display command (0x01) */
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
        /********** 4-BIT: Clear Display Lower Nibble - Latch **********/
        case INIT_4BIT_LOW_NIBBLE_CLEAR_DISPLAY_LOW :
            /* Latch lower nibble - Clear Display complete (~1.64ms execution) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_4BIT_HIGH_NIBBLE_ENTRY_MODE_HIGH;  /* Next: Entry Mode Set */
            break;  
        /********** 4-BIT: Entry Mode Set Upper Nibble - EN HIGH **********/
        case INIT_4BIT_HIGH_NIBBLE_ENTRY_MODE_HIGH : 
            /* Send upper 4 bits of Entry Mode Set (0b0000) */
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
        /********** 4-BIT: Entry Mode Set Upper Nibble - Latch **********/
        case INIT_4BIT_HIGH_NIBBLE_ENTRY_MODE_LOW :
            /* Latch upper nibble of Entry Mode Set */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            initSeq    = INIT_4BIT_LOW_NIBBLE_ENTRY_MODE_HIGH;  /* Next: Send lower nibble (final command) */
            break;  
        /********** 4-BIT: Entry Mode Set Lower Nibble - EN HIGH **********/
        case INIT_4BIT_LOW_NIBBLE_ENTRY_MODE_HIGH : 
            /* Send lower 4 bits of Entry Mode Set (01I/DS) - final init command */
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
        /********** 4-BIT: Entry Mode Lower Nibble - INITIALIZATION COMPLETE **********/
        case INIT_4BIT_LOW_NIBBLE_ENTRY_MODE_LOW :
            /* Latch lower nibble of Entry Mode Set (final init command) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                initSeq = INIT_FAILED;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            /* 4-bit initialization sequence complete - LCD ready for use */
            initSeq    = INIT_DONE;          /* Mark initialization complete */
            lcdState   = LCD_NO_ACTION;      /* Return state machine to idle */
            /* Notify user that initialization completed successfully */
            if(Lcd_Callback != NULL){
                Lcd_Callback(LCD_INIT_SUCEESSFULLY);  /* Success - LCD ready */
            }
            break;
        /********** INIT_DONE: Idle state - initialization already complete **********/
        case INIT_DONE :
            /* Do nothing, initialization already complete */
            /* This state persists after init finishes */
            break;
 
    }
}

/**
 * @brief Execute one step of the asynchronous string writing sequence
 * @details State machine that writes queued strings character-by-character
 *          Each call processes one state (one enable pulse or one character)
 * 
 * String Writing Process:
 *   8-bit mode:
 *     1. CURSOR_HIGH/LOW: Position cursor (if needed)
 *     2. 8_BIT_HIGH: Set RS=1, RW=0, write data byte, EN=1
 *     3. 8_BIT_LOW: EN=0 (latch), increment iterator, check for wrap/completion
 * 
 *   4-bit mode:
 *     1. HIGH_NIBBLE_CURSOR_HIGH/LOW: Position cursor upper nibble (if needed)
 *     2. LOW_NIBBLE_CURSOR_HIGH/LOW: Position cursor lower nibble (if needed)
 *     3. HIGH_NIBBLE_HIGH: Set RS=1, RW=0, write upper nibble, EN=1
 *     4. HIGH_NIBBLE_LOW: EN=0 (latch)
 *     5. LOW_NIBBLE_HIGH: Write lower nibble, EN=1
 *     6. LOW_NIBBLE_LOW: EN=0 (latch), increment iterator, check for wrap/completion
 * 
 * Line Wrapping:
 *   - Automatic wrap at column 16
 *   - Row 0 → Row 1, Row 1 → Row 0
 *   - Cursor position updated and state resets to CURSOR_HIGH
 * 
 * Queue Management:
 *   - Processes top string from queue
 *   - When string complete (null terminator), pops from queue
 *   - If queue empty: State → WRITE_STRING_DONE, invoke callback(LCD_OK)
 *   - If queue has more: Process next string (state → CURSOR_HIGH)
 * 
 * Error Handling:
 *   - Each GPIO/LCD operation checked for errors
 *   - On error: State → WRITE_STRING_DONE, invoke callback with error code
 *   - Prevents incomplete writes and provides error feedback
 * 
 * @note Called by lcdRunnableCBF() when lcdState == LCD_WRITE_STRING
 *       Non-blocking: Executes one state per call (5ms intervals)
 *       Uses global variables: writeStringSeq, iterator, lcdState, Lcd_Callback
 *       Queue functions: Queue_Top(), Queue_Pop(), Queue_IsEmpty()
 */
static void ExecuteWriteString(){
    LCD_Status_t retstat;          /* LCD function return status */
    GPIO_Status_t gpioStatus;      /* GPIO operation status */
    LCD_DataBuffer_t* PointerToBufferTop = Queue_Top();  /* Get pointer to current queued string */
    
    /* State machine for string writing sequence */
    switch(writeStringSeq){
        /********** 8-BIT MODE: Set cursor position for string start **********/
        case WRITE_STRING_8BIT_CURSOR_HIGH : 
            /* Send Set DDRAM Address command to position cursor */
            retstat    = LCD_SetCursor_Local(PointerToBufferTop->row,PointerToBufferTop->col,ALL_BITS);
            if(retstat != LCD_OK){
                writeStringSeq = WRITE_STRING_DONE;  /* Abort on error */
                lcdState = LCD_NO_ACTION;            /* Return to idle */
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retstat);  /* Notify user of error */
                }
                break;  /* Exit state machine */
            }
            /* Generate enable pulse HIGH (data setup time) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;  /* Abort on GPIO error */
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);  /* Report GPIO failure */
                }
                break;
            }
            writeStringSeq = WRITE_STRING_8BIT_CURSOR_LOW;  /* Next: Latch command */
            break;

        case WRITE_STRING_8BIT_CURSOR_LOW : 
            /* Generate enable pulse LOW (falling edge latches command) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            /* Check if current string is complete (null terminator reached) */
            if('\0' == PointerToBufferTop->buff[iterator]){
                Queue_Pop();  /* Remove completed string from queue */
                if(Queue_IsEmpty() == true){
                    /* No more strings to write */
                    writeStringSeq = WRITE_STRING_DONE;  /* Mark operation complete */
                    lcdState = LCD_NO_ACTION;            /* Return to idle */
                    /* Notify user that string write completed successfully */
                    if(Lcd_Callback != NULL){
                        Lcd_Callback(LCD_OK);  /* Success callback */
                    }
                }else{
                    /* More strings in queue - process next one */
                    writeStringSeq = WRITE_STRING_8BIT_CURSOR_HIGH;  /* Position cursor for next string */
                    lcdState = LCD_WRITE_STRING;  /* Keep state machine active */
                }
                iterator =0;  /* Reset character iterator for next string */
            }else{
                /* More characters to write in current string */
                writeStringSeq = WRITE_STRING_8_BIT_HIGH;  /* Write next character */
            }
            break;
        /********** 8-BIT MODE: Write character data (EN high) **********/
        case WRITE_STRING_8_BIT_HIGH :
            /* Set RS=1 to select data register (character data) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RS.port, LcdPinout.RS.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;  /* Abort on error */
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);  /* Report GPIO failure */
                }
                break;
            }
            /* Set RW=0 to select write operation */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RW.port, LcdPinout.RW.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            /* Write character byte to data pins (DB0-DB7) and increment iterator */
            retstat    = LCD_WriteByte(PointerToBufferTop->buff[iterator++]);
            if(retstat != LCD_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retstat);  /* Report write error */
                }
                break;
            }
            /* Generate enable pulse HIGH (data setup time) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            writeStringSeq = WRITE_STRING_8_BIT_LOW;  /* Next: Latch data */
            break;

        /********** 8-BIT MODE: Latch character data (EN low) and handle line wrap **********/
        case WRITE_STRING_8_BIT_LOW :
            /* Generate enable pulse LOW (falling edge latches data into LCD) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            /* Update cursor position tracking (LCD auto-increments internally) */
            LCD_CurrentCol++;
                
            /* Handle automatic line wrap for 16-column LCD */
            if (LCD_CurrentCol >= COLUMN_LENGTH){  /* Column 16 = overflow */
                LCD_CurrentCol = 0;  /* Reset to first column */
                    
                /* Toggle row (0↔1) for 2-line display */
                if (LCD_CurrentRow == 0){
                    LCD_CurrentRow = 1;  /* Move from row 0 to row 1 */
                }else{
                    LCD_CurrentRow = 0;  /* Wrap from row 1 back to row 0 */
                }
                /* Update buffer with new position for next character */
                PointerToBufferTop->col = LCD_CurrentCol;
                PointerToBufferTop->row = LCD_CurrentRow;
                /* Send cursor positioning command on next cycle */
                writeStringSeq = WRITE_STRING_8BIT_CURSOR_HIGH;
            }else{
                /* No wrap needed - check for string completion */
                if('\0' == PointerToBufferTop->buff[iterator]){
                    Queue_Pop();  /* Remove completed string from queue */
                    if(Queue_IsEmpty() == true){
                        /* All queued strings written */
                        writeStringSeq = WRITE_STRING_DONE;
                        lcdState = LCD_NO_ACTION;
                        /* Notify user that string write completed successfully */
                        if(Lcd_Callback != NULL){
                            Lcd_Callback(LCD_OK);  /* Success callback */
                        }
                    }else{
                        /* Process next queued string */
                        writeStringSeq = WRITE_STRING_8BIT_CURSOR_HIGH;  /* Position cursor */
                        lcdState = LCD_WRITE_STRING;
                    }
                    iterator =0;  /* Reset iterator for next string */
                }else{
                    /* Continue writing current string */
                    writeStringSeq = WRITE_STRING_8_BIT_HIGH;  /* Write next character */
                }
            }
            break;

        /********** 4-BIT MODE: Set cursor position - Send upper nibble of DDRAM address **********/
        case WRITE_STRING_4BIT_HIGH_NIBBLE_CURSOR_HIGH:
            /* Send upper 4 bits of Set DDRAM Address command (bits 7-4) */
            retstat    = LCD_SetCursor_Local(PointerToBufferTop->row,PointerToBufferTop->col,HIGH_NIBBLE);
            if(retstat != LCD_OK){
                writeStringSeq = WRITE_STRING_DONE;  /* Abort on error */
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retstat);  /* Report error to user */
                }
                break;
            }
            /* Generate enable pulse HIGH for upper nibble */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            writeStringSeq = WRITE_STRING_4BIT_HIGH_NIBBLE_CURSOR_LOW;  /* Next: Latch upper nibble */
            break;
        case WRITE_STRING_4BIT_HIGH_NIBBLE_CURSOR_LOW:
            /* Generate enable pulse LOW (latch upper nibble) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            writeStringSeq = WRITE_STRING_4BIT_LOW_NIBBLE_CURSOR_HIGH;  /* Next: Send lower nibble */
            break;
        /********** 4-BIT MODE: Set cursor position - Send lower nibble of DDRAM address **********/
        case WRITE_STRING_4BIT_LOW_NIBBLE_CURSOR_HIGH:
            /* Send lower 4 bits of Set DDRAM Address command (bits 3-0) */
            retstat    = LCD_SetCursor_Local(PointerToBufferTop->row,PointerToBufferTop->col,LOW_NIBBLE);
            if(retstat != LCD_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retstat);
                }
                break;
            }
            /* Generate enable pulse HIGH for lower nibble */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            writeStringSeq = WRITE_STRING_4BIT_LOW_NIBBLE_CURSOR_LOW;  /* Next: Latch lower nibble */
            break;
        case WRITE_STRING_4BIT_LOW_NIBBLE_CURSOR_LOW:
            /* Generate enable pulse LOW (latch lower nibble - cursor now positioned) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            /* Check if current string is complete (null terminator reached) */
            if('\0' == PointerToBufferTop->buff[iterator]){
                Queue_Pop();  /* Remove completed string from queue */
                if(Queue_IsEmpty() == true){
                    /* No more strings to write */
                    writeStringSeq = WRITE_STRING_DONE;
                    lcdState = LCD_NO_ACTION;
                    /* Notify user that string write completed successfully */
                    if(Lcd_Callback != NULL){
                        Lcd_Callback(LCD_OK);  /* Success callback */
                    }
                }else{
                    /* More strings in queue - process next one */
                    writeStringSeq = WRITE_STRING_4BIT_HIGH_NIBBLE_CURSOR_HIGH;
                    lcdState = LCD_WRITE_STRING;
                }
                iterator =0;  /* Reset character iterator */
            }else{
                /* More characters to write - start character write sequence */
                writeStringSeq = WRITE_STRING_4_BIT_HIGH_NIBBLE_HIGH;
            }
            break;

        /********** 4-BIT MODE: Write character upper nibble (EN high) **********/
        case WRITE_STRING_4_BIT_HIGH_NIBBLE_HIGH :
            /* Set RS=1 to select data register (character data) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RS.port, LcdPinout.RS.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            /* Set RW=0 to select write operation */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RW.port, LcdPinout.RW.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            /* Write upper 4 bits of character to DB4-DB7 (bits 7-4) */
            retstat    = LCD_WriteByte(PointerToBufferTop->buff[iterator] >> HIGH_NIBBLE);
            if(retstat != LCD_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retstat);
                }
                break;
            }
            /* Generate enable pulse HIGH for upper nibble */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            writeStringSeq = WRITE_STRING_4_BIT_HIGH_NIBBLE_LOW;  /* Next: Latch upper nibble */
            break;
        /********** 4-BIT MODE: Latch character upper nibble (EN low) **********/
        case WRITE_STRING_4_BIT_HIGH_NIBBLE_LOW :
            /* Generate enable pulse LOW (latch upper nibble) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            writeStringSeq = WRITE_STRING_4_BIT_LOW_NIBBLE_HIGH;  /* Next: Send lower nibble */
            break;  
        /********** 4-BIT MODE: Write character lower nibble (EN high) **********/
        case WRITE_STRING_4_BIT_LOW_NIBBLE_HIGH :
            /* Write lower 4 bits of character to DB4-DB7 (bits 3-0) and increment iterator */
            retstat    = LCD_WriteByte(PointerToBufferTop->buff[iterator++] >> LOW_NIBBLE);
            if(retstat != LCD_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retstat);
                }
                break;
            }
            /* Generate enable pulse HIGH for lower nibble */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            writeStringSeq = WRITE_STRING_4_BIT_LOW_NIBBLE_LOW;  /* Next: Latch lower nibble */
            break;
        /********** 4-BIT MODE: Latch character lower nibble (EN low) and handle line wrap **********/
        case WRITE_STRING_4_BIT_LOW_NIBBLE_LOW :
            /* Generate enable pulse LOW (latch lower nibble - complete character written) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                writeStringSeq = WRITE_STRING_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            /* Update cursor position tracking (LCD auto-increments internally) */
            LCD_CurrentCol++;
            /* Handle automatic line wrap for 16-column LCD */
            if (LCD_CurrentCol >= COLUMN_LENGTH){  /* Column 16 = overflow */
                LCD_CurrentCol = 0;  /* Reset to first column */
                /* Toggle row (0↔1) for 2-line display */
                if (LCD_CurrentRow == 0){
                    LCD_CurrentRow = 1;  /* Move from row 0 to row 1 */
                }else{
                    LCD_CurrentRow = 0;  /* Wrap from row 1 back to row 0 */
                }
                /* Update buffer with new position for next character */
                PointerToBufferTop->col = LCD_CurrentCol;
                PointerToBufferTop->row = LCD_CurrentRow;
                /* Send cursor positioning command on next cycle */
                writeStringSeq = WRITE_STRING_4BIT_HIGH_NIBBLE_CURSOR_HIGH;
            }else{
                /* No wrap needed - check for string completion */
                if('\0' == PointerToBufferTop->buff[iterator]){
                    Queue_Pop();  /* Remove completed string from queue */
                    if(Queue_IsEmpty() == true){
                        /* All queued strings written */
                        writeStringSeq = WRITE_STRING_DONE;
                        lcdState = LCD_NO_ACTION;
                        /* Notify user that string write completed successfully */
                        if(Lcd_Callback != NULL){
                            Lcd_Callback(LCD_WRITE_SUCCESSFULLY);  /* Success callback */
                        }
                    }else{
                        /* Process next queued string */
                        writeStringSeq = WRITE_STRING_4BIT_HIGH_NIBBLE_CURSOR_HIGH;  /* Position cursor */
                        lcdState = LCD_WRITE_STRING;
                    }
                    iterator =0;  /* Reset iterator for next string */
                }else{
                    /* Continue writing current string */
                    writeStringSeq = WRITE_STRING_4_BIT_HIGH_NIBBLE_HIGH;  /* Write next character */
                }
            }

    }
}

/**
 * @brief Initialize GPIO pins for LCD interface
 * @details Configures all GPIO pins required for LCD communication
 *          Pins initialized depend on bit operation mode:
 *          - 8-bit mode: 11 pins (DB0-DB7, RS, RW, EN)
 *          - 4-bit mode: 7 pins (DB4-DB7, RS, RW, EN)
 * 
 * GPIO Configuration:
 *   - Mode: Output (GPIO_MODE_OUTPUT)
 *   - Type: Push-pull (GPIO_OUTPUT_TYPE_PUSH_PULL)
 *   - Pull: No pull resistor (GPIO_NO_PULL)
 *   - Speed: Default speed (GPIO_SPEED_DEFAULT)
 *   - Alternate Function: None (GPIO_AF0)
 * 
 * Pin Initialization Order:
 *   8-bit mode: DB4, DB5, DB6, DB7, EN, RW, RS, DB0, DB1, DB2, DB3
 *   4-bit mode: DB4, DB5, DB6, DB7, EN, RW, RS
 * 
 * @return LCD_Status_t:
 *         - LCD_OK: All pins initialized successfully
 *         - LCD_WRONG_BIT_OPERATION: Invalid bit operation mode in configuration
 *         - LCD_INIT_ERROR: GPIO initialization failed for one or more pins
 * 
 * @note Must be called before any LCD operations
 *       Uses LcdCong.BitOperation to determine which pins to initialize
 *       Uses LcdPinout structure for port/pin information
 *       Stops initialization and returns error on first failure
 */
static LCD_Status_t LCD_enuInitGpioPins(){
    LCD_Status_t retStatus = LCD_OK;                            /* Function return status */

    /* Validate bit operation mode from configuration */
    if ((LcdCong.BitOperation != LCD_4_BIT_OPERATION)&&(LcdCong.BitOperation != LCD_8_BIT_OPERATION)){
        retStatus = LCD_WRONG_BIT_OPERATION;  /* Invalid bit operation mode */
    }else{
        uint8_t totalPins = 0;  /* Total number of pins to initialize */
        
        /* Determine number of pins based on operation mode */
        if(LcdCong.BitOperation == LCD_8_BIT_OPERATION){
            totalPins = 11;  /* 8 data pins + RS + RW + EN */
        }else{
            totalPins = 7;   /* 4 data pins (DB4-DB7) + RS + RW + EN */
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

        /* Initialize all pins in sequence */
        /* Note: In 8-bit mode, structure is: DB4-DB7, EN, RW, RS, DB0-DB3 */
        /*       In 4-bit mode, only first 7 are used: DB4-DB7, EN, RW, RS */
        for (uint8_t i = 0; i < totalPins; i++){
            gpioCfg.port = (ptr + i)->port;  /* Get port from pin structure */
            gpioCfg.pin = (ptr + i)->pin;    /* Get pin number from pin structure */
            gpioStat = GPIO_enuInit(&gpioCfg);  /* Initialize GPIO pin */

            /* Check if GPIO initialization failed */
            if (GPIO_OK != gpioStat){
                retStatus = LCD_INIT_ERROR;  /* Set error status */
                break;  /* Stop on first error */
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
    LCD_Status_t retStatus = LCD_NOT_OK;               /* Function return status */
    SYSTICK_Status_t systickStat = SYSTICK_NOT_OK;     /* Timer status */
    GPIO_Status_t gpioStatus = GPIO_NOT_OK;            /* GPIO operation status */
    uint8_t startSeqCount = 0;                         /* Counter for start sequence iterations */
        
    /* ========== Step 1: Wait for LCD power-up (>40ms after Vcc rises to 4.5V) ========== */
    systickStat = SYSTICK_Wait_ms(40);
    if (SYSTICK_OK != systickStat){
        return LCD_TIMER_ERROR;
    }
    
    /* ========== Step 2: Start Sequence - Send 0x30 three times ========== */
    /* Send 0x30 (high nibble only) three times to ensure LCD is in known state */
    for (startSeqCount = 0; startSeqCount < 3; startSeqCount++){
        /* Set RS=0 for command, RW=0 for write */
        gpioStatus = GPIO_enuSetPinVal(LcdPinout.RS.port, LcdPinout.RS.pin, GPIO_LOW);
        gpioStatus = GPIO_enuSetPinVal(LcdPinout.RW.port, LcdPinout.RW.pin, GPIO_LOW);
        
        /* Send 0x30 initialization command (high nibble only) */
        retStatus = LCD_WriteByte(0x30 >> HIGH_NIBBLE);
        if (LCD_OK != retStatus){
            return retStatus;
        }
        
        /* Generate enable pulse */
        retStatus = LCD_GenerateEnablePulse();
        if (LCD_OK != retStatus){
            return retStatus;
        }
        
        /* Wait between start sequence commands */
        systickStat = SYSTICK_Wait_ms(5);
        if (SYSTICK_OK != systickStat){
            return LCD_TIMER_ERROR;
        }
    }
    
    /* ========== Step 3: Switch to 4-bit mode - Send 0x20 ========== */
    gpioStatus = GPIO_enuSetPinVal(LcdPinout.RS.port, LcdPinout.RS.pin, GPIO_LOW);
    gpioStatus = GPIO_enuSetPinVal(LcdPinout.RW.port, LcdPinout.RW.pin, GPIO_LOW);
    retStatus = LCD_WriteByte(0x20 >> HIGH_NIBBLE);
    if (LCD_OK != retStatus){
        return retStatus;
    }
    
    retStatus = LCD_GenerateEnablePulse();
    if (LCD_OK != retStatus){
        return retStatus;
    }
    
    systickStat = SYSTICK_Wait_ms(1);
    if (SYSTICK_OK != systickStat){
        return LCD_TIMER_ERROR;
    }
    
    /* ========== Step 4: Function Set - Configure interface, lines, and font ========== */
    /* Send high nibble of Function Set command */
    retStatus = FunctionSet(LcdCong.BitOperation, LcdCong.LineDisplay, LcdCong.FontSize, HIGH_NIBBLE);
    if (LCD_OK != retStatus){
        return retStatus;
    }
    
    retStatus = LCD_GenerateEnablePulse();
    if (LCD_OK != retStatus){
        return retStatus;
    }
    
    /* Send low nibble of Function Set command */
    retStatus = FunctionSet(LcdCong.BitOperation, LcdCong.LineDisplay, LcdCong.FontSize, LOW_NIBBLE);
    if (LCD_OK != retStatus){
        return retStatus;
    }
    
    retStatus = LCD_GenerateEnablePulse();
    if (LCD_OK != retStatus){
        return retStatus;
    }
    
    systickStat = SYSTICK_Wait_ms(1);  /* Wait >4.1ms as per datasheet */
    if (SYSTICK_OK != systickStat){
        return LCD_TIMER_ERROR;
    }
    
    /* ========== Step 5: Display Control - Configure display, cursor, blink ========== */
    /* Send high nibble of Display Control command */
    retStatus = DisplayControl(LcdCong.Display, LcdCong.Cursor, LcdCong.Blink, HIGH_NIBBLE);
    if (LCD_OK != retStatus){
        return retStatus;
    }
    
    retStatus = LCD_GenerateEnablePulse();
    if (LCD_OK != retStatus){
        return retStatus;
    }
    
    /* Send low nibble of Display Control command */
    retStatus = DisplayControl(LcdCong.Display, LcdCong.Cursor, LcdCong.Blink, LOW_NIBBLE);
    if (LCD_OK != retStatus){
        return retStatus;
    }
    
    retStatus = LCD_GenerateEnablePulse();
    if (LCD_OK != retStatus){
        return retStatus;
    }
    
    systickStat = SYSTICK_Wait_ms(1);  /* Wait >100μs */
    if (SYSTICK_OK != systickStat){
        return LCD_TIMER_ERROR;
    }
    
    /* ========== Step 6: Clear Display - Clears all display and returns home ========== */
    /* Send high nibble of Clear Display command */
    retStatus = ClearDisplay(HIGH_NIBBLE);
    if (LCD_OK != retStatus){
        return retStatus;
    }
    
    retStatus = LCD_GenerateEnablePulse();
    if (LCD_OK != retStatus){
        return retStatus;
    }
    
    /* Send low nibble of Clear Display command */
    retStatus = ClearDisplay(LOW_NIBBLE);
    if (LCD_OK != retStatus){
        return retStatus;
    }
    
    retStatus = LCD_GenerateEnablePulse();
    if (LCD_OK != retStatus){
        return retStatus;
    }
    
    systickStat = SYSTICK_Wait_ms(2);  /* Wait for clear (1.64ms typical) */
    if (SYSTICK_OK != systickStat){
        return LCD_TIMER_ERROR;
    }
    
    /* ========== Step 7: Entry Mode Set - Configure increment and shift ========== */
    /* Send high nibble of Entry Mode Set command */
    retStatus = EnteryModeSet(LcdCong.DisplayShiftOperation, LcdCong.IncrementStatus, HIGH_NIBBLE);
    if (LCD_OK != retStatus){
        return retStatus;
    }
    
    retStatus = LCD_GenerateEnablePulse();
    if (LCD_OK != retStatus){
        return retStatus;
    }
    
    /* Send low nibble of Entry Mode Set command */
    retStatus = EnteryModeSet(LcdCong.DisplayShiftOperation, LcdCong.IncrementStatus, LOW_NIBBLE);
    if (LCD_OK != retStatus){
        return retStatus;
    }
    
    retStatus = LCD_GenerateEnablePulse();
    if (LCD_OK != retStatus){
        return retStatus;
    }
    
    /* ========== Initialization Complete ========== */
    return LCD_OK;
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

/**
 * @brief Execute one step of the asynchronous custom character creation sequence
 * @details State machine that writes custom character data to CGRAM byte-by-byte
 *          Each call processes one state (one enable pulse or one data byte)
 * 
 * Custom Character Creation Process:
 *   8-bit mode:
 *     1. CURSOR_HIGH/LOW: Set CGRAM address (location × 8)
 *     2. HIGH/LOW: Write 8 bytes of character data (one byte per row)
 *     3. SET_CURSOR_DDRAM_HIGH/LOW: Restore cursor to DDRAM (preserve position)
 * 
 *   4-bit mode:
 *     1. HIGH_NIBBLE_CURSOR_HIGH/LOW: Set CGRAM address upper nibble
 *     2. LOW_NIBBLE_CURSOR_HIGH/LOW: Set CGRAM address lower nibble
 *     3. HIGH_NIBBLE_HIGH/LOW: Write data upper nibble (repeated 8 times)
 *     4. LOW_NIBBLE_HIGH/LOW: Write data lower nibble (repeated 8 times)
 *     5. HIGH_NIBBLE_SET_CURSOR_DDRAM: Restore DDRAM address upper nibble
 *     6. LOW_NIBBLE_SET_CURSOR_DDRAM: Restore DDRAM address lower nibble
 * 
 * CGRAM Address Calculation:
 *   - Each character occupies 8 bytes in CGRAM
 *   - Address = location × 8 (location 0 = 0x00, location 1 = 0x08, etc.)
 *   - Valid locations: 0-7 for 5x8 font, 0-3 for 5x10 font
 * 
 * Character Data Format:
 *   - 8 bytes total (stored in CustomCharBuffer.charmap[])
 *   - Each byte = one row of pixels (5 bits used: 000xxxxx)
 *   - Bit 4 (MSB) = leftmost pixel, Bit 0 (LSB) = rightmost pixel
 *   - Iterator tracks which byte (0-7) is currently being written
 * 
 * Cursor Position Preservation:
 *   - After writing to CGRAM, cursor must be moved back to DDRAM
 *   - Uses LCD_CurrentRow and LCD_CurrentCol to restore position
 *   - Ensures subsequent writes appear at expected location
 * 
 * Error Handling:
 *   - Each GPIO/LCD operation checked for errors
 *   - On error: State → CREATE_CUSTOM_CHAR_DONE, invoke callback with error
 *   - Prevents partial character creation and provides error feedback
 * 
 * @note Called by lcdRunnableCBF() when lcdState == LCD_CREATE_CUSTOM_CHAR
 *       Non-blocking: Executes one state per call (5ms intervals)
 *       Uses global variables: createCustomCharSeq, customCharIterator, CustomCharBuffer
 *       Completion reported via callback(LCD_OK) when all 8 bytes written
 */
static void ExecutCreateCustomChar(){
    LCD_Status_t retstat;        /* LCD function return status */
    GPIO_Status_t gpioStatus;    /* GPIO operation status */
    
    /**************************************************************************
     * CUSTOM CHARACTER CREATION STATE MACHINE
     * 
     * Process Overview:
     *   1. Set CGRAM address (location × 8)
     *   2. Write 8 bytes of character data to CGRAM (one byte per row)
     *   3. Restore DDRAM address (return to normal display mode)
     * 
     * 4-BIT MODE Pattern:
     *   - Set CGRAM Address: HIGH_NIBBLE_CURSOR_HIGH/LOW, LOW_NIBBLE_CURSOR_HIGH/LOW
     *   - Write 8 Data Bytes: Loop 8 times
     *     * HIGH_NIBBLE_HIGH/LOW: Write upper 4 bits of byte
     *     * LOW_NIBBLE_HIGH/LOW: Write lower 4 bits of byte
     *   - Restore DDRAM: HIGH_NIBBLE_SET_CURSOR_DDRAM, LOW_NIBBLE_SET_CURSOR_DDRAM
     * 
     * 8-BIT MODE Pattern:
     *   - Set CGRAM Address: CURSOR_HIGH/LOW
     *   - Write 8 Data Bytes: Loop 8 times
     *     * 8BIT_HIGH/LOW: Write full byte
     *   - Restore DDRAM: SET_CURSOR_DDRAM_HIGH/LOW
     * 
     * customCharIterator: Tracks which byte (0-7) is being written
     **************************************************************************/
    
    /* State machine for custom character creation sequence */
    switch(createCustomCharSeq){

        /********** 4-BIT: Set CGRAM Address (upper nibble) **********/
        case CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_CURSOR_HIGH:
            /* Calculate CGRAM address: location × 8 (each char = 8 bytes) */
            /* Send upper nibble of Set CGRAM Address command (0b01ACxxxx) */
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
            /* CGRAM address now set - ready to write character data */
            customCharIterator = 0;  /* Reset iterator to write first byte (row 0) */
            createCustomCharSeq = CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_HIGH;  /* Start data write */
            break;
        /********** 4-BIT: Write Character Data Byte (upper nibble) **********/
        /* This state loops 8 times (customCharIterator 0-7) to write all 8 rows */
        case CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_HIGH:
            /* Set RS=1 for data write (not command) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RS.port, LcdPinout.RS.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            /* Set RW=0 for write operation */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RW.port, LcdPinout.RW.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            /* Write upper 4 bits of current character data byte */
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
            /* Check if all 8 bytes (8 rows) of character data have been written */
            if(customCharIterator >= SPECIAL_CHAR_LENGHT){  /* SPECIAL_CHAR_LENGHT = 8 */
                customCharIterator = 0;  /* Reset for next character creation */
                /* All character data written - restore DDRAM address */
                createCustomCharSeq = CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_SET_CURSOR_DDRAM_HIGH;
            }else{
                /* More bytes to write - continue with next row */
                createCustomCharSeq = CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_HIGH;
            }
            break;
        /********** 4-BIT: Restore DDRAM Address (return to display mode) **********/
        /* After writing to CGRAM, cursor must return to DDRAM for normal display */
        case CREATE_CUSTOM_CHAR_4BIT_HIGH_NIBBLE_SET_CURSOR_DDRAM_HIGH:
                /* Send upper nibble of Set DDRAM Address (restore cursor position) */
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
        /********** 4-BIT: Latch DDRAM Address - CREATION COMPLETE **********/
        case CREATE_CUSTOM_CHAR_4BIT_LOW_NIBBLE_SET_CURSOR_DDRAM_LOW :
                /* Latch lower nibble (cursor now back in DDRAM mode) */
                gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
                if(gpioStatus != GPIO_OK){
                    createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                    lcdState = LCD_NO_ACTION;
                    if(Lcd_Callback != NULL){
                        Lcd_Callback(LCD_GPIO_ERROR);
                    }
                    break;
                }
                /* Custom character creation complete */
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;  /* Mark operation complete */
                lcdState = LCD_NO_ACTION;                       /* Return to idle */
                /* Notify user that custom character creation completed successfully */
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_OK);  /* Character ready to display */
                }
                break;
            
        /********** 8-BIT MODE: Set CGRAM Address **********/
        case CREATE_CUSTOM_CHAR_8BIT_CURSOR_HIGH:
            /* Calculate CGRAM address: location × 8, send complete byte */
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
        /********** 8-BIT MODE: Write Character Data Byte (loops 8 times) **********/
        case CREATE_CUSTOM_CHAR_8BIT_HIGH:
            /* Set RS=1 for data write */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RS.port, LcdPinout.RS.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            /* Set RW=0 for write operation */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.RW.port, LcdPinout.RW.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            /* Write full byte of character data (one row) and increment iterator */
            retstat   = LCD_WriteByte(CustomCharBuffer.charmap[customCharIterator++]);
            if(retstat != LCD_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(retstat);
                }
                break;
            }
            /* Generate enable pulse HIGH */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_HIGH);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            createCustomCharSeq = CREATE_CUSTOM_CHAR_8BIT_LOW;  /* Next: Latch data */
            break;
        /********** 8-BIT MODE: Latch Data Byte **********/
        case CREATE_CUSTOM_CHAR_8BIT_LOW:
            /* Generate enable pulse LOW (latch byte) */
            gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
            if(gpioStatus != GPIO_OK){
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                lcdState = LCD_NO_ACTION;
                if(Lcd_Callback != NULL){
                    Lcd_Callback(LCD_GPIO_ERROR);
                }
                break;
            }
            /* Check if all 8 bytes written */
            if(customCharIterator >= SPECIAL_CHAR_LENGHT){  /* All 8 rows complete */
                customCharIterator = 0;  /* Reset for next character */
                createCustomCharSeq = CREATE_CUSTOM_CHAR_8BIT_SET_CURSOR_DDRAM_HIGH;  /* Restore DDRAM */
            }else{
                createCustomCharSeq = CREATE_CUSTOM_CHAR_8BIT_HIGH;  /* Write next byte */
            }
            break;
        /********** 8-BIT MODE: Restore DDRAM Address **********/
        case CREATE_CUSTOM_CHAR_8BIT_SET_CURSOR_DDRAM_HIGH:
                /* Send Set DDRAM Address command (return to display mode) */
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
        /********** 8-BIT MODE: Latch DDRAM Address - CREATION COMPLETE **********/
        case CREATE_CUSTOM_CHAR_8BIT_SET_CURSOR_DDRAM_LOW:
                /* Latch DDRAM address (cursor now back in display mode) */
                gpioStatus = GPIO_enuSetPinVal(LcdPinout.EN.port, LcdPinout.EN.pin, GPIO_LOW);
                if(gpioStatus != GPIO_OK){
                    createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;
                    lcdState = LCD_NO_ACTION;
                    if(Lcd_Callback != NULL){
                        Lcd_Callback(LCD_GPIO_ERROR);
                    }
                    break;
                }
                /* Custom character creation complete */
                createCustomCharSeq = CREATE_CUSTOM_CHAR_DONE;  /* Mark operation complete */
                lcdState = LCD_NO_ACTION;                       /* Return to idle */
                /* Notify user that custom character creation completed successfully */
                if(Lcd_Callback != NULL){
                Lcd_Callback(LCD_CREATE_CUSTOM_CHAR_SUCCESSFULLY);  /* Character ready to display */
                }
                break;

        /********** DONE: Idle state - creation already complete **********/
        case CREATE_CUSTOM_CHAR_DONE:
            /* Do nothing, creation already complete */
            /* This state persists after character creation finishes */
            break;
            
    }
}

/**
 * @brief Write single character asynchronously at current cursor position
 * @details Non-blocking function that queues single character for display
 *          Character written over next scheduler cycle
 * 
 * Operation:
 *   - Character queued as single-character string (null-terminated)
 *   - Uses same state machine as LCD_enuAsynWriteString()
 *   - Cursor advances automatically after write
 * 
 * @param displayedChar ASCII character to display (0x20-0xFF)
 * 
 * @return LCD_Status_t:
 *         - LCD_OK: Character queued successfully
 *         - LCD_BUSY: Another async operation in progress
 *         - LCD_NOT_INITIALIZED: LCD initialization not complete
 * 
 * @note For multiple characters, use LCD_enuAsynWriteString() (more efficient)
 *       Character written at current cursor position
 *       Position tracked automatically (LCD_CurrentRow, LCD_CurrentCol)
 */
LCD_Status_t LCD_enuAsynWriteCharacter(uint8_t displayedChar){
    LCD_Status_t retStatus = LCD_NOT_OK;
    
    /* Check if LCD is busy with another operation */
    if(LCD_NO_ACTION!=lcdState){
        retStatus = LCD_BUSY;  /* Operation already in progress */
    }else{
        /* Check if LCD has been initialized successfully */
        if(INIT_FAILED == initSeq){
            retStatus = LCD_NOT_INITIALIZED;  /* Init not complete or failed */
        }else{
            /* Prepare buffer with single character (null-terminated) */
            LCD_DataBuffer_t lcdBuffer;
            lcdBuffer.buff[0] = displayedChar;  /* Character to display */
            lcdBuffer.buff[1] = '\0';            /* Null terminator */
            Queue_Push(&lcdBuffer);             /* Add to display queue */
            
            /* Set initial state based on bit operation mode (skip cursor positioning) */
            if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
                writeStringSeq = WRITE_STRING_4_BIT_HIGH_NIBBLE_HIGH;  /* Start with data write */
            }else{
                writeStringSeq = WRITE_STRING_8_BIT_HIGH;  /* Start with data write */
            }
            
            /* Activate state machine */
            lcdState = LCD_WRITE_STRING;
            retStatus = LCD_OK;
        }
    }
    return retStatus;
}


/**
 * @brief Display custom character asynchronously at current cursor position
 * @details Non-blocking function that queues custom character display
 *          Character must be created first using LCD_enuAsynCreateCustomChar()
 * 
 * Operation:
 *   - Custom character code (0-7) queued as single-character string
 *   - Uses same write mechanism as normal characters
 *   - Displays character pattern stored in CGRAM at specified location
 * 
 * @param location Custom character location (0-7 for 5x8 font, 0-3 for 5x10 font)
 * 
 * @return LCD_Status_t:
 *         - LCD_OK: Operation queued successfully
 *         - LCD_WRONG_LOCATION: Invalid location (>7)
 *         - LCD_BUSY: Another async operation in progress
 *         - LCD_NOT_INITIALIZED: LCD initialization not complete
 * 
 * @note Custom character must exist in CGRAM before calling this
 *       If character not created, random pattern may be displayed
 *       Cursor advances automatically after display
 *       Character written at current cursor position
 */
LCD_Status_t LCD_AsynDisplayCustomChar(uint8_t location){
    LCD_Status_t retStatus = LCD_NOT_OK;
    
    /* Check if LCD is busy with another operation */
    if(LCD_NO_ACTION!=lcdState){
        retStatus = LCD_BUSY;  /* Operation already in progress */
    }else{
        /* Check if LCD has been initialized successfully */
        if(INIT_FAILED == initSeq){
            retStatus = LCD_NOT_INITIALIZED;  /* Init not complete or failed */
        }else{
            /* Validate location (only 0-7 valid for 8 custom characters in 5x8 font) */
            if (location > LOCATION_MASK){
                retStatus = LCD_WRONG_LOCATION;  /* Invalid custom character location */
            }else{
                /* Prepare buffer with character code (0-7) */
                LCD_DataBuffer_t lcdBuffer;
                lcdBuffer.buff[0] = location;  /* Custom character code */
                lcdBuffer.buff[1] = '\0';       /* Null terminator */
                Queue_Push(&lcdBuffer);        /* Add to display queue */
            
                /* Set initial state based on bit operation mode */
                if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
                    writeStringSeq = WRITE_STRING_4_BIT_HIGH_NIBBLE_HIGH;
                }else{
                    writeStringSeq = WRITE_STRING_8_BIT_HIGH;
                }
                
                /* Activate state machine */
                lcdState = LCD_WRITE_STRING;
                retStatus = LCD_OK;
            }
        }
    }
    return retStatus;
}

/**
 * @brief Display custom character asynchronously at specified position
 * @details Non-blocking function that positions cursor then displays custom character
 *          Combines cursor positioning and character display in one operation
 * 
 * Operation Sequence:
 *   1. Set cursor to specified row and column
 *   2. Display custom character from CGRAM
 *   3. Cursor advances automatically after display
 * 
 * @param location Custom character location (0-7 for 5x8 font, 0-3 for 5x10 font)
 * @param row Row number (0 or 1 for 16x2 LCD)
 * @param col Column number (0-15 for 16-column LCD)
 * 
 * @return LCD_Status_t:
 *         - LCD_OK: Operation queued successfully
 *         - LCD_WRONG_LOCATION: Invalid location (>7)
 *         - LCD_BUSY: Another async operation in progress
 *         - LCD_NOT_INITIALIZED: LCD initialization not complete
 * 
 * @note More efficient than separate SetCursorPosition + DisplayCustomChar calls
 *       Custom character must be created first in CGRAM
 *       Position is validated during execution (not immediately)
 *       Invalid position will trigger callback with error status
 */
LCD_Status_t LCD_AsynDisplayCustomCharAtPosition(uint8_t location, uint8_t row, uint8_t col){
    LCD_Status_t retStatus = LCD_NOT_OK;
    
    /* Check if LCD is busy with another operation */
    if(LCD_NO_ACTION!=lcdState){
        retStatus = LCD_BUSY;  /* Operation already in progress */
    }else{
        /* Check if LCD has been initialized successfully */
        if(INIT_FAILED == initSeq){
            retStatus = LCD_NOT_INITIALIZED;  /* Init not complete or failed */
        }else{
            /* Validate location (only 0-7 valid for 8 custom characters) */
            if (location > LOCATION_MASK){
                retStatus = LCD_WRONG_LOCATION;  /* Invalid custom character location */
            }else{
                /* Prepare buffer with character code and position */
                LCD_DataBuffer_t lcdBuffer;
                lcdBuffer.buff[0] = location;  /* Custom character code (0-7) */
                lcdBuffer.buff[1] = '\0';       /* Null terminator */
                lcdBuffer.row = row;           /* Target row position */
                lcdBuffer.col = col;           /* Target column position */
                Queue_Push(&lcdBuffer);        /* Add to display queue */
            
                /* Set initial state based on bit operation mode (start with cursor positioning) */
                if(LcdCong.BitOperation == LCD_4_BIT_OPERATION){
                    writeStringSeq = WRITE_STRING_4BIT_HIGH_NIBBLE_CURSOR_HIGH;
                }else{
                    writeStringSeq = WRITE_STRING_8BIT_CURSOR_HIGH;
                }
                
                /* Activate state machine */
                lcdState = LCD_WRITE_STRING;
                retStatus = LCD_OK;
            }
        }
    }
    return retStatus;
}


/**
 * @brief Register user callback for asynchronous operation notifications
 * @details Sets callback function to be invoked when async operations complete or encounter errors
 *          Enables event-driven programming model for LCD operations
 * 
 * Callback Invocation Scenarios:
 *   Success events:
 *     - LCD_INIT_SUCEESSFULLY: Initialization completed successfully
 *     - LCD_OK: String write or custom char creation completed
 *   
 *   Error events:
 *     - LCD_GPIO_ERROR: GPIO operation failed during execution
 *     - LCD_TIMER_ERROR: Timer/delay operation failed
 *     - LCD_NOT_OK: General operation failure
 *     - Other specific error codes as appropriate
 * 
 * Usage Pattern:
 *   1. Define callback function with signature: void callback(LCD_Status_t status)
 *   2. Register callback using this function before starting async operations
 *   3. Start async operations (Init, WriteString, CreateCustomChar, etc.)
 *   4. Callback invoked automatically when operations complete
 * 
 * @param callback Function pointer to user callback (LCD_Callback_t type)
 *                 Pass NULL to unregister callback
 * 
 * @note Callback executes in scheduler context (keep execution time short)
 *       Avoid blocking operations or long computations in callback
 *       Only one callback can be registered at a time (replaces previous)
 *       Callback is shared across all async operations
 * 
 * Example:
 * @code
 *   void myLcdCallback(LCD_Status_t status) {
 *       switch(status) {
 *           case LCD_INIT_SUCEESSFULLY:
 *               // Start displaying data
 *               LCD_enuAsynWriteString("Ready!");
 *               break;
 *           case LCD_GPIO_ERROR:
 *               // Handle GPIO error
 *               errorHandler();
 *               break;
 *           case LCD_OK:
 *               // Operation completed successfully
 *               break;
 *           default:
 *               // Handle other errors
 *               break;
 *       }
 *   }
 *   
 *   // Register callback before starting operations
 *   LCD_vdAsyncRegisterCallback(myLcdCallback);
 *   LCD_enuAsynInit();
 * @endcode
 */
void LCD_vdAsyncRegisterCallback(LCD_Callback_t callback){
    Lcd_Callback = callback;  /* Store callback function pointer */
}