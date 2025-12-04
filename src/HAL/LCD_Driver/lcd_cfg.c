/******************************************************************************
 * @file    lcd_config.c
 * @brief   LCD Driver Hardware and Operational Configuration
 * @details User configuration file for LCD pin mapping and behavior settings
 * @author  Eng.Gemy
 * 
 * @note    Modify these settings according to your hardware setup
 ******************************************************************************/

#include "LIB/stdtypes.h"
#include "HAL/LCD_Driver/lcd.h"

/******************************************************************************
 * LCD PIN CONFIGURATION - 8-BIT MODE
 * 
 * @brief Hardware pin assignments for LCD connections (11 pins total)
 * 
 * Available Ports:
 *    LCD_PORT_A, LCD_PORT_B, LCD_PORT_C, LCD_PORT_D, LCD_PORT_E, LCD_PORT_H
 * 
 * Available Pins (per port):
 *    LCD_PIN_0 through LCD_PIN_15 (16 pins per port)
 * 
 * Pin Functions:
 *    RS  : Register Select (0=Command, 1=Data)
 *    RW  : Read/Write (0=Write, 1=Read)
 *    EN  : Enable (Falling edge latches data)
 *    DB0-DB7 : 8-bit data bus (DB0=LSB, DB7=MSB)
 ******************************************************************************/
const LCD_Pinout_8BitMode_t LcdPinout = {
    .RS  = {.port = LCD_PORT_A, .pin = LCD_PIN_0},   /* Register Select pin */
    .RW  = {.port = LCD_PORT_A, .pin = LCD_PIN_1},   /* Read/Write control pin */
    .EN  = {.port = LCD_PORT_A, .pin = LCD_PIN_2},   /* Enable (latch) signal */
    .DB0 = {.port = LCD_PORT_A, .pin = LCD_PIN_3},   /* Data bit 0 (LSB) */
    .DB1 = {.port = LCD_PORT_A, .pin = LCD_PIN_4},   /* Data bit 1 */
    .DB2 = {.port = LCD_PORT_A, .pin = LCD_PIN_5},   /* Data bit 2 */
    .DB3 = {.port = LCD_PORT_A, .pin = LCD_PIN_6},   /* Data bit 3 */
    .DB4 = {.port = LCD_PORT_A, .pin = LCD_PIN_7},   /* Data bit 4 */
    .DB5 = {.port = LCD_PORT_A, .pin = LCD_PIN_8},   /* Data bit 5 */
    .DB6 = {.port = LCD_PORT_A, .pin = LCD_PIN_9},   /* Data bit 6 */
    .DB7 = {.port = LCD_PORT_A, .pin = LCD_PIN_10},  /* Data bit 7 (MSB) */
};

/******************************************************************************
 * LCD OPERATIONAL CONFIGURATION
 * 
 * @brief LCD behavior and display settings
 ******************************************************************************/
LCD_Config_t LcdCong = {
    
    /***************************************************************************
     * BitOperation - Interface Data Bus Width
     * 
     * Options:
     *    LCD_8_BIT_OPERATION : 8-bit interface (DB0-DB7, 11 pins total)
     *    LCD_4_BIT_OPERATION : 4-bit interface (DB4-DB7, 7 pins total)
     ***************************************************************************/
    .BitOperation = LCD_8_BIT_OPERATION,
    
    /***************************************************************************
     * LineDisplay - Number of Display Lines
     * 
     * Options:
     *    LCD_1_LINE_DISPLAY : Single line display
     *    LCD_2_LINE_DISPLAY : Two line display
     ***************************************************************************/
    .LineDisplay = LCD_2_LINE_DISPLAY,
    
    /***************************************************************************
     * FontSize - Character Font Dot Matrix Size
     * 
     * Options:
     *    LCD_5_7_DOT_FONT  : 5x8 dots (standard, works with 1 or 2 lines)
     *    LCD_5_10_DOT_FONT : 5x10 dots (tall, only 1-line mode)
     ***************************************************************************/
    .FontSize = LCD_5_7_DOT_FONT,
    
    /***************************************************************************
     * Display - Display Visibility Control
     * 
     * Options:
     *    LCD_DISPLAY_ON  : Display content visible
     *    LCD_DISPLAY_OFF : Display content hidden (data preserved)
     ***************************************************************************/
    .Display = LCD_DISPLAY_ON,
    
    /***************************************************************************
     * Cursor - Cursor Underscore Visibility
     * 
     * Options:
     *    LCD_CURSOR_ON  : Underscore cursor visible
     *    LCD_CURSOR_OFF : Underscore cursor hidden
     ***************************************************************************/
    .Cursor = LCD_CURSOR_ON,
    
    /***************************************************************************
     * Blink - Cursor Blink Effect (Blinking Block)
     * 
     * Options:
     *    LCD_BLINK_ON  : Cursor blinks (~1Hz)
     *    LCD_BLINK_OFF : Cursor steady (no blink)
     * 
     * @note Requires Cursor to be ON to see blink effect
     ***************************************************************************/
    .Blink = LCD_BLINK_ON,
    
    /***************************************************************************
     * IncrementStatus - Cursor Movement Direction After Write
     * 
     * Options:
     *    LCD_INCREMENT : Cursor moves right (left-to-right text)
     *    LCD_DECREMENT : Cursor moves left (right-to-left text)
     ***************************************************************************/
    .IncrementStatus = LCD_INCREMENT,
    
    /***************************************************************************
     * DisplayShiftOperation - Automatic Display Shift After Write
     * 
     * Options:
     *    LCD_NO_SHIFT   : Display stationary (normal mode)
     *    LCD_AUTO_SHIFT : Display shifts automatically (scrolling effect)
     * 
     * @note Shift direction depends on IncrementStatus setting
     ***************************************************************************/
    .DisplayShiftOperation = LCD_NO_SHIFT,
};

/******************************************************************************
 * CONFIGURATION SUMMARY
 * 
 * Pin Configuration: 8-bit mode using Port A pins 0-10
 * Display Type:      16x2 LCD (2-line, 5x8 font)
 * Interface:         8-bit parallel
 * Initial State:     Display ON, Cursor ON, Blink ON
 * Text Direction:    Left-to-right (increment)
 * Scroll Mode:       No auto-shift (stationary)
 ******************************************************************************/

/******************************************************************************
 * ALL AVAILABLE PORT OPTIONS
 * 
 * LCD_PORT_A : GPIO Port A
 * LCD_PORT_B : GPIO Port B
 * LCD_PORT_C : GPIO Port C
 * LCD_PORT_D : GPIO Port D
 * LCD_PORT_E : GPIO Port E
 * LCD_PORT_H : GPIO Port H
 ******************************************************************************/

/******************************************************************************
 * ALL AVAILABLE PIN OPTIONS (PER PORT)
 * 
 * LCD_PIN_0  through LCD_PIN_15
 ******************************************************************************/

/******************************************************************************
 * 4-BIT MODE PIN CONFIGURATION (ALTERNATIVE)
 * 
 * @note For 4-bit mode, use LCD_Pinout_4BitMode_t instead:
 * 
 * const LCD_Pinout_4BitMode_t LcdPinout = {
 *     .RS  = {.port = LCD_PORT_X, .pin = LCD_PIN_X},
 *     .RW  = {.port = LCD_PORT_X, .pin = LCD_PIN_X},
 *     .EN  = {.port = LCD_PORT_X, .pin = LCD_PIN_X},
 *     .DB4 = {.port = LCD_PORT_X, .pin = LCD_PIN_X},
 *     .DB5 = {.port = LCD_PORT_X, .pin = LCD_PIN_X},
 *     .DB6 = {.port = LCD_PORT_X, .pin = LCD_PIN_X},
 *     .DB7 = {.port = LCD_PORT_X, .pin = LCD_PIN_X},
 * };
 * 
 * And set: .BitOperation = LCD_4_BIT_OPERATION
 ******************************************************************************/

/******************************************************************************
 * END OF CONFIGURATION FILE
 ******************************************************************************/