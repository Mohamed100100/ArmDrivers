#include "./LIB/stdtypes.h"
#include "./MCAL/RCC_Driver/rcc_int.h"

#include "./HAL/MCU_Driver/mcu_cfg.h"
#include "./HAL/MCU_Driver/mcu.h"

/*
 * Function: MCU_enuInit
 * Description: Initializes the MCU clock system and peripheral clocks
 *              Supports two modes of operation:
 *              1. Compile-time configuration (when NULL is passed)
 *              2. Runtime configuration (when valid config pointer is passed)
 * Parameters:
 *   - localMcuConfig: Pointer to MCU_Config_t structure
 *                     Pass NULL to use compile-time configuration from mcu_cfg.h
 *                     Pass &MCU_Configs to use runtime configuration structure
 * Returns: MCU_Status_t indicating success or specific error condition
 * 
 * Function performs the following steps:
 * 1. Sets clock source values for HSI and HSE
 * 2. Enables and selects system clock source (HSI, HSE, or PLL)
 * 3. Configures PLL parameters if PLL is selected
 * 4. Sets AHB, APB1, and APB2 bus prescalers
 * 5. Enables peripheral clocks for requested peripherals on all buses
 */
MCU_Status_t MCU_enuInit(MCU_Config_t *localMcuConfig) {

    /* 
     * BRANCH 1: Compile-time configuration mode
     * When NULL pointer is passed, use preprocessor-defined macros from mcu_cfg.h
     * This allows configuration to be fixed at compile time
     */
    if(NULL == localMcuConfig){
        /* Initialize RCC status variable to track operation results */
        RCC_Status_t status = RCC_NOT_OK;

        /* Set the HSI clock source frequency value for RCC driver calculations */
        RCC_HSI_ClockSourceValue = MCU_HSI_CLOCK_SOURCE_VALUE;
        
        /* Set the HSE clock source frequency value for RCC driver calculations */
        RCC_HSE_ClockSourceValue = MCU_HSE_CLOCK_SOURCE_VALUE;

        /* 
         * Configure system clock source based on compile-time setting
         * Three possible sources: HSI (internal), HSE (external), or PLL (multiplied)
         */
        
        /* Option 1: High Speed Internal oscillator (16 MHz RC oscillator) */
        if(MCU_SYSCLK_HSI == MCU_SYSCLK_SOURCE){
            /* Enable the HSI oscillator */
            status = RCC_EnableHSI();
            if (RCC_OK != status) {
                /* Return error if HSI failed to enable (e.g., timeout) */
                return (MCU_Status_t)status;
            }
            /* Switch system clock to HSI */
            status = RCC_SetSysClock(RCC_SYSCLK_HSI);
            if (RCC_OK != status) {
                /* Return error if system clock switching failed */
                return (MCU_Status_t)status;
            }
        }
        /* Option 2: High Speed External oscillator (external crystal/clock) */
        else if( MCU_SYSCLK_SOURCE == MCU_SYSCLK_HSE){
            /* Enable the HSE oscillator */
            status = RCC_EnableHSE();
            if (RCC_OK != status) {
                /* Return error if HSE failed to enable (e.g., crystal not working) */
                return (MCU_Status_t)status;
            }
            /* Switch system clock to HSE */
            status = RCC_SetSysClock(RCC_SYSCLK_HSE);
            if (RCC_OK != status) {
                /* Return error if system clock switching failed */
                return (MCU_Status_t)status;
            }
        }
        /* Option 3: Phase-Locked Loop (generates high-frequency clock from HSI/HSE) */
        else if(MCU_SYSCLK_SOURCE == MCU_SYSCLK_PLL){
            /* Configure PLL with multiplication and division factors
             * Formula: PLL_output = (Input_clock / PLLM) × PLLN / PLLP
             * PLLQ is used for USB/SDIO/RNG peripherals (48 MHz target) */
            status = RCC_ConfigurePLL(MCU_PLL_M, MCU_PLL_N, MCU_PLL_P,
                                       MCU_PLL_Q, MCU_PLL_SOURCE);
            if (RCC_OK != status) {
                /* Return error if PLL parameters are invalid */
                return (MCU_Status_t)status;
            }

            /* Enable the PLL and wait for it to lock */
            status = RCC_EnablePLL();
            if (RCC_OK != status) {
                /* Return error if PLL failed to lock (e.g., timeout) */
                return (MCU_Status_t)status;
            }

            /* Switch system clock to PLL output */
            status = RCC_SetSysClock(RCC_SYSCLK_PLL);
            if (RCC_OK != status) {
                /* Return error if system clock switching failed */
                return (MCU_Status_t)status;
            }
        }
        /* Invalid system clock source configured */
        else{
            return (MCU_WRONG_SYSCLK_SOURCE);
        }

        /* 
         * Configure bus prescalers to divide system clock for different buses
         * This ensures peripheral buses don't exceed their maximum frequencies
         */
        
        /* Set AHB prescaler (divides SYSCLK to generate HCLK for CPU, memory, DMA) */
        status = RCC_SetAHBPrescaler(MCU_AHB_PRESCALER);
        if (RCC_OK != status) {
            /* Return error if prescaler value is invalid */
            return (MCU_Status_t)status;
        }

        /* Set APB1 prescaler (divides HCLK to generate PCLK1, max 42 MHz) */
        status = RCC_SetAPB1Prescaler(MCU_APB1_PRESCALER);
        if (RCC_OK != status) {
            /* Return error if prescaler value is invalid */
            return (MCU_Status_t)status;
        }

        /* Set APB2 prescaler (divides HCLK to generate PCLK2, max 84 MHz) */
        status = RCC_SetAPB2Prescaler(MCU_APB2_PRESCALER);
        if (RCC_OK != status) {
            /* Return error if prescaler value is invalid */
            return (MCU_Status_t)status;
        }

        /*
         * Enable peripheral clocks on each bus
         * Only enable if at least one peripheral is requested
         * Multiple peripherals can be enabled by ORing their clock enable bits
         */
        
        /* Enable AHB1 peripheral clocks (GPIO, DMA, CRC, etc.) */
        if(MCU_AHB1_PERIPHERALS_ENABLE != MCU_AHB1_NO_PERIPHERAL){
            status = RCC_EnablePeripheralClock(RCC_AHB1_BUS,MCU_AHB1_PERIPHERALS_ENABLE);
            if (RCC_OK != status) {
                /* Return error if peripheral enable failed */
                return (MCU_Status_t)status;
            }
        }

        /* Enable AHB2 peripheral clocks (USB OTG FS) */
        if(MCU_AHB2_PERIPHERALS_ENABLE != MCU_AHB2_NO_PERIPHERAL){
            status = RCC_EnablePeripheralClock(RCC_AHB2_BUS,MCU_AHB2_PERIPHERALS_ENABLE);
            if (RCC_OK != status) {
                /* Return error if peripheral enable failed */
                return (MCU_Status_t)status;
            }
        }

        /* Enable APB1 peripheral clocks (Timers, SPI, I2C, USART, etc.) */
        if(MCU_APB1_PERIPHERALS_ENABLE != MCU_APB1_NO_PERIPHERAL){
            status = RCC_EnablePeripheralClock(RCC_APB1_BUS,MCU_APB1_PERIPHERALS_ENABLE);
            if (RCC_OK != status) {
                /* Return error if peripheral enable failed */
                return (MCU_Status_t)status;
            }
        }

        /* Enable APB2 peripheral clocks (Timers, ADC, SPI, USART, SYSCFG, etc.) */
        if(MCU_APB2_PERIPHERALS_ENABLE != MCU_APB2_NO_PERIPHERAL){
            status = RCC_EnablePeripheralClock(RCC_APB2_BUS,MCU_APB2_PERIPHERALS_ENABLE);
            if (RCC_OK != status) {
                /* Return error if peripheral enable failed */
                return (MCU_Status_t)status;
            }
        }
        
        /* Return final status (should be RCC_OK if all operations succeeded) */
        return status;

    }
    /* 
     * BRANCH 2: Runtime configuration mode
     * When pointer to MCU_Configs structure is passed, use runtime configuration
     * This allows configuration to be changed at runtime
     */
    else if(&MCU_Configs == localMcuConfig){

        /* Initialize RCC status variable to track operation results */
        RCC_Status_t status = RCC_NOT_OK;

        /* Set the HSI clock source frequency value from configuration structure */
        RCC_HSI_ClockSourceValue = MCU_Configs.MCU_HSI_ClockSource;
        
        /* Set the HSE clock source frequency value from configuration structure */
        RCC_HSE_ClockSourceValue = MCU_Configs.MCU_HSI_ClockSource;

        /* 
         * Configure system clock source based on runtime configuration
         * Three possible sources: HSI (internal), HSE (external), or PLL (multiplied)
         */
        
        /* Option 1: High Speed Internal oscillator (16 MHz RC oscillator) */
        if(MCU_Configs.MCU_SystemClockSource == MCU_SYSCLK_HSI){
            /* Enable the HSI oscillator */
            status = RCC_EnableHSI();
            if (RCC_OK != status) {
                /* Return error if HSI failed to enable (e.g., timeout) */
                return (MCU_Status_t)status;
            }
            /* Switch system clock to HSI */
            status = RCC_SetSysClock(RCC_SYSCLK_HSI);
            if (RCC_OK != status) {
                /* Return error if system clock switching failed */
                return (MCU_Status_t)status;
            }
         }
         /* Option 2: High Speed External oscillator (external crystal/clock) */
         else if(MCU_Configs.MCU_SystemClockSource == MCU_SYSCLK_HSE){
            /* Enable the HSE oscillator */
            status = RCC_EnableHSE();
            if (RCC_OK != status) {
                /* Return error if HSE failed to enable (e.g., crystal not working) */
                return (MCU_Status_t)status;
            }
            /* Switch system clock to HSE */
            status = RCC_SetSysClock(RCC_SYSCLK_HSE);
            if (RCC_OK != status) {
                /* Return error if system clock switching failed */
                return (MCU_Status_t)status;
            }
         }
         /* Option 3: Phase-Locked Loop (generates high-frequency clock from HSI/HSE) */
         else if(MCU_Configs.MCU_SystemClockSource == MCU_SYSCLK_PLL){

            /* Configure PLL with multiplication and division factors from config structure
             * Formula: PLL_output = (Input_clock / PLLM) × PLLN / PLLP
             * PLLQ is used for USB/SDIO/RNG peripherals (48 MHz target) */
            status = RCC_ConfigurePLL(MCU_Configs.MCU_PLLM, MCU_Configs.MCU_PLLN, MCU_Configs.MCU_PLLP,
                                    MCU_Configs.MCU_PLLQ, MCU_Configs.MCU_PLLClockSource);
            if (RCC_OK != status) {
                /* Return error if PLL parameters are invalid */
                return (MCU_Status_t)status;
            }

            /* Enable the PLL and wait for it to lock */
            status = RCC_EnablePLL();
            if (RCC_OK != status) {
                /* Return error if PLL failed to lock (e.g., timeout) */
                return (MCU_Status_t)status;
            }

            /* Switch system clock to PLL output */
            status = RCC_SetSysClock(RCC_SYSCLK_PLL);
            if (RCC_OK != status) {
                /* Return error if system clock switching failed */
                return (MCU_Status_t)status;
            }
         }
         /* Invalid system clock source in configuration */
         else{
            return MCU_WRONG_SYSCLK_SOURCE;
         }

        /* 
         * Configure bus prescalers from configuration structure
         * This ensures peripheral buses don't exceed their maximum frequencies
         */
        
        /* Set AHB prescaler from configuration (divides SYSCLK to generate HCLK) */
        status = RCC_SetAHBPrescaler(MCU_Configs.MCU_AHP_Prescaler);
        if (RCC_OK != status) {
            /* Return error if prescaler value is invalid */
            return (MCU_Status_t)status;
        }

        /* Set APB1 prescaler from configuration (divides HCLK to generate PCLK1, max 42 MHz) */
        status = RCC_SetAPB1Prescaler(MCU_Configs.MCU_APB1_Prescaler);
        if (RCC_OK != status) {
            /* Return error if prescaler value is invalid */
            return (MCU_Status_t)status;
        }

        /* Set APB2 prescaler from configuration (divides HCLK to generate PCLK2, max 84 MHz) */
        status = RCC_SetAPB2Prescaler(MCU_Configs.MCU_APB2_Prescaler);
        if (RCC_OK != status) {
            /* Return error if prescaler value is invalid */
            return (MCU_Status_t)status;
        }

        /*
         * Enable peripheral clocks on each bus based on configuration structure
         * Only enable if at least one peripheral is requested
         * Multiple peripherals can be enabled by ORing their clock enable bits
         */
        
        /* Enable AHB1 peripheral clocks from configuration (GPIO, DMA, CRC, etc.) */
        if(MCU_Configs.MCU_AHB1_PrephralEnable != MCU_AHB1_NO_PERIPHERAL){
            status = RCC_EnablePeripheralClock(RCC_AHB1_BUS,MCU_Configs.MCU_AHB1_PrephralEnable);
            if (RCC_OK != status) {
                /* Return error if peripheral enable failed */
                return (MCU_Status_t)status;
            }
        }

        /* Enable AHB2 peripheral clocks from configuration (USB OTG FS) */
        if(MCU_Configs.MCU_AHB2_PrephralEnable != MCU_AHB2_NO_PERIPHERAL){
            status = RCC_EnablePeripheralClock(RCC_AHB2_BUS,MCU_Configs.MCU_AHB2_PrephralEnable);
            if (RCC_OK != status) {
                /* Return error if peripheral enable failed */
                return (MCU_Status_t)status;
            }
        }

        /* Enable APB1 peripheral clocks from configuration (Timers, SPI, I2C, USART, etc.) */
        if(MCU_Configs.MCU_APB1_PrephralEnable != MCU_APB1_NO_PERIPHERAL){
            status = RCC_EnablePeripheralClock(RCC_APB1_BUS,MCU_Configs.MCU_APB1_PrephralEnable);
            if (RCC_OK != status) {
                /* Return error if peripheral enable failed */
                return (MCU_Status_t)status;
            }
        }

        /* Enable APB2 peripheral clocks from configuration (Timers, ADC, SPI, USART, SYSCFG, etc.) */
        if(MCU_Configs.MCU_APB2_PrephralEnable != MCU_APB2_NO_PERIPHERAL){
            status = RCC_EnablePeripheralClock(RCC_APB2_BUS,MCU_Configs.MCU_APB2_PrephralEnable);
        if (RCC_OK != status) {
            /* Return error if peripheral enable failed */
            return (MCU_Status_t)status;
            }
        }
        
        /* Return final status (should be RCC_OK if all operations succeeded) */
        return status;
    
    }
    /* 
     * BRANCH 3: Invalid configuration pointer
     * Pointer is neither NULL nor pointing to MCU_Configs structure
     */
    else{
        return (MCU_WRONG_CONFIG);
    }
}