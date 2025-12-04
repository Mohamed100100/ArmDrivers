/******************************************************************************
 * @file    GPIO_PRIV.H
 * @author  Eng.Gemy
 * @brief   GPIO Driver Private Header File
 *          This file contains the hardware register definitions, base addresses,
 *          and internal structures for the GPIO peripheral
 * @note    This file should NOT be included by application code
 ******************************************************************************/

#ifndef GPIO_PRIV_H
#define GPIO_PRIV_H


#include "./LIB/stdtypes.h"

/******************************************************************************
 *                        GPIO BASE ADDRESSES
 * @brief Memory-mapped base addresses for GPIO ports on STM32
 * @author Eng.Gemy
 ******************************************************************************/
#define GPIOA_BASE_ADDR 0x40020000  /**< GPIO Port A base address in AHB1 bus */
#define GPIOB_BASE_ADDR 0x40020400  /**< GPIO Port B base address in AHB1 bus */
#define GPIOC_BASE_ADDR 0x40020800  /**< GPIO Port C base address in AHB1 bus */
#define GPIOD_BASE_ADDR 0x40020C00  /**< GPIO Port D base address in AHB1 bus */
#define GPIOE_BASE_ADDR 0x40021000  /**< GPIO Port E base address in AHB1 bus */
#define GPIOH_BASE_ADDR 0x40021C00  /**< GPIO Port H base address in AHB1 bus */

/******************************************************************************
 *                        VALIDATION MASKS
 * @brief Bit masks used for input parameter validation
 * @details These masks help verify that input parameters are within valid ranges
 * @author Eng.Gemy
 ******************************************************************************/
//                                   0b10987654321098765432109876543210
#define GPIO_MODE_MASK_CHECK        (0b11111111111111111111111111111100UL)  /**< Mode validation mask - checks bits 2-31 */
#define GPIO_OUTPUTTYPE_MASK_CHECK  (0b11111111111111111111111111111110UL)  /**< Output type validation mask - checks bits 1-31 */
#define GPIO_SPEED_MASK_CHECK       (0b11111111111111111111111111111100UL)  /**< Speed validation mask - checks bits 2-31 */
#define GPIO_PULL_MASK_CHECK        (3UL)                                   /**< Pull resistor validation mask */
#define GPIO_PORT_MASK_CHECK        (6UL)                                   /**< Port validation mask */
#define GPIO_PIN_MASK_CHECK         (0b11111111111111111111111111110000UL)  /**< Pin validation mask - checks bits 4-31 */
#define GPIO_ALTERNATIVE_MASK_CHECK (0b11111111111111111111111111110000UL)  /**< Alternate function validation mask - checks bits 4-31 */
#define GPIO_ALTERNATIVE_HIGH_CHECK (0b00000000000000000000000000001000UL)  /**< AF high register check - bit 3 determines AFRL/AFRH */
#define GPIO_ALTERNATIVE_THREE_BITS (0b00000000000000000000000000000111UL)  /**< AF lower 3 bits mask - position within AFR register */





/******************************************************************************
 *                   GPIO MODE REGISTER (MODER) STRUCTURES
 * @brief Structures for GPIO port mode register access
 * @details Each pin has 2 bits to configure mode (Input/Output/AF/Analog)
 * @author Eng.Gemy
 ******************************************************************************/
typedef struct{
    volatile uint32_t MODER0  : 2;  /**< Mode bits for pin 0 */
    volatile uint32_t MODER1  : 2;  /**< Mode bits for pin 1 */
    volatile uint32_t MODER2  : 2;  /**< Mode bits for pin 2 */
    volatile uint32_t MODER3  : 2;  /**< Mode bits for pin 3 */
    volatile uint32_t MODER4  : 2;  /**< Mode bits for pin 4 */
    volatile uint32_t MODER5  : 2;  /**< Mode bits for pin 5 */
    volatile uint32_t MODER6  : 2;  /**< Mode bits for pin 6 */
    volatile uint32_t MODER7  : 2;  /**< Mode bits for pin 7 */
    volatile uint32_t MODER8  : 2;  /**< Mode bits for pin 8 */
    volatile uint32_t MODER9  : 2;  /**< Mode bits for pin 9 */
    volatile uint32_t MODER10 : 2;  /**< Mode bits for pin 10 */
    volatile uint32_t MODER11 : 2;  /**< Mode bits for pin 11 */
    volatile uint32_t MODER12 : 2;  /**< Mode bits for pin 12 */
    volatile uint32_t MODER13 : 2;  /**< Mode bits for pin 13 */
    volatile uint32_t MODER14 : 2;  /**< Mode bits for pin 14 */
    volatile uint32_t MODER15 : 2;  /**< Mode bits for pin 15 */
}GPIO_MODER_t;

/**
 * @brief Union for flexible MODER register access
 * @author Eng.Gemy
 */
typedef union 
{     
    GPIO_MODER_t BIT_FIELDS;        /**< Access individual pin mode bits */
    volatile uint32_t     ALL_FIELDS;   /**< Access entire register as 32-bit word */
}GPIO_MODER_U_t;    


/******************************************************************************
 *                   GPIO OUTPUT TYPE REGISTER (OTYPER) STRUCTURES
 * @brief Structures for GPIO port output type register access
 * @details Each pin has 1 bit to configure output type (Push-Pull/Open-Drain)
 * @author Eng.Gemy
 ******************************************************************************/
typedef struct{ 
    volatile uint32_t OT0        : 1;   /**< Output type for pin 0 */
    volatile uint32_t OT1        : 1;   /**< Output type for pin 1 */
    volatile uint32_t OT2        : 1;   /**< Output type for pin 2 */
    volatile uint32_t OT3        : 1;   /**< Output type for pin 3 */
    volatile uint32_t OT4        : 1;   /**< Output type for pin 4 */
    volatile uint32_t OT5        : 1;   /**< Output type for pin 5 */
    volatile uint32_t OT6        : 1;   /**< Output type for pin 6 */
    volatile uint32_t OT7        : 1;   /**< Output type for pin 7 */
    volatile uint32_t OT8        : 1;   /**< Output type for pin 8 */
    volatile uint32_t OT9        : 1;   /**< Output type for pin 9 */
    volatile uint32_t OT10       : 1;   /**< Output type for pin 10 */
    volatile uint32_t OT11       : 1;   /**< Output type for pin 11 */
    volatile uint32_t OT12       : 1;   /**< Output type for pin 12 */
    volatile uint32_t OT13       : 1;   /**< Output type for pin 13 */
    volatile uint32_t OT14       : 1;   /**< Output type for pin 14 */
    volatile uint32_t OT15       : 1;   /**< Output type for pin 15 */
    volatile uint32_t reserved   :16;   /**< Reserved bits (16-31) */
}GPIO_OTYPER_t;

/**
 * @brief Union for flexible OTYPER register access
 * @author Eng.Gemy
 */
typedef union 
{     
    GPIO_OTYPER_t BIT_FIELDS;       /**< Access individual pin output type bits */
    volatile uint32_t     ALL_FIELDS;   /**< Access entire register as 32-bit word */
}GPIO_OTYPER_U_t;

/******************************************************************************
 *                   GPIO OUTPUT SPEED REGISTER (OSPEEDR) STRUCTURES
 * @brief Structures for GPIO port output speed register access
 * @details Each pin has 2 bits to configure output speed (Low/Medium/High/Very High)
 * @author Eng.Gemy
 ******************************************************************************/
typedef struct{ 
    volatile uint32_t OSPEEDR0  : 2;    /**< Output speed for pin 0 */
    volatile uint32_t OSPEEDR1  : 2;    /**< Output speed for pin 1 */
    volatile uint32_t OSPEEDR2  : 2;    /**< Output speed for pin 2 */
    volatile uint32_t OSPEEDR3  : 2;    /**< Output speed for pin 3 */
    volatile uint32_t OSPEEDR4  : 2;    /**< Output speed for pin 4 */
    volatile uint32_t OSPEEDR5  : 2;    /**< Output speed for pin 5 */
    volatile uint32_t OSPEEDR6  : 2;    /**< Output speed for pin 6 */
    volatile uint32_t OSPEEDR7  : 2;    /**< Output speed for pin 7 */
    volatile uint32_t OSPEEDR8  : 2;    /**< Output speed for pin 8 */
    volatile uint32_t OSPEEDR9  : 2;    /**< Output speed for pin 9 */
    volatile uint32_t OSPEEDR10 : 2;    /**< Output speed for pin 10 */
    volatile uint32_t OSPEEDR11 : 2;    /**< Output speed for pin 11 */
    volatile uint32_t OSPEEDR12 : 2;    /**< Output speed for pin 12 */
    volatile uint32_t OSPEEDR13 : 2;    /**< Output speed for pin 13 */
    volatile uint32_t OSPEEDR14 : 2;    /**< Output speed for pin 14 */
    volatile uint32_t OSPEEDR15 : 2;    /**< Output speed for pin 15 */
}GPIO_OSPEEDR_t;

/**
 * @brief Union for flexible OSPEEDR register access
 * @author Eng.Gemy
 */
typedef union 
{     
    GPIO_OSPEEDR_t        BIT_FIELDS;   /**< Access individual pin speed bits */
    volatile uint32_t     ALL_FIELDS;   /**< Access entire register as 32-bit word */
}GPIO_OSPEEDR_U_t;

/******************************************************************************
 *                   GPIO PULL-UP/PULL-DOWN REGISTER (PUPDR) STRUCTURES
 * @brief Structures for GPIO port pull-up/pull-down register access
 * @details Each pin has 2 bits to configure pull resistors (None/Pull-Up/Pull-Down)
 * @author Eng.Gemy
 ******************************************************************************/
typedef struct{ 
    volatile uint32_t PUPDR0  : 2;  /**< Pull-up/pull-down for pin 0 */
    volatile uint32_t PUPDR1  : 2;  /**< Pull-up/pull-down for pin 1 */
    volatile uint32_t PUPDR2  : 2;  /**< Pull-up/pull-down for pin 2 */
    volatile uint32_t PUPDR3  : 2;  /**< Pull-up/pull-down for pin 3 */
    volatile uint32_t PUPDR4  : 2;  /**< Pull-up/pull-down for pin 4 */
    volatile uint32_t PUPDR5  : 2;  /**< Pull-up/pull-down for pin 5 */
    volatile uint32_t PUPDR6  : 2;  /**< Pull-up/pull-down for pin 6 */
    volatile uint32_t PUPDR7  : 2;  /**< Pull-up/pull-down for pin 7 */
    volatile uint32_t PUPDR8  : 2;  /**< Pull-up/pull-down for pin 8 */
    volatile uint32_t PUPDR9  : 2;  /**< Pull-up/pull-down for pin 9 */
    volatile uint32_t PUPDR10 : 2;  /**< Pull-up/pull-down for pin 10 */
    volatile uint32_t PUPDR11 : 2;  /**< Pull-up/pull-down for pin 11 */
    volatile uint32_t PUPDR12 : 2;  /**< Pull-up/pull-down for pin 12 */
    volatile uint32_t PUPDR13 : 2;  /**< Pull-up/pull-down for pin 13 */
    volatile uint32_t PUPDR14 : 2;  /**< Pull-up/pull-down for pin 14 */
    volatile uint32_t PUPDR15 : 2;  /**< Pull-up/pull-down for pin 15 */
}GPIO_PUPDR_t;

/**
 * @brief Union for flexible PUPDR register access
 * @author Eng.Gemy
 */
typedef union 
{     
    GPIO_PUPDR_t        BIT_FIELDS;     /**< Access individual pin pull configuration bits */
    volatile uint32_t     ALL_FIELDS;   /**< Access entire register as 32-bit word */
}GPIO_PUPDR_U_t;

/******************************************************************************
 *                   GPIO INPUT DATA REGISTER (IDR) STRUCTURES
 * @brief Structures for GPIO port input data register access
 * @details Each pin has 1 bit representing the input value (Read-Only)
 * @author Eng.Gemy
 ******************************************************************************/
typedef struct{
    volatile uint32_t IDR0  : 1;    /**< Input data for pin 0 */
    volatile uint32_t IDR1  : 1;    /**< Input data for pin 1 */
    volatile uint32_t IDR2  : 1;    /**< Input data for pin 2 */
    volatile uint32_t IDR3  : 1;    /**< Input data for pin 3 */
    volatile uint32_t IDR4  : 1;    /**< Input data for pin 4 */
    volatile uint32_t IDR5  : 1;    /**< Input data for pin 5 */
    volatile uint32_t IDR6  : 1;    /**< Input data for pin 6 */
    volatile uint32_t IDR7  : 1;    /**< Input data for pin 7 */
    volatile uint32_t IDR8  : 1;    /**< Input data for pin 8 */
    volatile uint32_t IDR9  : 1;    /**< Input data for pin 9 */
    volatile uint32_t IDR10 : 1;    /**< Input data for pin 10 */
    volatile uint32_t IDR11 : 1;    /**< Input data for pin 11 */
    volatile uint32_t IDR12 : 1;    /**< Input data for pin 12 */
    volatile uint32_t IDR13 : 1;    /**< Input data for pin 13 */
    volatile uint32_t IDR14 : 1;    /**< Input data for pin 14 */
    volatile uint32_t IDR15 : 1;    /**< Input data for pin 15 */
    volatile uint32_t reserved :16; /**< Reserved bits (16-31) */
}GPIO_IDR_t;

/**
 * @brief Union for flexible IDR register access
 * @author Eng.Gemy
 */
typedef union 
{     
    GPIO_IDR_t            BIT_FIELDS;   /**< Access individual pin input bits */
    volatile uint32_t     ALL_FIELDS;   /**< Access entire register as 32-bit word */
}GPIO_IDR_U_t;

/******************************************************************************
 *                   GPIO OUTPUT DATA REGISTER (ODR) STRUCTURES
 * @brief Structures for GPIO port output data register access
 * @details Each pin has 1 bit representing the output value
 * @author Eng.Gemy
 ******************************************************************************/
typedef struct{
    volatile uint32_t ODR0  : 1;    /**< Output data for pin 0 */
    volatile uint32_t ODR1  : 1;    /**< Output data for pin 1 */
    volatile uint32_t ODR2  : 1;    /**< Output data for pin 2 */
    volatile uint32_t ODR3  : 1;    /**< Output data for pin 3 */
    volatile uint32_t ODR4  : 1;    /**< Output data for pin 4 */
    volatile uint32_t ODR5  : 1;    /**< Output data for pin 5 */
    volatile uint32_t ODR6  : 1;    /**< Output data for pin 6 */
    volatile uint32_t ODR7  : 1;    /**< Output data for pin 7 */
    volatile uint32_t ODR8  : 1;    /**< Output data for pin 8 */
    volatile uint32_t ODR9  : 1;    /**< Output data for pin 9 */
    volatile uint32_t ODR10 : 1;    /**< Output data for pin 10 */
    volatile uint32_t ODR11 : 1;    /**< Output data for pin 11 */
    volatile uint32_t ODR12 : 1;    /**< Output data for pin 12 */
    volatile uint32_t ODR13 : 1;    /**< Output data for pin 13 */
    volatile uint32_t ODR14 : 1;    /**< Output data for pin 14 */
    volatile uint32_t ODR15 : 1;    /**< Output data for pin 15 */
    volatile uint32_t reserved :16; /**< Reserved bits (16-31) */
}GPIO_ODR_t;

/**
 * @brief Union for flexible ODR register access
 * @author Eng.Gemy
 */
typedef union 
{     
    GPIO_ODR_t        BIT_FIELDS;       /**< Access individual pin output bits */
    volatile uint32_t     ALL_FIELDS;   /**< Access entire register as 32-bit word */
}GPIO_ODR_U_t;

/******************************************************************************
 *                   GPIO BIT SET/RESET REGISTER (BSRR) STRUCTURES
 * @brief Structures for GPIO port bit set/reset register access
 * @details Atomic bit manipulation register (Write-Only)
 *          Bits 0-15: Set pins, Bits 16-31: Reset pins
 * @author Eng.Gemy
 ******************************************************************************/
typedef struct{
    volatile uint32_t BS0  : 1;     /**< Set pin 0 */
    volatile uint32_t BS1  : 1;     /**< Set pin 1 */
    volatile uint32_t BS2  : 1;     /**< Set pin 2 */
    volatile uint32_t BS3  : 1;     /**< Set pin 3 */
    volatile uint32_t BS4  : 1;     /**< Set pin 4 */
    volatile uint32_t BS5  : 1;     /**< Set pin 5 */
    volatile uint32_t BS6  : 1;     /**< Set pin 6 */
    volatile uint32_t BS7  : 1;     /**< Set pin 7 */
    volatile uint32_t BS8  : 1;     /**< Set pin 8 */
    volatile uint32_t BS9  : 1;     /**< Set pin 9 */
    volatile uint32_t BS10 : 1;     /**< Set pin 10 */
    volatile uint32_t BS11 : 1;     /**< Set pin 11 */
    volatile uint32_t BS12 : 1;     /**< Set pin 12 */
    volatile uint32_t BS13 : 1;     /**< Set pin 13 */
    volatile uint32_t BS14 : 1;     /**< Set pin 14 */
    volatile uint32_t BS15 : 1;     /**< Set pin 15 */
    volatile uint32_t BR0  : 1;     /**< Reset pin 0 */
    volatile uint32_t BR1  : 1;     /**< Reset pin 1 */
    volatile uint32_t BR2  : 1;     /**< Reset pin 2 */
    volatile uint32_t BR3  : 1;     /**< Reset pin 3 */
    volatile uint32_t BR4  : 1;     /**< Reset pin 4 */
    volatile uint32_t BR5  : 1;     /**< Reset pin 5 */
    volatile uint32_t BR6  : 1;     /**< Reset pin 6 */
    volatile uint32_t BR7  : 1;     /**< Reset pin 7 */
    volatile uint32_t BR8  : 1;     /**< Reset pin 8 */
    volatile uint32_t BR9  : 1;     /**< Reset pin 9 */
    volatile uint32_t BR10 : 1;     /**< Reset pin 10 */
    volatile uint32_t BR11 : 1;     /**< Reset pin 11 */
    volatile uint32_t BR12 : 1;     /**< Reset pin 12 */
    volatile uint32_t BR13 : 1;     /**< Reset pin 13 */
    volatile uint32_t BR14 : 1;     /**< Reset pin 14 */
    volatile uint32_t BR15 : 1;     /**< Reset pin 15 */
}GPIO_BSRR_t;

/**
 * @brief Union for flexible BSRR register access
 * @author Eng.Gemy
 */
typedef union 
{     
    GPIO_BSRR_t        BIT_FIELDS;      /**< Access individual set/reset bits */
    volatile uint32_t     ALL_FIELDS;   /**< Access entire register as 32-bit word */
}GPIO_BSRR_U_t;

/******************************************************************************
 *                   GPIO LOCK REGISTER (LCKR) STRUCTURES
 * @brief Structures for GPIO port configuration lock register access
 * @details This register is used to lock the port configuration
 *          Requires special lock sequence to activate
 * @author Eng.Gemy
 ******************************************************************************/
typedef struct{
    volatile uint32_t LCK0  : 1;    /**< Lock configuration for pin 0 */
    volatile uint32_t LCK1  : 1;    /**< Lock configuration for pin 1 */
    volatile uint32_t LCK2  : 1;    /**< Lock configuration for pin 2 */
    volatile uint32_t LCK3  : 1;    /**< Lock configuration for pin 3 */
    volatile uint32_t LCK4  : 1;    /**< Lock configuration for pin 4 */
    volatile uint32_t LCK5  : 1;    /**< Lock configuration for pin 5 */
    volatile uint32_t LCK6  : 1;    /**< Lock configuration for pin 6 */
    volatile uint32_t LCK7  : 1;    /**< Lock configuration for pin 7 */
    volatile uint32_t LCK8  : 1;    /**< Lock configuration for pin 8 */
    volatile uint32_t LCK9  : 1;    /**< Lock configuration for pin 9 */
    volatile uint32_t LCK10 : 1;    /**< Lock configuration for pin 10 */
    volatile uint32_t LCK11 : 1;    /**< Lock configuration for pin 11 */
    volatile uint32_t LCK12 : 1;    /**< Lock configuration for pin 12 */
    volatile uint32_t LCK13 : 1;    /**< Lock configuration for pin 13 */
    volatile uint32_t LCK14 : 1;    /**< Lock configuration for pin 14 */
    volatile uint32_t LCK15 : 1;    /**< Lock configuration for pin 15 */
    volatile uint32_t LCKK  : 1;    /**< Lock key - used to activate lock sequence */
    volatile uint32_t reserved :15; /**< Reserved bits (17-31) */
}GPIO_LCKR_t;

/**
 * @brief Union for flexible LCKR register access
 * @author Eng.Gemy
 */
typedef union 
{     
    GPIO_LCKR_t        BIT_FIELDS;      /**< Access individual lock bits */
    volatile uint32_t     ALL_FIELDS;   /**< Access entire register as 32-bit word */
}GPIO_LCKR_U_t;

/******************************************************************************
 *                   GPIO ALTERNATE FUNCTION LOW REGISTER (AFRL) STRUCTURES
 * @brief Structures for GPIO alternate function low register access
 * @details Configures alternate function for pins 0-7 (4 bits per pin: AF0-AF15)
 * @author Eng.Gemy
 ******************************************************************************/
typedef struct{
    volatile uint32_t AFRL0  : 4;   /**< Alternate function for pin 0 */
    volatile uint32_t AFRL1  : 4;   /**< Alternate function for pin 1 */
    volatile uint32_t AFRL2  : 4;   /**< Alternate function for pin 2 */
    volatile uint32_t AFRL3  : 4;   /**< Alternate function for pin 3 */
    volatile uint32_t AFRL4  : 4;   /**< Alternate function for pin 4 */
    volatile uint32_t AFRL5  : 4;   /**< Alternate function for pin 5 */
    volatile uint32_t AFRL6  : 4;   /**< Alternate function for pin 6 */
    volatile uint32_t AFRL7  : 4;   /**< Alternate function for pin 7 */
}GPIO_AFRL_t;


/**
 * @brief Union for flexible AFRL register access
 * @author Eng.Gemy
 */
typedef union 
{     
    GPIO_AFRL_t        BIT_FIELDS;      /**< Access individual alternate function bits for pins 0-7 */
    volatile uint32_t     ALL_FIELDS;   /**< Access entire register as 32-bit word */
}GPIO_AFRL_U_t;

/******************************************************************************
 *                   GPIO ALTERNATE FUNCTION HIGH REGISTER (AFRH) STRUCTURES
 * @brief Structures for GPIO alternate function high register access
 * @details Configures alternate function for pins 8-15 (4 bits per pin: AF0-AF15)
 * @author Eng.Gemy
 ******************************************************************************/
typedef struct{
    volatile uint32_t AFRH8  : 4;   /**< Alternate function for pin 8 */
    volatile uint32_t AFRH9  : 4;   /**< Alternate function for pin 9 */
    volatile uint32_t AFRH10 : 4;   /**< Alternate function for pin 10 */
    volatile uint32_t AFRH11 : 4;   /**< Alternate function for pin 11 */
    volatile uint32_t AFRH12 : 4;   /**< Alternate function for pin 12 */
    volatile uint32_t AFRH13 : 4;   /**< Alternate function for pin 13 */
    volatile uint32_t AFRH14 : 4;   /**< Alternate function for pin 14 */
    volatile uint32_t AFRH15 : 4;   /**< Alternate function for pin 15 */
}GPIO_AFRH_t;

/**
 * @brief Union for flexible AFRH register access
 * @author Eng.Gemy
 */
typedef union 
{     
    GPIO_AFRH_t        BIT_FIELDS;      /**< Access individual alternate function bits for pins 8-15 */
    volatile uint32_t     ALL_FIELDS;   /**< Access entire register as 32-bit word */
}GPIO_AFRH_U_t;

/******************************************************************************
 *                   GPIO REGISTERS STRUCTURE
 * @brief Complete GPIO peripheral register structure
 * @details This structure maps all GPIO registers in their hardware order
 *          Starting from the base address of each GPIO port (A-H)
 * @note Register offsets (from base address):
 *       MODER:   0x00  -  Mode register
 *       OTYPER:  0x04  -  Output type register
 *       OSPEEDR: 0x08  -  Output speed register
 *       PUPDR:   0x0C  -  Pull-up/pull-down register
 *       IDR:     0x10  -  Input data register
 *       ODR:     0x14  -  Output data register
 *       BSRR:    0x18  -  Bit set/reset register
 *       LCKR:    0x1C  -  Lock register
 *       AFRL:    0x20  -  Alternate function low register
 *       AFRH:    0x24  -  Alternate function high register
 * @author Eng.Gemy
 ******************************************************************************/
typedef struct{
    GPIO_MODER_U_t   MODER;      // GPIO port mode register                  (Offset: 0x00)
    GPIO_OTYPER_U_t  OTYPER;     // GPIO port output type register           (Offset: 0x04)
    GPIO_OSPEEDR_U_t OSPEEDR;    // GPIO port output speed register          (Offset: 0x08)
    GPIO_PUPDR_U_t   PUPDR;      // GPIO port pull-up/pull-down register     (Offset: 0x0C)
    GPIO_IDR_U_t     IDR;        // GPIO port input data register            (Offset: 0x10)
    GPIO_ODR_U_t     ODR;        // GPIO port output data register           (Offset: 0x14)
    GPIO_BSRR_U_t    BSRR;       // GPIO port bit set/reset register         (Offset: 0x18)
    GPIO_LCKR_U_t    LCKR;       // GPIO port configuration lock register    (Offset: 0x1C)
    GPIO_AFRL_U_t    AFRL;       // GPIO alternate function low register     (Offset: 0x20)
    GPIO_AFRH_U_t    AFRH;       // GPIO alternate function high register    (Offset: 0x24)
}GPIO_Registers_t;



#endif // GPIO_PRIV_H

/******************************************************************************
 *                           END OF FILE
 * @author Eng.Gemy
 * @note This private header file contains all hardware-specific definitions
 *       for the GPIO peripheral. Application code should only use the public
 *       interface defined in GPIO_INT.H
 ******************************************************************************/