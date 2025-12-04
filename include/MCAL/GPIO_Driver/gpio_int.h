/******************************************************************************
 * @file    GPIO_INT.H
 * @author  Eng.Gemy
 * @brief   GPIO Driver Interface Header File
 *          This file contains all the function prototypes, type definitions,
 *          and enumerations for the GPIO driver
 ******************************************************************************/

#ifndef GPIO_INT_H
#define GPIO_INT_H  

/******************************************************************************
 * @brief GPIO Status Enumeration
 * @details Defines all possible return status codes for GPIO operations
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
    GPIO_OK = 0,                    /**< Operation completed successfully */
    GPIO_NOT_OK,                    /**< Operation failed */
    GPIO_NULL_PTR,                  /**< Null pointer passed as argument */
    GPIO_WRONG_MODE,                /**< Invalid GPIO mode selected */
    GPIO_WRONG_PORT,                /**< Invalid GPIO port selected */
    GPIO_WRONG_PIN,                 /**< Invalid GPIO pin selected */
    GPIO_WRONG_OUTPUT_TYPE,         /**< Invalid output type selected */
    GPIO_WRONG_PULL,                /**< Invalid pull configuration selected */
    GPIO_WRONG_ALTARNATIVE,         /**< Invalid alternate function selected */
    GPIO_WRONG_SPEED,               /**< Invalid speed configuration selected */
    GPIO_WRONG_VALUE,               /**< Invalid pin value */
    GPIO_ERROR                      /**< General GPIO error */
}GPIO_Status_t;

/******************************************************************************
 * @brief GPIO Mode Enumeration
 * @details Defines the operating modes for GPIO pins
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
    GPIO_MODE_INPUT                 = 0,    /**< Input mode (digital input) */
    GPIO_MODE_OUTPUT                = 1,    /**< Output mode (digital output) */
    GPIO_MODE_ALTERNATE_FUNCTION    = 2,    /**< Alternate function mode (peripheral) */
    GPIO_MODE_ANALOG                = 3     /**< Analog mode (ADC/DAC) */
}GPIO_Mode_t;

/******************************************************************************
 * @brief GPIO Output Type Enumeration
 * @details Defines the output driver configuration for GPIO pins
 * @note Only applicable when mode is OUTPUT or ALTERNATE_FUNCTION
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
    GPIO_OUTPUT_TYPE_PUSH_PULL = 0,         /**< Push-pull output (strong high and low) */
    GPIO_OUTPUT_TYPE_OPEN_DRAIN             /**< Open-drain output (requires external pull-up) */
}GPIO_OutputType_t;

/******************************************************************************
 * @brief GPIO Speed Enumeration
 * @details Defines the output speed (slew rate) for GPIO pins
 * @note Only applicable when mode is OUTPUT or ALTERNATE_FUNCTION
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
    GPIO_SPEED_DEFAULT = 0,                 /**< Default speed (same as LOW) */
    GPIO_SPEED_LOW = 0,                     /**< Low speed (up to 8 MHz) */
    GPIO_SPEED_MEDIUM,                      /**< Medium speed (up to 50 MHz) */
    GPIO_SPEED_HIGH,                        /**< High speed (up to 100 MHz) */
    GPIO_SPEED_VERY_HIGH                    /**< Very high speed (up to 180 MHz) */
}GPIO_Speed_t;

/******************************************************************************
 * @brief GPIO Pull Configuration Enumeration
 * @details Defines the internal pull-up/pull-down resistor configuration
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
    GPIO_NO_PULL = 0,                       /**< No pull-up or pull-down */
    GPIO_PULL_UP,                           /**< Pull-up resistor enabled */
    GPIO_PULL_DOWN                          /**< Pull-down resistor enabled */
}GPIO_Pull_t;

/******************************************************************************
 * @brief GPIO Port Enumeration
 * @details Defines available GPIO ports
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
    GPIO_PORT_A = 0,        /**< GPIO Port A (0b000) */
    GPIO_PORT_B = 1,        /**< GPIO Port B (0b001) */
    GPIO_PORT_C = 2,        /**< GPIO Port C (0b010) */
    GPIO_PORT_D = 3,        /**< GPIO Port D (0b011) */
    GPIO_PORT_E = 4,        /**< GPIO Port E (0b100) */
    GPIO_PORT_H = 5         /**< GPIO Port H (0b101) */
}GPIO_Port_t;

/******************************************************************************
 * @brief GPIO Pin Enumeration
 * @details Defines GPIO pin numbers (0-15 for each port)
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
    GPIO_PIN_0 = 0,         /**< GPIO Pin 0 */
    GPIO_PIN_1,             /**< GPIO Pin 1 */
    GPIO_PIN_2,             /**< GPIO Pin 2 */
    GPIO_PIN_3,             /**< GPIO Pin 3 */
    GPIO_PIN_4,             /**< GPIO Pin 4 */
    GPIO_PIN_5,             /**< GPIO Pin 5 */
    GPIO_PIN_6,             /**< GPIO Pin 6 */
    GPIO_PIN_7,             /**< GPIO Pin 7 */
    GPIO_PIN_8,             /**< GPIO Pin 8 */
    GPIO_PIN_9,             /**< GPIO Pin 9 */
    GPIO_PIN_10,            /**< GPIO Pin 10 */
    GPIO_PIN_11,            /**< GPIO Pin 11 */
    GPIO_PIN_12,            /**< GPIO Pin 12 */
    GPIO_PIN_13,            /**< GPIO Pin 13 */
    GPIO_PIN_14,            /**< GPIO Pin 14 */
    GPIO_PIN_15             /**< GPIO Pin 15 */
}GPIO_Pin_t;

/******************************************************************************
 * @brief GPIO Alternate Function Enumeration
 * @details Defines alternate function mapping (AF0-AF15)
 * @note Only applicable when mode is ALTERNATE_FUNCTION
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
    GPIO_AF0 = 0,           /**< Alternate Function 0 */
    GPIO_AF1,               /**< Alternate Function 1 */
    GPIO_AF2,               /**< Alternate Function 2 */
    GPIO_AF3,               /**< Alternate Function 3 */
    GPIO_AF4,               /**< Alternate Function 4 */
    GPIO_AF5,               /**< Alternate Function 5 */
    GPIO_AF6,               /**< Alternate Function 6 */
    GPIO_AF7,               /**< Alternate Function 7 */
    GPIO_AF8,               /**< Alternate Function 8 */
    GPIO_AF9,               /**< Alternate Function 9 */
    GPIO_AF10,              /**< Alternate Function 10 */
    GPIO_AF11,              /**< Alternate Function 11 */
    GPIO_AF12,              /**< Alternate Function 12 */
    GPIO_AF13,              /**< Alternate Function 13 */
    GPIO_AF14,              /**< Alternate Function 14 */
    GPIO_AF15               /**< Alternate Function 15 */
}GPIO_AlternateFunction_t;

/******************************************************************************
 * @brief GPIO Configuration Structure
 * @details Contains all configuration parameters for a GPIO pin
 * @author Eng.Gemy
 ******************************************************************************/
typedef struct
{
    GPIO_Port_t port;                           /**< GPIO Port (A, B, C, D, E, H) */
    GPIO_Pin_t pin;                             /**< GPIO Pin number (0-15) */
    GPIO_Mode_t mode;                           /**< Pin mode (Input/Output/AF/Analog) */
    GPIO_OutputType_t outputType;               /**< Output type (Push-Pull/Open-Drain) - Used only if mode is OUTPUT or AF */
    GPIO_Speed_t speed;                         /**< Output speed (Low/Medium/High/Very High) - Used only if mode is OUTPUT or AF */
    GPIO_Pull_t pull;                           /**< Pull resistor configuration (None/Up/Down) */
    GPIO_AlternateFunction_t alternateFunction; /**< Alternate function selection (AF0-AF15) - Used only if mode is ALTERNATE_FUNCTION */   
}GPIO_cfg_t;

/******************************************************************************
 * @brief GPIO Pin Value Enumeration
 * @details Defines logical pin values (HIGH/LOW)
 * @note Values are optimized for direct register manipulation
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
    GPIO_HIGH = 0,          /**< Logic HIGH (set bit in BSRR lower 16 bits) */
    GPIO_LOW  = 16          /**< Logic LOW (set bit in BSRR upper 16 bits) */
}GPIO_Val_t;

/******************************************************************************
 *                           FUNCTION PROTOTYPES
 ******************************************************************************/

/******************************************************************************
 * @brief Initialize GPIO pin with specified configuration
 * @param[in] Copy_pstGPIOCfg Pointer to GPIO configuration structure
 * @return GPIO_Status_t Status of the operation
 * @retval GPIO_OK          Initialization successful
 * @retval GPIO_NULL_PTR    Null pointer passed
 * @retval GPIO_ERROR       Configuration error
 * @author Eng.Gemy
 ******************************************************************************/
GPIO_Status_t GPIO_enuInit(GPIO_cfg_t *Copy_pstGPIOCfg);

/******************************************************************************
 * @brief Set GPIO pin to HIGH or LOW
 * @param[in] Copy_Port GPIO port (A, B, C, D, E, H)
 * @param[in] Copy_Pin  GPIO pin number (0-15)
 * @param[in] Copy_Val  Pin value (GPIO_HIGH or GPIO_LOW)
 * @return GPIO_Status_t Status of the operation
 * @retval GPIO_OK              Operation successful
 * @retval GPIO_WRONG_PORT      Invalid port
 * @retval GPIO_WRONG_PIN       Invalid pin
 * @retval GPIO_WRONG_VALUE     Invalid value
 * @author Eng.Gemy
 ******************************************************************************/
GPIO_Status_t GPIO_enuSetPinVal(GPIO_Port_t Copy_Port, GPIO_Pin_t Copy_Pin, GPIO_Val_t Copy_Val);

/******************************************************************************
 * @brief Toggle GPIO pin value (HIGH to LOW or LOW to HIGH)
 * @param[in] Copy_Port GPIO port (A, B, C, D, E, H)
 * @param[in] Copy_Pin  GPIO pin number (0-15)
 * @return GPIO_Status_t Status of the operation
 * @retval GPIO_OK              Operation successful
 * @retval GPIO_WRONG_PORT      Invalid port
 * @retval GPIO_WRONG_PIN       Invalid pin
 * @author Eng.Gemy
 ******************************************************************************/
GPIO_Status_t GPIO_enuFlipPinVal(GPIO_Port_t Copy_Port, GPIO_Pin_t Copy_Pin);

/******************************************************************************
 * @brief Set GPIO pin mode (Input/Output/AF/Analog)
 * @param[in] Copy_Port GPIO port (A, B, C, D, E, H)
 * @param[in] Copy_Pin  GPIO pin number (0-15)
 * @param[in] Copy_Mode Desired pin mode
 * @return GPIO_Status_t Status of the operation
 * @retval GPIO_OK              Operation successful
 * @retval GPIO_WRONG_PORT      Invalid port
 * @retval GPIO_WRONG_PIN       Invalid pin
 * @retval GPIO_WRONG_MODE      Invalid mode
 * @author Eng.Gemy
 ******************************************************************************/
GPIO_Status_t GPIO_enuSetPinMode(GPIO_Port_t Copy_Port, GPIO_Pin_t Copy_Pin, GPIO_Mode_t Copy_Mode);

/******************************************************************************
 * @brief Set alternate function for GPIO pin
 * @param[in] Copy_Port GPIO port (A, B, C, D, E, H)
 * @param[in] Copy_Pin  GPIO pin number (0-15)
 * @param[in] Copy_AF   Alternate function (AF0-AF15)
 * @return GPIO_Status_t Status of the operation
 * @retval GPIO_OK                  Operation successful
 * @retval GPIO_WRONG_PORT          Invalid port
 * @retval GPIO_WRONG_PIN           Invalid pin
 * @retval GPIO_WRONG_ALTARNATIVE   Invalid alternate function
 * @author Eng.Gemy
 ******************************************************************************/
GPIO_Status_t GPIO_enuSetAltFunc(GPIO_Port_t Copy_Port, GPIO_Pin_t Copy_Pin, GPIO_AlternateFunction_t Copy_AF);

/******************************************************************************
 * @brief Set GPIO pin output type (Push-Pull/Open-Drain)
 * @param[in] Copy_Port     GPIO port (A, B, C, D, E, H)
 * @param[in] Copy_Pin      GPIO pin number (0-15)
 * @param[in] Copy_OutType  Output type
 * @return GPIO_Status_t Status of the operation
 * @retval GPIO_OK                  Operation successful
 * @retval GPIO_WRONG_PORT          Invalid port
 * @retval GPIO_WRONG_PIN           Invalid pin
 * @retval GPIO_WRONG_OUTPUT_TYPE   Invalid output type
 * @author Eng.Gemy
 ******************************************************************************/
GPIO_Status_t GPIO_enuSetPinOutType(GPIO_Port_t Copy_Port, GPIO_Pin_t Copy_Pin, GPIO_OutputType_t Copy_OutType);

/******************************************************************************
 * @brief Set GPIO pin pull resistor configuration
 * @param[in] Copy_Port GPIO port (A, B, C, D, E, H)
 * @param[in] Copy_Pin  GPIO pin number (0-15)
 * @param[in] Copy_Pull Pull configuration (None/Up/Down)
 * @return GPIO_Status_t Status of the operation
 * @retval GPIO_OK              Operation successful
 * @retval GPIO_WRONG_PORT      Invalid port
 * @retval GPIO_WRONG_PIN       Invalid pin
 * @retval GPIO_WRONG_PULL      Invalid pull configuration
 * @author Eng.Gemy
 ******************************************************************************/
GPIO_Status_t GPIO_enuSetPinPull(GPIO_Port_t Copy_Port, GPIO_Pin_t Copy_Pin, GPIO_Pull_t Copy_Pull);

/******************************************************************************
 * @brief Set GPIO pin output speed
 * @param[in] Copy_Port  GPIO port (A, B, C, D, E, H)
 * @param[in] Copy_Pin   GPIO pin number (0-15)
 * @param[in] Copy_Speed Output speed configuration
 * @return GPIO_Status_t Status of the operation
 * @retval GPIO_OK              Operation successful
 * @retval GPIO_WRONG_PORT      Invalid port
 * @retval GPIO_WRONG_PIN       Invalid pin
 * @retval GPIO_WRONG_SPEED     Invalid speed configuration
 * @author Eng.Gemy
 ******************************************************************************/
GPIO_Status_t GPIO_enuSetPinSpeed(GPIO_Port_t Copy_Port, GPIO_Pin_t Copy_Pin, GPIO_Speed_t Copy_Speed);

/******************************************************************************
 * @brief Read current value of GPIO pin
 * @param[in]  Copy_Port GPIO port (A, B, C, D, E, H)
 * @param[in]  Copy_Pin  GPIO pin number (0-15)
 * @param[out] Copy_pVal Pointer to store the read value
 * @return GPIO_Status_t Status of the operation
 * @retval GPIO_OK              Operation successful
 * @retval GPIO_WRONG_PORT      Invalid port
 * @retval GPIO_WRONG_PIN       Invalid pin
 * @retval GPIO_NULL_PTR        Null pointer passed
 * @author Eng.Gemy
 ******************************************************************************/
GPIO_Status_t GPIO_enuReadPinVal(GPIO_Port_t Copy_Port, GPIO_Pin_t Copy_Pin, uint8_t *Copy_pVal);

#endif // GPIO_INT_H