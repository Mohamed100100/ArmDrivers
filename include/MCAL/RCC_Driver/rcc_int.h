/******************************************************************************
 * @file    RCC_INT.H
 * @author  Eng.Gemy
 * @brief   RCC (Reset and Clock Control) Driver Interface Header File
 *          This file contains all the function prototypes, type definitions,
 *          and macros for the RCC driver to configure system clocks and
 *          peripheral clocks on STM32 microcontrollers
 * @note    RCC is responsible for:
 *          - System clock configuration (HSI/HSE/PLL)
 *          - Peripheral clock enable/disable
 *          - Clock prescaler configuration
 *          - Reset management
 ******************************************************************************/

#ifndef RCC_INT_H
#define RCC_INT_H

/******************************************************************************
 *                        PLL SOURCE SELECTION
 * @brief Macros to select PLL input clock source
 * @author Eng.Gemy
 ******************************************************************************/
#define RCC_PLL_SOURCE_HSI   0U     /**< PLL source is HSI (High Speed Internal) oscillator */
#define RCC_PLL_SOURCE_HSE   1U     /**< PLL source is HSE (High Speed External) oscillator */


/******************************************************************************
 *                        BUS SELECTION MASKS
 * @brief Macros to identify which bus a peripheral is connected to
 * @details These masks are used to select the appropriate bus for peripheral
 *          clock enable/disable operations
 * @author Eng.Gemy
 ******************************************************************************/
#define RCC_AHB1_BUS    0b0001U     /**< AHB1 Bus identifier (Advanced High-performance Bus 1) */
#define RCC_AHB2_BUS    0b0010U     /**< AHB2 Bus identifier (Advanced High-performance Bus 2) */
#define RCC_APB1_BUS    0b0100U     /**< APB1 Bus identifier (Advanced Peripheral Bus 1 - Low speed) */
#define RCC_APB2_BUS    0b1000U     /**< APB2 Bus identifier (Advanced Peripheral Bus 2 - High speed) */

/******************************************************************************
 *                   AHB1 PERIPHERAL CLOCK ENABLE MASKS
 * @brief Clock enable masks for peripherals on AHB1 bus
 * @details Bit[35:32] encodes bus type (0b0001 = AHB1)
 *          Bit[31:0] encodes peripheral bit position in AHB1ENR register
 * @note Format: 0bBBBBPPPPPPPPPPPPPPPPPPPPPPPPPPPP
 *               BBBB = Bus identifier (0b0001 for AHB1)
 *               P... = Peripheral bit position
 * @author Eng.Gemy
 ******************************************************************************/
// Define peripheral clock enable masks for RCC AHB
//                                                      0b012310987654321098765432109876543210
#define RCC_AHB1_GPIOA_CLOCK                 (uint64_t)(0b000100000000000000000000000000000001ULL)  /**< GPIOA clock on AHB1 (bit 0) */
#define RCC_AHB1_GPIOB_CLOCK                 (uint64_t)(0b000100000000000000000000000000000010ULL)  /**< GPIOB clock on AHB1 (bit 1) */
#define RCC_AHB1_GPIOC_CLOCK                 (uint64_t)(0b000100000000000000000000000000000100ULL)  /**< GPIOC clock on AHB1 (bit 2) */
#define RCC_AHB1_GPIOD_CLOCK                 (uint64_t)(0b000100000000000000000000000000001000ULL)  /**< GPIOD clock on AHB1 (bit 3) */
#define RCC_AHB1_GPIOE_CLOCK                 (uint64_t)(0b000100000000000000000000000000010000ULL)  /**< GPIOE clock on AHB1 (bit 4) */
#define RCC_AHB1_GPIOH_CLOCK                 (uint64_t)(0b000100000000000000000000000010000000ULL)  /**< GPIOH clock on AHB1 (bit 7) */
#define RCC_AHB1_CRC_CLOCK                   (uint64_t)(0b000100000000000000000001000000000000ULL)  /**< CRC clock on AHB1 (bit 12) */
#define RCC_AHB1_DMA1_CLOCK                  (uint64_t)(0b000100000000001000000000000000000000ULL)  /**< DMA1 clock on AHB1 (bit 21) */
#define RCC_AHB1_DMA2_CLOCK                  (uint64_t)(0b000100000000010000000000000000000000ULL)  /**< DMA2 clock on AHB1 (bit 22) */

/******************************************************************************
 *                   AHB2 PERIPHERAL CLOCK ENABLE MASKS
 * @brief Clock enable masks for peripherals on AHB2 bus
 * @details Bit[35:32] encodes bus type (0b0010 = AHB2)
 *          Bit[31:0] encodes peripheral bit position in AHB2ENR register
 * @author Eng.Gemy
 ******************************************************************************/
// Define peripheral clock enable masks for RCC AHB2
//                                                      0b012310987654321098765432109876543210
#define RCC_AHB2_OTGFS_CLOCK                 (uint64_t)(0b001000000000000000000000000010000000ULL)  /**< USB OTG FS clock on AHB2 (bit 7) */

/******************************************************************************
 *                   APB1 PERIPHERAL CLOCK ENABLE MASKS
 * @brief Clock enable masks for peripherals on APB1 bus (Low-speed peripherals)
 * @details Bit[35:32] encodes bus type (0b0100 = APB1)
 *          Bit[31:0] encodes peripheral bit position in APB1ENR register
 * @note APB1 typically runs at lower frequency (max 45 MHz on STM32F4)
 * @author Eng.Gemy
 ******************************************************************************/
// Define peripheral clock enable masks for RCC APB1
//                                                     0b012310987654321098765432109876543210
#define RCC_APB1_TIMER2_CLOCK               (uint64_t)(0b010000000000000000000000000000000001ULL)  /**< Timer 2 clock on APB1 (bit 0) */
#define RCC_APB1_TIMER3_CLOCK               (uint64_t)(0b010000000000000000000000000000000010ULL)  /**< Timer 3 clock on APB1 (bit 1) */
#define RCC_APB1_TIMER4_CLOCK               (uint64_t)(0b010000000000000000000000000000000100ULL)  /**< Timer 4 clock on APB1 (bit 2) */
#define RCC_APB1_TIMER5_CLOCK               (uint64_t)(0b010000000000000000000000000000001000ULL)  /**< Timer 5 clock on APB1 (bit 3) */
#define RCC_APB1_WWDG_CLOCK                 (uint64_t)(0b010000000000000000000000100000000000ULL)  /**< Window Watchdog clock on APB1 (bit 11) */
#define RCC_APB1_SPI2_CLOCK                 (uint64_t)(0b010000000000000000000100000000000000ULL)  /**< SPI2 clock on APB1 (bit 14) */
#define RCC_APB1_SPI3_CLOCK                 (uint64_t)(0b010000000000000000001000000000000000ULL)  /**< SPI3 clock on APB1 (bit 15) */
#define RCC_APB1_USART2_CLOCK               (uint64_t)(0b010000000000000000100000000000000000ULL)  /**< USART2 clock on APB1 (bit 17) */
#define RCC_APB1_I2C1_CLOCK                 (uint64_t)(0b010000000000001000000000000000000000ULL)  /**< I2C1 clock on APB1 (bit 21) */
#define RCC_APB1_I2C2_CLOCK                 (uint64_t)(0b010000000000010000000000000000000000ULL)  /**< I2C2 clock on APB1 (bit 22) */
#define RCC_APB1_I2C3_CLOCK                 (uint64_t)(0b010000000000100000000000000000000000ULL)  /**< I2C3 clock on APB1 (bit 23) */
#define RCC_APB1_PWR_CLOCK                  (uint64_t)(0b010000010000000000000000000000000000ULL)  /**< Power interface clock on APB1 (bit 28) */

/******************************************************************************
 *                   APB2 PERIPHERAL CLOCK ENABLE MASKS
 * @brief Clock enable masks for peripherals on APB2 bus (High-speed peripherals)
 * @details Bit[35:32] encodes bus type (0b1000 = APB2)
 *          Bit[31:0] encodes peripheral bit position in APB2ENR register
 * @note APB2 typically runs at higher frequency than APB1
 * @author Eng.Gemy
 ******************************************************************************/
// Define peripheral clock enable masks for RCC APB2
//                                                     0b012310987654321098765432109876543210
#define RCC_APB2_TIMER1_CLOCK               (uint64_t)(0b100000000000000000000000000000000001ULL)  /**< Timer 1 clock on APB2 (bit 0) */
#define RCC_APB2_USART1_CLOCK               (uint64_t)(0b100000000000000000000000000000010000ULL)  /**< USART1 clock on APB2 (bit 4) */
#define RCC_APB2_USART6_CLOCK               (uint64_t)(0b100000000000000000000000000000100000ULL)  /**< USART6 clock on APB2 (bit 5) */
#define RCC_APB2_ADC1_CLOCK                 (uint64_t)(0b100000000000000000000000000100000000ULL)  /**< ADC1 clock on APB2 (bit 8) */
#define RCC_APB2_SDIO_CLOCK                 (uint64_t)(0b100000000000000000000000100000000000ULL)  /**< SDIO clock on APB2 (bit 11) */
#define RCC_APB2_SPI1_CLOCK                 (uint64_t)(0b100000000000000000000001000000000000ULL)  /**< SPI1 clock on APB2 (bit 12) */
#define RCC_APB2_SPI4_CLOCK                 (uint64_t)(0b100000000000000000000010000000000000ULL)  /**< SPI4 clock on APB2 (bit 13) */
#define RCC_APB2_SYSCFG_CLOCK               (uint64_t)(0b100000000000000000000100000000000000ULL)  /**< System configuration controller clock on APB2 (bit 14) */
#define RCC_APB2_TIMER9_CLOCK               (uint64_t)(0b100000000000000000010000000000000000ULL)  /**< Timer 9 clock on APB2 (bit 16) */
#define RCC_APB2_TIMER10_CLOCK              (uint64_t)(0b100000000000000000100000000000000000ULL)  /**< Timer 10 clock on APB2 (bit 17) */
#define RCC_APB2_TIMER11_CLOCK              (uint64_t)(0b100000000000000001000000000000000000ULL)  /**< Timer 11 clock on APB2 (bit 18) */



/******************************************************************************
 * @brief System Clock Source Enumeration
 * @details Defines available system clock sources for STM32
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
RCC_SYSCLK_HSI = 0,      /**< High Speed Internal oscillator (16 MHz typical) */
RCC_SYSCLK_HSE = 1,      /**< High Speed External oscillator (4-26 MHz external crystal) */
RCC_SYSCLK_PLL = 2       /**< Phase-Locked Loop (can multiply HSI/HSE to higher frequencies) */
}RCC_ClockSrc_t; 

/******************************************************************************
 * @brief AHB Prescaler Enumeration
 * @details Defines division factors for AHB clock derived from system clock
 * @note AHB clock is used by CPU core, memory, and DMA
 * @note Bit positions match CFGR register HPRE field encoding
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
//                       0b10987654321098765432109876543210
RCC_AHB_NO_DIVISION    = 0b00000000000000000000000000000000,  /**< SYSCLK not divided (AHB = SYSCLK) */
RCC_AHB_DIVIDED_BY_2   = 0b00000000000000000000000010000000,  /**< SYSCLK divided by 2 */
RCC_AHB_DIVIDED_BY_4   = 0b00000000000000000000000010010000,  /**< SYSCLK divided by 4 */
RCC_AHB_DIVIDED_BY_8   = 0b00000000000000000000000010100000,  /**< SYSCLK divided by 8 */
RCC_AHB_DIVIDED_BY_16  = 0b00000000000000000000000010110000,  /**< SYSCLK divided by 16 */
RCC_AHB_DIVIDED_BY_64  = 0b00000000000000000000000011000000,  /**< SYSCLK divided by 64 */
RCC_AHB_DIVIDED_BY_128 = 0b00000000000000000000000011010000,  /**< SYSCLK divided by 128 */
RCC_AHB_DIVIDED_BY_256 = 0b00000000000000000000000011100000,  /**< SYSCLK divided by 256 */
RCC_AHB_DIVIDED_BY_512 = 0b00000000000000000000000011110000   /**< SYSCLK divided by 512 */
}RCC_AHPPrescaler_t;

/******************************************************************************
 * @brief APB1 Prescaler Enumeration
 * @details Defines division factors for APB1 clock derived from AHB clock
 * @note APB1 is the low-speed peripheral bus (max 45 MHz on STM32F4)
 * @note Bit positions match CFGR register PPRE1 field encoding
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
//                        0b10987654321098765432109876543210
RCC_APB1_NO_DIVISION    = 0b00000000000000000000000000000000,  /**< AHB clock not divided (APB1 = AHB) */
RCC_APB1_DIVIDED_BY_2   = 0b00000000000000000001000000000000,  /**< AHB clock divided by 2 */
RCC_APB1_DIVIDED_BY_4   = 0b00000000000000000001010000000000,  /**< AHB clock divided by 4 */
RCC_APB1_DIVIDED_BY_8   = 0b00000000000000000001100000000000,  /**< AHB clock divided by 8 */
RCC_APB1_DIVIDED_BY_16  = 0b00000000000000000001110000000000   /**< AHB clock divided by 16 */
}RCC_APB1Prescaler_t;

/******************************************************************************
 * @brief APB2 Prescaler Enumeration
 * @details Defines division factors for APB2 clock derived from AHB clock
 * @note APB2 is the high-speed peripheral bus (max 90 MHz on STM32F4)
 * @note Bit positions match CFGR register PPRE2 field encoding
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
//                        0b10987654321098765432109876543210
RCC_APB2_NO_DIVISION    = 0b00000000000000000000000000000000,  /**< AHB clock not divided (APB2 = AHB) */
RCC_APB2_DIVIDED_BY_2   = 0b00000000000000001000000000000000,  /**< AHB clock divided by 2 */
RCC_APB2_DIVIDED_BY_4   = 0b00000000000000001010000000000000,  /**< AHB clock divided by 4 */
RCC_APB2_DIVIDED_BY_8   = 0b00000000000000001100000000000000,  /**< AHB clock divided by 8 */
RCC_APB2_DIVIDED_BY_16  = 0b00000000000000001110000000000000   /**< AHB clock divided by 16 */
}RCC_APB2Prescaler_t;



/******************************************************************************
 * @brief RCC Status Enumeration
 * @details Defines all possible return status codes for RCC operations
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
    RCC_OK = 0,                                 /**< Operation completed successfully */
    RCC_NOT_OK,                                 /**< Operation failed */
    RCC_TIMEOUT,                                /**< Timeout occurred waiting for clock ready */
    RCC_WRONG_PLL_CONFIG,                       /**< Invalid PLL configuration parameters */
    RCC_PLL_ALREADY_ENABLED,                    /**< Cannot configure PLL while it's enabled */
    RCC_WRONG_BUS_SELECTION,                    /**< Invalid bus identifier */
    RCC_WRONG_PEREPHRAL_SELECTION,              /**< Invalid peripheral selection */
    RCC_WRONG_PEREPHRAL_WITHBUS_SELECTION,      /**< Peripheral and bus combination mismatch */
    RCC_PLL_ERROR_M,                            /**< Invalid PLL M divider value */
    RCC_PLL_ERROR_N,                            /**< Invalid PLL N multiplier value */
    RCC_PLL_ERROR_P,                            /**< Invalid PLL P divider value */
    RCC_PLL_ERROR_Q,                            /**< Invalid PLL Q divider value */
    RCC_PLL_ERROR_SOURCE,                       /**< Invalid PLL source selection */
    RCC_WRONG_SYSCLK_SOURCE,                    /**< Invalid system clock source */
    RCC_WRONG_AHB_PRESCALER,                    /**< Invalid AHB prescaler value */
    RCC_WRONG_APB_PRESCALER,                    /**< Invalid APB prescaler value */
    RCC_WRONG_CLOCK_SOURCE,                     /**< Invalid clock source */
    RCC_ERROR                                   /**< General RCC error */
}RCC_Status_t;

/******************************************************************************
 *                   GLOBAL CLOCK FREQUENCY VARIABLES
 * @brief Global variables to store clock source frequencies
 * @note These should be set by the application to match actual hardware
 * @author Eng.Gemy
 ******************************************************************************/
uint32_t RCC_HSI_ClockSourceValue;  /**< HSI oscillator frequency (typically 16 MHz) */
uint32_t RCC_HSE_ClockSourceValue;  /**< HSE oscillator frequency (external crystal, e.g., 8 MHz, 25 MHz) */

/******************************************************************************
 *                   HSI (HIGH SPEED INTERNAL) FUNCTIONS
 * @brief Functions to control HSI oscillator
 * @note HSI is the default clock source after reset (16 MHz ±1% accuracy)
 * @author Eng.Gemy
 ******************************************************************************/

/**
 * @brief Enable HSI oscillator
 * @return RCC_Status_t Status of the operation
 * @retval RCC_OK       HSI enabled successfully
 * @retval RCC_TIMEOUT  Timeout waiting for HSI ready
 */
RCC_Status_t RCC_EnableHSI(void);

/**
 * @brief Disable HSI oscillator
 * @return RCC_Status_t Status of the operation
 * @retval RCC_OK       HSI disabled successfully
 * @note Cannot disable HSI if it's the current system clock source
 */
RCC_Status_t RCC_DisableHSI(void);

/**
 * @brief Check if HSI oscillator is ready
 * @return uint8_t HSI ready status
 * @retval 1 HSI is ready
 * @retval 0 HSI is not ready
 */
uint8_t RCC_IsHSIReady(void);

/******************************************************************************
 *                   HSE (HIGH SPEED EXTERNAL) FUNCTIONS
 * @brief Functions to control HSE oscillator
 * @note HSE requires external crystal or oscillator (typically 4-26 MHz)
 * @author Eng.Gemy
 ******************************************************************************/

/**
 * @brief Enable HSE oscillator
 * @return RCC_Status_t Status of the operation
 * @retval RCC_OK       HSE enabled successfully
 * @retval RCC_TIMEOUT  Timeout waiting for HSE ready
 * @note Requires external crystal or oscillator to be connected
 */
RCC_Status_t RCC_EnableHSE(void);

/**
 * @brief Disable HSE oscillator
 * @return RCC_Status_t Status of the operation
 * @retval RCC_OK       HSE disabled successfully
 * @note Cannot disable HSE if it's the current system clock source or PLL source
 */
RCC_Status_t RCC_DisableHSE(void);

/**
 * @brief Check if HSE oscillator is ready
 * @return uint8_t HSE ready status
 * @retval 1 HSE is ready
 * @retval 0 HSE is not ready
 */
uint8_t RCC_IsHSEReady(void);


/******************************************************************************
 *                   PLL (PHASE-LOCKED LOOP) FUNCTIONS
 * @brief Functions to control and configure PLL
 * @note PLL formula: VCO = (Input_Clock / PLLM) * PLLN
 *                    PLL_Output = VCO / PLLP
 *                    USB_Clock = VCO / PLLQ
 * @warning PLL must be disabled before configuration
 * @author Eng.Gemy
 ******************************************************************************/

/**
 * @brief Configure PLL parameters
 * @details This function configures all PLL multiplication and division factors
 * 
 * @param[in] Copy_PLLM    PLL M divider (2-63)
 *                         Divides input clock to VCO input (must be 1-2 MHz)
 * @param[in] Copy_PLLN    PLL N multiplier (50-432)
 *                         Multiplies VCO input (VCO output must be 100-432 MHz)
 * @param[in] Copy_PLLP    PLL P divider (2, 4, 6, or 8)
 *                         Divides VCO output for system clock
 * @param[in] Copy_PLLQ    PLL Q divider (2-15)
 *                         Divides VCO output for USB/SDIO (must be 48 MHz for USB)
 * @param[in] Copy_PLLSource PLL input source (RCC_PLL_SOURCE_HSI or RCC_PLL_SOURCE_HSE)
 * 
 * @return RCC_Status_t Status of the operation
 * @retval RCC_OK                       Configuration successful
 * @retval RCC_PLL_ALREADY_ENABLED      PLL is currently enabled (disable first)
 * @retval RCC_PLL_ERROR_M              Invalid PLLM value
 * @retval RCC_PLL_ERROR_N              Invalid PLLN value
 * @retval RCC_PLL_ERROR_P              Invalid PLLP value
 * @retval RCC_PLL_ERROR_Q              Invalid PLLQ value
 * @retval RCC_PLL_ERROR_SOURCE         Invalid PLL source
 * 
 * @note PLL must be disabled before calling this function
 * @warning Ensure PLL output frequency does not exceed maximum allowed (168 MHz for STM32F401)
 * 
 * @example For 84 MHz system clock from 16 MHz HSI:
 *          PLLM=16, PLLN=336, PLLP=4, PLLQ=7, Source=HSI
 *          VCO = (16MHz / 16) * 336 = 336 MHz
 *          System = 336 MHz / 4 = 84 MHz
 *          USB = 336 MHz / 7 = 48 MHz
 */
RCC_Status_t RCC_ConfigurePLL(uint8_t Copy_PLLM, uint16_t Copy_PLLN, uint8_t Copy_PLLP, uint8_t Copy_PLLQ, uint8_t Copy_PLLSource);

/**
 * @brief Enable PLL
 * @return RCC_Status_t Status of the operation
 * @retval RCC_OK       PLL enabled successfully
 * @retval RCC_TIMEOUT  Timeout waiting for PLL ready
 * @note PLL must be configured before enabling
 */
RCC_Status_t RCC_EnablePLL(void);

/**
 * @brief Disable PLL
 * @return RCC_Status_t Status of the operation
 * @retval RCC_OK       PLL disabled successfully
 * @note Cannot disable PLL if it's the current system clock source
 */
RCC_Status_t RCC_DisablePLL(void);

/**
 * @brief Check if PLL is ready
 * @return uint8_t PLL ready status
 * @retval 1 PLL is locked and ready
 * @retval 0 PLL is not ready
 */
uint8_t RCC_IsPLLReady(void);

/******************************************************************************
 *                   SYSTEM CLOCK CONFIGURATION FUNCTIONS
 * @brief Functions to configure system clock source and prescalers
 * @author Eng.Gemy
 ******************************************************************************/

/**
 * @brief Set system clock source
 * @details This function switches the system clock to the specified source
 * 
 * @param[in] clockSource Desired clock source (RCC_SYSCLK_HSI/HSE/PLL)
 * 
 * @return RCC_Status_t Status of the operation
 * @retval RCC_OK                   System clock switched successfully
 * @retval RCC_WRONG_SYSCLK_SOURCE  Invalid clock source
 * @retval RCC_TIMEOUT              Timeout waiting for clock switch
 * 
 * @note Ensure the target clock source is enabled and ready before switching
 * @warning Flash latency must be configured appropriately for higher frequencies
 */
RCC_Status_t RCC_SetSysClock(RCC_ClockSrc_t clockSource);

/**
 * @brief Get current system clock source
 * @details This function reads which clock is currently used as system clock
 * 
 * @param[out] Pointer to store current system clock source
 * 
 * @return RCC_Status_t Status of the operation
 * @retval RCC_OK       Successfully read current system clock
 * @retval RCC_NOT_OK   Failed to read or invalid pointer
 */
RCC_Status_t RCC_GetSystemClockSource(RCC_ClockSrc_t *);

/**
 * @brief Set AHB prescaler
 * @details Configures the AHB clock frequency by dividing system clock
 * 
 * @param[in] AHBPrescaler AHB division factor (from RCC_AHPPrescaler_t enum)
 * 
 * @return RCC_Status_t Status of the operation
 * @retval RCC_OK                   AHB prescaler set successfully
 * @retval RCC_WRONG_AHB_PRESCALER  Invalid prescaler value
 * 
 * @note AHB clock drives: CPU core, memory, DMA, and core peripherals
 * @note Maximum AHB frequency: 84 MHz (STM32F401) or 168 MHz (STM32F4xx)
 */
RCC_Status_t RCC_SetAHBPrescaler(RCC_AHPPrescaler_t AHBPrescaler);

/**
 * @brief Set APB1 prescaler
 * @details Configures the APB1 clock frequency by dividing AHB clock
 * 
 * @param[in] APBP1rescaler APB1 division factor (from RCC_APB1Prescaler_t enum)
 * 
 * @return RCC_Status_t Status of the operation
 * @retval RCC_OK                   APB1 prescaler set successfully
 * @retval RCC_WRONG_APB_PRESCALER  Invalid prescaler value
 * 
 * @note APB1 is the low-speed peripheral bus
 * @note Maximum APB1 frequency: 42 MHz (STM32F401) or 45 MHz (STM32F4xx)
 * @note APB1 peripherals: TIM2-5, USART2-3, I2C1-3, SPI2-3, etc.
 */
RCC_Status_t RCC_SetAPB1Prescaler(RCC_APB1Prescaler_t APBP1rescaler);

/**
 * @brief Set APB2 prescaler
 * @details Configures the APB2 clock frequency by dividing AHB clock
 * 
 * @param[in] APBP2rescaler APB2 division factor (from RCC_APB2Prescaler_t enum)
 * 
 * @return RCC_Status_t Status of the operation
 * @retval RCC_OK                   APB2 prescaler set successfully
 * @retval RCC_WRONG_APB_PRESCALER  Invalid prescaler value
 * 
 * @note APB2 is the high-speed peripheral bus
 * @note Maximum APB2 frequency: 84 MHz (STM32F401) or 90 MHz (STM32F4xx)
 * @note APB2 peripherals: TIM1, TIM9-11, USART1, USART6, SPI1, SPI4, ADC, SDIO, SYSCFG
 */
RCC_Status_t RCC_SetAPB2Prescaler(RCC_APB2Prescaler_t APBP2rescaler);

/******************************************************************************
 *                   PERIPHERAL CLOCK CONTROL FUNCTIONS
 * @brief Functions to enable/disable/reset peripheral clocks
 * @note Peripherals must have their clocks enabled before use
 * @author Eng.Gemy
 ******************************************************************************/

/**
 * @brief Enable peripheral clock
 * @details This function enables clock for specified peripheral(s) on a bus
 * 
 * @param[in] bus                  Bus identifier (RCC_AHB1_BUS/AHB2_BUS/APB1_BUS/APB2_BUS)
 * @param[in] PeripheralClockMask  64-bit mask containing bus and peripheral information
 *                                 Upper 4 bits: Bus type
 *                                 Lower 32 bits: Peripheral bit position
 * 
 * @return RCC_Status_t Status of the operation
 * @retval RCC_OK                                  Clock enabled successfully
 * @retval RCC_WRONG_BUS_SELECTION                 Invalid bus identifier
 * @retval RCC_WRONG_PEREPHRAL_SELECTION           Invalid peripheral mask
 * @retval RCC_WRONG_PEREPHRAL_WITHBUS_SELECTION   Peripheral-bus mismatch
 * 
 * @note Multiple peripherals can be enabled by ORing their masks
 * @example Enable GPIOA and GPIOB:
 *          RCC_EnablePeripheralClock(RCC_AHB1_BUS, 
 *                                    RCC_AHB1_GPIOA_CLOCK | RCC_AHB1_GPIOB_CLOCK);
 */
RCC_Status_t RCC_EnablePeripheralClock(uint8_t bus,uint64_t PeripheralClockMask);

/**
 * @brief Disable peripheral clock
 * @details This function disables clock for specified peripheral(s) on a bus
 * 
 * @param[in] bus                  Bus identifier (RCC_AHB1_BUS/AHB2_BUS/APB1_BUS/APB2_BUS)
 * @param[in] PeripheralClockMask  64-bit mask containing bus and peripheral information
 * 
 * @return RCC_Status_t Status of the operation
 * @retval RCC_OK                                  Clock disabled successfully
 * @retval RCC_WRONG_BUS_SELECTION                 Invalid bus identifier
 * @retval RCC_WRONG_PEREPHRAL_SELECTION           Invalid peripheral mask
 * @retval RCC_WRONG_PEREPHRAL_WITHBUS_SELECTION   Peripheral-bus mismatch
 * 
 * @note Disabling peripheral clock reduces power consumption
 * @warning Ensure peripheral is not in use before disabling its clock
 */
RCC_Status_t RCC_DisablePeripheralClock(uint8_t bus,uint64_t PeripheralClockMask);

/**
 * @brief Reset all peripheral clocks
 * @details This function resets all peripherals on all buses to their default state
 * 
 * @return void
 * 
 * @note This performs a hardware reset on all peripheral registers
 * @warning This will reset ALL peripherals - use with caution
 * @warning After reset, peripheral clocks remain enabled but peripheral configuration is lost
 */
void RCC_ResetALLPeripheralClock(void);

/**
 * @brief Reset specific peripheral clock
 * @details This function performs a hardware reset on specified peripheral(s)
 * 
 * @param[in] bus                  Bus identifier (RCC_AHB1_BUS/AHB2_BUS/APB1_BUS/APB2_BUS)
 * @param[in] PeripheralClockMask  64-bit mask containing bus and peripheral information
 * 
 * @return RCC_Status_t Status of the operation
 * @retval RCC_OK                                  Reset successful
 * @retval RCC_WRONG_BUS_SELECTION                 Invalid bus identifier
 * @retval RCC_WRONG_PEREPHRAL_SELECTION           Invalid peripheral mask
 * @retval RCC_WRONG_PEREPHRAL_WITHBUS_SELECTION   Peripheral-bus mismatch
 * 
 * @note Reset is performed by setting then clearing the reset bit
 * @note Peripheral configuration registers return to reset values
 * @note Peripheral clock remains enabled after reset
 * 
 * @example Reset USART2:
 *          RCC_ResetPeripheralClock(RCC_APB1_BUS, RCC_APB1_USART2_CLOCK);
 */
RCC_Status_t RCC_ResetPeripheralClock(uint8_t bus,uint64_t PeripheralClockMask);








#endif  // RCC_INT_H - End of header guard

/******************************************************************************
 *                           END OF FILE
 * @author Eng.Gemy
 * 
 * @note RCC Driver Feature Summary:
 *       ✓ HSI oscillator control (16 MHz internal)
 *       ✓ HSE oscillator control (external crystal)
 *       ✓ PLL configuration and control (up to 168 MHz)
 *       ✓ System clock source selection
 *       ✓ AHB/APB1/APB2 prescaler configuration
 *       ✓ Peripheral clock enable/disable
 *       ✓ Peripheral reset functionality
 *       ✓ Comprehensive error checking
 * 
 * @warning Important Clock Configuration Rules:
 *          1. Enable clock source before using it as system clock
 *          2. Configure PLL before enabling it
 *          3. Set appropriate Flash latency for higher frequencies
 *          4. Ensure APB1 ≤ 45 MHz and APB2 ≤ 90 MHz (STM32F4 typical)
 *          5. Enable peripheral clock before accessing peripheral registers
 *          6. Disable peripheral or switch clock before disabling clock source
 * 
 * @example Typical Clock Configuration Sequence:
 *          1. Enable HSE: RCC_EnableHSE()
 *          2. Wait for HSE ready: while(!RCC_IsHSEReady())
 *          3. Configure PLL: RCC_ConfigurePLL(8, 336, 4, 7, RCC_PLL_SOURCE_HSE)
 *          4. Enable PLL: RCC_EnablePLL()
 *          5. Wait for PLL ready: while(!RCC_IsPLLReady())
 *          6. Set prescalers: RCC_SetAHBPrescaler(), RCC_SetAPB1Prescaler()
 *          7. Configure Flash latency
 *          8. Switch system clock: RCC_SetSysClock(RCC_SYSCLK_PLL)
 *          9. Enable peripheral clocks as needed
 ******************************************************************************/