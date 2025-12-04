/******************************************************************************
 * @file    RCC_PRIV.H
 * @author  Eng.Gemy
 * @brief   RCC (Reset and Clock Control) Driver Private Header File
 *          This file contains the hardware register definitions, base addresses,
 *          masks, and internal structures for the RCC peripheral
 * @date    [Current Date]
 * @version 1.0
 * @note    This file should NOT be included by application code
 *          It contains low-level hardware definitions for RCC driver implementation
 ******************************************************************************/

#ifndef RCC_PRIV_H
#define RCC_PRIV_H


/******************************************************************************
 *                        TIMEOUT VALUES
 * @brief Software timeout values for clock stabilization
 * @details These values represent loop counts to wait for clock ready flags
 *          Adjust based on system frequency and requirements
 * @author Eng.Gemy
 ******************************************************************************/
#define HSE_TIMEOUT_VALUE   100000U   /**< HSE stabilization timeout count (external crystal startup time) */
#define HSI_TIMEOUT_VALUE   50000U    /**< HSI stabilization timeout count (internal oscillator startup time) */
#define PLL_TIMEOUT_VALUE   1000000U  /**< PLL lock timeout count (PLL stabilization time) */

/******************************************************************************
 *                        RCC BASE ADDRESS
 * @brief Memory-mapped base address for RCC peripheral
 * @author Eng.Gemy
 ******************************************************************************/
// Base address of RCC peripheral registers
#define RCC_BASE_ADDRESS    (0x40023800U)  /**< RCC peripheral base address on AHB1 bus */

/******************************************************************************
 *                        PERIPHERAL VALIDATION MASKS
 * @brief Masks to validate peripheral clock enable bits for each bus
 * @details These masks define which bits are valid in each bus's ENR register
 *          Set bits indicate valid peripheral positions
 * @author Eng.Gemy
 ******************************************************************************/
//                             0b10987654321098765432109876543210
#define AHB1_PERPHRALS_MASK    0b11111111100111111110111101100000ULL  /**< Valid peripheral bits for AHB1 bus */
#define AHB2_PERPHRALS_MASK    0b11111111111111111111111101111111ULL  /**< Valid peripheral bits for AHB2 bus */
#define APB1_PERPHRALS_MASK    0b11101111000111010011011111110000ULL  /**< Valid peripheral bits for APB1 bus */
#define APB2_PERPHRALS_MASK    0b11111111111110001000011011001110ULL  /**< Valid peripheral bits for APB2 bus */
#define BUS_MASK               0b11110000UL                           /**< Mask to extract bus identifier from clock mask */

/******************************************************************************
 *                        AHB PRESCALER MASKS
 * @brief Masks for AHB prescaler configuration and validation
 * @author Eng.Gemy
 ******************************************************************************/
//                                       0b10987654321098765432109876543210
#define AHB_PRESCALER_NO_DIVISION_MASK   0b00000000000000000000000010000000ULL  /**< Bit pattern indicating AHB no division */
#define AHB_PRESCALER_CORRECTION_MASK    0b11111111111111111111111100001111ULL  /**< Mask to clear HPRE bits in CFGR register */

/******************************************************************************
 *                        APB1 PRESCALER MASKS
 * @brief Masks for APB1 prescaler configuration and validation
 * @author Eng.Gemy
 ******************************************************************************/
//                                       0b10987654321098765432109876543210
#define APB1_PRESCALER_NO_DIVISION_MASK  0b00000000000000000001000000000000ULL  /**< Bit pattern indicating APB1 no division */
#define APB1_PRESCALER_CORRECTION_MASK   0b11111111111111111110001111111111ULL  /**< Mask to clear PPRE1 bits in CFGR register */

/******************************************************************************
 *                        APB2 PRESCALER MASKS
 * @brief Masks for APB2 prescaler configuration and validation
 * @author Eng.Gemy
 ******************************************************************************/
//                                       0b10987654321098765432109876543210
#define APB2_PRESCALER_NO_DIVISION_MASK  0b00000000000000001000000000000000ULL  /**< Bit pattern indicating APB2 no division */
#define APB2_PRESCALER_CORRECTION_MASK   0b11111111111111110001111111111111ULL  /**< Mask to clear PPRE2 bits in CFGR register */



/******************************************************************************
 *                   RCC CONTROL REGISTER (CR) STRUCTURES
 * @brief RCC Control Register bit fields
 * @details This register controls the clock sources (HSI, HSE, PLL, PLLI2S)
 * @note Register offset from RCC base: 0x00
 * @author Eng.Gemy
 ******************************************************************************/
/**
 * RCC_CR_t Structure
 * RCC Control Register bit fields
 * This register controls the clock sources (HSI, HSE, PLL, PLLI2S)
 */
typedef struct 
{
    volatile uint32_t HSION       : 1;    /**< Bit 0: HSI (High Speed Internal) oscillator enable */
    volatile uint32_t HSIRDY      : 1;    /**< Bit 1: HSI oscillator clock ready flag (read-only) */
    volatile uint32_t Reserved1   : 1;    /**< Bit 2: Reserved bit */
    volatile uint32_t HSITRIM     : 5;    /**< Bits 3-7: HSI oscillator trimming bits (adjusts HSI frequency ±1%) */
    volatile uint32_t HSICAL      : 8;    /**< Bits 8-15: HSI oscillator calibration bits (factory calibration, read-only) */
    volatile uint32_t HSEON       : 1;    /**< Bit 16: HSE (High Speed External) oscillator enable */
    volatile uint32_t HSERDY      : 1;    /**< Bit 17: HSE oscillator clock ready flag (read-only) */
    volatile uint32_t HSEBYP      : 1;    /**< Bit 18: HSE oscillator bypass (allows external clock input instead of crystal) */
    volatile uint32_t CSSON       : 1;    /**< Bit 19: Clock Security System enable (detects HSE failure and switches to HSI) */
    volatile uint32_t Reserved2   : 4;    /**< Bits 20-23: Reserved bits */
    volatile uint32_t PLLON       : 1;    /**< Bit 24: PLL (Phase Locked Loop) enable */
    volatile uint32_t PLLRDY      : 1;    /**< Bit 25: PLL clock ready flag (read-only) */
    volatile uint32_t PLLI2SON    : 1;    /**< Bit 26: PLLI2S (PLL for I2S audio) enable */
    volatile uint32_t PLLI2SRDY   : 1;    /**< Bit 27: PLLI2S clock ready flag (read-only) */
    volatile uint32_t Reserved3   : 4;    /**< Bits 28-31: Reserved bits */
}RCC_CR_t;

/**
 * Union to access RCC_CR as either complete 32-bit value or individual bit fields
 * @author Eng.Gemy
 */
typedef union 
{
    volatile uint32_t ALL_FIELDS; /**< Access entire 32-bit register at once */
    RCC_CR_t BIT_FIELDS;          /**< Access individual bit fields by name */
}RCC_CR_REG_t;

/******************************************************************************
 *                   RCC PLL CONFIGURATION REGISTER (PLLCFGR) STRUCTURES
 * @brief RCC PLL Configuration Register bit fields
 * @details This register configures the PLL divider and multiplier values
 * @note Register offset from RCC base: 0x04
 * @note PLL formula: VCO_freq = (Input_freq / PLLM) * PLLN
 *                    PLL_output = VCO_freq / PLLP
 *                    USB_freq = VCO_freq / PLLQ
 * @author Eng.Gemy
 ******************************************************************************/
/**
 * RCC_PLLCFGR_t Structure
 * RCC PLL Configuration Register bit fields
 * This register configures the PLL divider and multiplier values
 */
typedef struct {
    volatile uint32_t PLLM        : 6;    /**< Bits 0-5: PLL division factor M (2-63), divides input to 1-2 MHz for VCO */
    volatile uint32_t PLLN        : 9;    /**< Bits 6-14: PLL multiplication factor N (50-432), multiplies VCO input */
    volatile uint32_t Reserved1   : 1;    /**< Bit 15: Reserved bit */
    volatile uint32_t PLLP        : 2;    /**< Bits 16-17: PLL division factor P for main system clock (2,4,6,8) */
    volatile uint32_t Reserved2   : 4;    /**< Bits 18-21: Reserved bits */
    volatile uint32_t PLLSRC      : 1;    /**< Bit 22: PLL clock source select (0=HSI, 1=HSE) */
    volatile uint32_t Reserved3   : 1;    /**< Bit 23: Reserved bit */
    volatile uint32_t PLLQ        : 4;    /**< Bits 24-27: PLL division factor Q for USB/SDIO (2-15), must produce 48 MHz for USB */
    volatile uint32_t Reserved4   : 4;    /**< Bits 28-31: Reserved bits */
}RCC_PLLCFGR_t;

/**
 * Union to access RCC_PLLCFGR as either complete 32-bit value or individual bit fields
 * @author Eng.Gemy
 */
typedef union
{
    volatile uint32_t ALL_FIELDS; /**< Access entire 32-bit register at once */
    RCC_PLLCFGR_t BIT_FIELDS;     /**< Access individual bit fields by name */
}RCC_PLLCFGR_REG_t;

/******************************************************************************
 *                   RCC CLOCK CONFIGURATION REGISTER (CFGR) STRUCTURES
 * @brief RCC Clock Configuration Register bit fields
 * @details This register controls clock source selection, prescalers, and output clocks
 * @note Register offset from RCC base: 0x08
 * @author Eng.Gemy
 ******************************************************************************/
/**
 * RCC_CFGR_t Structure
 * RCC Clock Configuration Register bit fields
 * This register controls clock source selection, clock dividers, and output clocks
 */
typedef struct {    
    volatile uint32_t SW          : 2;    /**< Bits 0-1: System clock switch (00=HSI, 01=HSE, 10=PLL) */
    volatile uint32_t SWS         : 2;    /**< Bits 2-3: System clock switch status (read-only, same encoding as SW) */
    volatile uint32_t HPRE        : 4;    /**< Bits 4-7: AHB prescaler (divides SYSCLK for AHB bus) */
    volatile uint32_t Reserved1   : 2;    /**< Bits 8-9: Reserved bits */
    volatile uint32_t PPRE1       : 3;    /**< Bits 10-12: APB1 prescaler (divides AHB clock for APB1 low-speed bus) */
    volatile uint32_t PPRE2       : 3;    /**< Bits 13-15: APB2 prescaler (divides AHB clock for APB2 high-speed bus) */
    volatile uint32_t RTCPRE      : 5;    /**< Bits 16-20: RTC clock prescaler (divides HSE for RTC, 2-31) */
    volatile uint32_t MCO1        : 2;    /**< Bits 21-22: Microcontroller Clock Output 1 select */
    volatile uint32_t I2SSRC      : 1;    /**< Bit 23: I2S clock source select */
    volatile uint32_t MCO1PRE     : 3;    /**< Bits 24-26: MCO1 prescaler (divides MCO1 output) */
    volatile uint32_t MCO2PRE     : 3;    /**< Bits 27-29: MCO2 prescaler (divides MCO2 output) */
    volatile uint32_t MCO2        : 2;    /**< Bits 30-31: Microcontroller Clock Output 2 select */
}RCC_CFGR_t;

/**
 * Union to access RCC_CFGR as either complete 32-bit value or individual bit fields
 * @author Eng.Gemy
 */
typedef union
{
    volatile uint32_t ALL_FIELDS; /**< Access entire 32-bit register at once */
    RCC_CFGR_t BIT_FIELDS;        /**< Access individual bit fields by name */
}RCC_CFGR_REG_t;

/******************************************************************************
 *                   RCC CLOCK INTERRUPT REGISTER (CIR) STRUCTURES
 * @brief RCC Clock Interrupt Register bit fields
 * @details This register contains interrupt flags and control bits for clock events
 * @note Register offset from RCC base: 0x0C
 * @author Eng.Gemy
 ******************************************************************************/
/**
 * RCC_CIR_t Structure
 * RCC Clock Interrupt Register bit fields
 * This register contains interrupt flags and interrupt enable/clear bits for clock events
 */
typedef struct {
    // Interrupt flags (read-only status bits)
    volatile uint32_t LSIRDYF     : 1;    /**< Bit 0: LSI (Low Speed Internal) clock ready interrupt flag */
    volatile uint32_t LSERDYF     : 1;    /**< Bit 1: LSE (Low Speed External) clock ready interrupt flag */
    volatile uint32_t HSIRDYF     : 1;    /**< Bit 2: HSI clock ready interrupt flag */
    volatile uint32_t HSERDYF     : 1;    /**< Bit 3: HSE clock ready interrupt flag */
    volatile uint32_t PLLRDYF     : 1;    /**< Bit 4: PLL clock ready interrupt flag */
    volatile uint32_t PLLI2SRDYF  : 1;    /**< Bit 5: PLLI2S clock ready interrupt flag */
    volatile uint32_t Reserved1   : 1;    /**< Bit 6: Reserved bit */
    volatile uint32_t CSSF        : 1;    /**< Bit 7: Clock Security System interrupt flag (HSE failure detected) */
    
    // Interrupt enable bits
    volatile uint32_t LSIRDYIE    : 1;    /**< Bit 8: LSI ready interrupt enable */
    volatile uint32_t LSERDYIE    : 1;    /**< Bit 9: LSE ready interrupt enable */
    volatile uint32_t HSIRDYIE    : 1;    /**< Bit 10: HSI ready interrupt enable */
    volatile uint32_t HSERDYIE    : 1;    /**< Bit 11: HSE ready interrupt enable */
    volatile uint32_t PLLRDYIE    : 1;    /**< Bit 12: PLL ready interrupt enable */
    volatile uint32_t PLLI2SRDYIE : 1;    /**< Bit 13: PLLI2S ready interrupt enable */
    volatile uint32_t Reserved2   : 2;    /**< Bits 14-15: Reserved bits */
    
    // Interrupt clear bits (write 1 to clear corresponding flag)
    volatile uint32_t LSIRDYC     : 1;    /**< Bit 16: LSI ready interrupt flag clear (write 1 to clear) */
    volatile uint32_t LSERDYC     : 1;    /**< Bit 17: LSE ready interrupt flag clear (write 1 to clear) */
    volatile uint32_t HSIRDYC     : 1;    /**< Bit 18: HSI ready interrupt flag clear (write 1 to clear) */
    volatile uint32_t HSERDYC     : 1;    /**< Bit 19: HSE ready interrupt flag clear (write 1 to clear) */
    volatile uint32_t PLLRDYC     : 1;    /**< Bit 20: PLL ready interrupt flag clear (write 1 to clear) */
    volatile uint32_t PLLI2SRDYC  : 1;    /**< Bit 21: PLLI2S ready interrupt flag clear (write 1 to clear) */
    volatile uint32_t Reserved3   : 1;    /**< Bit 22: Reserved bit */
    volatile uint32_t CSSC        : 1;    /**< Bit 23: Clock Security System interrupt flag clear (write 1 to clear) */
    volatile uint32_t Reserved4   : 8;    /**< Bits 24-31: Reserved bits */
}RCC_CIR_t;

/**
 * Union to access RCC_CIR as either complete 32-bit value or individual bit fields
 * @author Eng.Gemy
 */
typedef union
{
    volatile uint32_t ALL_FIELDS; /**< Access entire 32-bit register at once */
    RCC_CIR_t BIT_FIELDS;         /**< Access individual bit fields by name */
}RCC_CIR_REG_t;

/******************************************************************************
 *                   RCC AHB1 PERIPHERAL RESET REGISTER (AHB1RSTR) STRUCTURES
 * @brief RCC AHB1 Peripheral Reset Register bit fields
 * @details This register controls reset signals for AHB1 bus peripherals
 * @note Register offset from RCC base: 0x10
 * @note Write 1 to reset peripheral, write 0 to release from reset
 * @author Eng.Gemy
 ******************************************************************************/
/**
 * RCC_AHB1RSTR_t Structure
 * RCC AHB1 Peripheral Reset Register bit fields
 * This register controls reset signals for AHB1 bus peripherals
 */
typedef struct {
    volatile uint32_t GPIOARST    : 1;    /**< Bit 0: GPIOA reset */
    volatile uint32_t GPIOBRST    : 1;    /**< Bit 1: GPIOB reset */
    volatile uint32_t GPIOCRST    : 1;    /**< Bit 2: GPIOC reset */
    volatile uint32_t GPIODRST    : 1;    /**< Bit 3: GPIOD reset */
    volatile uint32_t GPIOERST    : 1;    /**< Bit 4: GPIOE reset */
    volatile uint32_t Reserved1   : 2;    /**< Bits 5-6: Reserved bits */
    volatile uint32_t GPIOHRST    : 1;    /**< Bit 7: GPIOH reset */
    volatile uint32_t Reserved2   : 4;    /**< Bits 8-11: Reserved bits */
    volatile uint32_t CRCRST      : 1;    /**< Bit 12: CRC (Cyclic Redundancy Check) unit reset */
    volatile uint32_t Reserved3   : 8;    /**< Bits 13-20: Reserved bits */
    volatile uint32_t DMA1RST     : 1;    /**< Bit 21: DMA1 (Direct Memory Access 1) reset */
    volatile uint32_t DMA2RST     : 1;    /**< Bit 22: DMA2 (Direct Memory Access 2) reset */
    volatile uint32_t Reserved4   : 9;    /**< Bits 23-31: Reserved bits */
}RCC_AHB1RSTR_t;

/**
 * Union to access RCC_AHB1RSTR as either complete 32-bit value or individual bit fields
 * @author Eng.Gemy
 */
typedef union
{
    volatile uint32_t ALL_FIELDS; /**< Access entire 32-bit register at once */
    RCC_AHB1RSTR_t BIT_FIELDS;    /**< Access individual bit fields by name */
}RCC_AHB1RSTR_REG_t;

/******************************************************************************
 *                   RCC AHB2 PERIPHERAL RESET REGISTER (AHB2RSTR) STRUCTURES
 * @brief RCC AHB2 Peripheral Reset Register bit fields
 * @details This register controls reset signals for AHB2 bus peripherals
 * @note Register offset from RCC base: 0x14
 * @author Eng.Gemy
 ******************************************************************************/
/**
 * RCC_AHB2RSTR_t Structure
 * RCC AHB2 Peripheral Reset Register bit fields
 * This register controls reset signals for AHB2 bus peripherals
 */
typedef struct {
    volatile uint32_t Reserved1   : 7;    /**< Bits 0-6: Reserved bits */
    volatile uint32_t OTGFSRST    : 1;    /**< Bit 7: USB OTG (On-The-Go) FS (Full-Speed) device reset */
    volatile uint32_t Reserved2   : 24;   /**< Bits 8-31: Reserved bits */
}RCC_AHB2RSTR_t;

/**
 * Union to access RCC_AHB2RSTR as either complete 32-bit value or individual bit fields
 * @author Eng.Gemy
 */
typedef union
{
    volatile uint32_t ALL_FIELDS; /**< Access entire 32-bit register at once */
    RCC_AHB2RSTR_t BIT_FIELDS;    /**< Access individual bit fields by name */
}RCC_AHB2RSTR_REG_t;

/******************************************************************************
 *                   RCC APB1 PERIPHERAL RESET REGISTER (APB1RSTR) STRUCTURES
 * @brief RCC APB1 Peripheral Reset Register bit fields
 * @details This register controls reset signals for APB1 low-speed bus peripherals
 * @note Register offset from RCC base: 0x20
 * @author Eng.Gemy
 ******************************************************************************/
/**
 * RCC_APB1RSTR_t Structure
 * RCC APB1 Peripheral Reset Register bit fields
 * This register controls reset signals for APB1 low-speed bus peripherals
 */
typedef struct {
    volatile uint32_t TIM2RST     : 1;    /**< Bit 0: TIM2 (Timer 2) reset */
    volatile uint32_t TIM3RST     : 1;    /**< Bit 1: TIM3 (Timer 3) reset */
    volatile uint32_t TIM4RST     : 1;    /**< Bit 2: TIM4 (Timer 4) reset */
    volatile uint32_t TIM5RST     : 1;    /**< Bit 3: TIM5 (Timer 5) reset */
    volatile uint32_t Reserved1   : 7;    /**< Bits 4-10: Reserved bits */
    volatile uint32_t WWDGRST     : 1;    /**< Bit 11: WWDG (Window WatchDog) reset */
    volatile uint32_t Reserved2   : 2;    /**< Bits 12-13: Reserved bits */
    volatile uint32_t SPI2RST     : 1;    /**< Bit 14: SPI2 (Serial Peripheral Interface 2) reset */
    volatile uint32_t SPI3RST     : 1;    /**< Bit 15: SPI3 (Serial Peripheral Interface 3) reset */
    volatile uint32_t Reserved3   : 1;    /**< Bit 16: Reserved bit */
    volatile uint32_t USART2RST   : 1;    /**< Bit 17: USART2 (Universal Synchronous/Asynchronous Receiver/Transmitter 2) reset */
    volatile uint32_t Reserved4   : 3;    /**< Bits 18-20: Reserved bits */
    volatile uint32_t I2C1RST     : 1;    /**< Bit 21: I2C1 (Inter-Integrated Circuit 1) reset */
    volatile uint32_t I2C2RST     : 1;    /**< Bit 22: I2C2 (Inter-Integrated Circuit 2) reset */
    volatile uint32_t I2C3RST     : 1;    /**< Bit 23: I2C3 (Inter-Integrated Circuit 3) reset */
    volatile uint32_t Reserved5   : 4;    /**< Bits 24-27: Reserved bits */
    volatile uint32_t PWRRST      : 1;    /**< Bit 28: PWR (Power) interface reset */
    volatile uint32_t Reserved6   : 3;    /**< Bits 29-31: Reserved bits */
}RCC_APB1RSTR_t;

/**
 * Union to access RCC_APB1RSTR as either complete 32-bit value or individual bit fields
 * @author Eng.Gemy
 */
typedef union
{
    volatile uint32_t ALL_FIELDS; /**< Access entire 32-bit register at once */
    RCC_APB1RSTR_t BIT_FIELDS;    /**< Access individual bit fields by name */
}RCC_APB1RSTR_REG_t;

/******************************************************************************
 *                   RCC APB2 PERIPHERAL RESET REGISTER (APB2RSTR) STRUCTURES
 * @brief RCC APB2 Peripheral Reset Register bit fields
 * @details This register controls reset signals for APB2 high-speed bus peripherals
 * @note Register offset from RCC base: 0x24
 * @author Eng.Gemy
 ******************************************************************************/
/**
 * RCC_APB2RSTR_t Structure
 * RCC APB2 Peripheral Reset Register bit fields
 * This register controls reset signals for APB2 high-speed bus peripherals
 */
typedef struct {
    volatile uint32_t TIM1RST     : 1;    /**< Bit 0: TIM1 (Timer 1) reset */
    volatile uint32_t Reserved1   : 3;    /**< Bits 1-3: Reserved bits */
    volatile uint32_t USART1RST   : 1;    /**< Bit 4: USART1 reset */
    volatile uint32_t USART6RST   : 1;    /**< Bit 5: USART6 reset */
    volatile uint32_t Reserved2   : 2;    /**< Bits 6-7: Reserved bits */
    volatile uint32_t ADC1RST     : 1;    /**< Bit 8: ADC1 (Analog-to-Digital Converter 1) reset */
    volatile uint32_t Reserved3   : 2;    /**< Bits 9-10: Reserved bits */
    volatile uint32_t SDIORST     : 1;    /**< Bit 11: SDIO (Secure Digital Input/Output) reset */
    volatile uint32_t SPI1RST     : 1;    /**< Bit 12: SPI1 (Serial Peripheral Interface 1) reset */
    volatile uint32_t SPI4RST     : 1;    /**< Bit 13: SPI4 (Serial Peripheral Interface 4) reset */
    volatile uint32_t SYSCFGRST   : 1;    /**< Bit 14: SYSCFG (System Configuration Controller) reset */
    volatile uint32_t Reserved4   : 1;    /**< Bit 15: Reserved bit */
    volatile uint32_t TIM9RST     : 1;    /**< Bit 16: TIM9 (Timer 9) reset */
    volatile uint32_t TIM10RST    : 1;    /**< Bit 17: TIM10 (Timer 10) reset */
    volatile uint32_t TIM11RST    : 1;    /**< Bit 18: TIM11 (Timer 11) reset */
    volatile uint32_t Reserved5   : 13;   /**< Bits 19-31: Reserved bits */
}RCC_APB2RSTR_t;

/**
 * Union to access RCC_APB2RSTR as either complete 32-bit value or individual bit fields
 * @author Eng.Gemy
 */
typedef union
{
    volatile uint32_t ALL_FIELDS; /**< Access entire 32-bit register at once */
    RCC_APB2RSTR_t BIT_FIELDS;    /**< Access individual bit fields by name */
}RCC_APB2RSTR_REG_t;

/******************************************************************************
 *                   RCC AHB1 PERIPHERAL CLOCK ENABLE REGISTER (AHB1ENR) STRUCTURES
 * @brief RCC AHB1 Peripheral Clock Enable Register bit fields
 * @details This register enables clocks for AHB1 bus peripherals
 * @note Register offset from RCC base: 0x30
 * @note Write 1 to enable peripheral clock, 0 to disable
 * @author Eng.Gemy
 ******************************************************************************/
/**
 * RCC_AHB1ENR_t Structure
 * RCC AHB1 Peripheral Clock Enable Register bit fields
 * This register enables clocks for AHB1 bus peripherals
 */
typedef struct {
    volatile uint32_t GPIOAEN     : 1;    /**< Bit 0: GPIOA clock enable */
    volatile uint32_t GPIOBEN     : 1;    /**< Bit 1: GPIOB clock enable */
    volatile uint32_t GPIOCEN     : 1;    /**< Bit 2: GPIOC clock enable */
    volatile uint32_t GPIODEN     : 1;    /**< Bit 3: GPIOD clock enable */
    volatile uint32_t GPIOEEN     : 1;    /**< Bit 4: GPIOE clock enable */
    volatile uint32_t Reserved1   : 2;    /**< Bits 5-6: Reserved bits */
    volatile uint32_t GPIOHEN     : 1;    /**< Bit 7: GPIOH clock enable */
    volatile uint32_t Reserved2   : 4;    /**< Bits 8-11: Reserved bits */
    volatile uint32_t CRCEN       : 1;    /**< Bit 12: CRC (Cyclic Redundancy Check) unit clock enable */
    volatile uint32_t Reserved3   : 8;    /**< Bits 13-20: Reserved bits */
    volatile uint32_t DMA1EN      : 1;    /**< Bit 21: DMA1 clock enable */
    volatile uint32_t DMA2EN      : 1;    /**< Bit 22: DMA2 clock enable */
    volatile uint32_t Reserved4   : 9;    /**< Bits 23-31: Reserved bits */
}RCC_AHB1ENR_t;

/**
 * Union to access RCC_AHB1ENR as either complete 32-bit value or individual bit fields
 * @author Eng.Gemy
 */
typedef union
{
    volatile uint32_t ALL_FIELDS; /**< Access entire 32-bit register at once */
    RCC_AHB1ENR_t BIT_FIELDS;     /**< Access individual bit fields by name */
}RCC_AHB1ENR_REG_t;

/******************************************************************************
 *                   RCC AHB2 PERIPHERAL CLOCK ENABLE REGISTER (AHB2ENR) STRUCTURES
 * @brief RCC AHB2 Peripheral Clock Enable Register bit fields
 * @details This register enables clocks for AHB2 bus peripherals
 * @note Register offset from RCC base: 0x34
 * @author Eng.Gemy
 ******************************************************************************/
/**
 * RCC_AHB2ENR_t Structure
 * RCC AHB2 Peripheral Clock Enable Register bit fields
 * This register enables clocks for AHB2 bus peripherals
 */
typedef struct {
    volatile uint32_t Reserved1   : 7;    /**< Bits 0-6: Reserved bits */
    volatile uint32_t OTGFSEN     : 1;    /**< Bit 7: USB OTG FS device clock enable */
    volatile uint32_t Reserved2   : 24;   /**< Bits 8-31: Reserved bits */
}RCC_AHB2ENR_t;

/**
 * Union to access RCC_AHB2ENR as either complete 32-bit value or individual bit fields
 * @author Eng.Gemy
 */
typedef union
{
    volatile uint32_t ALL_FIELDS; /**< Access entire 32-bit register at once */
    RCC_AHB2ENR_t BIT_FIELDS;     /**< Access individual bit fields by name */
}RCC_AHB2ENR_REG_t;

/******************************************************************************
 *                   RCC APB1 PERIPHERAL CLOCK ENABLE REGISTER (APB1ENR) STRUCTURES
 * @brief RCC APB1 Peripheral Clock Enable Register bit fields
 * @details This register enables clocks for APB1 low-speed bus peripherals
 * @note Register offset from RCC base: 0x40
 * @author Eng.Gemy
 ******************************************************************************/
/**
 * RCC_APB1ENR_t Structure
 * RCC APB1 Peripheral Clock Enable Register bit fields
 * This register enables clocks for APB1 low-speed bus peripherals
 */
typedef struct {
    volatile uint32_t TIM2EN      : 1;    /**< Bit 0: TIM2 clock enable */
    volatile uint32_t TIM3EN      : 1;    /**< Bit 1: TIM3 clock enable */
    volatile uint32_t TIM4EN      : 1;    /**< Bit 2: TIM4 clock enable */
    volatile uint32_t TIM5EN      : 1;    /**< Bit 3: TIM5 clock enable */
    volatile uint32_t Reserved1   : 7;    /**< Bits 4-10: Reserved bits */
    volatile uint32_t WWDGEN      : 1;    /**< Bit 11: WWDG clock enable */
    volatile uint32_t Reserved2   : 2;    /**< Bits 12-13: Reserved bits */
    volatile uint32_t SPI2EN      : 1;    /**< Bit 14: SPI2 clock enable */
    volatile uint32_t SPI3EN      : 1;    /**< Bit 15: SPI3 clock enable */
    volatile uint32_t Reserved3   : 1;    /**< Bit 16: Reserved bit */
    volatile uint32_t USART2EN    : 1;    /**< Bit 17: USART2 clock enable */
    volatile uint32_t Reserved4   : 3;    /**< Bits 18-20: Reserved bits */
    volatile uint32_t I2C1EN      : 1;    /**< Bit 21: I2C1 clock enable */
    volatile uint32_t I2C2EN      : 1;    /**< Bit 22: I2C2 clock enable */
    volatile uint32_t I2C3EN      : 1;    /**< Bit 23: I2C3 clock enable */
    volatile uint32_t Reserved5   : 4;    /**< Bits 24-27: Reserved bits */
    volatile uint32_t PWREN       : 1;    /**< Bit 28: PWR interface clock enable */
    volatile uint32_t Reserved6   : 3;    /**< Bits 29-31: Reserved bits */
}RCC_APB1ENR_t;

/**
 * Union to access RCC_APB1ENR as either complete 32-bit value or individual bit fields
 * @author Eng.Gemy
 */
typedef union
{
    volatile uint32_t ALL_FIELDS; /**< Access entire 32-bit register at once */
    RCC_APB1ENR_t BIT_FIELDS;     /**< Access individual bit fields by name */
}RCC_APB1ENR_REG_t;

/******************************************************************************
 *                   RCC APB2 PERIPHERAL CLOCK ENABLE REGISTER (APB2ENR) STRUCTURES
 * @brief RCC APB2 Peripheral Clock Enable Register bit fields
 * @details This register enables clocks for APB2 high-speed bus peripherals
 * @note Register offset from RCC base: 0x44
 * @author Eng.Gemy
 ******************************************************************************/
/**
 * RCC_APB2ENR_t Structure
 * RCC APB2 Peripheral Clock Enable Register bit fields
 * This register enables clocks for APB2 high-speed bus peripherals
 */
typedef struct {
    volatile uint32_t TIM1EN      : 1;    /**< Bit 0: TIM1 clock enable */
    volatile uint32_t Reserved1   : 3;    /**< Bits 1-3: Reserved bits */
    volatile uint32_t USART1EN    : 1;    /**< Bit 4: USART1 clock enable */
    volatile uint32_t USART6EN    : 1;    /**< Bit 5: USART6 clock enable */
    volatile uint32_t Reserved2   : 2;    /**< Bits 6-7: Reserved bits */
    volatile uint32_t ADC1EN      : 1;    /**< Bit 8: ADC1 clock enable */
    volatile uint32_t Reserved3   : 2;    /**< Bits 9-10: Reserved bits */
    volatile uint32_t SDIOEN      : 1;    /**< Bit 11: SDIO clock enable */
    volatile uint32_t SPI1EN      : 1;    /**< Bit 12: SPI1 clock enable */
    volatile uint32_t SPI4EN      : 1;    /**< Bit 13: SPI4 clock enable */
    volatile uint32_t SYSCFGEN    : 1;    /**< Bit 14: SYSCFG clock enable */
    volatile uint32_t Reserved4   : 1;    /**< Bit 15: Reserved bit */
    volatile uint32_t TIM9EN      : 1;    /**< Bit 16: TIM9 clock enable */
    volatile uint32_t TIM10EN     : 1;    /**< Bit 17: TIM10 clock enable */
    volatile uint32_t TIM11EN     : 1;    /**< Bit 18: TIM11 clock enable */
    volatile uint32_t Reserved5   : 13;   /**< Bits 19-31: Reserved bits */
}RCC_APB2ENR_t;

/**
 * Union to access RCC_APB2ENR as either complete 32-bit value or individual bit fields
 * @author Eng.Gemy
 */
typedef union
{
    volatile uint32_t ALL_FIELDS; /**< Access entire 32-bit register at once */
    RCC_APB2ENR_t BIT_FIELDS;     /**< Access individual bit fields by name */
}RCC_APB2ENR_REG_t;

/******************************************************************************
 *                   RCC AHB1 LOW POWER MODE CLOCK ENABLE REGISTER (AHB1LPENR) STRUCTURES
 * @brief RCC AHB1 Peripheral Clock Enable in Low Power Mode Register
 * @details This register enables clocks for AHB1 peripherals during sleep mode
 * @note Register offset from RCC base: 0x50
 * @author Eng.Gemy
 ******************************************************************************/
/**
 * RCC_AHB1LPENR_t Structure
 * RCC AHB1 Peripheral Clock Enable in Low Power Mode Register bit fields
 * This register enables clocks for AHB1 peripherals when in low-power sleep mode
 */
typedef struct
{
    volatile uint32_t GPIOALPEN   : 1;    /**< Bit 0: GPIOA clock enable during sleep mode */
    volatile uint32_t GPIOBLPEN   : 1;    /**< Bit 1: GPIOB clock enable during sleep mode */
    volatile uint32_t GPIOCLPEN   : 1;    /**< Bit 2: GPIOC clock enable during sleep mode */
    volatile uint32_t GPIODLPEN   : 1;    /**< Bit 3: GPIOD clock enable during sleep mode */
    volatile uint32_t GPIOELPEN   : 1;    /**< Bit 4: GPIOE clock enable during sleep mode */
    volatile uint32_t Reserved1   : 2;    /**< Bits 5-6: Reserved bits */
    volatile uint32_t GPIOHLPEN   : 1;    /**< Bit 7: GPIOH clock enable during sleep mode */
    volatile uint32_t Reserved2   : 4;    /**< Bits 8-11: Reserved bits */
    volatile uint32_t CRCLPEN     : 1;    /**< Bit 12: CRC clock enable during sleep mode */
    volatile uint32_t Reserved3   : 2;    /**< Bits 13-14: Reserved bits */
    volatile uint32_t FLITFLPEN   : 1;    /**< Bit 15: Flash interface clock enable during sleep mode */
    volatile uint32_t SRAM1LPEN   : 1;    /**< Bit 16: SRAM1 clock enable during sleep mode */
    volatile uint32_t Reserved4   : 5;    /**< Bits 17-21: Reserved bits */
    volatile uint32_t DMA1LPEN    : 1;    /**< Bit 22: DMA1 clock enable during sleep mode */
    volatile uint32_t DMA2LPEN    : 1;    /**< Bit 23: DMA2 clock enable during sleep mode */
    volatile uint32_t Reserved5   : 9;    /**< Bits 24-31: Reserved bits */
}RCC_AHB1LPENR_t;

/**
 * Union to access RCC_AHB1LPENR as either complete 32-bit value or individual bit fields
 * @author Eng.Gemy
 */
typedef union
{
    volatile uint32_t ALL_FIELDS; /**< Access entire 32-bit register at once */
    RCC_AHB1LPENR_t BIT_FIELDS;   /**< Access individual bit fields by name */
}RCC_AHB1LPENR_REG_t;

/******************************************************************************
 *                   RCC AHB2 LOW POWER MODE CLOCK ENABLE REGISTER (AHB2LPENR) STRUCTURES
 * @brief RCC AHB2 Peripheral Clock Enable in Low Power Mode Register
 * @details This register enables clocks for AHB2 peripherals during sleep mode
 * @note Register offset from RCC base: 0x54
 * @author Eng.Gemy
 ******************************************************************************/
/**
 * RCC_AHB2LPENR_t Structure
 * RCC AHB2 Peripheral Clock Enable in Low Power Mode Register bit fields
 * This register enables clocks for AHB2 peripherals when in low-power sleep mode
 */
typedef struct
{
    volatile uint32_t Reserved1   : 7;    /**< Bits 0-6: Reserved bits */
    volatile uint32_t OTGFSLPEN   : 1;    /**< Bit 7: USB OTG FS clock enable during sleep mode */
    volatile uint32_t Reserved2   : 24;   /**< Bits 8-31: Reserved bits */
}RCC_AHB2LPENR_t;

/**
 * Union to access RCC_AHB2LPENR as either complete 32-bit value or individual bit fields
 * @author Eng.Gemy
 */
typedef union
{
    volatile uint32_t ALL_FIELDS; /**< Access entire 32-bit register at once */
    RCC_AHB2LPENR_t BIT_FIELDS;   /**< Access individual bit fields by name */
}RCC_AHB2LPENR_REG_t;

/******************************************************************************
 *                   RCC APB1 LOW POWER MODE CLOCK ENABLE REGISTER (APB1LPENR) STRUCTURES
 * @brief RCC APB1 Peripheral Clock Enable in Low Power Mode Register
 * @details This register enables clocks for APB1 peripherals during sleep mode
 * @note Register offset from RCC base: 0x60
 * @author Eng.Gemy
 ******************************************************************************/
/**
 * RCC_APB1LPENR_t Structure
 * RCC APB1 Peripheral Clock Enable in Low Power Mode Register bit fields
 * This register enables clocks for APB1 peripherals when in low-power sleep mode
 */
typedef struct
{
    volatile uint32_t TIM2LPEN    : 1;    /**< Bit 0: TIM2 clock enable during sleep mode */
    volatile uint32_t TIM3LPEN    : 1;    /**< Bit 1: TIM3 clock enable during sleep mode */
    volatile uint32_t TIM4LPEN    : 1;    /**< Bit 2: TIM4 clock enable during sleep mode */
    volatile uint32_t TIM5LPEN    : 1;    /**< Bit 3: TIM5 clock enable during sleep mode */
    volatile uint32_t Reserved1   : 7;    /**< Bits 4-10: Reserved bits */
    volatile uint32_t WWDGLPEN    : 1;    /**< Bit 11: WWDG clock enable during sleep mode */
    volatile uint32_t Reserved2   : 2;    /**< Bits 12-13: Reserved bits */
    volatile uint32_t SPI2LPEN    : 1;    /**< Bit 14: SPI2 clock enable during sleep mode */
    volatile uint32_t SPI3LPEN    : 1;    /**< Bit 15: SPI3 clock enable during sleep mode */
    volatile uint32_t Reserved3   : 1;    /**< Bit 16: Reserved bit */
    volatile uint32_t USART2LPEN  : 1;    /**< Bit 17: USART2 clock enable during sleep mode */
    volatile uint32_t Reserved4   : 3;    /**< Bits 18-20: Reserved bits */
    volatile uint32_t I2C1LPEN    : 1;    /**< Bit 21: I2C1 clock enable during sleep mode */
    volatile uint32_t I2C2LPEN    : 1;    /**< Bit 22: I2C2 clock enable during sleep mode */
    volatile uint32_t I2C3LPEN    : 1;    /**< Bit 23: I2C3 clock enable during sleep mode */
    volatile uint32_t Reserved5   : 4;    /**< Bits 24-27: Reserved bits */
    volatile uint32_t PWRLPEN     : 1;    /**< Bit 28: PWR interface clock enable during sleep mode */
    volatile uint32_t Reserved6   : 3;    /**< Bits 29-31: Reserved bits */
}RCC_APB1LPENR_t;

/**
 * Union to access RCC_APB1LPENR as either complete 32-bit value or individual bit fields
 * @author Eng.Gemy
 */
typedef union
{
    volatile uint32_t ALL_FIELDS; /**< Access entire 32-bit register at once */
    RCC_APB1LPENR_t BIT_FIELDS;   /**< Access individual bit fields by name */
}RCC_APB1LPENR_REG_t;

/******************************************************************************
 *                   RCC APB2 LOW POWER MODE CLOCK ENABLE REGISTER (APB2LPENR) STRUCTURES
 * @brief RCC APB2 Peripheral Clock Enable in Low Power Mode Register
 * @details This register enables clocks for APB2 peripherals during sleep mode
 * @note Register offset from RCC base: 0x64
 * @author Eng.Gemy
 ******************************************************************************/
/**
 * RCC_APB2LPENR_t Structure
 * RCC APB2 Peripheral Clock Enable in Low Power Mode Register bit fields
 * This register enables clocks for APB2 peripherals when in low-power sleep mode
 */
typedef struct
{
    volatile uint32_t TIM1LPEN    : 1;    /**< Bit 0: TIM1 clock enable during sleep mode */
    volatile uint32_t Reserved1   : 3;    /**< Bits 1-3: Reserved bits */
    volatile uint32_t USART1LPEN  : 1;    /**< Bit 4: USART1 clock enable during sleep mode */
    volatile uint32_t USART6LPEN  : 1;    /**< Bit 5: USART6 clock enable during sleep mode */
    volatile uint32_t Reserved2   : 2;    /**< Bits 6-7: Reserved bits */
    volatile uint32_t ADC1LPEN    : 1;    /**< Bit 8: ADC1 clock enable during sleep mode */
    volatile uint32_t Reserved3   : 2;    /**< Bits 9-10: Reserved bits */
    volatile uint32_t SDIOLPEN    : 1;    /**< Bit 11: SDIO clock enable during sleep mode */
    volatile uint32_t SPI1LPEN    : 1;    /**< Bit 12: SPI1 clock enable during sleep mode */
    volatile uint32_t SPI4LPEN    : 1;    /**< Bit 13: SPI4 clock enable during sleep mode */
    volatile uint32_t SYSCFGLPEN  : 1;    /**< Bit 14: SYSCFG clock enable during sleep mode */
    volatile uint32_t Reserved4   : 1;    /**< Bit 15: Reserved bit */
    volatile uint32_t TIM9LPEN    : 1;    /**< Bit 16: TIM9 clock enable during sleep mode */
    volatile uint32_t TIM10LPEN   : 1;    /**< Bit 17: TIM10 clock enable during sleep mode */
    volatile uint32_t TIM11LPEN   : 1;    /**< Bit 18: TIM11 clock enable during sleep mode */
    volatile uint32_t Reserved5   : 13;   /**< Bits 19-31: Reserved bits */
}RCC_APB2LPENR_t;

/**
 * Union to access RCC_APB2LPENR as either complete 32-bit value or individual bit fields
 * @author Eng.Gemy
 */
typedef union
{
    volatile uint32_t ALL_FIELDS; /**< Access entire 32-bit register at once */
    RCC_APB2LPENR_t BIT_FIELDS;   /**< Access individual bit fields by name */
}RCC_APB2LPENR_REG_t;

/******************************************************************************
 *                   RCC BACKUP DOMAIN CONTROL REGISTER (BDCR) STRUCTURES
 * @brief RCC Backup Domain Control Register
 * @details This register controls LSE oscillator and RTC clock selection
 * @note Register offset from RCC base: 0x70
 * @note This register is in the backup domain (retained during standby/VBAT)
 * @author Eng.Gemy
 ******************************************************************************/
/**
 * RCC_BDCR_t Structure
 * RCC Backup Domain Control Register bit fields
 * This register controls LSE (Low Speed External) oscillator and RTC clock selection
 */
typedef struct
{
    volatile uint32_t LSEON       : 1;    /**< Bit 0: LSE (32.768 kHz) oscillator enable */
    volatile uint32_t LSERDY      : 1;    /**< Bit 1: LSE oscillator clock ready flag (read-only) */
    volatile uint32_t LSEBYP      : 1;    /**< Bit 2: LSE oscillator bypass (external clock input) */
    volatile uint32_t Reserved1   : 5;    /**< Bits 3-7: Reserved bits */
    volatile uint32_t RTCSEL      : 2;    /**< Bits 8-9: RTC clock source (00=No clock, 01=LSE, 10=LSI, 11=HSE/prescaler) */
    volatile uint32_t Reserved2   : 5;    /**< Bits 10-14: Reserved bits */
    volatile uint32_t RTCEN       : 1;    /**< Bit 15: RTC clock enable */
    volatile uint32_t BDRST       : 1;    /**< Bit 16: Backup domain software reset (resets RTC and LSE config) */
    volatile uint32_t Reserved3   : 15;   /**< Bits 17-31: Reserved bits */
}RCC_BDCR_t;

/**
 * Union to access RCC_BDCR as either complete 32-bit value or individual bit fields
 * @author Eng.Gemy
 */
typedef union
{
    volatile uint32_t ALL_FIELDS; /**< Access entire 32-bit register at once */
    RCC_BDCR_t BIT_FIELDS;        /**< Access individual bit fields by name */
}RCC_BDCR_REG_t;

/******************************************************************************
 *                   RCC CONTROL/STATUS REGISTER (CSR) STRUCTURES
 * @brief RCC Control/Status Register
 * @details This register contains LSI control and reset status flags
 * @note Register offset from RCC base: 0x74
 * @author Eng.Gemy
 ******************************************************************************/
/**
 * RCC_CSR_t Structure
 * RCC Control/Status Register bit fields
 * This register contains clock control and reset status information
 */
typedef struct 
{
    volatile uint32_t LSIRDYF     : 1;    /**< Bit 0: LSI (32 kHz internal) oscillator enable */
    volatile uint32_t LSERDYF     : 1;    /**< Bit 1: LSI oscillator ready flag (read-only) */
    volatile uint32_t Reserved1   : 22;   /**< Bits 2-23: Reserved bits */
    volatile uint32_t RMVF        : 1;    /**< Bit 24: Remove reset flags (write 1 to clear all reset flags) */
    volatile uint32_t BORRSTF     : 1;    /**< Bit 25: BOR (Brown Out Reset) flag - reset caused by power drop below threshold */
    volatile uint32_t PINRSTF     : 1;    /**< Bit 26: PIN reset flag - reset caused by NRST pin */
    volatile uint32_t PORRSTF     : 1;    /**< Bit 27: POR (Power On Reset) flag - reset caused by power-on */
    volatile uint32_t SFTRSTF     : 1;    /**< Bit 28: Software reset flag - reset caused by software reset */
    volatile uint32_t IWDGRSTF    : 1;    /**< Bit 29: IWDG (Independent WatchDog) reset flag */
    volatile uint32_t WWDGRSTF    : 1;    /**< Bit 30: WWDG (Window WatchDog) reset flag */
    volatile uint32_t LPWRRSTF    : 1;    /**< Bit 31: Low-power reset flag - reset from standby mode */
}RCC_CSR_t;

/**
 * Union to access RCC_CSR as either complete 32-bit value or individual bit fields
 * @author Eng.Gemy
 */
typedef union
{
    volatile uint32_t ALL_FIELDS; /**< Access entire 32-bit register at once */
    RCC_CSR_t BIT_FIELDS;         /**< Access individual bit fields by name */
}RCC_CSR_REG_t;

/******************************************************************************
 *                   RCC SPREAD SPECTRUM CLOCK GENERATION REGISTER (SSCGR) STRUCTURES
 * @brief RCC Spread Spectrum Clock Generation Register
 * @details This register configures spread spectrum for EMI reduction
 * @note Register offset from RCC base: 0x80
 * @note Spread spectrum reduces electromagnetic interference by modulating PLL frequency
 * @author Eng.Gemy
 ******************************************************************************/
/**
 * RCC_SSCGR_t Structure
 * RCC Spread Spectrum Clock Generation Register bit fields
 * This register configures spread spectrum modulation for reducing EMI (Electromagnetic Interference)
 */
typedef struct 
{
    volatile uint32_t MODPER  : 13;       /**< Bits 0-12: Modulation period - controls spread spectrum period */
    volatile uint32_t INCSTEP : 15;       /**< Bits 13-27: Increment step - controls frequency deviation per step */
    volatile uint32_t Reserved : 2;       /**< Bits 28-29: Reserved bits */
    volatile uint32_t SPREADSEL : 1;      /**< Bit 30: Spread select (0=center spread, 1=down spread) */
    volatile uint32_t SSCGEN    : 1;      /**< Bit 31: Spread spectrum modulation enable */
}
RCC_SSCGR_t;

/**
 * Union to access RCC_SSCGR as either complete 32-bit value or individual bit fields
 * @author Eng.Gemy
 */
typedef union
{
    volatile uint32_t ALL_FIELDS; /**< Access entire 32-bit register at once */
    RCC_SSCGR_t BIT_FIELDS;       /**< Access individual bit fields by name */
}RCC_SSCGR_REG_t;

/******************************************************************************
 *                   RCC PLLI2S CONFIGURATION REGISTER (PLLI2SCFGR) STRUCTURES
 * @brief RCC PLLI2S Configuration Register
 * @details This register configures PLLI2S for I2S audio interface
 * @note Register offset from RCC base: 0x84
 * @note PLLI2S formula: PLLI2S_output = ((Input_freq / PLLM) * PLLI2SN) / PLLI2SR
 * @author Eng.Gemy
 ******************************************************************************/
/**
 * RCC_PLLI2SCFGR_t Structure
 * RCC PLLI2S Configuration Register bit fields
 * This register configures the PLLI2S (PLL for I2S) clock generator
 */
typedef struct 
{
    volatile uint32_t Reserved1 : 6;      /**< Bits 0-5: Reserved bits (PLLM is shared with main PLL) */
    volatile uint32_t PLLI2SN   : 9;      /**< Bits 6-14: PLLI2S multiplication factor N (50-432) */
    volatile uint32_t Reserved2 : 13;     /**< Bits 15-27: Reserved bits */
    volatile uint32_t PLLI2SR   : 3;      /**< Bits 28-30: PLLI2S division factor R (2-7) for I2S clock */
    volatile uint32_t Reserved3 : 1;      /**< Bit 31: Reserved bit */
}
RCC_PLLI2SCFGR_t;

/**
 * Union to access RCC_PLLI2SCFGR as either complete 32-bit value or individual bit fields
 * @author Eng.Gemy
 */
typedef union
{
    volatile uint32_t ALL_FIELDS; /**< Access entire 32-bit register at once */
    RCC_PLLI2SCFGR_t BIT_FIELDS;  /**< Access individual bit fields by name */
}RCC_PLLI2SCFGR_REG_t;

/******************************************************************************
 *                   RCC DEDICATED CLOCKS CONFIGURATION REGISTER (DCKCFGR) STRUCTURES
 * @brief RCC Dedicated Clocks Configuration Register
 * @details This register configures dedicated clock dividers
 * @note Register offset from RCC base: 0x8C
 * @author Eng.Gemy
 ******************************************************************************/
/**
 * RCC_DCKCFGR_t Structure
 * RCC Dedicated Clocks Configuration Register bit fields
 * This register configures specific clock dividers for dedicated peripherals
 */
typedef struct 
{
    volatile uint32_t Reserved1  : 24;    /**< Bits 0-23: Reserved bits */
    volatile uint32_t TIMPRE     : 1;     /**< Bit 24: Timer clock prescaler (0=standard, 1=x2/x4 depending on APB prescaler) */
    volatile uint32_t Reserved2  : 7;     /**< Bits 25-31: Reserved bits */
}
RCC_DCKCFGR_t;

/**
 * Union to access RCC_DCKCFGR as either complete 32-bit value or individual bit fields
 * @author Eng.Gemy
 */
typedef union
{
    volatile uint32_t ALL_FIELDS; /**< Access entire 32-bit register at once */
    RCC_DCKCFGR_t BIT_FIELDS;     /**< Access individual bit fields by name */
}RCC_DCKCFGR_REG_t;

/******************************************************************************
 *                   RCC COMPLETE REGISTER STRUCTURE
 * @brief Complete RCC peripheral register map
 * @details This structure maps all RCC registers in their exact hardware order
 * @note Base address: 0x40023800
 * @note Total register space: 0x90 bytes
 * @author Eng.Gemy
 ******************************************************************************/
typedef struct 
{
    RCC_CR_REG_t          CR;             /**< Offset 0x00: RCC Clock Control Register */
    RCC_PLLCFGR_REG_t     PLLCFGR;        /**< Offset 0x04: RCC PLL Configuration Register */
    RCC_CFGR_REG_t       CFGR;            /**< Offset 0x08: RCC Clock Configuration Register */
    RCC_CIR_REG_t        CIR;             /**< Offset 0x0C: RCC Clock Interrupt Register */
    RCC_AHB1RSTR_REG_t   AHB1RSTR;        /**< Offset 0x10: RCC AHB1 Peripheral Reset Register */
    RCC_AHB2RSTR_REG_t   AHB2RSTR;        /**< Offset 0x14: RCC AHB2 Peripheral Reset Register */
    uint32_t             Reserved1[2];    /**< Offset 0x18-0x1C: Reserved space (AHB3RSTR not present in STM32F401) */
    RCC_APB1RSTR_REG_t   APB1RSTR;        /**< Offset 0x20: RCC APB1 Peripheral Reset Register */
    RCC_APB2RSTR_REG_t   APB2RSTR;        /**< Offset 0x24: RCC APB2 Peripheral Reset Register */
    uint32_t             Reserved2[2];    /**< Offset 0x28-0x2C: Reserved space */
    RCC_AHB1ENR_REG_t    AHB1ENR;         /**< Offset 0x30: RCC AHB1 Peripheral Clock Enable Register */
    RCC_AHB2ENR_REG_t    AHB2ENR;         /**< Offset 0x34: RCC AHB2 Peripheral Clock Enable Register */
    uint32_t             Reserved3[2];    /**< Offset 0x38-0x3C: Reserved space (AHB3ENR not present) */
    RCC_APB1ENR_REG_t    APB1ENR;         /**< Offset 0x40: RCC APB1 Peripheral Clock Enable Register */
    RCC_APB2ENR_REG_t    APB2ENR;         /**< Offset 0x44: RCC APB2 Peripheral Clock Enable Register */
    uint32_t             Reserved4[2];    /**< Offset 0x48-0x4C: Reserved space */
    RCC_AHB1LPENR_REG_t  AHB1LPENR;       /**< Offset 0x50: RCC AHB1 Clock Enable in Low Power Mode Register */
    RCC_AHB2LPENR_REG_t  AHB2LPENR;       /**< Offset 0x54: RCC AHB2 Clock Enable in Low Power Mode Register */
    uint32_t             Reserved5[2];    /**< Offset 0x58-0x5C: Reserved space (AHB3LPENR not present) */
    RCC_APB1LPENR_REG_t  APB1LPENR;       /**< Offset 0x60: RCC APB1 Clock Enable in Low Power Mode Register */
    RCC_APB2LPENR_REG_t  APB2LPENR;       /**< Offset 0x64: RCC APB2 Clock Enable in Low Power Mode Register */
    uint32_t             Reserved6[2];    /**< Offset 0x68-0x6C: Reserved space */
    RCC_BDCR_REG_t      BDCR;             /**< Offset 0x70: RCC Backup Domain Control Register */
    RCC_CSR_REG_t       CSR;              /**< Offset 0x74: RCC Control/Status Register */
    uint32_t             Reserved7[2];    /**< Offset 0x78-0x7C: Reserved space */
    RCC_SSCGR_REG_t     SSCGR;            /**< Offset 0x80: RCC Spread Spectrum Clock Generation Register */
    RCC_PLLI2SCFGR_REG_t PLLI2SCFGR;      /**< Offset 0x84: RCC PLLI2S Configuration Register */
    uint32_t             Reserved8;       /**< Offset 0x88: Reserved space (PLLSAI not present in STM32F401) */
    RCC_DCKCFGR_REG_t  DCKCFGR;           /**< Offset 0x8C: RCC Dedicated Clocks Configuration Register */
}RCC_Registers_t;


/******************************************************************************
 *                   RCC PERIPHERAL POINTER DEFINITION
 * @brief Pointer to RCC peripheral registers
 * @details This pointer provides direct access to RCC hardware registers
 *          by casting the RCC base address to the RCC_Registers_t structure
 * @warning Do not modify this pointer - it points to hardware registers
 * @author Eng.Gemy
 * @example
 *     // Enable GPIOA clock:
 *     RCC_Registers->AHB1ENR.BIT_FIELDS.GPIOAEN = 1;
 *     
 *     // Set system clock to PLL:
 *     RCC_Registers->CFGR.BIT_FIELDS.SW = 2;
 ******************************************************************************/
RCC_Registers_t *RCC_Registers = (RCC_Registers_t *)RCC_BASE_ADDRESS; /**< Base address of RCC peripheral */




#endif  // RCC_PRIV_H - End of header guard

/******************************************************************************
 *                           END OF FILE
 * @author Eng.Gemy
 * 
 * @note RCC Private Header Summary:
 *       ✓ All RCC register structures defined with bit-field access
 *       ✓ Unions for flexible register access (bit-field or 32-bit word)
 *       ✓ Timeout values for clock stabilization
 *       ✓ Validation masks for parameter checking
 *       ✓ Prescaler configuration masks
 *       ✓ Complete register map matching STM32F401 hardware layout
 *       ✓ Reserved spaces properly aligned for correct memory mapping
 * 
 * @warning Important Notes:
 *          1. This file is for internal driver use only
 *          2. Register structures match STM32F401 hardware exactly
 *          3. Reserved fields maintain proper register alignment
 *          4. Bit-field order depends on compiler (little-endian assumed)
 *          5. RCC_Registers pointer must not be modified
 * 
 * @note Register Access Methods:
 *       - Bit-field access: RCC_Registers->CR.BIT_FIELDS.HSION = 1;
 *       - Direct access: RCC_Registers->CR.ALL_FIELDS |= (1 << 0);
 *       
 * @note Memory Map Verification:
 *       Base Address: 0x40023800
 *       ├── 0x00: CR        (Control Register)
 *       ├── 0x04: PLLCFGR   (PLL Configuration)
 *       ├── 0x08: CFGR      (Clock Configuration)
 *       ├── 0x0C: CIR       (Clock Interrupt)
 *       ├── 0x10: AHB1RSTR  (AHB1 Reset)
 *       ├── 0x14: AHB2RSTR  (AHB2 Reset)
 *       ├── 0x18-0x1C: Reserved
 *       ├── 0x20: APB1RSTR  (APB1 Reset)
 *       ├── 0x24: APB2RSTR  (APB2 Reset)
 *       ├── 0x28-0x2C: Reserved
 *       ├── 0x30: AHB1ENR   (AHB1 Clock Enable)
 *       ├── 0x34: AHB2ENR   (AHB2 Clock Enable)
 *       ├── 0x38-0x3C: Reserved
 *       ├── 0x40: APB1ENR   (APB1 Clock Enable)
 *       ├── 0x44: APB2ENR   (APB2 Clock Enable)
 *       ├── 0x48-0x4C: Reserved
 *       ├── 0x50: AHB1LPENR (AHB1 Low Power Enable)
 *       ├── 0x54: AHB2LPENR (AHB2 Low Power Enable)
 *       ├── 0x58-0x5C: Reserved
 *       ├── 0x60: APB1LPENR (APB1 Low Power Enable)
 *       ├── 0x64: APB2LPENR (APB2 Low Power Enable)
 *       ├── 0x68-0x6C: Reserved
 *       ├── 0x70: BDCR      (Backup Domain Control)
 *       ├── 0x74: CSR       (Control/Status)
 *       ├── 0x78-0x7C: Reserved
 *       ├── 0x80: SSCGR     (Spread Spectrum Clock Generation)
 *       ├── 0x84: PLLI2SCFGR (PLLI2S Configuration)
 *       ├── 0x88: Reserved
 *       └── 0x8C: DCKCFGR   (Dedicated Clocks Configuration)
 * 
 * @note Clock Tree Overview:
 *       External Sources:
 *       ├── HSE (4-26 MHz crystal/oscillator)
 *       └── LSE (32.768 kHz crystal)
 *       
 *       Internal Sources:
 *       ├── HSI (16 MHz RC oscillator)
 *       └── LSI (32 kHz RC oscillator)
 *       
 *       PLL (Phase-Locked Loop):
 *       ├── Input: HSI or HSE
 *       ├── VCO = (Input / PLLM) × PLLN
 *       ├── System Clock = VCO / PLLP
 *       └── USB/SDIO = VCO / PLLQ (must be 48 MHz)
 *       
 *       System Clock Path:
 *       SYSCLK (HSI/HSE/PLL)
 *       └── AHB Prescaler (÷1,2,4,8,16,64,128,256,512)
 *           ├── AHB Bus (CPU, Memory, DMA)
 *           ├── APB1 Prescaler (÷1,2,4,8,16) → APB1 Bus (max 45 MHz)
 *           └── APB2 Prescaler (÷1,2,4,8,16) → APB2 Bus (max 90 MHz)
 * 
 * @note Typical Clock Configuration Sequence:
 *       1. Enable HSE oscillator
 *       2. Wait for HSE ready flag (with timeout)
 *       3. Configure Flash latency based on target frequency
 *       4. Configure PLL parameters (M, N, P, Q)
 *       5. Enable PLL
 *       6. Wait for PLL ready flag (with timeout)
 *       7. Configure AHB/APB1/APB2 prescalers
 *       8. Switch system clock to PLL
 *       9. Verify system clock switch status
 *       10. Optionally disable unused oscillators (HSI)
 * 
 * @note Power Consumption Optimization:
 *       - Disable unused clock sources (HSE, HSI, PLL)
 *       - Disable unused peripheral clocks
 *       - Use lower system clock frequency when possible
 *       - Enable clock gating in low-power modes (LPENR registers)
 *       - Use LSI instead of LSE if precision not required
 * 
 * @note Common PLL Configurations for STM32F401:
 *       
 *       84 MHz from 16 MHz HSI:
 *       PLLM=16, PLLN=336, PLLP=4, PLLQ=7
 *       VCO = (16MHz / 16) × 336 = 336 MHz
 *       SYSCLK = 336 MHz / 4 = 84 MHz
 *       USB = 336 MHz / 7 = 48 MHz
 *       
 *       84 MHz from 8 MHz HSE:
 *       PLLM=8, PLLN=336, PLLP=4, PLLQ=7
 *       VCO = (8MHz / 8) × 336 = 336 MHz
 *       SYSCLK = 336 MHz / 4 = 84 MHz
 *       USB = 336 MHz / 7 = 48 MHz
 *       
 *       84 MHz from 25 MHz HSE:
 *       PLLM=25, PLLN=336, PLLP=4, PLLQ=7
 *       VCO = (25MHz / 25) × 336 = 336 MHz
 *       SYSCLK = 336 MHz / 4 = 84 MHz
 *       USB = 336 MHz / 7 = 48 MHz
 * 
 * @note Reset Sources (CSR Register Flags):
 *       - LPWRRSTF: Low-power management reset
 *       - WWDGRSTF: Window watchdog reset
 *       - IWDGRSTF: Independent watchdog reset
 *       - SFTRSTF: Software reset
 *       - PORRSTF: Power-on/power-down reset
 *       - PINRSTF: NRST pin reset
 *       - BORRSTF: Brown-out reset
 * 
 * @note Peripheral Clock Enable Procedure:
 *       1. Enable clock source (HSI/HSE/PLL)
 *       2. Configure system clock and prescalers
 *       3. Enable peripheral clock in appropriate ENR register
 *       4. Peripheral is now ready to use
 *       5. Optionally configure low-power clock enable (LPENR)
 * 
 * @note Peripheral Reset Procedure:
 *       1. Set peripheral reset bit in appropriate RSTR register (write 1)
 *       2. Clear peripheral reset bit (write 0)
 *       3. Peripheral registers are now at reset values
 *       4. Peripheral clock remains enabled (if previously enabled)
 *       5. Reconfigure peripheral as needed
 * 
 * @warning Critical Safety Notes:
 *          ⚠ Never disable clock source while it's selected as SYSCLK
 *          ⚠ Never disable PLL while it's selected as SYSCLK
 *          ⚠ Always configure Flash latency before increasing frequency
 *          ⚠ Ensure APB1 ≤ 45 MHz and APB2 ≤ 90 MHz (STM32F401)
 *          ⚠ Ensure VCO frequency is within valid range (100-432 MHz)
 *          ⚠ PLL must be disabled before changing PLLCFGR
 *          ⚠ USB/SDIO requires exactly 48 MHz from PLLQ
 *          ⚠ Use timeout mechanisms to prevent infinite loops
 * 
 * @note Debug Tips:
 *       - Use SWS field to verify active system clock source
 *       - Monitor ready flags before switching clocks
 *       - Check CSS flag for HSE failure detection
 *       - Use MCO1/MCO2 pins to output clocks for measurement
 *       - Verify prescaler settings affect peripheral operation
 *       - Check reset flags in CSR to determine reset cause
 * 
 * @version History:
 *          1.0 - Initial release (Eng.Gemy)
 *              - Complete register definitions
 *              - All bit-field structures
 *              - Validation masks
 *              - Comprehensive documentation
 ******************************************************************************/