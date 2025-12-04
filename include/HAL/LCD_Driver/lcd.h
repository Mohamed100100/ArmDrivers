/******************************************************************************
 * @file    lcd.h
 * @brief   LCD 16x2 Driver Header File (HD44780 Compatible)
 * @details Type definitions, enumerations, structures, and function prototypes
 *          for LCD driver. Supports 8-bit and 4-bit operation modes.
 * @author  Eng.Gemy
 ******************************************************************************/

#ifndef LCD_H
#define LCD_H

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
    LCD_WRONG_LOCATION       /**< Invalid custom character location (valid: 0-7) */
} LCD_Status_t;

/******************************************************************************
 * LCD CONFIGURATION ENUMERATIONS
 ******************************************************************************/

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
    LCD_PinInfo_t  DB0;  /**< Data bit 0 (LSB) */
    LCD_PinInfo_t  DB1;  /**< Data bit 1 */
    LCD_PinInfo_t  DB2;  /**< Data bit 2 */
    LCD_PinInfo_t  DB3;  /**< Data bit 3 */
    LCD_PinInfo_t  DB4;  /**< Data bit 4 */
    LCD_PinInfo_t  DB5;  /**< Data bit 5 */
    LCD_PinInfo_t  DB6;  /**< Data bit 6 */
    LCD_PinInfo_t  DB7;  /**< Data bit 7 (MSB) */
    LCD_PinInfo_t  EN;   /**< Enable pin (latch signal) */
    LCD_PinInfo_t  RW;   /**< Read/Write pin (0=Write, 1=Read) */
    LCD_PinInfo_t  RS;   /**< Register Select (0=Command, 1=Data) */
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
 * FUNCTION PROTOTYPES
 ******************************************************************************/

/* Initialization */
LCD_Status_t LCD_enuInit();

/* Character Output */
LCD_Status_t LCD_enuWriteCharacter(uint8_t displayedChar);

/* Display Control */
LCD_Status_t LCD_enuClearDisplay();
LCD_Status_t LCD_enuSetDisplay(LCD_Display_t displayState);
LCD_Status_t LCD_enuReturnHome();

/* Cursor Control */
LCD_Status_t LCD_enuSetCursor(LCD_Cursor_t cursorState);
LCD_Status_t LCD_enuSetBlink(LCD_Blink_t blinkState);
LCD_Status_t LCD_enuSetCursorPosition(uint8_t row, uint8_t col);

/* Text Direction and Shift */
LCD_Status_t LCD_enuSetIncrementDecrementMode(LCD_IncDec_t incrementDecrement);
LCD_Status_t LCD_enuDisplayShift(LCD_DisplayShift_t displayShift);

/* Display Configuration */
LCD_Status_t LCD_enuSetFontSize(LCD_FontSize_t FontSize);
LCD_Status_t LCD_enuSetLineDisplay(LCD_LineDisplay_t LineDisplay);
LCD_Status_t LCD_enuSetBitOperation(LCD_BitOperation_t bitOperation);

/* Custom Characters */
LCD_Status_t LCD_enuCreateCustomChar(uint8_t location, const uint8_t charmap[8]);
LCD_Status_t LCD_enuWriteCustomChar(uint8_t location);

#endif // LCD_H