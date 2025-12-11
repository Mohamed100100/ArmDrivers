/******************************************************************************
 * @file    lcd.h
 * @brief   LCD 16x2 Driver Header File (HD44780 Compatible)
 * @details Type definitions, enumerations, structures, and function prototypes
 *          for LCD driver. Supports 8-bit and 4-bit operation modes.
 * @author  Eng.Gemy
 ******************************************************************************/

#ifndef LCD_H
#define LCD_H




/**
 * @brief Bit operation modes for nibble shifting in LCD commands/data
 * @details Determines how many bits to shift when sending bytes to LCD
 *          Used internally by driver to support both 4-bit and 8-bit modes
 * 
 * ALL_BITS (0):
 *   - No shift operation
 *   - Used in 8-bit mode to send complete byte
 *   - Example: 0b11001010 >> 0 = 0b11001010
 * 
 * HIGH_NIBBLE (4):
 *   - Shift right by 4 bits
 *   - Used in 4-bit mode to send upper 4 bits first
 *   - Example: 0b11001010 >> 4 = 0b00001100
 * 
 * LOW_NIBBLE (0):
 *   - No shift (same as ALL_BITS but used in 4-bit context)
 *   - Used in 4-bit mode to send lower 4 bits
 *   - Example: 0b11001010 >> 0 = 0b11001010 (then masked to 0b00001010)
 * 
 * @note In 4-bit mode, commands/data are sent as two operations:
 *       First HIGH_NIBBLE, then LOW_NIBBLE
 */
typedef enum {
    ALL_BITS    = 0,  /**< No shift - complete byte (8-bit mode) */
    HIGH_NIBBLE = 4,  /**< Shift right 4 - upper nibble (4-bit mode) */
    LOW_NIBBLE  = 0   /**< No shift - lower nibble (4-bit mode) */
}Bits_t;
/******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "LIB/stdtypes.h"

/******************************************************************************
 * STATUS CODES
 ******************************************************************************/

/**
 * @brief LCD driver return status codes
 */
typedef enum {
    LCD_NOT_OK,              /**< General error - operation failed */
    LCD_OK,                  /**< Operation completed successfully */
    LCD_NULL_PTR,            /**< NULL pointer passed as parameter */
    LCD_INIT_ERROR,          /**< Initialization failed */
    LCD_GPIO_ERROR,          /**< GPIO operation error */
    LCD_TIMER_ERROR,         /**< Timer/delay operation error */
    LCD_WRONG_ROW,           /**< Invalid row number (valid: 0-1) */
    LCD_WRONG_COLUMN,        /**< Invalid column number (valid: 0-15) */
    LCD_ERROR_SPECIALCHAR,   /**< Custom character creation failed */
    LCD_WRONG_LOCATION,      /**< Invalid custom character location (valid: 0-7) */
    LCD_NOT_INITIALIZED,
    LCD_FAILED_TO_INIT,
    LCD_BUSY,
    LCD_WRONG_BIT_OPERATION,
    LCD_INIT_SUCEESSFULLY,
    LCD_WRITE_SUCCESSFULLY,
    LCD_CREATE_CUSTOM_CHAR_SUCCESSFULLY,
} LCD_Status_t;

/******************************************************************************
 * LCD CONFIGURATION ENUMERATIONS
 ******************************************************************************/
/**
 * @brief Callback function type for asynchronous LCD operations
 * @details User-defined function called when async operations complete or encounter errors
 *          Enables event-driven programming without blocking delays
 * 
 * Callback Invocation Scenarios:
 *   - Initialization complete: status = LCD_INIT_SUCEESSFULLY
 *   - String write complete: status = LCD_WRITE_SUCCESSFULLY
 *   - Custom char created: status = LCD_CREATE_CUSTOM_CHAR_SUCCESSFULLY
 *   - GPIO error: status = LCD_GPIO_ERROR
 *   - Timer error: status = LCD_TIMER_ERROR
 *   - Other errors: status = (specific error code)
 * 
 * @param status LCD operation result (LCD_OK, LCD_GPIO_ERROR, etc.)
 * 
 * @note Callback executes in scheduler context (keep it short and non-blocking)
 *       Register callback using LCD_vdAsyncRegisterCallback() before starting operations
 * 
 * Example:
 * @code
 *   void myLcdCallback(LCD_Status_t status) {
 *       if (status == LCD_INIT_SUCEESSFULLY) {
 *           // LCD ready - start displaying data
 *       } else {
 *           // Handle error
 *       }
 *   }
 *   LCD_vdAsyncRegisterCallback(myLcdCallback);
 * @endcode
 */
typedef void (*LCD_Callback_t)(LCD_Status_t status);

/**
 * @brief Data bus width selection (8-bit or 4-bit interface)
 */
typedef enum {
    LCD_8_BIT_OPERATION = 1,  /**< 8-bit mode - uses DB0-DB7 (11 pins total) */
    LCD_4_BIT_OPERATION = 0,  /**< 4-bit mode - uses DB4-DB7 (7 pins total) */
} LCD_BitOperation_t;

/**
 * @brief Number of display lines (1-line or 2-line)
 */
typedef enum {
    LCD_1_LINE_DISPLAY = 0,  /**< Single line display */
    LCD_2_LINE_DISPLAY = 1,  /**< Dual line display (typical for 16x2) */
} LCD_LineDisplay_t;

/**
 * @brief Character font size selection
 */
typedef enum {
    LCD_5_7_DOT_FONT  = 0,  /**< 5x8 dot font (standard, 8 custom chars) */
    LCD_5_10_DOT_FONT = 1,  /**< 5x10 dot font (tall, 4 custom chars, 1-line only) */
} LCD_FontSize_t;

/**
 * @brief Display ON/OFF control
 */
typedef enum {
    LCD_DISPLAY_ON  = 1,  /**< Display visible */
    LCD_DISPLAY_OFF = 0,  /**< Display hidden (data preserved) */
} LCD_Display_t;

/**
 * @brief Cursor visibility control (underscore line)
 */
typedef enum {
    LCD_CURSOR_ON  = 1,  /**< Cursor visible */
    LCD_CURSOR_OFF = 0,  /**< Cursor hidden */
} LCD_Cursor_t;

/**
 * @brief Cursor blink control (blinking block)
 */
typedef enum {
    LCD_BLINK_ON  = 1,  /**< Cursor blinks (~1Hz) */
    LCD_BLINK_OFF = 0,  /**< Cursor steady */
} LCD_Blink_t;

/******************************************************************************
 * GPIO PORT AND PIN DEFINITIONS
 ******************************************************************************/

/**
 * @brief GPIO port selection
 */
typedef enum {
    LCD_PORT_A = 0,  /**< GPIO Port A - 000b */
    LCD_PORT_B = 1,  /**< GPIO Port B - 001b */
    LCD_PORT_C = 2,  /**< GPIO Port C - 010b */
    LCD_PORT_D = 3,  /**< GPIO Port D - 011b */
    LCD_PORT_E = 4,  /**< GPIO Port E - 100b */
    LCD_PORT_H = 5   /**< GPIO Port H - 101b */
} LCD_Port_t;

/**
 * @brief GPIO pin number within port (0-15)
 */
typedef enum {
    LCD_PIN_0 = 0,   /**< Pin 0 */
    LCD_PIN_1,       /**< Pin 1 */
    LCD_PIN_2,       /**< Pin 2 */
    LCD_PIN_3,       /**< Pin 3 */
    LCD_PIN_4,       /**< Pin 4 */
    LCD_PIN_5,       /**< Pin 5 */
    LCD_PIN_6,       /**< Pin 6 */
    LCD_PIN_7,       /**< Pin 7 */
    LCD_PIN_8,       /**< Pin 8 */
    LCD_PIN_9,       /**< Pin 9 */
    LCD_PIN_10,      /**< Pin 10 */
    LCD_PIN_11,      /**< Pin 11 */
    LCD_PIN_12,      /**< Pin 12 */
    LCD_PIN_13,      /**< Pin 13 */
    LCD_PIN_14,      /**< Pin 14 */
    LCD_PIN_15       /**< Pin 15 */
} LCD_Pin_t;

/******************************************************************************
 * CURSOR AND DISPLAY BEHAVIOR ENUMERATIONS
 ******************************************************************************/

/**
 * @brief Cursor movement direction after character write
 */
typedef enum {
    LCD_INCREMENT = 1,  /**< Move right (left-to-right text) */
    LCD_DECREMENT = 0,  /**< Move left (right-to-left text) */
} LCD_IncDec_t;

/**
 * @brief Automatic display shift after character write
 */
typedef enum {
    LCD_NO_SHIFT   = 0,  /**< No shift (normal operation) */
    LCD_AUTO_SHIFT = 1,  /**< Auto-shift for scrolling effect */
} LCD_DisplayShift_t;

/******************************************************************************
 * PIN CONFIGURATION STRUCTURES
 ******************************************************************************/

/**
 * @brief Single pin information (port + pin number)
 */
typedef struct {
    LCD_Port_t port;  /**< GPIO port */
    LCD_Pin_t  pin;   /**< Pin number */
} LCD_PinInfo_t;

/**
 * @brief Pin configuration for 8-bit mode (11 pins total)
 */
typedef struct {
    LCD_PinInfo_t  DB4;  /**< Data bit 4 */
    LCD_PinInfo_t  DB5;  /**< Data bit 5 */
    LCD_PinInfo_t  DB6;  /**< Data bit 6 */
    LCD_PinInfo_t  DB7;  /**< Data bit 7 (MSB) */
    LCD_PinInfo_t  EN;   /**< Enable pin (latch signal) */
    LCD_PinInfo_t  RW;   /**< Read/Write pin (0=Write, 1=Read) */
    LCD_PinInfo_t  RS;   /**< Register Select (0=Command, 1=Data) */
    LCD_PinInfo_t  DB0;  /**< Data bit 0 (LSB) */
    LCD_PinInfo_t  DB1;  /**< Data bit 1 */
    LCD_PinInfo_t  DB2;  /**< Data bit 2 */
    LCD_PinInfo_t  DB3;  /**< Data bit 3 */
} LCD_Pinout_8BitMode_t;

/**
 * @brief Pin configuration for 4-bit mode (7 pins total)
 */
typedef struct {
    LCD_PinInfo_t  DB4;  /**< Data bit 4 (lower nibble bit) */
    LCD_PinInfo_t  DB5;  /**< Data bit 5 */
    LCD_PinInfo_t  DB6;  /**< Data bit 6 */
    LCD_PinInfo_t  DB7;  /**< Data bit 7 (upper nibble bit) */
    LCD_PinInfo_t  EN;   /**< Enable pin (latch signal) */
    LCD_PinInfo_t  RW;   /**< Read/Write pin (0=Write, 1=Read) */
    LCD_PinInfo_t  RS;   /**< Register Select (0=Command, 1=Data) */
} LCD_Pinout_4BitMode_t;

/**
 * @brief Union to hold either 8-bit or 4-bit pin configuration
 */
typedef union {
    LCD_Pinout_8BitMode_t *Pinout8BitMode;  /**< 8-bit mode pinout */
    LCD_Pinout_4BitMode_t *Pinout4BitMode;  /**< 4-bit mode pinout */
} LCD_PinoutMode_t;

/******************************************************************************
 * LCD CONFIGURATION STRUCTURE
 ******************************************************************************/

/**
 * @brief Complete LCD configuration parameters
 * @note All fields must be initialized before calling LCD_enuInit()
 */
typedef struct {
    LCD_BitOperation_t BitOperation;           /**< 8-bit or 4-bit mode */
    LCD_LineDisplay_t  LineDisplay;            /**< 1-line or 2-line display */
    LCD_FontSize_t     FontSize;               /**< 5x8 or 5x10 font */
    LCD_Display_t      Display;                /**< Display ON/OFF */
    LCD_Cursor_t       Cursor;                 /**< Cursor ON/OFF */
    LCD_Blink_t        Blink;                  /**< Blink ON/OFF */
    LCD_IncDec_t       IncrementStatus;        /**< Increment/decrement direction */
    LCD_DisplayShift_t DisplayShiftOperation;  /**< Auto-shift ON/OFF */
} LCD_Config_t;

/******************************************************************************
 * SYNCHRONOUS FUNCTION PROTOTYPES
 * @details These functions block execution until operation completes
 *          Suitable for simple applications or non-time-critical code
 *          Use asynchronous versions for better responsiveness
 ******************************************************************************/

/**
 * @brief Initialize LCD synchronously (blocking)
 * @details Performs complete LCD initialization with blocking delays
 *          Execution blocks for ~50-100ms during initialization
 * @return LCD_Status_t:
 *         - LCD_OK: Initialization successful
 *         - LCD_INIT_ERROR: GPIO initialization failed
 *         - LCD_TIMER_ERROR: Delay timer error
 *         - LCD_GPIO_ERROR: GPIO operation failed
 * @note Simpler than async version but blocks entire system during init
 *       Requires LcdCong and LcdPinout to be configured before calling
 */
LCD_Status_t LCD_enuSynInit();

/**
 * @brief Write single character synchronously (blocking)
 * @details Displays one ASCII character at current cursor position
 *          Cursor advances automatically after write
 * @param displayedChar ASCII character to display (0x20-0xFF)
 * @return LCD_Status_t:
 *         - LCD_OK: Character written successfully
 *         - LCD_GPIO_ERROR: GPIO operation failed
 * @note For strings, call repeatedly or use async string functions
 *       Blocks for ~1-2ms per character
 */
LCD_Status_t LCD_enuSyncWriteCharacter(uint8_t displayedChar);

/**
 * @brief Clear entire LCD display synchronously (blocking)
 * @details Clears all characters and returns cursor to home (0,0)
 *          Display data RAM (DDRAM) is filled with space (0x20)
 * @return LCD_Status_t:
 *         - LCD_OK: Display cleared successfully
 *         - LCD_GPIO_ERROR: GPIO operation failed
 * @note Blocks for ~2ms (HD44780 clear command execution time)
 *       Cursor position reset to row 0, column 0
 */
LCD_Status_t LCD_enuSyncClearDisplay();

/**
 * @brief Turn LCD display ON or OFF synchronously (blocking)
 * @details Controls display visibility without affecting DDRAM content
 *          Display OFF: Screen blank but data preserved
 *          Display ON: Content visible again
 * @param displayState LCD_DISPLAY_ON or LCD_DISPLAY_OFF
 * @return LCD_Status_t:
 *         - LCD_OK: Display state changed successfully
 *         - LCD_GPIO_ERROR: GPIO operation failed
 * @note Useful for blinking entire display or power saving
 *       Cursor and blink settings are also hidden when display is OFF
 */
LCD_Status_t LCD_enuSyncSetDisplay(LCD_Display_t displayState);

/**
 * @brief Return cursor to home position (0,0) synchronously (blocking)
 * @details Moves cursor to top-left corner without clearing display
 *          Also resets any display shift to original position
 * @return LCD_Status_t:
 *         - LCD_OK: Cursor returned home successfully
 *         - LCD_GPIO_ERROR: GPIO operation failed
 * @note Blocks for ~2ms (HD44780 return home execution time)
 *       Display content remains unchanged
 */
LCD_Status_t LCD_enuSyncReturnHome();

/**
 * @brief Show or hide cursor underline synchronously (blocking)
 * @details Controls visibility of cursor underline (_) at current position
 * @param cursorState LCD_CURSOR_ON or LCD_CURSOR_OFF
 * @return LCD_Status_t:
 *         - LCD_OK: Cursor state changed successfully
 *         - LCD_GPIO_ERROR: GPIO operation failed
 * @note Independent of blink setting (both can be enabled simultaneously)
 *       Cursor appears as underline on character cell
 */
LCD_Status_t LCD_enuSyncSetCursor(LCD_Cursor_t cursorState);

/**
 * @brief Enable or disable cursor blink synchronously (blocking)
 * @details Controls blinking block cursor at current position (~1Hz)
 * @param blinkState LCD_BLINK_ON or LCD_BLINK_OFF
 * @return LCD_Status_t:
 *         - LCD_OK: Blink state changed successfully
 *         - LCD_GPIO_ERROR: GPIO operation failed
 * @note Independent of cursor underline (both can be enabled together)
 *       Blink appears as filled block that alternates with underline
 */
LCD_Status_t LCD_enuSyncSetBlink(LCD_Blink_t blinkState);

/**
 * @brief Set cursor to specific row and column synchronously (blocking)
 * @details Positions cursor at specified location for next write operation
 *          Does not display any character
 * @param row Row number (0 or 1 for 16x2 LCD)
 * @param col Column number (0-15 for 16-column LCD)
 * @return LCD_Status_t:
 *         - LCD_OK: Cursor positioned successfully
 *         - LCD_WRONG_ROW: Invalid row (not 0 or 1)
 *         - LCD_WRONG_COLUMN: Invalid column (>15)
 *         - LCD_GPIO_ERROR: GPIO operation failed
 * @note Position is tracked in LCD_CurrentRow and LCD_CurrentCol variables
 *       Next character write will appear at this position
 */
LCD_Status_t LCD_enuSyncSetCursorPosition(uint8_t row, uint8_t col);

/**
 * @brief Set cursor movement direction synchronously (blocking)
 * @details Controls whether cursor moves right or left after character write
 *          LCD_INCREMENT: Cursor moves right (normal left-to-right text)
 *          LCD_DECREMENT: Cursor moves left (right-to-left text)
 * @param incrementDecrement LCD_INCREMENT or LCD_DECREMENT
 * @return LCD_Status_t:
 *         - LCD_OK: Direction changed successfully
 *         - LCD_GPIO_ERROR: GPIO operation failed
 * @note Affects entry mode setting in HD44780 controller
 *       Typically set to INCREMENT for English text
 */
LCD_Status_t LCD_enuSyncSetIncrementDecrementMode(LCD_IncDec_t incrementDecrement);

/**
 * @brief Enable or disable automatic display shift synchronously (blocking)
 * @details Controls whether entire display shifts after each character write
 *          LCD_AUTO_SHIFT: Display shifts (scrolling effect)
 *          LCD_NO_SHIFT: Display stationary (normal operation)
 * @param displayShift LCD_AUTO_SHIFT or LCD_NO_SHIFT
 * @return LCD_Status_t:
 *         - LCD_OK: Shift mode changed successfully
 *         - LCD_GPIO_ERROR: GPIO operation failed
 * @note Rarely used - creates automatic scrolling text effect
 *       Usually set to LCD_NO_SHIFT for normal operation
 */
LCD_Status_t LCD_enuSyncDisplayShift(LCD_DisplayShift_t displayShift);

/**
 * @brief Set character font size synchronously (blocking)
 * @details Changes between 5x8 and 5x10 dot matrix fonts
 *          5x8 (LCD_5_7_DOT_FONT): Standard font, 8 custom chars available
 *          5x10 (LCD_5_10_DOT_FONT): Tall font, 4 custom chars, 1-line mode only
 * @param FontSize LCD_5_7_DOT_FONT or LCD_5_10_DOT_FONT
 * @return LCD_Status_t:
 *         - LCD_OK: Font changed successfully
 *         - LCD_GPIO_ERROR: GPIO operation failed
 * @note 5x10 font only works in 1-line display mode
 *       Changing font updates global configuration LcdCong.FontSize
 */
LCD_Status_t LCD_enuSyncSetFontSize(LCD_FontSize_t FontSize);

/**
 * @brief Set number of display lines synchronously (blocking)
 * @details Configures LCD for 1-line or 2-line display mode
 *          Most 16x2 LCDs use 2-line mode
 * @param LineDisplay LCD_1_LINE_DISPLAY or LCD_2_LINE_DISPLAY
 * @return LCD_Status_t:
 *         - LCD_OK: Line mode changed successfully
 *         - LCD_GPIO_ERROR: GPIO operation failed
 * @note Typically set to LCD_2_LINE_DISPLAY for 16x2 LCD
 *       1-line mode required for 5x10 font
 */
LCD_Status_t LCD_enuSyncSetLineDisplay(LCD_LineDisplay_t LineDisplay);

/**
 * @brief Create custom character in CGRAM synchronously (blocking)
 * @details Defines custom 5x8 character pattern in CGRAM
 *          Up to 8 custom characters (locations 0-7) for 5x8 font
 *          Up to 4 custom characters (locations 0-3) for 5x10 font
 * @param location Character location (0-7 for 5x8, 0-3 for 5x10)
 * @param charmap[8] Array of 8 bytes, each byte defines one row of pixels
 *                   Bit format per byte: 000xxxxx (5 LSBs used)
 *                   Bit 4=leftmost pixel, Bit 0=rightmost pixel
 *                   1=pixel ON, 0=pixel OFF
 * @return LCD_Status_t:
 *         - LCD_OK: Custom character created successfully
 *         - LCD_NULL_PTR: charmap is NULL pointer
 *         - LCD_WRONG_LOCATION: Invalid location (>7)
 *         - LCD_GPIO_ERROR: GPIO operation failed
 * @note Cursor returns to previous position after creation
 *       Display character using LCD_enuSyncWriteCustomChar(location)
 * 
 * Example 5x8 smiley face:
 * @code
 *   uint8_t smiley[8] = {
 *       0b00000,  // Row 0
 *       0b01010,  // Row 1: Eyes
 *       0b01010,  // Row 2: Eyes
 *       0b00000,  // Row 3
 *       0b10001,  // Row 4: Mouth ends
 *       0b01110,  // Row 5: Mouth
 *       0b00000,  // Row 6
 *       0b00000   // Row 7
 *   };
 *   LCD_enuSyncCreateCustomChar(0, smiley);
 * @endcode
 */
LCD_Status_t LCD_enuSyncCreateCustomChar(uint8_t location, const uint8_t charmap[8]);

/**
 * @brief Display custom character at current cursor position synchronously (blocking)
 * @details Writes custom character code to display predefined custom character
 *          Character must be created first using LCD_enuSyncCreateCustomChar()
 * @param location Custom character location (0-7 for 5x8, 0-3 for 5x10)
 * @return LCD_Status_t:
 *         - LCD_OK: Custom character displayed successfully
 *         - LCD_WRONG_LOCATION: Invalid location (>7)
 *         - LCD_GPIO_ERROR: GPIO operation failed
 * @note Cursor advances automatically after display
 *       If character not created, random pattern may appear
 */
LCD_Status_t LCD_enuSyncWriteCustomChar(uint8_t location);

/******************************************************************************
 * ASYNCHRONOUS FUNCTION PROTOTYPES
 * @details Non-blocking functions that use OS scheduler for execution
 *          Operations complete over multiple scheduler cycles
 *          Callback notification when operation completes or error occurs
 *          Ideal for responsive applications and RTOS environments
 ******************************************************************************/

/**
 * @brief Initialize LCD asynchronously (non-blocking)
 * @details Starts LCD initialization via OS scheduler
 *          Returns immediately - init completes in background
 *          Callback invoked with LCD_INIT_SUCEESSFULLY when complete
 * @return LCD_Status_t:
 *         - LCD_OK: Async init started successfully
 *         - LCD_FAILED_TO_INIT: GPIO init or scheduler registration failed
 * @note Register callback first using LCD_vdAsyncRegisterCallback()
 *       Do not use LCD until callback indicates success
 *       Requires OS scheduler to be running (SCHED_Start())
 *       Init takes ~50-100ms to complete (non-blocking)
 */
LCD_Status_t LCD_enuAsynInit();

/**
 * @brief Write single character asynchronously (non-blocking)
 * @details Queues single character for display
 *          Character written over next scheduler cycles
 * @param displayedChar ASCII character to display (0x20-0xFF)
 * @return LCD_Status_t:
 *         - LCD_OK: Character queued successfully
 *         - LCD_BUSY: Another operation in progress
 *         - LCD_NOT_INITIALIZED: LCD not initialized
 * @note Cursor advances automatically after write
 *       For multiple characters, use LCD_enuAsynWriteString()
 */
LCD_Status_t LCD_enuAsynWriteCharacter(uint8_t displayedChar);

/**
 * @brief Write string asynchronously (non-blocking)
 * @details Queues null-terminated string for display
 *          Characters written one-per-cycle by scheduler
 *          Automatic line wrapping at 16 characters
 * @param displayedString Pointer to null-terminated ASCII string
 * @return LCD_Status_t:
 *         - LCD_OK: String queued successfully
 *         - LCD_NULL_PTR: String pointer is NULL
 *         - LCD_BUSY: Another operation in progress
 *         - LCD_NOT_INITIALIZED: LCD not initialized
 * @note String written at current cursor position
 *       Callback invoked when string fully written
 *       Multiple strings can be queued (queue-based)
 *       String buffer must remain valid until write completes
 */
LCD_Status_t LCD_enuAsynWriteString(uint8_t* displayedString);

/**
 * @brief Write string at specific position asynchronously (non-blocking)
 * @details Positions cursor then writes string
 *          Combines position set + string write in one operation
 * @param displayedString Pointer to null-terminated ASCII string
 * @param row Row number (0 or 1)
 * @param col Column number (0-15)
 * @return LCD_Status_t:
 *         - LCD_OK: Operation queued successfully
 *         - LCD_NULL_PTR: String pointer is NULL
 *         - LCD_BUSY: Another operation in progress
 *         - LCD_NOT_INITIALIZED: LCD not initialized
 *         - LCD_WRONG_ROW: Invalid row
 *         - LCD_WRONG_COLUMN: Invalid column
 * @note More efficient than separate position + write calls
 *       Automatic line wrapping from specified position
 */
LCD_Status_t LCD_enuAsynWriteStringAtPosition(uint8_t* displayedString, uint8_t row, uint8_t col);

/**
 * @brief Create custom character asynchronously (non-blocking)
 * @details Queues custom character creation in CGRAM
 *          Character data written over multiple scheduler cycles
 * @param location Character location (0-7 for 5x8, 0-3 for 5x10)
 * @param charmap[8] Array of 8 bytes defining character pattern
 * @return LCD_Status_t:
 *         - LCD_OK: Operation queued successfully
 *         - LCD_NULL_PTR: charmap is NULL
 *         - LCD_WRONG_LOCATION: Invalid location (>7)
 *         - LCD_BUSY: Another operation in progress
 *         - LCD_NOT_INITIALIZED: LCD not initialized
 * @note Character data copied internally (safe to free charmap after call)
 *       Cursor position preserved (returns to previous position)
 *       Callback invoked when creation complete
 */
LCD_Status_t LCD_enuAsynCreateCustomChar(uint8_t location, const uint8_t charmap[8]);

/**
 * @brief Display custom character asynchronously (non-blocking)
 * @details Queues custom character display at current cursor position
 * @param location Custom character location (0-7 for 5x8, 0-3 for 5x10)
 * @return LCD_Status_t:
 *         - LCD_OK: Operation queued successfully
 *         - LCD_WRONG_LOCATION: Invalid location (>7)
 *         - LCD_BUSY: Another operation in progress
 *         - LCD_NOT_INITIALIZED: LCD not initialized
 * @note Character must be created first using LCD_enuAsynCreateCustomChar()
 *       Cursor advances automatically after display
 */
LCD_Status_t LCD_AsynDisplayCustomChar(uint8_t location);

/**
 * @brief Display custom character at specific position asynchronously (non-blocking)
 * @details Positions cursor then displays custom character
 * @param location Custom character location (0-7 for 5x8, 0-3 for 5x10)
 * @param row Row number (0 or 1)
 * @param col Column number (0-15)
 * @return LCD_Status_t:
 *         - LCD_OK: Operation queued successfully
 *         - LCD_WRONG_LOCATION: Invalid location (>7)
 *         - LCD_BUSY: Another operation in progress
 *         - LCD_NOT_INITIALIZED: LCD not initialized
 *         - LCD_WRONG_ROW: Invalid row
 *         - LCD_WRONG_COLUMN: Invalid column
 * @note More efficient than separate position + display calls
 */
LCD_Status_t LCD_AsynDisplayCustomCharAtPosition(uint8_t location, uint8_t row, uint8_t col);

/**
 * @brief Register callback function for asynchronous operation notifications
 * @details Sets user callback to be invoked when async operations complete or encounter errors
 *          Enables event-driven LCD programming
 * @param callback Function pointer to callback (or NULL to unregister)
 * @note Call this before starting any async operations
 *       Callback executes in scheduler context (keep it short)
 *       Only one callback can be registered at a time
 * 
 * Example:
 * @code
 *   void myCallback(LCD_Status_t status) {
 *       if (status == LCD_INIT_SUCEESSFULLY) {
 *           LCD_enuAsynWriteString("Hello World");
 *       }
 *   }
 *   
 *   LCD_vdAsyncRegisterCallback(myCallback);
 *   LCD_enuAsynInit();
 * @endcode
 */
void LCD_vdAsyncRegisterCallback(LCD_Callback_t callback);

#endif // LCD_H