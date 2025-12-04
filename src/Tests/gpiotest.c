#include "./LIB/stdtypes.h"
#include "./MCAL/GPIO_Driver/gpio_int.h"
// #include <stdio.h>

int gpioTest(void)
{
    GPIO_Status_t status;
    GPIO_cfg_t gpio_config;
    uint8_t pin_value;
    
    printf("=== GPIO Driver Test Suite ===\n\n");
    
    /* ========== Test 1: GPIO_enuInit - Valid Configuration ========== */
    printf("Test 1: GPIO_enuInit - Valid Configuration\n");
    gpio_config.port = GPIO_PORT_A;
    gpio_config.pin = GPIO_PIN_5;
    gpio_config.mode = GPIO_MODE_OUTPUT;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_HIGH;
    gpio_config.pull = GPIO_NO_PULL;
    gpio_config.alternateFunction = GPIO_AF0;
    
    status = GPIO_enuInit(&gpio_config);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - Valid configuration for PA5 as output
    
    /* ========== Test 2: GPIO_enuInit - NULL Pointer ========== */
    printf("Test 2: GPIO_enuInit - NULL Pointer\n");
    status = GPIO_enuInit(NULL);
    printf("Expected: GPIO_NULL_PTR, Got: %d\n\n", status);
    // Expected: GPIO_NULL_PTR (2) - NULL pointer passed
    
    /* ========== Test 3: GPIO_enuInit - Invalid Mode ========== */
    printf("Test 3: GPIO_enuInit - Invalid Mode\n");
    gpio_config.port = GPIO_PORT_B;
    gpio_config.pin = GPIO_PIN_3;
    gpio_config.mode = (GPIO_Mode_t)0xFF; // Invalid mode
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_MEDIUM;
    gpio_config.pull = GPIO_PULL_UP;
    gpio_config.alternateFunction = GPIO_AF1;
    
    status = GPIO_enuInit(&gpio_config);
    printf("Expected: GPIO_WRONG_MODE, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_MODE (3) - Invalid mode value
    
    /* ========== Test 4: GPIO_enuInit - Invalid Port ========== */
    printf("Test 4: GPIO_enuInit - Invalid Port\n");
    gpio_config.port = (GPIO_Port_t)0xFF; // Invalid port
    gpio_config.pin = GPIO_PIN_7;
    gpio_config.mode = GPIO_MODE_INPUT;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_LOW;
    gpio_config.pull = GPIO_PULL_DOWN;
    gpio_config.alternateFunction = GPIO_AF2;
    
    status = GPIO_enuInit(&gpio_config);
    printf("Expected: GPIO_WRONG_PORT, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_PORT (4) - Invalid port value
    
    /* ========== Test 5: GPIO_enuInit - Invalid Pin ========== */
    printf("Test 5: GPIO_enuInit - Invalid Pin\n");
    gpio_config.port = GPIO_PORT_C;
    gpio_config.pin = (GPIO_Pin_t)0xFF; // Invalid pin
    gpio_config.mode = GPIO_MODE_ALTERNATE_FUNCTION;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_OPEN_DRAIN;
    gpio_config.speed = GPIO_SPEED_VERY_HIGH;
    gpio_config.pull = GPIO_NO_PULL;
    gpio_config.alternateFunction = GPIO_AF3;
    
    status = GPIO_enuInit(&gpio_config);
    printf("Expected: GPIO_WRONG_PIN, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_PIN (5) - Invalid pin value
    
    /* ========== Test 6: GPIO_enuInit - Invalid Output Type ========== */
    printf("Test 6: GPIO_enuInit - Invalid Output Type\n");
    gpio_config.port = GPIO_PORT_D;
    gpio_config.pin = GPIO_PIN_12;
    gpio_config.mode = GPIO_MODE_OUTPUT;
    gpio_config.outputType = (GPIO_OutputType_t)0xFF; // Invalid output type
    gpio_config.speed = GPIO_SPEED_MEDIUM;
    gpio_config.pull = GPIO_PULL_UP;
    gpio_config.alternateFunction = GPIO_AF4;
    
    status = GPIO_enuInit(&gpio_config);
    printf("Expected: GPIO_WRONG_OUTPUT_TYPE, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_OUTPUT_TYPE (6) - Invalid output type value
    
    /* ========== Test 7: GPIO_enuInit - Invalid Pull ========== */
    printf("Test 7: GPIO_enuInit - Invalid Pull\n");
    gpio_config.port = GPIO_PORT_E;
    gpio_config.pin = GPIO_PIN_9;
    gpio_config.mode = GPIO_MODE_ANALOG;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_HIGH;
    gpio_config.pull = (GPIO_Pull_t)0xFF; // Invalid pull
    gpio_config.alternateFunction = GPIO_AF5;
    
    status = GPIO_enuInit(&gpio_config);
    printf("Expected: GPIO_WRONG_PULL, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_PULL (7) - Invalid pull value
    
    /* ========== Test 8: GPIO_enuInit - Invalid Alternate Function ========== */
    printf("Test 8: GPIO_enuInit - Invalid Alternate Function\n");
    gpio_config.port = GPIO_PORT_H;
    gpio_config.pin = GPIO_PIN_0;
    gpio_config.mode = GPIO_MODE_ALTERNATE_FUNCTION;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_OPEN_DRAIN;
    gpio_config.speed = GPIO_SPEED_LOW;
    gpio_config.pull = GPIO_NO_PULL;
    gpio_config.alternateFunction = (GPIO_AlternateFunction_t)0xFF; // Invalid AF
    
    status = GPIO_enuInit(&gpio_config);
    printf("Expected: GPIO_WRONG_ALTARNATIVE, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_ALTARNATIVE (8) - Invalid alternate function value
    
    /* ========== Test 9: GPIO_enuInit - Invalid Speed ========== */
    printf("Test 9: GPIO_enuInit - Invalid Speed\n");
    gpio_config.port = GPIO_PORT_A;
    gpio_config.pin = GPIO_PIN_1;
    gpio_config.mode = GPIO_MODE_OUTPUT;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = (GPIO_Speed_t)0xFF; // Invalid speed
    gpio_config.pull = GPIO_PULL_DOWN;
    gpio_config.alternateFunction = GPIO_AF6;
    
    status = GPIO_enuInit(&gpio_config);
    printf("Expected: GPIO_WRONG_SPEED, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_SPEED (9) - Invalid speed value
    
    /* ========== Test 10: GPIO_enuInit - Alternate Function Low Register (Pin < 8) ========== */
    printf("Test 10: GPIO_enuInit - AF Low Register (Pin < 8)\n");
    gpio_config.port = GPIO_PORT_B;
    gpio_config.pin = GPIO_PIN_7;
    gpio_config.mode = GPIO_MODE_ALTERNATE_FUNCTION;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_VERY_HIGH;
    gpio_config.pull = GPIO_NO_PULL;
    gpio_config.alternateFunction = GPIO_AF7;
    
    status = GPIO_enuInit(&gpio_config);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - Valid AF configuration for pin < 8 (AFRL register)
    
    /* ========== Test 11: GPIO_enuInit - Alternate Function High Register (Pin >= 8) ========== */
    printf("Test 11: GPIO_enuInit - AF High Register (Pin >= 8)\n");
    gpio_config.port = GPIO_PORT_C;
    gpio_config.pin = GPIO_PIN_10;
    gpio_config.mode = GPIO_MODE_ALTERNATE_FUNCTION;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_OPEN_DRAIN;
    gpio_config.speed = GPIO_SPEED_HIGH;
    gpio_config.pull = GPIO_PULL_UP;
    gpio_config.alternateFunction = GPIO_AF8;
    
    status = GPIO_enuInit(&gpio_config);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - Valid AF configuration for pin >= 8 (AFRH register)
    
    /* ========== Test 12: GPIO_enuSetPinVal - Set Pin HIGH ========== */
    printf("Test 12: GPIO_enuSetPinVal - Set Pin HIGH\n");
    status = GPIO_enuSetPinVal(GPIO_PORT_A, GPIO_PIN_5, GPIO_HIGH);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - Sets PA5 to HIGH
    
    /* ========== Test 13: GPIO_enuSetPinVal - Set Pin LOW ========== */
    printf("Test 13: GPIO_enuSetPinVal - Set Pin LOW\n");
    status = GPIO_enuSetPinVal(GPIO_PORT_A, GPIO_PIN_5, GPIO_LOW);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - Sets PA5 to LOW
    
    /* ========== Test 14: GPIO_enuSetPinVal - Invalid Port ========== */
    printf("Test 14: GPIO_enuSetPinVal - Invalid Port\n");
    status = GPIO_enuSetPinVal((GPIO_Port_t)0xFF, GPIO_PIN_3, GPIO_HIGH);
    printf("Expected: GPIO_WRONG_PORT, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_PORT (4) - Invalid port value
    
    /* ========== Test 15: GPIO_enuSetPinVal - Invalid Pin ========== */
    printf("Test 15: GPIO_enuSetPinVal - Invalid Pin\n");
    status = GPIO_enuSetPinVal(GPIO_PORT_B, (GPIO_Pin_t)0xFF, GPIO_LOW);
    printf("Expected: GPIO_WRONG_PIN, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_PIN (5) - Invalid pin value
    
    /* ========== Test 16: GPIO_enuSetPinVal - Invalid Value ========== */
    printf("Test 16: GPIO_enuSetPinVal - Invalid Value\n");
    status = GPIO_enuSetPinVal(GPIO_PORT_C, GPIO_PIN_8, (GPIO_Val_t)50);
    printf("Expected: GPIO_WRONG_VALUE, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_VALUE (10) - Invalid value (not GPIO_HIGH or GPIO_LOW)
    
    /* ========== Test 17: GPIO_enuFlipPinVal - Valid ========== */
    printf("Test 17: GPIO_enuFlipPinVal - Valid\n");
    status = GPIO_enuFlipPinVal(GPIO_PORT_A, GPIO_PIN_5);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - Toggles PA5 state
    
    /* ========== Test 18: GPIO_enuFlipPinVal - Invalid Port ========== */
    printf("Test 18: GPIO_enuFlipPinVal - Invalid Port\n");
    status = GPIO_enuFlipPinVal((GPIO_Port_t)0xFF, GPIO_PIN_2);
    printf("Expected: GPIO_WRONG_PORT, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_PORT (4) - Invalid port value
    
    /* ========== Test 19: GPIO_enuFlipPinVal - Invalid Pin ========== */
    printf("Test 19: GPIO_enuFlipPinVal - Invalid Pin\n");
    status = GPIO_enuFlipPinVal(GPIO_PORT_D, (GPIO_Pin_t)0xFF);
    printf("Expected: GPIO_WRONG_PIN, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_PIN (5) - Invalid pin value
    
    /* ========== Test 20: GPIO_enuSetPinMode - Input Mode ========== */
    printf("Test 20: GPIO_enuSetPinMode - Input Mode\n");
    status = GPIO_enuSetPinMode(GPIO_PORT_E, GPIO_PIN_4, GPIO_MODE_INPUT);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - Sets PE4 to input mode
    
    /* ========== Test 21: GPIO_enuSetPinMode - Output Mode ========== */
    printf("Test 21: GPIO_enuSetPinMode - Output Mode\n");
    status = GPIO_enuSetPinMode(GPIO_PORT_B, GPIO_PIN_13, GPIO_MODE_OUTPUT);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - Sets PB13 to output mode
    
    /* ========== Test 22: GPIO_enuSetPinMode - Alternate Function Mode ========== */
    printf("Test 22: GPIO_enuSetPinMode - Alternate Function Mode\n");
    status = GPIO_enuSetPinMode(GPIO_PORT_C, GPIO_PIN_6, GPIO_MODE_ALTERNATE_FUNCTION);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - Sets PC6 to alternate function mode
    
    /* ========== Test 23: GPIO_enuSetPinMode - Analog Mode ========== */
    printf("Test 23: GPIO_enuSetPinMode - Analog Mode\n");
    status = GPIO_enuSetPinMode(GPIO_PORT_A, GPIO_PIN_0, GPIO_MODE_ANALOG);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - Sets PA0 to analog mode
    
    /* ========== Test 24: GPIO_enuSetPinMode - Invalid Port ========== */
    printf("Test 24: GPIO_enuSetPinMode - Invalid Port\n");
    status = GPIO_enuSetPinMode((GPIO_Port_t)0xFF, GPIO_PIN_11, GPIO_MODE_OUTPUT);
    printf("Expected: GPIO_WRONG_PORT, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_PORT (4) - Invalid port value
    
    /* ========== Test 25: GPIO_enuSetPinMode - Invalid Pin ========== */
    printf("Test 25: GPIO_enuSetPinMode - Invalid Pin\n");
    status = GPIO_enuSetPinMode(GPIO_PORT_H, (GPIO_Pin_t)0xFF, GPIO_MODE_INPUT);
    printf("Expected: GPIO_WRONG_PIN, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_PIN (5) - Invalid pin value
    
    /* ========== Test 26: GPIO_enuSetPinMode - Invalid Mode ========== */
    printf("Test 26: GPIO_enuSetPinMode - Invalid Mode\n");
    status = GPIO_enuSetPinMode(GPIO_PORT_D, GPIO_PIN_15, (GPIO_Mode_t)0xFF);
    printf("Expected: GPIO_WRONG_MODE, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_MODE (3) - Invalid mode value
    
    /* ========== Test 27: GPIO_enuSetAltFunc - AF0 to AF7 ========== */
    printf("Test 27: GPIO_enuSetAltFunc - AF0 to AF7\n");
    status = GPIO_enuSetAltFunc(GPIO_PORT_A, GPIO_PIN_2, GPIO_AF7);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - Sets PA2 alternate function to AF7 (AFRL register)
    
    /* ========== Test 28: GPIO_enuSetAltFunc - AF8 to AF15 ========== */
    printf("Test 28: GPIO_enuSetAltFunc - AF8 to AF15\n");
    status = GPIO_enuSetAltFunc(GPIO_PORT_B, GPIO_PIN_9, GPIO_AF12);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - Sets PB9 alternate function to AF12 (AFRH register)
    
    /* ========== Test 29: GPIO_enuSetAltFunc - Invalid Port ========== */
    printf("Test 29: GPIO_enuSetAltFunc - Invalid Port\n");
    status = GPIO_enuSetAltFunc((GPIO_Port_t)0xFF, GPIO_PIN_14, GPIO_AF5);
    printf("Expected: GPIO_WRONG_PORT, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_PORT (4) - Invalid port value
    
    /* ========== Test 30: GPIO_enuSetAltFunc - Invalid Pin ========== */
    printf("Test 30: GPIO_enuSetAltFunc - Invalid Pin\n");
    status = GPIO_enuSetAltFunc(GPIO_PORT_C, (GPIO_Pin_t)0xFF, GPIO_AF3);
    printf("Expected: GPIO_WRONG_PIN, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_PIN (5) - Invalid pin value
    
    /* ========== Test 31: GPIO_enuSetAltFunc - Invalid Alternate Function ========== */
    printf("Test 31: GPIO_enuSetAltFunc - Invalid Alternate Function\n");
    status = GPIO_enuSetAltFunc(GPIO_PORT_E, GPIO_PIN_5, (GPIO_AlternateFunction_t)0xFF);
    printf("Expected: GPIO_WRONG_ALTARNATIVE, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_ALTARNATIVE (8) - Invalid alternate function value
    
    /* ========== Test 32: GPIO_enuSetPinOutType - Push Pull ========== */
    printf("Test 32: GPIO_enuSetPinOutType - Push Pull\n");
    status = GPIO_enuSetPinOutType(GPIO_PORT_A, GPIO_PIN_8, GPIO_OUTPUT_TYPE_PUSH_PULL);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - Sets PA8 output type to push-pull
    
    /* ========== Test 33: GPIO_enuSetPinOutType - Open Drain ========== */
    printf("Test 33: GPIO_enuSetPinOutType - Open Drain\n");
    status = GPIO_enuSetPinOutType(GPIO_PORT_B, GPIO_PIN_6, GPIO_OUTPUT_TYPE_OPEN_DRAIN);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - Sets PB6 output type to open-drain
    
    /* ========== Test 34: GPIO_enuSetPinOutType - Invalid Port ========== */
    printf("Test 34: GPIO_enuSetPinOutType - Invalid Port\n");
    status = GPIO_enuSetPinOutType((GPIO_Port_t)0xFF, GPIO_PIN_10, GPIO_OUTPUT_TYPE_PUSH_PULL);
    printf("Expected: GPIO_WRONG_PORT, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_PORT (4) - Invalid port value
    
    /* ========== Test 35: GPIO_enuSetPinOutType - Invalid Pin ========== */
    printf("Test 35: GPIO_enuSetPinOutType - Invalid Pin\n");
    status = GPIO_enuSetPinOutType(GPIO_PORT_C, (GPIO_Pin_t)0xFF, GPIO_OUTPUT_TYPE_OPEN_DRAIN);
    printf("Expected: GPIO_WRONG_PIN, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_PIN (5) - Invalid pin value
    
    /* ========== Test 36: GPIO_enuSetPinOutType - Invalid Output Type ========== */
    printf("Test 36: GPIO_enuSetPinOutType - Invalid Output Type\n");
    status = GPIO_enuSetPinOutType(GPIO_PORT_D, GPIO_PIN_1, (GPIO_OutputType_t)0xFF);
    printf("Expected: GPIO_WRONG_OUTPUT_TYPE, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_OUTPUT_TYPE (6) - Invalid output type value
    
    /* ========== Test 37: GPIO_enuSetPinPull - No Pull ========== */
    printf("Test 37: GPIO_enuSetPinPull - No Pull\n");
    status = GPIO_enuSetPinPull(GPIO_PORT_E, GPIO_PIN_12, GPIO_NO_PULL);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - Sets PE12 to no pull
    
    /* ========== Test 38: GPIO_enuSetPinPull - Pull Up ========== */
    printf("Test 38: GPIO_enuSetPinPull - Pull Up\n");
    status = GPIO_enuSetPinPull(GPIO_PORT_H, GPIO_PIN_0, GPIO_PULL_UP);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - Sets PH0 to pull-up
    
    /* ========== Test 39: GPIO_enuSetPinPull - Pull Down ========== */
    printf("Test 39: GPIO_enuSetPinPull - Pull Down\n");
    status = GPIO_enuSetPinPull(GPIO_PORT_A, GPIO_PIN_15, GPIO_PULL_DOWN);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - Sets PA15 to pull-down
    
    /* ========== Test 40: GPIO_enuSetPinPull - Invalid Port ========== */
    printf("Test 40: GPIO_enuSetPinPull - Invalid Port\n");
    status = GPIO_enuSetPinPull((GPIO_Port_t)0xFF, GPIO_PIN_7, GPIO_PULL_UP);
    printf("Expected: GPIO_WRONG_PORT, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_PORT (4) - Invalid port value
    
    /* ========== Test 41: GPIO_enuSetPinPull - Invalid Pin ========== */
    printf("Test 41: GPIO_enuSetPinPull - Invalid Pin\n");
    status = GPIO_enuSetPinPull(GPIO_PORT_B, (GPIO_Pin_t)0xFF, GPIO_NO_PULL);
    printf("Expected: GPIO_WRONG_PIN, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_PIN (5) - Invalid pin value
    
    /* ========== Test 42: GPIO_enuSetPinPull - Invalid Pull ========== */
    printf("Test 42: GPIO_enuSetPinPull - Invalid Pull\n");
    status = GPIO_enuSetPinPull(GPIO_PORT_C, GPIO_PIN_4, (GPIO_Pull_t)0xFF);
    printf("Expected: GPIO_WRONG_PULL, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_PULL (7) - Invalid pull value
    
    /* ========== Test 43: GPIO_enuSetPinSpeed - Low Speed ========== */
    printf("Test 43: GPIO_enuSetPinSpeed - Low Speed\n");
    status = GPIO_enuSetPinSpeed(GPIO_PORT_D, GPIO_PIN_3, GPIO_SPEED_LOW);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - Sets PD3 speed to low
    
    /* ========== Test 44: GPIO_enuSetPinSpeed - Medium Speed ========== */
    printf("Test 44: GPIO_enuSetPinSpeed - Medium Speed\n");
    status = GPIO_enuSetPinSpeed(GPIO_PORT_E, GPIO_PIN_8, GPIO_SPEED_MEDIUM);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - Sets PE8 speed to medium
    
    /* ========== Test 45: GPIO_enuSetPinSpeed - High Speed ========== */
    printf("Test 45: GPIO_enuSetPinSpeed - High Speed\n");
    status = GPIO_enuSetPinSpeed(GPIO_PORT_A, GPIO_PIN_11, GPIO_SPEED_HIGH);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - Sets PA11 speed to high
    
    /* ========== Test 46: GPIO_enuSetPinSpeed - Very High Speed ========== */
    printf("Test 46: GPIO_enuSetPinSpeed - Very High Speed\n");
    status = GPIO_enuSetPinSpeed(GPIO_PORT_B, GPIO_PIN_14, GPIO_SPEED_VERY_HIGH);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - Sets PB14 speed to very high
    
    /* ========== Test 47: GPIO_enuSetPinSpeed - Invalid Port ========== */
    printf("Test 47: GPIO_enuSetPinSpeed - Invalid Port\n");
    status = GPIO_enuSetPinSpeed((GPIO_Port_t)0xFF, GPIO_PIN_9, GPIO_SPEED_HIGH);
    printf("Expected: GPIO_WRONG_PORT, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_PORT (4) - Invalid port value
    
    /* ========== Test 48: GPIO_enuSetPinSpeed - Invalid Pin ========== */
    printf("Test 48: GPIO_enuSetPinSpeed - Invalid Pin\n");
    status = GPIO_enuSetPinSpeed(GPIO_PORT_C, (GPIO_Pin_t)0xFF, GPIO_SPEED_MEDIUM);
    printf("Expected: GPIO_WRONG_PIN, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_PIN (5) - Invalid pin value
    
    /* ========== Test 49: GPIO_enuSetPinSpeed - Invalid Speed ========== */
    printf("Test 49: GPIO_enuSetPinSpeed - Invalid Speed\n");
    status = GPIO_enuSetPinSpeed(GPIO_PORT_H, GPIO_PIN_1, (GPIO_Speed_t)0xFF);
    printf("Expected: GPIO_WRONG_SPEED, Got: %d\n\n", status);
    // Expected: GPIO_WRONG_SPEED (9) - Invalid speed value
    
    /* ========== Test 50: Multiple Pins Configuration on Same Port ========== */
    printf("Test 50: Multiple Pins Configuration on Same Port\n");
    gpio_config.port = GPIO_PORT_A;
    gpio_config.mode = GPIO_MODE_OUTPUT;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_HIGH;
    gpio_config.pull = GPIO_NO_PULL;
    gpio_config.alternateFunction = GPIO_AF0;
    
    // Configure PA0
    gpio_config.pin = GPIO_PIN_0;
    status = GPIO_enuInit(&gpio_config);
    printf("PA0 Configuration - Expected: GPIO_OK, Got: %d\n", status);
    
    // Configure PA1
    gpio_config.pin = GPIO_PIN_1;
    status = GPIO_enuInit(&gpio_config);
    printf("PA1 Configuration - Expected: GPIO_OK, Got: %d\n", status);
    
    // Configure PA2
    gpio_config.pin = GPIO_PIN_2;
    status = GPIO_enuInit(&gpio_config);
    printf("PA2 Configuration - Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) for all three - Multiple pins on same port configured successfully
    
    /* ========== Test 51: All Ports Configuration ========== */
    printf("Test 51: All Ports Configuration\n");
    gpio_config.pin = GPIO_PIN_0;
    gpio_config.mode = GPIO_MODE_INPUT;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_LOW;
    gpio_config.pull = GPIO_PULL_UP;
    gpio_config.alternateFunction = GPIO_AF0;
    
    // Configure all available ports
    gpio_config.port = GPIO_PORT_A;
    status = GPIO_enuInit(&gpio_config);
    printf("Port A - Expected: GPIO_OK, Got: %d\n", status);
    
    gpio_config.port = GPIO_PORT_B;
    status = GPIO_enuInit(&gpio_config);
    printf("Port B - Expected: GPIO_OK, Got: %d\n", status);
    
    gpio_config.port = GPIO_PORT_C;
    status = GPIO_enuInit(&gpio_config);
    printf("Port C - Expected: GPIO_OK, Got: %d\n", status);
    
    gpio_config.port = GPIO_PORT_D;
    status = GPIO_enuInit(&gpio_config);
    printf("Port D - Expected: GPIO_OK, Got: %d\n", status);
    
    gpio_config.port = GPIO_PORT_E;
    status = GPIO_enuInit(&gpio_config);
    printf("Port E - Expected: GPIO_OK, Got: %d\n", status);
    
    gpio_config.port = GPIO_PORT_H;
    status = GPIO_enuInit(&gpio_config);
    printf("Port H - Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) for all - All ports configured successfully
    
    /* ========== Test 52: All Pins Configuration ========== */
    printf("Test 52: All Pins Configuration (Port B)\n");
    gpio_config.port = GPIO_PORT_B;
    gpio_config.mode = GPIO_MODE_OUTPUT;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_MEDIUM;
    gpio_config.pull = GPIO_NO_PULL;
    gpio_config.alternateFunction = GPIO_AF0;
    
    for(uint8_t pin = GPIO_PIN_0; pin <= GPIO_PIN_15; pin++){
        gpio_config.pin = (GPIO_Pin_t)pin;
        status = GPIO_enuInit(&gpio_config);
        printf("Pin %d - Expected: GPIO_OK, Got: %d\n", pin, status);
    }
    printf("\n");
    // Expected: GPIO_OK (0) for all pins - All 16 pins configured successfully
    
    /* ========== Test 53: All Modes Configuration ========== */
    printf("Test 53: All Modes Configuration\n");
    gpio_config.port = GPIO_PORT_C;
    gpio_config.pin = GPIO_PIN_0;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_HIGH;
    gpio_config.pull = GPIO_NO_PULL;
    gpio_config.alternateFunction = GPIO_AF1;
    
    gpio_config.mode = GPIO_MODE_INPUT;
    status = GPIO_enuInit(&gpio_config);
    printf("Mode INPUT - Expected: GPIO_OK, Got: %d\n", status);
    
    gpio_config.mode = GPIO_MODE_OUTPUT;
    status = GPIO_enuInit(&gpio_config);
    printf("Mode OUTPUT - Expected: GPIO_OK, Got: %d\n", status);
    
    gpio_config.mode = GPIO_MODE_ALTERNATE_FUNCTION;
    status = GPIO_enuInit(&gpio_config);
    printf("Mode ALTERNATE_FUNCTION - Expected: GPIO_OK, Got: %d\n", status);
    
    gpio_config.mode = GPIO_MODE_ANALOG;
    status = GPIO_enuInit(&gpio_config);
    printf("Mode ANALOG - Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) for all modes - All modes configured successfully
    
    /* ========== Test 54: All Output Types Configuration ========== */
    printf("Test 54: All Output Types Configuration\n");
    gpio_config.port = GPIO_PORT_D;
    gpio_config.pin = GPIO_PIN_5;
    gpio_config.mode = GPIO_MODE_OUTPUT;
    gpio_config.speed = GPIO_SPEED_MEDIUM;
    gpio_config.pull = GPIO_NO_PULL;
    gpio_config.alternateFunction = GPIO_AF0;
    
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    status = GPIO_enuInit(&gpio_config);
    printf("Output Type PUSH_PULL - Expected: GPIO_OK, Got: %d\n", status);
    
    gpio_config.outputType = GPIO_OUTPUT_TYPE_OPEN_DRAIN;
    status = GPIO_enuInit(&gpio_config);
    printf("Output Type OPEN_DRAIN - Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) for both - Both output types configured successfully
    
    /* ========== Test 55: All Pull Configurations ========== */
    printf("Test 55: All Pull Configurations\n");
    gpio_config.port = GPIO_PORT_E;
    gpio_config.pin = GPIO_PIN_10;
    gpio_config.mode = GPIO_MODE_INPUT;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_LOW;
    gpio_config.alternateFunction = GPIO_AF0;
    
    gpio_config.pull = GPIO_NO_PULL;
    status = GPIO_enuInit(&gpio_config);
    printf("Pull NO_PULL - Expected: GPIO_OK, Got: %d\n", status);
    
    gpio_config.pull = GPIO_PULL_UP;
    status = GPIO_enuInit(&gpio_config);
    printf("Pull PULL_UP - Expected: GPIO_OK, Got: %d\n", status);
    
    gpio_config.pull = GPIO_PULL_DOWN;
    status = GPIO_enuInit(&gpio_config);
    printf("Pull PULL_DOWN - Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) for all - All pull configurations set successfully
    
    /* ========== Test 56: All Speed Configurations ========== */
    printf("Test 56: All Speed Configurations\n");
    gpio_config.port = GPIO_PORT_A;
    gpio_config.pin = GPIO_PIN_12;
    gpio_config.mode = GPIO_MODE_OUTPUT;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.pull = GPIO_NO_PULL;
    gpio_config.alternateFunction = GPIO_AF0;
    
    gpio_config.speed = GPIO_SPEED_LOW;
    status = GPIO_enuInit(&gpio_config);
    printf("Speed LOW - Expected: GPIO_OK, Got: %d\n", status);
    
    gpio_config.speed = GPIO_SPEED_MEDIUM;
    status = GPIO_enuInit(&gpio_config);
    printf("Speed MEDIUM - Expected: GPIO_OK, Got: %d\n", status);
    
    gpio_config.speed = GPIO_SPEED_HIGH;
    status = GPIO_enuInit(&gpio_config);
    printf("Speed HIGH - Expected: GPIO_OK, Got: %d\n", status);
    
    gpio_config.speed = GPIO_SPEED_VERY_HIGH;
    status = GPIO_enuInit(&gpio_config);
    printf("Speed VERY_HIGH - Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) for all - All speed configurations set successfully
    
    /* ========== Test 57: All Alternate Functions Configuration ========== */
    printf("Test 57: All Alternate Functions Configuration\n");
    gpio_config.port = GPIO_PORT_B;
    gpio_config.pin = GPIO_PIN_15;
    gpio_config.mode = GPIO_MODE_ALTERNATE_FUNCTION;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_HIGH;
    gpio_config.pull = GPIO_NO_PULL;
    
    for(uint8_t af = GPIO_AF0; af <= GPIO_AF15; af++){
        gpio_config.alternateFunction = (GPIO_AlternateFunction_t)af;
        status = GPIO_enuInit(&gpio_config);
        printf("AF%d - Expected: GPIO_OK, Got: %d\n", af, status);
    }
    printf("\n");
    // Expected: GPIO_OK (0) for all AF0-AF15 - All alternate functions configured successfully
    
    /* ========== Test 58: Toggle Pin Multiple Times ========== */
    printf("Test 58: Toggle Pin Multiple Times\n");
    // First configure pin as output
    gpio_config.port = GPIO_PORT_C;
    gpio_config.pin = GPIO_PIN_13;
    gpio_config.mode = GPIO_MODE_OUTPUT;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_LOW;
    gpio_config.pull = GPIO_NO_PULL;
    gpio_config.alternateFunction = GPIO_AF0;
    status = GPIO_enuInit(&gpio_config);
    printf("Pin Configuration - Expected: GPIO_OK, Got: %d\n", status);
    
    // Toggle 5 times
    for(uint8_t i = 0; i < 5; i++){
        status = GPIO_enuFlipPinVal(GPIO_PORT_C, GPIO_PIN_13);
        printf("Toggle %d - Expected: GPIO_OK, Got: %d\n", i+1, status);
    }
    printf("\n");
    // Expected: GPIO_OK (0) for all - Pin toggled 5 times successfully
    
    /* ========== Test 59: Set Pin HIGH then LOW Sequence ========== */
    printf("Test 59: Set Pin HIGH then LOW Sequence\n");
    // Configure pin as output
    gpio_config.port = GPIO_PORT_D;
    gpio_config.pin = GPIO_PIN_8;
    gpio_config.mode = GPIO_MODE_OUTPUT;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_MEDIUM;
    gpio_config.pull = GPIO_NO_PULL;
    gpio_config.alternateFunction = GPIO_AF0;
    status = GPIO_enuInit(&gpio_config);
    printf("Pin Configuration - Expected: GPIO_OK, Got: %d\n", status);
    
    // Set HIGH
    status = GPIO_enuSetPinVal(GPIO_PORT_D, GPIO_PIN_8, GPIO_HIGH);
    printf("Set HIGH - Expected: GPIO_OK, Got: %d\n", status);
    
    // Set LOW
    status = GPIO_enuSetPinVal(GPIO_PORT_D, GPIO_PIN_8, GPIO_LOW);
    printf("Set LOW - Expected: GPIO_OK, Got: %d\n", status);
    
    // Set HIGH again
    status = GPIO_enuSetPinVal(GPIO_PORT_D, GPIO_PIN_8, GPIO_HIGH);
    printf("Set HIGH again - Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) for all - Pin set HIGH/LOW sequence successful
    
    /* ========== Test 60: Change Pin Mode Dynamically ========== */
    printf("Test 60: Change Pin Mode Dynamically\n");
    // Configure as INPUT first
    gpio_config.port = GPIO_PORT_E;
    gpio_config.pin = GPIO_PIN_2;
    gpio_config.mode = GPIO_MODE_INPUT;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_LOW;
    gpio_config.pull = GPIO_PULL_UP;
    gpio_config.alternateFunction = GPIO_AF0;
    status = GPIO_enuInit(&gpio_config);
    printf("Initial INPUT mode - Expected: GPIO_OK, Got: %d\n", status);
    
    // Change to OUTPUT
    status = GPIO_enuSetPinMode(GPIO_PORT_E, GPIO_PIN_2, GPIO_MODE_OUTPUT);
    printf("Change to OUTPUT mode - Expected: GPIO_OK, Got: %d\n", status);
    
    // Change to ALTERNATE_FUNCTION
    status = GPIO_enuSetPinMode(GPIO_PORT_E, GPIO_PIN_2, GPIO_MODE_ALTERNATE_FUNCTION);
    printf("Change to ALTERNATE_FUNCTION mode - Expected: GPIO_OK, Got: %d\n", status);
    
    // Change to ANALOG
    status = GPIO_enuSetPinMode(GPIO_PORT_E, GPIO_PIN_2, GPIO_MODE_ANALOG);
    printf("Change to ANALOG mode - Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) for all - Pin mode changed dynamically
    
    /* ========== Test 61: Change Alternate Function Dynamically ========== */
    printf("Test 61: Change Alternate Function Dynamically\n");
    // Configure as AF first
    gpio_config.port = GPIO_PORT_A;
    gpio_config.pin = GPIO_PIN_9;
    gpio_config.mode = GPIO_MODE_ALTERNATE_FUNCTION;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_HIGH;
    gpio_config.pull = GPIO_NO_PULL;
    gpio_config.alternateFunction = GPIO_AF7;
    status = GPIO_enuInit(&gpio_config);
    printf("Initial AF7 - Expected: GPIO_OK, Got: %d\n", status);
    
    // Change to AF1
    status = GPIO_enuSetAltFunc(GPIO_PORT_A, GPIO_PIN_9, GPIO_AF1);
    printf("Change to AF1 - Expected: GPIO_OK, Got: %d\n", status);
    
    // Change to AF12
    status = GPIO_enuSetAltFunc(GPIO_PORT_A, GPIO_PIN_9, GPIO_AF12);
    printf("Change to AF12 - Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) for all - Alternate function changed dynamically
    
    /* ========== Test 62: Change Output Type Dynamically ========== */
    printf("Test 62: Change Output Type Dynamically\n");
    // Configure as OUTPUT first
    gpio_config.port = GPIO_PORT_B;
    gpio_config.pin = GPIO_PIN_4;
    gpio_config.mode = GPIO_MODE_OUTPUT;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_MEDIUM;
    gpio_config.pull = GPIO_NO_PULL;
    gpio_config.alternateFunction = GPIO_AF0;
    status = GPIO_enuInit(&gpio_config);
    printf("Initial PUSH_PULL - Expected: GPIO_OK, Got: %d\n", status);
    
    // Change to OPEN_DRAIN
    status = GPIO_enuSetPinOutType(GPIO_PORT_B, GPIO_PIN_4, GPIO_OUTPUT_TYPE_OPEN_DRAIN);
    printf("Change to OPEN_DRAIN - Expected: GPIO_OK, Got: %d\n", status);
    
    // Change back to PUSH_PULL
    status = GPIO_enuSetPinOutType(GPIO_PORT_B, GPIO_PIN_4, GPIO_OUTPUT_TYPE_PUSH_PULL);
    printf("Change back to PUSH_PULL - Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) for all - Output type changed dynamically
    
    /* ========== Test 63: Change Pull Configuration Dynamically ========== */
    printf("Test 63: Change Pull Configuration Dynamically\n");
    // Configure with NO_PULL first
    gpio_config.port = GPIO_PORT_C;
    gpio_config.pin = GPIO_PIN_7;
    gpio_config.mode = GPIO_MODE_INPUT;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_LOW;
    gpio_config.pull = GPIO_NO_PULL;
    gpio_config.alternateFunction = GPIO_AF0;
    status = GPIO_enuInit(&gpio_config);
    printf("Initial NO_PULL - Expected: GPIO_OK, Got: %d\n", status);
    
    // Change to PULL_UP
    status = GPIO_enuSetPinPull(GPIO_PORT_C, GPIO_PIN_7, GPIO_PULL_UP);
    printf("Change to PULL_UP - Expected: GPIO_OK, Got: %d\n", status);
    
    // Change to PULL_DOWN
    status = GPIO_enuSetPinPull(GPIO_PORT_C, GPIO_PIN_7, GPIO_PULL_DOWN);
    printf("Change to PULL_DOWN - Expected: GPIO_OK, Got: %d\n", status);
    
    // Change back to NO_PULL
    status = GPIO_enuSetPinPull(GPIO_PORT_C, GPIO_PIN_7, GPIO_NO_PULL);
    printf("Change back to NO_PULL - Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) for all - Pull configuration changed dynamically
    
    /* ========== Test 64: Change Speed Dynamically ========== */
    printf("Test 64: Change Speed Dynamically\n");
    // Configure with LOW speed first
    gpio_config.port = GPIO_PORT_D;
    gpio_config.pin = GPIO_PIN_11;
    gpio_config.mode = GPIO_MODE_OUTPUT;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_LOW;
    gpio_config.pull = GPIO_NO_PULL;
    gpio_config.alternateFunction = GPIO_AF0;
    status = GPIO_enuInit(&gpio_config);
    printf("Initial LOW speed - Expected: GPIO_OK, Got: %d\n", status);
    
    // Change to MEDIUM
    status = GPIO_enuSetPinSpeed(GPIO_PORT_D, GPIO_PIN_11, GPIO_SPEED_MEDIUM);
    printf("Change to MEDIUM speed - Expected: GPIO_OK, Got: %d\n", status);
    
    // Change to HIGH
    status = GPIO_enuSetPinSpeed(GPIO_PORT_D, GPIO_PIN_11, GPIO_SPEED_HIGH);
    printf("Change to HIGH speed - Expected: GPIO_OK, Got: %d\n", status);
    
    // Change to VERY_HIGH
    status = GPIO_enuSetPinSpeed(GPIO_PORT_D, GPIO_PIN_11, GPIO_SPEED_VERY_HIGH);
    printf("Change to VERY_HIGH speed - Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) for all - Speed changed dynamically
    
    /* ========== Test 65: Boundary Test - Pin 0 and Pin 15 ========== */
    printf("Test 65: Boundary Test - Pin 0 and Pin 15\n");
    gpio_config.port = GPIO_PORT_H;
    gpio_config.mode = GPIO_MODE_OUTPUT;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_LOW;
    gpio_config.pull = GPIO_NO_PULL;
    gpio_config.alternateFunction = GPIO_AF0;
    
    // Test Pin 0
    gpio_config.pin = GPIO_PIN_0;
    status = GPIO_enuInit(&gpio_config);
    printf("Pin 0 Configuration - Expected: GPIO_OK, Got: %d\n", status);
    
    // Test Pin 15
    gpio_config.pin = GPIO_PIN_15;
    status = GPIO_enuInit(&gpio_config);
    printf("Pin 15 Configuration - Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) for both - Boundary pins configured successfully
    
    /* ========== Test 66: Boundary Test - AF Low/High Register Transition ========== */
    printf("Test 66: Boundary Test - AF Low/High Register Transition\n");
    gpio_config.port = GPIO_PORT_A;
    gpio_config.mode = GPIO_MODE_ALTERNATE_FUNCTION;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_MEDIUM;
    gpio_config.pull = GPIO_NO_PULL;
    gpio_config.alternateFunction = GPIO_AF5;
    
    // Test Pin 7 (last pin in AFRL)
    gpio_config.pin = GPIO_PIN_7;
    status = GPIO_enuInit(&gpio_config);
    printf("Pin 7 (AFRL) - Expected: GPIO_OK, Got: %d\n", status);
    
    // Test Pin 8 (first pin in AFRH)
    gpio_config.pin = GPIO_PIN_8;
    status = GPIO_enuInit(&gpio_config);
    printf("Pin 8 (AFRH) - Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) for both - AF register transition handled correctly
    
    /* ========== Test 67: Multiple Function Calls on Same Pin ========== */
    printf("Test 67: Multiple Function Calls on Same Pin\n");
    // Initialize pin
    gpio_config.port = GPIO_PORT_E;
    gpio_config.pin = GPIO_PIN_6;
    gpio_config.mode = GPIO_MODE_OUTPUT;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_HIGH;
    gpio_config.pull = GPIO_NO_PULL;
    gpio_config.alternateFunction = GPIO_AF0;
    status = GPIO_enuInit(&gpio_config);
    printf("Init - Expected: GPIO_OK, Got: %d\n", status);
    
    // Set pin HIGH
    status = GPIO_enuSetPinVal(GPIO_PORT_E, GPIO_PIN_6, GPIO_HIGH);
    printf("Set HIGH - Expected: GPIO_OK, Got: %d\n", status);
    
    // Change mode
    status = GPIO_enuSetPinMode(GPIO_PORT_E, GPIO_PIN_6, GPIO_MODE_INPUT);
    printf("Change Mode - Expected: GPIO_OK, Got: %d\n", status);
    
    // Change pull
    status = GPIO_enuSetPinPull(GPIO_PORT_E, GPIO_PIN_6, GPIO_PULL_UP);
    printf("Change Pull - Expected: GPIO_OK, Got: %d\n", status);
    
    // Change speed
    status = GPIO_enuSetPinSpeed(GPIO_PORT_E, GPIO_PIN_6, GPIO_SPEED_LOW);
    printf("Change Speed - Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) for all - Multiple operations on same pin successful
    
    /* ========== Test 68: Edge Case - GPIO_SPEED_DEFAULT ========== */
    printf("Test 68: Edge Case - GPIO_SPEED_DEFAULT\n");
    gpio_config.port = GPIO_PORT_B;
    gpio_config.pin = GPIO_PIN_11;
    gpio_config.mode = GPIO_MODE_OUTPUT;
    gpio_config.outputType = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_config.speed = GPIO_SPEED_DEFAULT; // Same as GPIO_SPEED_LOW
    gpio_config.pull = GPIO_NO_PULL;
    gpio_config.alternateFunction = GPIO_AF0;
    status = GPIO_enuInit(&gpio_config);
    printf("Expected: GPIO_OK, Got: %d\n\n", status);
    // Expected: GPIO_OK (0) - GPIO_SPEED_DEFAULT (same as LOW) configured successfully
    
    printf("=== Test Suite Complete ===\n");
    printf("Total Tests: 68\n");
    
    // Infinite loop (typical for embedded systems)
    while(1){
        // Main loop - application code would go here
    }
    
    return 0;
}