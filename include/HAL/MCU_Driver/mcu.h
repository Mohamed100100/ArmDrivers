
#ifndef MCU_H
#define MCU_H

/*
 * Enumeration of possible return status codes for MCU driver functions
 * Used to indicate success, failure, or specific error conditions during MCU configuration
 */
typedef enum {
    MCU_OK = 0,                                   /* Operation completed successfully */
    MCU_NOT_OK,                                   /* General error or operation failed */
    MCU_TIMEOUT,                                  /* Operation timed out (e.g., waiting for clock to stabilize) */
    MCU_WRONG_PLL_CONFIG,                         /* Invalid PLL configuration parameters */
    MCU_PLL_ALREADY_ENABLED,                      /* Attempted to configure PLL while it's already running */
    MCU_WRONG_BUS_SELECTION,                      /* Invalid bus identifier provided */
    MCU_WRONG_PEREPHRAL_SELECTION,                /* Invalid peripheral identifier provided */
    MCU_WRONG_PEREPHRAL_WITHBUS_SELECTION,        /* Peripheral doesn't exist on specified bus */
    MCU_PLL_ERROR_M,                              /* PLL M divider parameter out of valid range */
    MCU_PLL_ERROR_N,                              /* PLL N multiplier parameter out of valid range */
    MCU_PLL_ERROR_P,                              /* PLL P divider parameter out of valid range */
    MCU_PLL_ERROR_Q,                              /* PLL Q divider parameter out of valid range */
    MCU_PLL_ERROR_SOURCE,                         /* Invalid PLL clock source selected */
    MCU_WRONG_SYSCLK_SOURCE,                      /* Invalid system clock source selected */
    MCU_WRONG_AHB_PRESCALER,                      /* Invalid AHB prescaler value */
    MCU_WRONG_APB_PRESCALER,                      /* Invalid APB prescaler value */
    MCU_WRONG_CONFIG,                             /* General configuration error */
    MCU_ERROR                                     /* Generic error condition */
}MCU_Status_t;




/* Constant to indicate no peripherals should be enabled on AHB1 bus */
#define MCU_AHB1_NO_PERIPHERAL            (0UL)

/* Constant to indicate no peripherals should be enabled on AHB2 bus */
#define MCU_AHB2_NO_PERIPHERAL            (0UL)

/* Constant to indicate no peripherals should be enabled on APB1 bus */
#define MCU_APB1_NO_PERIPHERAL            (0UL)

/* Constant to indicate no peripherals should be enabled on APB2 bus */
#define MCU_APB2_NO_PERIPHERAL            (0UL)


/* PLL clock source selection - High Speed Internal oscillator (16 MHz RC oscillator) */
#define MCU_PLL_SOURCE_HSI                RCC_PLL_SOURCE_HSI

/* PLL clock source selection - High Speed External oscillator (external crystal/oscillator) */
#define MCU_PLL_SOURCE_HSE                RCC_PLL_SOURCE_HSE


/* Bus identifier for Advanced High-performance Bus 1 (connects high-speed peripherals) */
#define MCU_AHB1_BUS                      RCC_AHB1_BUS

/* Bus identifier for Advanced High-performance Bus 2 (connects USB OTG peripheral) */
#define MCU_AHB2_BUS                      RCC_AHB2_BUS

/* Bus identifier for Advanced Peripheral Bus 1 (low-speed peripherals, max 42 MHz) */
#define MCU_APB1_BUS                      RCC_APB1_BUS

/* Bus identifier for Advanced Peripheral Bus 2 (high-speed peripherals, max 84 MHz) */
#define MCU_APB2_BUS                      RCC_APB2_BUS      

// Define peripheral clock enable masks for RCC AHB1 bus
/* Clock enable bit mask for GPIO Port A on AHB1 bus */
#define MCU_AHB1_GPIOA_CLOCK              RCC_AHB1_GPIOA_CLOCK

/* Clock enable bit mask for GPIO Port B on AHB1 bus */
#define MCU_AHB1_GPIOB_CLOCK              RCC_AHB1_GPIOB_CLOCK

/* Clock enable bit mask for GPIO Port C on AHB1 bus */
#define MCU_AHB1_GPIOC_CLOCK              RCC_AHB1_GPIOC_CLOCK

/* Clock enable bit mask for GPIO Port D on AHB1 bus */
#define MCU_AHB1_GPIOD_CLOCK              RCC_AHB1_GPIOD_CLOCK

/* Clock enable bit mask for GPIO Port E on AHB1 bus */
#define MCU_AHB1_GPIOE_CLOCK              RCC_AHB1_GPIOE_CLOCK

/* Clock enable bit mask for GPIO Port H on AHB1 bus */
#define MCU_AHB1_GPIOH_CLOCK              RCC_AHB1_GPIOH_CLOCK

/* Clock enable bit mask for CRC (Cyclic Redundancy Check) calculation unit on AHB1 bus */
#define MCU_AHB1_CRC_CLOCK                RCC_AHB1_CRC_CLOCK  

/* Clock enable bit mask for DMA1 (Direct Memory Access controller 1) on AHB1 bus */
#define MCU_AHB1_DMA1_CLOCK               RCC_AHB1_DMA1_CLOCK 

/* Clock enable bit mask for DMA2 (Direct Memory Access controller 2) on AHB1 bus */
#define MCU_AHB1_DMA2_CLOCK               RCC_AHB1_DMA2_CLOCK 

// Define peripheral clock enable masks for RCC AHB2 bus
/* Clock enable bit mask for USB OTG Full Speed peripheral on AHB2 bus */
#define MCU_AHB2_OTGFS_CLOCK              RCC_AHB2_OTGFS_CLOCK

// Define peripheral clock enable masks for RCC APB1 bus (low-speed peripherals)
/* Clock enable bit mask for Timer 2 on APB1 bus */
#define MCU_APB1_TIMER2_CLOCK             RCC_APB1_TIMER2_CLOCK

/* Clock enable bit mask for Timer 3 on APB1 bus */
#define MCU_APB1_TIMER3_CLOCK             RCC_APB1_TIMER3_CLOCK

/* Clock enable bit mask for Timer 4 on APB1 bus */
#define MCU_APB1_TIMER4_CLOCK             RCC_APB1_TIMER4_CLOCK

/* Clock enable bit mask for Timer 5 on APB1 bus */
#define MCU_APB1_TIMER5_CLOCK             RCC_APB1_TIMER5_CLOCK

/* Clock enable bit mask for Window Watchdog on APB1 bus */
#define MCU_APB1_WWDG_CLOCK               RCC_APB1_WWDG_CLOCK

/* Clock enable bit mask for SPI2 (Serial Peripheral Interface 2) on APB1 bus */
#define MCU_APB1_SPI2_CLOCK               RCC_APB1_SPI2_CLOCK

/* Clock enable bit mask for SPI3 (Serial Peripheral Interface 3) on APB1 bus */
#define MCU_APB1_SPI3_CLOCK               RCC_APB1_SPI3_CLOCK

/* Clock enable bit mask for USART2 (Universal Synchronous/Asynchronous Receiver/Transmitter 2) on APB1 bus */
#define MCU_APB1_USART2_CLOCK             RCC_APB1_USART2_CLOCK

/* Clock enable bit mask for I2C1 (Inter-Integrated Circuit 1) on APB1 bus */
#define MCU_APB1_I2C1_CLOCK               RCC_APB1_I2C1_CLOCK

/* Clock enable bit mask for I2C2 (Inter-Integrated Circuit 2) on APB1 bus */
#define MCU_APB1_I2C2_CLOCK               RCC_APB1_I2C2_CLOCK

/* Clock enable bit mask for I2C3 (Inter-Integrated Circuit 3) on APB1 bus */
#define MCU_APB1_I2C3_CLOCK               RCC_APB1_I2C3_CLOCK

/* Clock enable bit mask for Power interface on APB1 bus */
#define MCU_APB1_PWR_CLOCK                RCC_APB1_PWR_CLOCK    


// Define peripheral clock enable masks for RCC APB2 bus (high-speed peripherals)
/* Clock enable bit mask for Timer 1 (Advanced control timer) on APB2 bus */
#define MCU_APB2_TIMER1_CLOCK             RCC_APB2_TIMER1_CLOCK

/* Clock enable bit mask for USART1 on APB2 bus */
#define MCU_APB2_USART1_CLOCK             RCC_APB2_USART1_CLOCK

/* Clock enable bit mask for USART6 on APB2 bus */
#define MCU_APB2_USART6_CLOCK             RCC_APB2_USART6_CLOCK

/* Clock enable bit mask for ADC1 (Analog-to-Digital Converter 1) on APB2 bus */
#define MCU_APB2_ADC1_CLOCK               RCC_APB2_ADC1_CLOCK

/* Clock enable bit mask for SDIO (Secure Digital Input/Output) interface on APB2 bus */
#define MCU_APB2_SDIO_CLOCK               RCC_APB2_SDIO_CLOCK

/* Clock enable bit mask for SPI1 on APB2 bus */
#define MCU_APB2_SPI1_CLOCK               RCC_APB2_SPI1_CLOCK

/* Clock enable bit mask for SPI4 on APB2 bus */
#define MCU_APB2_SPI4_CLOCK               RCC_APB2_SPI4_CLOCK

/* Clock enable bit mask for System Configuration controller on APB2 bus */
#define MCU_APB2_SYSCFG_CLOCK             RCC_APB2_SYSCFG_CLOCK

/* Clock enable bit mask for Timer 9 on APB2 bus */
#define MCU_APB2_TIMER9_CLOCK             RCC_APB2_TIMER9_CLOCK

/* Clock enable bit mask for Timer 10 on APB2 bus */
#define MCU_APB2_TIMER10_CLOCK            RCC_APB2_TIMER10_CLOCK

/* Clock enable bit mask for Timer 11 on APB2 bus */
#define MCU_APB2_TIMER11_CLOCK            RCC_APB2_TIMER11_CLOCK


/*
 * Enumeration of available system clock sources
 * Determines which oscillator/PLL drives the system clock
 */
typedef enum {
MCU_SYSCLK_HSI = 0,      /* High Speed Internal - 16 MHz RC oscillator */
MCU_SYSCLK_HSE = 1,      /* High Speed External - External crystal/oscillator */
MCU_SYSCLK_PLL = 2       /* Phase-Locked Loop - Multiplied clock from HSI or HSE */
}MCU_ClockSrc_t; 

/*
 * Enumeration of AHB (Advanced High-performance Bus) prescaler values
 * Divides the system clock to generate the AHB clock
 * Bits [7:4] of RCC_CFGR register
 */
typedef enum {
MCU_AHB_NO_DIVISION    = 0b00000000000000000000000000000000,  /* SYSCLK not divided (HCLK = SYSCLK) */
MCU_AHB_DIVIDED_BY_2   = 0b00000000000000000000000010000000,  /* SYSCLK divided by 2 */
MCU_AHB_DIVIDED_BY_4   = 0b00000000000000000000000010010000,  /* SYSCLK divided by 4 */
MCU_AHB_DIVIDED_BY_8   = 0b00000000000000000000000010100000,  /* SYSCLK divided by 8 */
MCU_AHB_DIVIDED_BY_16  = 0b00000000000000000000000010110000,  /* SYSCLK divided by 16 */
MCU_AHB_DIVIDED_BY_64  = 0b00000000000000000000000011000000,  /* SYSCLK divided by 64 */
MCU_AHB_DIVIDED_BY_128 = 0b00000000000000000000000011010000,  /* SYSCLK divided by 128 */
MCU_AHB_DIVIDED_BY_256 = 0b00000000000000000000000011100000,  /* SYSCLK divided by 256 */
MCU_AHB_DIVIDED_BY_512 = 0b00000000000000000000000011110000,  /* SYSCLK divided by 512 */
}MCU_AHPPrescaler_t;

/*
 * Enumeration of APB1 (Advanced Peripheral Bus 1) prescaler values
 * Divides the AHB clock to generate the APB1 clock (low-speed peripheral bus)
 * Bits [12:10] of RCC_CFGR register
 * Maximum APB1 frequency is 42 MHz
 */
typedef enum {
MCU_APB1_NO_DIVISION    = 0b00000000000000000000000000000000,  /* HCLK not divided (PCLK1 = HCLK) */
MCU_APB1_DIVIDED_BY_2   = 0b00000000000000000001000000000000,  /* HCLK divided by 2 */
MCU_APB1_DIVIDED_BY_4   = 0b00000000000000000001010000000000,  /* HCLK divided by 4 */
MCU_APB1_DIVIDED_BY_8   = 0b00000000000000000001100000000000,  /* HCLK divided by 8 */
MCU_APB1_DIVIDED_BY_16  = 0b00000000000000000001110000000000   /* HCLK divided by 16 */
}MCU_APB1Prescaler_t;

/*
 * Enumeration of APB2 (Advanced Peripheral Bus 2) prescaler values
 * Divides the AHB clock to generate the APB2 clock (high-speed peripheral bus)
 * Bits [15:13] of RCC_CFGR register
 * Maximum APB2 frequency is 84 MHz
 */
typedef enum {
MCU_APB2_NO_DIVISION    = 0b00000000000000000000000000000000,  /* HCLK not divided (PCLK2 = HCLK) */
MCU_APB2_DIVIDED_BY_2   = 0b00000000000000001000000000000000,  /* HCLK divided by 2 */
MCU_APB2_DIVIDED_BY_4   = 0b00000000000000001010000000000000,  /* HCLK divided by 4 */
MCU_APB2_DIVIDED_BY_8   = 0b00000000000000001100000000000000,  /* HCLK divided by 8 */
MCU_APB2_DIVIDED_BY_16  = 0b00000000000000001110000000000000   /* HCLK divided by 16 */
}MCU_APB2Prescaler_t;


/*
 * Structure containing complete MCU clock and peripheral configuration
 * This structure holds all settings needed to initialize the microcontroller's
 * clock system and enable required peripheral clocks
 */
typedef struct{
/* Bit mask of peripherals to enable on AHB1 bus (OR multiple MCU_AHB1_xxx_CLOCK values) */
uint64_t              MCU_AHB1_PrephralEnable;

/* Bit mask of peripherals to enable on AHB2 bus (OR multiple MCU_AHB2_xxx_CLOCK values) */
uint64_t              MCU_AHB2_PrephralEnable;

/* Bit mask of peripherals to enable on APB1 bus (OR multiple MCU_APB1_xxx_CLOCK values) */
uint64_t              MCU_APB1_PrephralEnable;

/* Bit mask of peripherals to enable on APB2 bus (OR multiple MCU_APB2_xxx_CLOCK values) */
uint64_t              MCU_APB2_PrephralEnable;

/* System clock source selection (HSI, HSE, or PLL) */
MCU_ClockSrc_t        MCU_SystemClockSource;

/* AHB bus prescaler - divides system clock to generate AHB clock */
MCU_AHPPrescaler_t    MCU_AHP_Prescaler;

/* APB1 bus prescaler - divides AHB clock to generate APB1 clock (max 42 MHz) */
MCU_APB1Prescaler_t   MCU_APB1_Prescaler;

/* APB2 bus prescaler - divides AHB clock to generate APB2 clock (max 84 MHz) */
MCU_APB2Prescaler_t   MCU_APB2_Prescaler;

/* HSI oscillator frequency in Hz (typically 16,000,000) */
uint32_t              MCU_HSI_ClockSource;

/* HSE oscillator frequency in Hz (depends on external crystal, typically 8,000,000 or 25,000,000) */
uint32_t              MCU_HSE_ClockSource;

/* PLL multiplication factor for VCO - Valid range: 50 to 432
 * VCO output frequency = PLL input clock / PLLM × PLLN */
uint16_t              MCU_PLLN;

/* PLL clock source selection (MCU_PLL_SOURCE_HSI or MCU_PLL_SOURCE_HSE) */
uint8_t               MCU_PLLClockSource;

/* PLL input division factor - Valid range: 2 to 63
 * Used to scale PLL input frequency into the 1-2 MHz range for VCO */
uint8_t               MCU_PLLM;

/* PLL main output division factor - Valid values: 2, 4, 6, or 8
 * Divides VCO frequency to generate main PLL output (system clock) */
uint8_t               MCU_PLLP;

/* PLL division factor for USB OTG FS, SDIO and RNG clocks - Valid range: 2 to 15
 * Used to generate 48 MHz clock required by USB, SDIO, and RNG peripherals */
uint8_t               MCU_PLLQ;
}MCU_Config_t;

/* 
 * External declaration of the MCU configuration structure
 * This structure should be defined in the application configuration file
 * with all the required clock and peripheral settings
 */
extern const MCU_Config_t MCU_Configs; 

/*
 * Function: MCU_enuInit
 * Description: Initializes the MCU clock system and peripheral clocks according to configuration
 * Parameters:
 *   - Pointer to MCU_Config_t structure containing clock and peripheral configuration
 * Returns: MCU_Status_t indicating success or specific error condition
 * Note: This function configures:
 *       - System clock source (HSI/HSE/PLL)
 *       - PLL parameters if PLL is used
 *       - AHB, APB1, and APB2 prescalers
 *       - Peripheral clock enables for all buses
 */
MCU_Status_t MCU_enuInit(MCU_Config_t *);

#endif // MCU_H
