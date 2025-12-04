/******************************************************************************
 * @file    GPIO.C
 * @author  Eng.Gemy
 * @brief   GPIO Driver Implementation File
 *          This file contains the implementation of all GPIO driver functions
 *          for STM32 microcontrollers
 * @date    [Current Date]
 * @version 1.0
 ******************************************************************************/

#include "./LIB/stdtypes.h"

#include "./MCAL/GPIO_Driver/gpio_cfg.h"
#include "./MCAL/GPIO_Driver/gpio_priv.h"
#include "./MCAL/GPIO_Driver/gpio_int.h"

/******************************************************************************
 * @brief GPIO Base Addresses Array
 * @details Array containing base addresses for all GPIO ports
 *          Index corresponds to GPIO_Port_t enumeration values
 * @note Array order: [0]=GPIOA, [1]=GPIOB, [2]=GPIOC, [3]=GPIOD, [4]=GPIOE, [5]=GPIOH
 * @author Eng.Gemy
 ******************************************************************************/
const uint32_t GPIO_Base_Addreses[] = {
    GPIOA_BASE_ADDR,    /**< Index 0: GPIO Port A base address */
    GPIOB_BASE_ADDR,    /**< Index 1: GPIO Port B base address */
    GPIOC_BASE_ADDR,    /**< Index 2: GPIO Port C base address */
    GPIOD_BASE_ADDR,    /**< Index 3: GPIO Port D base address */
    GPIOE_BASE_ADDR,    /**< Index 4: GPIO Port E base address */
    GPIOH_BASE_ADDR     /**< Index 5: GPIO Port H base address */
};

/******************************************************************************
 * @brief Initialize GPIO pin with specified configuration
 * @details This function configures a GPIO pin with all specified parameters:
 *          - Pin mode (Input/Output/Alternate Function/Analog)
 *          - Output type (Push-Pull/Open-Drain)
 *          - Pull resistor (None/Pull-Up/Pull-Down)
 *          - Output speed (Low/Medium/High/Very High)
 *          - Alternate function (AF0-AF15)
 * 
 * @param[in] GPIO_cfg Pointer to GPIO configuration structure
 * 
 * @return GPIO_Status_t Status of the initialization
 * @retval GPIO_OK                  Initialization successful
 * @retval GPIO_NULL_PTR            Null pointer passed
 * @retval GPIO_WRONG_MODE          Invalid mode value
 * @retval GPIO_WRONG_PORT          Invalid port value
 * @retval GPIO_WRONG_PIN           Invalid pin value
 * @retval GPIO_WRONG_OUTPUT_TYPE   Invalid output type value
 * @retval GPIO_WRONG_PULL          Invalid pull configuration value
 * @retval GPIO_WRONG_ALTARNATIVE   Invalid alternate function value
 * @retval GPIO_WRONG_SPEED         Invalid speed value
 * 
 * @note This function performs comprehensive parameter validation before
 *       configuring the GPIO registers
 * @warning Ensure GPIO clock is enabled before calling this function
 * @author Eng.Gemy
 ******************************************************************************/
GPIO_Status_t GPIO_enuInit(GPIO_cfg_t * GPIO_cfg){
    
    /* Local variable to hold function return status */
    GPIO_Status_t status = GPIO_NOT_OK;
    
    /* Check if configuration pointer is NULL */
    if(NULL == GPIO_cfg){
        status = GPIO_NULL_PTR;
    }else{
        /* Validate mode parameter (must be 0-3, so bits 2-31 must be clear) */
        if(0 != ((GPIO_cfg->mode) & GPIO_MODE_MASK_CHECK)){
            status = GPIO_WRONG_MODE;
        }else{
            /* Validate port parameter (must be 0-5, check against mask) */
            if(GPIO_cfg->port > GPIO_PORT_MASK_CHECK){
                status = GPIO_WRONG_PORT;
            }else{
                /* Validate pin parameter (must be 0-15, so bits 4-31 must be clear) */
                if(0 != ((GPIO_cfg->pin) & GPIO_PIN_MASK_CHECK)){
                    status = GPIO_WRONG_PIN;
                }else{
                    /* Validate output type parameter (must be 0-1, so bits 1-31 must be clear) */
                    if(0 != ((GPIO_cfg->outputType) & GPIO_OUTPUTTYPE_MASK_CHECK)){
                        status = GPIO_WRONG_OUTPUT_TYPE;
                    }else{
                        /* Validate pull resistor parameter (must be 0-2) */
                        if(GPIO_cfg->pull > GPIO_PULL_MASK_CHECK){
                            status = GPIO_WRONG_PULL;
                        }else{
                            /* Validate alternate function parameter (must be 0-15, so bits 4-31 must be clear) */
                            if(0 != ((GPIO_cfg->alternateFunction) & GPIO_ALTERNATIVE_MASK_CHECK)){
                                status = GPIO_WRONG_ALTARNATIVE;
                            }else{
                                /* Validate speed parameter (must be 0-3, so bits 2-31 must be clear) */
                                if(0 != ((GPIO_cfg->speed) & GPIO_SPEED_MASK_CHECK)){
                                    status = GPIO_WRONG_SPEED;
                                }else{
                                    /* All parameters are valid, proceed with GPIO configuration */
                                    
                                    /* Configure MODER register: Set pin mode (2 bits per pin, shift by pin*2) */
                                    ((GPIO_Registers_t *)(GPIO_Base_Addreses[GPIO_cfg->port]))->MODER.ALL_FIELDS    |= (GPIO_cfg->mode)       << ((GPIO_cfg->pin) << 1);
                                    
                                    /* Configure OTYPER register: Set output type (1 bit per pin) */
                                    ((GPIO_Registers_t *)(GPIO_Base_Addreses[GPIO_cfg->port]))->OTYPER.ALL_FIELDS   |= (GPIO_cfg->outputType) << (GPIO_cfg->pin);
                                    
                                    /* Configure PUPDR register: Set pull-up/pull-down (2 bits per pin, shift by pin*2) */
                                    ((GPIO_Registers_t *)(GPIO_Base_Addreses[GPIO_cfg->port]))->PUPDR.ALL_FIELDS    |= (GPIO_cfg->pull)       << ((GPIO_cfg->pin) << 1);
                                    
                                    /* Configure OSPEEDR register: Set output speed (2 bits per pin, shift by pin*2) */
                                    ((GPIO_Registers_t *)(GPIO_Base_Addreses[GPIO_cfg->port]))->OSPEEDR.ALL_FIELDS  |= (GPIO_cfg->speed)      << ((GPIO_cfg->pin) << 1);

                                    /* Check if pin is in lower range (0-7) or higher range (8-15) for alternate function */
                                    if(0 == ((GPIO_cfg->pin) & GPIO_ALTERNATIVE_HIGH_CHECK)){                          
                                        /* Pin 0-7: Use AFRL register (4 bits per pin, shift by pin*4) */
                                        ((GPIO_Registers_t *)(GPIO_Base_Addreses[GPIO_cfg->port]))->AFRL.ALL_FIELDS  |= (GPIO_cfg->alternateFunction) << ((GPIO_cfg->pin) << 2);
                                    }else{
                                        /* Pin 8-15: Use AFRH register (mask lower 3 bits to get position 0-7, shift by position*4) */
                                        ((GPIO_Registers_t *)(GPIO_Base_Addreses[GPIO_cfg->port]))->AFRH.ALL_FIELDS  |= (GPIO_cfg->alternateFunction) << ((GPIO_ALTERNATIVE_THREE_BITS&(GPIO_cfg->pin)) << 2);
                                    }
                                    
                                    /* Configuration successful */
                                    status = GPIO_OK;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    /* Return status of initialization */
    return status;
}


/******************************************************************************
 * @brief Set GPIO pin to HIGH or LOW
 * @details This function sets the output value of a GPIO pin using the BSRR
 *          register for atomic operation (no read-modify-write required)
 * 
 * @param[in] port GPIO port (GPIO_PORT_A to GPIO_PORT_H)
 * @param[in] pin  GPIO pin number (GPIO_PIN_0 to GPIO_PIN_15)
 * @param[in] val  Pin value (GPIO_HIGH or GPIO_LOW)
 * 
 * @return GPIO_Status_t Status of the operation
 * @retval GPIO_OK          Operation successful
 * @retval GPIO_WRONG_PORT  Invalid port
 * @retval GPIO_WRONG_PIN   Invalid pin
 * @retval GPIO_WRONG_VALUE Invalid value
 * 
 * @note GPIO_HIGH = 0 (sets bit in BSRR[0-15])
 *       GPIO_LOW = 16 (sets bit in BSRR[16-31] which resets the pin)
 * @note This function uses BSRR register for atomic bit set/reset operation
 * @author Eng.Gemy
 ******************************************************************************/
GPIO_Status_t GPIO_enuSetPinVal(GPIO_Port_t port,GPIO_Pin_t pin,GPIO_Val_t val){
    
    /* Local variable to hold function return status */
    GPIO_Status_t status = GPIO_NOT_OK;

    /* Validate port parameter */
    if(port > GPIO_PORT_MASK_CHECK){
        status = GPIO_WRONG_PORT;
    }else{
        /* Validate pin parameter (bits 4-31 must be clear) */
        if(0 != ((pin) & GPIO_PIN_MASK_CHECK)){
            status = GPIO_WRONG_PIN;
        }else{
            /* Validate value parameter (must be GPIO_HIGH=0 or GPIO_LOW=16) */
            if((GPIO_HIGH != val) && (GPIO_LOW != val)){
                status = GPIO_WRONG_VALUE;
            }else{
                /* Set/Reset pin using BSRR register
                 * If val=GPIO_HIGH (0): (1<<pin)<<0 sets BS[pin] -> sets pin HIGH
                 * If val=GPIO_LOW (16): (1<<pin)<<16 sets BR[pin] -> resets pin LOW
                 */
                ((GPIO_Registers_t *)(GPIO_Base_Addreses[port]))->BSRR.ALL_FIELDS  |= ((1 << pin) << val);
                status = GPIO_OK;
            }
        }
    }

    /* Return status of operation */
    return status;
}

/******************************************************************************
 * @brief Toggle GPIO pin value (HIGH to LOW or LOW to HIGH)
 * @details This function flips the current output state of a GPIO pin by
 *          using XOR operation on the ODR register
 * 
 * @param[in] port GPIO port (GPIO_PORT_A to GPIO_PORT_H)
 * @param[in] pin  GPIO pin number (GPIO_PIN_0 to GPIO_PIN_15)
 * 
 * @return GPIO_Status_t Status of the operation
 * @retval GPIO_OK          Operation successful
 * @retval GPIO_WRONG_PORT  Invalid port
 * @retval GPIO_WRONG_PIN   Invalid pin
 * 
 * @note This function uses XOR operation on ODR register to toggle pin state
 * @warning This is not an atomic operation (read-modify-write cycle)
 * @author Eng.Gemy
 ******************************************************************************/
GPIO_Status_t GPIO_enuFlipPinVal(GPIO_Port_t port,GPIO_Pin_t pin){
    
    /* Local variable to hold function return status */
    GPIO_Status_t status = GPIO_NOT_OK;

    /* Validate port parameter */
    if(port > GPIO_PORT_MASK_CHECK){
        status = GPIO_WRONG_PORT;
    }else{
        /* Validate pin parameter (bits 4-31 must be clear) */
        if(0 != ((pin) & GPIO_PIN_MASK_CHECK)){
            status = GPIO_WRONG_PIN;
        }else{
            /* Toggle pin using XOR operation on ODR register
             * XOR with 1 flips the bit: 0^1=1, 1^1=0
             */  
            ((GPIO_Registers_t *)(GPIO_Base_Addreses[port]))->ODR.ALL_FIELDS  ^= (1 << pin);
        }
    }

    /* Return status of operation */
    return status;
}

/******************************************************************************
 * @brief Read current value of GPIO pin
 * @details This function reads the current input state of a GPIO pin from
 *          the IDR register and stores it in the provided pointer
 * 
 * @param[in]  port GPIO port (GPIO_PORT_A to GPIO_PORT_H)
 * @param[in]  pin  GPIO pin number (GPIO_PIN_0 to GPIO_PIN_15)
 * @param[out] val  Pointer to store the read value (0 or 1)
 * 
 * @return GPIO_Status_t Status of the operation
 * @retval GPIO_OK          Operation successful
 * @retval GPIO_WRONG_PORT  Invalid port
 * @retval GPIO_WRONG_PIN   Invalid pin
 * @retval GPIO_NULL_PTR    Null pointer passed for val parameter
 * 
 * @note The function reads from IDR register which reflects the actual
 *       pin state (input or output)
 * @author Eng.Gemy
 ******************************************************************************/
GPIO_Status_t GPIO_enuReadPinVal(GPIO_Port_t port,GPIO_Pin_t pin , uint8_t *val){
    
    /* Local variable to hold function return status */
    GPIO_Status_t status = GPIO_NOT_OK;

    /* Validate port parameter */
    if(port > GPIO_PORT_MASK_CHECK){
        status = GPIO_WRONG_PORT;
    }else{
        /* Validate pin parameter (bits 4-31 must be clear) */
        if(0 != ((pin) & GPIO_PIN_MASK_CHECK)){
            status = GPIO_WRONG_PIN;
        }else{
            /* Check if value pointer is NULL */
            if(NULL == val){
                status = GPIO_NULL_PTR;
            
            }else{
                /* Clear the output variable first */
                *val = 0;
                
                /* Read pin value from IDR register
                 * 1. Read IDR register
                 * 2. Shift right by pin position to move target bit to LSB
                 * 3. AND with 1 to extract only the LSB (pin value)
                 */
                *val = ((((GPIO_Registers_t *)(GPIO_Base_Addreses[port]))->IDR.ALL_FIELDS) >> pin)& 1 ;
                status = GPIO_OK;
            }
        }
    }

    /* Return status of operation */
    return status;
}


/******************************************************************************
 * @brief Set GPIO pin mode (Input/Output/Alternate Function/Analog)
 * @details This function configures the mode of a GPIO pin by writing to
 *          the MODER register
 * 
 * @param[in] port GPIO port (GPIO_PORT_A to GPIO_PORT_H)
 * @param[in] pin  GPIO pin number (GPIO_PIN_0 to GPIO_PIN_15)
 * @param[in] mode Pin mode (GPIO_MODE_INPUT/OUTPUT/ALTERNATE_FUNCTION/ANALOG)
 * 
 * @return GPIO_Status_t Status of the operation
 * @retval GPIO_OK          Operation successful
 * @retval GPIO_WRONG_PORT  Invalid port
 * @retval GPIO_WRONG_PIN   Invalid pin
 * @retval GPIO_WRONG_MODE  Invalid mode
 * 
 * @note Each pin uses 2 bits in MODER register, so shift amount is (pin * 2)
 * @author Eng.Gemy
 ******************************************************************************/
GPIO_Status_t GPIO_enuSetPinMode(GPIO_Port_t port,GPIO_Pin_t pin,GPIO_Mode_t mode){
    
    /* Local variable to hold function return status */
    GPIO_Status_t status = GPIO_NOT_OK;

    /* Validate port parameter */
    if(port > GPIO_PORT_MASK_CHECK){
        status = GPIO_WRONG_PORT;
    }else{
        /* Validate pin parameter (bits 4-31 must be clear) */
        if(0 != ((pin) & GPIO_PIN_MASK_CHECK)){
            status = GPIO_WRONG_PIN;
        }else{
            /* Validate mode parameter (bits 2-31 must be clear) */
            if(0 != ((mode) & GPIO_MODE_MASK_CHECK)){
                status = GPIO_WRONG_MODE;
            }else{
                /* Configure MODER register
                 * Each pin uses 2 bits, so multiply pin by 2 (pin << 1)
                 * OR operation sets the mode bits
                 */                                    
                ((GPIO_Registers_t *)(GPIO_Base_Addreses[port]))->MODER.ALL_FIELDS    |= mode << (pin << 1);
                status = GPIO_OK;
            } 
        }
    }

    /* Return status of operation */
    return status;
}

/******************************************************************************
 * @brief Set alternate function for GPIO pin
 * @details This function configures the alternate function mapping for a GPIO
 *          pin. Pins 0-7 use AFRL register, pins 8-15 use AFRH register
 * 
 * @param[in] port               GPIO port (GPIO_PORT_A to GPIO_PORT_H)
 * @param[in] pin                GPIO pin number (GPIO_PIN_0 to GPIO_PIN_15)
 * @param[in] alternateFunction  Alternate function (GPIO_AF0 to GPIO_AF15)
 * 
 * @return GPIO_Status_t Status of the operation
 * @retval GPIO_OK                  Operation successful
 * @retval GPIO_WRONG_PORT          Invalid port
 * @retval GPIO_WRONG_PIN           Invalid pin
 * @retval GPIO_WRONG_ALTARNATIVE   Invalid alternate function
 * 
 * @note Each pin uses 4 bits in AFR registers, so shift amount is (pin * 4)
 * @note Pins 0-7 use AFRL, pins 8-15 use AFRH
 * @author Eng.Gemy
 ******************************************************************************/
GPIO_Status_t GPIO_enuSetAltFunc(GPIO_Port_t port,GPIO_Pin_t pin,GPIO_AlternateFunction_t alternateFunction){
    
    /* Local variable to hold function return status */
    GPIO_Status_t status = GPIO_NOT_OK;

    /* Validate port parameter */
    if(port > GPIO_PORT_MASK_CHECK){
        status = GPIO_WRONG_PORT;
    }else{
        /* Validate pin parameter (bits 4-31 must be clear) */
        if(0 != ((pin) & GPIO_PIN_MASK_CHECK)){
            status = GPIO_WRONG_PIN;
        }else{
            /* Validate alternate function parameter (bits 4-31 must be clear) */
            if(0 != (alternateFunction & GPIO_ALTERNATIVE_MASK_CHECK)){
                status = GPIO_WRONG_ALTARNATIVE;
            }else{
                /* Check which AFR register to use based on pin number */                                    
                
                /* Check bit 3 of pin number (0 for pins 0-7, 1 for pins 8-15) */
                if(0 == (pin & GPIO_ALTERNATIVE_HIGH_CHECK)){                          
                    /* Pins 0-7: Use AFRL register
                     * Each pin uses 4 bits, so multiply pin by 4 (pin << 2)
                     */
                    ((GPIO_Registers_t *)(GPIO_Base_Addreses[port]))->AFRL.ALL_FIELDS  |= (alternateFunction) << (pin << 2);
                }else{
                                       /* Pins 8-15: Use AFRH register
                     * Mask lower 3 bits of pin to get position 0-7 within AFRH
                     * Then multiply by 4 (position << 2)
                     */
                    ((GPIO_Registers_t *)(GPIO_Base_Addreses[port]))->AFRH.ALL_FIELDS  |= (alternateFunction) << ((GPIO_ALTERNATIVE_THREE_BITS&pin) << 2);
                }
                status = GPIO_OK;
            } 
        }
    }

    /* Return status of operation */
    return status;
}

/******************************************************************************
 * @brief Set GPIO pin output type (Push-Pull/Open-Drain)
 * @details This function configures the output type of a GPIO pin by writing
 *          to the OTYPER register
 * 
 * @param[in] port       GPIO port (GPIO_PORT_A to GPIO_PORT_H)
 * @param[in] pin        GPIO pin number (GPIO_PIN_0 to GPIO_PIN_15)
 * @param[in] outputType Output type (GPIO_OUTPUT_TYPE_PUSH_PULL or GPIO_OUTPUT_TYPE_OPEN_DRAIN)
 * 
 * @return GPIO_Status_t Status of the operation
 * @retval GPIO_OK                  Operation successful
 * @retval GPIO_WRONG_PORT          Invalid port
 * @retval GPIO_WRONG_PIN           Invalid pin
 * @retval GPIO_WRONG_OUTPUT_TYPE   Invalid output type
 * 
 * @note Each pin uses 1 bit in OTYPER register
 * @note Push-pull can drive both HIGH and LOW, open-drain can only pull LOW
 * @author Eng.Gemy
 ******************************************************************************/
GPIO_Status_t GPIO_enuSetPinOutType(GPIO_Port_t port,GPIO_Pin_t pin,GPIO_OutputType_t outputType){

    /* Local variable to hold function return status */
    GPIO_Status_t status = GPIO_NOT_OK;

    /* Validate port parameter */
    if(port > GPIO_PORT_MASK_CHECK){
        status = GPIO_WRONG_PORT;
    }else{
        /* Validate pin parameter (bits 4-31 must be clear) */
        if(0 != ((pin) & GPIO_PIN_MASK_CHECK)){
            status = GPIO_WRONG_PIN;
        }else{
            /* Validate output type parameter (bits 1-31 must be clear) */
            if(0 != (outputType & GPIO_OUTPUTTYPE_MASK_CHECK)){
                    
                status = GPIO_WRONG_OUTPUT_TYPE;                                  
            }else{
                /* Configure OTYPER register
                 * Each pin uses 1 bit, so shift by pin number directly
                 * 0: Push-pull (reset state)
                 * 1: Open-drain
                 */
                ((GPIO_Registers_t *)(GPIO_Base_Addreses[port]))->OTYPER.ALL_FIELDS   |= (outputType) << (pin);
                status = GPIO_OK;
            } 
        }
    }

    /* Return status of operation */
    return status;
}

/******************************************************************************
 * @brief Set GPIO pin pull-up/pull-down configuration
 * @details This function configures the internal pull resistors for a GPIO pin
 *          by writing to the PUPDR register
 * 
 * @param[in] port GPIO port (GPIO_PORT_A to GPIO_PORT_H)
 * @param[in] pin  GPIO pin number (GPIO_PIN_0 to GPIO_PIN_15)
 * @param[in] pull Pull configuration (GPIO_NO_PULL/GPIO_PULL_UP/GPIO_PULL_DOWN)
 * 
 * @return GPIO_Status_t Status of the operation
 * @retval GPIO_OK          Operation successful
 * @retval GPIO_WRONG_PORT  Invalid port
 * @retval GPIO_WRONG_PIN   Invalid pin
 * @retval GPIO_WRONG_PULL  Invalid pull configuration
 * 
 * @note Each pin uses 2 bits in PUPDR register, so shift amount is (pin * 2)
 * @note Pull resistors are useful for inputs to avoid floating states
 * @author Eng.Gemy
 ******************************************************************************/
GPIO_Status_t GPIO_enuSetPinPull(GPIO_Port_t port,GPIO_Pin_t pin,GPIO_Pull_t pull){
    
    /* Local variable to hold function return status */
    GPIO_Status_t status = GPIO_NOT_OK;

    /* Validate port parameter */
    if(port > GPIO_PORT_MASK_CHECK){
        status = GPIO_WRONG_PORT;
    }else{
        /* Validate pin parameter (bits 4-31 must be clear) */
        if(0 != ((pin) & GPIO_PIN_MASK_CHECK)){
            status = GPIO_WRONG_PIN;
        }else{
            /* Validate pull parameter (must be 0, 1, or 2) */
            if(pull > GPIO_PULL_MASK_CHECK){            
                status = GPIO_WRONG_PULL;                                  
            }else{
                /* Configure PUPDR register
                 * Each pin uses 2 bits, so multiply pin by 2 (pin << 1)
                 * 00: No pull-up, no pull-down
                 * 01: Pull-up
                 * 10: Pull-down
                 * 11: Reserved
                 */                    
                ((GPIO_Registers_t *)(GPIO_Base_Addreses[port]))->PUPDR.ALL_FIELDS    |= (pull) << (pin << 1);
                status = GPIO_OK;
            } 
        }
    }

    /* Return status of operation */
    return status; 
}

/******************************************************************************
 * @brief Set GPIO pin output speed
 * @details This function configures the output speed (slew rate) of a GPIO pin
 *          by writing to the OSPEEDR register
 * 
 * @param[in] port  GPIO port (GPIO_PORT_A to GPIO_PORT_H)
 * @param[in] pin   GPIO pin number (GPIO_PIN_0 to GPIO_PIN_15)
 * @param[in] speed Output speed (GPIO_SPEED_LOW/MEDIUM/HIGH/VERY_HIGH)
 * 
 * @return GPIO_Status_t Status of the operation
 * @retval GPIO_OK          Operation successful
 * @retval GPIO_WRONG_PORT  Invalid port
 * @retval GPIO_WRONG_PIN   Invalid pin
 * @retval GPIO_WRONG_SPEED Invalid speed
 * 
 * @note Each pin uses 2 bits in OSPEEDR register, so shift amount is (pin * 2)
 * @note Higher speed increases EMI but allows faster signal transitions
 * @note Speed options:
 *       - Low: up to 8 MHz
 *       - Medium: up to 50 MHz
 *       - High: up to 100 MHz
 *       - Very High: up to 180 MHz
 * @author Eng.Gemy
 ******************************************************************************/
GPIO_Status_t GPIO_enuSetPinSpeed(GPIO_Port_t port,GPIO_Pin_t pin,GPIO_Speed_t speed){
    
    /* Local variable to hold function return status */
    GPIO_Status_t status = GPIO_NOT_OK;

    /* Validate port parameter */
    if(port > GPIO_PORT_MASK_CHECK){
        status = GPIO_WRONG_PORT;
    }else{
        /* Validate pin parameter (bits 4-31 must be clear) */
        if(0 != ((pin) & GPIO_PIN_MASK_CHECK)){
            status = GPIO_WRONG_PIN;
        }else{
            /* Validate speed parameter (bits 2-31 must be clear) */
             if(0 != ((speed) & GPIO_SPEED_MASK_CHECK)){                    
                status = GPIO_WRONG_SPEED;                                  
            }else{
                /* Configure OSPEEDR register
                 * Each pin uses 2 bits, so multiply pin by 2 (pin << 1)
                 * 00: Low speed
                 * 01: Medium speed
                 * 10: High speed
                 * 11: Very high speed
                 */                                       
                ((GPIO_Registers_t *)(GPIO_Base_Addreses[port]))->OSPEEDR.ALL_FIELDS  |= (speed) << (pin << 1);
                status = GPIO_OK;
            } 
        }
    }

    /* Return status of operation */
    return status; 
}

/******************************************************************************
 *                           END OF FILE
 * @author Eng.Gemy
 * 
 * @note Implementation Summary:
 *       - All functions perform comprehensive parameter validation
 *       - GPIO registers are accessed through base address array
 *       - Bit manipulation is used for efficient register access
 *       - BSRR register is used for atomic set/reset operations
 *       - Each function returns detailed status codes for error handling
 *       
 * @warning Always ensure GPIO clock is enabled before using these functions
 * 
 * @example Basic Usage:
 *     GPIO_cfg_t led_cfg = {
 *         .port = GPIO_PORT_A,
 *         .pin = GPIO_PIN_5,
 *         .mode = GPIO_MODE_OUTPUT,
 *         .outputType = GPIO_OUTPUT_TYPE_PUSH_PULL,
 *         .speed = GPIO_SPEED_LOW,
 *         .pull = GPIO_NO_PULL
 *     };
 *     
 *     // Initialize GPIO
 *     GPIO_enuInit(&led_cfg);
 *     
 *     // Set pin HIGH
 *     GPIO_enuSetPinVal(GPIO_PORT_A, GPIO_PIN_5, GPIO_HIGH);
 *     
 *     // Toggle pin
 *     GPIO_enuFlipPinVal(GPIO_PORT_A, GPIO_PIN_5);
 *     
 *     // Read pin value
 *     uint8_t value;
 *     GPIO_enuReadPinVal(GPIO_PORT_A, GPIO_PIN_5, &value);
 ******************************************************************************/