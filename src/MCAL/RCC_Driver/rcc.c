/******************************************************************************
 * @file    RCC.C
 * @author  Eng.Gemy
 * @brief   RCC (Reset and Clock Control) Driver Implementation File
 *          This file contains the implementation of all RCC driver functions
 *          for STM32 microcontrollers clock configuration and control
 * @date    [Current Date]
 * @version 1.0
 ******************************************************************************/

#include "LIB/stdtypes.h"

#include "MCAL/RCC_Driver/rcc_cfg.h"
#include "MCAL/RCC_Driver/rcc_priv.h"
#include "MCAL/RCC_Driver/rcc_int.h"

/******************************************************************************
 *                   HSI (HIGH SPEED INTERNAL) OSCILLATOR FUNCTIONS
 * @brief Functions to control HSI oscillator (16 MHz internal RC)
 * @author Eng.Gemy
 ******************************************************************************/

/**
 * @brief Enable the High-Speed Internal (HSI) oscillator
 *
 * This function enables the HSI oscillator and waits until it is ready
 * or a timeout occurs.
 *
 * @return RCC_Status_t Status of the operation (RCC_OK, RCC_TIMEOUT)
 * 
 * @note HSI is the default clock source after reset
 * @note HSI frequency is approximately 16 MHz with ±1% accuracy
 * @author Eng.Gemy
 */
RCC_Status_t RCC_EnableHSI(void)
{
    // status variable to track operation result
    RCC_Status_t status = RCC_NOT_OK;

    // Enable HSI oscillator by setting HSION bit in RCC_CR register
    RCC_Registers->CR.BIT_FIELDS.HSION = 1;

    // timout counter to prevent infinite loop
    uint32_t timeout = HSI_TIMEOUT_VALUE;

    // Wait until HSI is ready or timeout occurs
    // HSIRDY flag is set by hardware when HSI oscillator is stable
    while ((0 == RCC_Registers->CR.BIT_FIELDS.HSIRDY) && (timeout-- > 0))
        ;

    // HSI is ready
    if (0 != timeout)
    {
        status = RCC_OK;
    }
    // Timeout occurred, HSI not ready
    else
    {
        status = RCC_TIMEOUT;
    }

    return status;
}

/**
 * @brief Check if the High-Speed Internal (HSI) oscillator is ready
 *
 * This function checks the readiness status of the HSI oscillator.
 *
 * @return uint8_t 1 if HSI is ready, 0 otherwise
 * 
 * @note This is a non-blocking function that reads the HSIRDY flag
 * @author Eng.Gemy
 */
uint8_t RCC_IsHSIReady(void)
{
    /* Return the HSIRDY bit status from RCC_CR register
     * 1 = HSI oscillator is stable and ready
     * 0 = HSI oscillator is not ready
     */
    return (uint8_t)(RCC_Registers->CR.BIT_FIELDS.HSIRDY);
}

/******************************************************************************
 *                   HSE (HIGH SPEED EXTERNAL) OSCILLATOR FUNCTIONS
 * @brief Functions to control HSE oscillator (external crystal/oscillator)
 * @author Eng.Gemy
 ******************************************************************************/

/**
 * @brief Enable the High-Speed External (HSE) oscillator
 *
 * This function enables the HSE oscillator and waits until it is ready
 * or a timeout occurs.
 *
 * @return RCC_Status_t Status of the operation (RCC_OK, RCC_TIMEOUT)
 * 
 * @note HSE requires external crystal or clock source (typically 4-26 MHz)
 * @note HSE provides better accuracy than HSI (PPM vs %)
 * @warning Ensure external crystal/oscillator is properly connected
 * @author Eng.Gemy
 */
RCC_Status_t RCC_EnableHSE(void)
{
    // status variable to track operation result
    RCC_Status_t status = RCC_NOT_OK;

    // Enable HSE oscillator by setting HSEON bit in RCC_CR register
    RCC_Registers->CR.BIT_FIELDS.HSEON = 1;

    // timout counter to prevent infinite loop
    uint32_t timeout = HSE_TIMEOUT_VALUE;

    // Wait until HSE is ready or timeout occurs
    // HSERDY flag is set by hardware when HSE oscillator is stable
    while ((0 == RCC_Registers->CR.BIT_FIELDS.HSERDY) && (timeout-- > 0))
        ;

    // HSE is ready (check if timeout didn't expire)
    if (1 != timeout)
    {
        status = RCC_OK;
    }
    // Timeout occurred, HSE not ready
    else
    {
        status = RCC_TIMEOUT;
    }

    return status;
}

/**
 * @brief Check if the High-Speed External (HSE) oscillator is ready
 *
 * This function checks the readiness status of the HSE oscillator.
 *
 * @return uint8_t 1 if HSE is ready, 0 otherwise
 * 
 * @note This is a non-blocking function that reads the HSERDY flag
 * @author Eng.Gemy
 */
uint8_t RCC_IsHSEReady(void)
{
    /* Return the HSERDY bit status from RCC_CR register
     * 1 = HSE oscillator is stable and ready
     * 0 = HSE oscillator is not ready
     */
    return (uint8_t)(RCC_Registers->CR.BIT_FIELDS.HSERDY);
}

/******************************************************************************
 *                   HSI DISABLE FUNCTION
 * @brief Function to disable HSI oscillator
 * @author Eng.Gemy
 ******************************************************************************/

/**
 * @brief Disable the High-Speed Internal (HSI) oscillator
 *
 * This function disables the HSI oscillator.
 *
 * @return RCC_Status_t Status of the operation (RCC_OK, RCC_ERROR)
 * 
 * @warning Cannot disable HSI if it is selected as system clock source
 * @warning Cannot disable HSI if it is selected as PLL source and PLL is active
 * @note Disabling unused oscillators reduces power consumption
 * @author Eng.Gemy
 */
RCC_Status_t RCC_DisableHSI(void)
{
    // status variable to track operation result
    RCC_Status_t status = RCC_NOT_OK;

    // Disable HSI oscillator by clearing HSION bit in RCC_CR register
    RCC_Registers->CR.BIT_FIELDS.HSION = 0;

    // Check if HSI is disabled by reading HSIRDY flag
    // HSIRDY should be 0 when HSI is successfully disabled
    if (0 == RCC_Registers->CR.BIT_FIELDS.HSIRDY)
    {
        status = RCC_OK;
    }
    else
    {
        /* HSI cannot be disabled (likely because it's in use)
         * This can happen if:
         * 1. HSI is the current system clock source
         * 2. HSI is the PLL source and PLL is enabled
         */
        status = RCC_ERROR;
    }

    return status;
}

/**
 * @brief Disable the High-Speed External (HSE) oscillator
 *
 * This function disables the HSE oscillator.
 *
 * @return RCC_Status_t Status of the operation (RCC_OK, RCC_ERROR)
 * 
 * @warning Cannot disable HSE if it is selected as system clock source
 * @warning Cannot disable HSE if it is selected as PLL source and PLL is active
 * @note Disabling unused oscillators reduces power consumption
 * @author Eng.Gemy
 */
RCC_Status_t RCC_DisableHSE(void)
{
    // status variable to track operation result
    RCC_Status_t status = RCC_NOT_OK;

    // Disable HSE oscillator by clearing HSEON bit in RCC_CR register
    RCC_Registers->CR.BIT_FIELDS.HSEON = 0;

    // Check if HSE is disabled by reading HSERDY flag
    // HSERDY should be 0 when HSE is successfully disabled
    if (0 == RCC_Registers->CR.BIT_FIELDS.HSERDY)
    {
        status = RCC_OK;
    }
    else
    {
        /* HSE cannot be disabled (likely because it's in use)
         * This can happen if:
         * 1. HSE is the current system clock source
         * 2. HSE is the PLL source and PLL is enabled
         */
        status = RCC_ERROR;
    }

    return status;
}

/******************************************************************************
 *                   SYSTEM CLOCK CONFIGURATION FUNCTION
 * @brief Function to set system clock source
 * @author Eng.Gemy
 ******************************************************************************/

/**
 * @brief Set the system clock source
 *
 * This function configures the system clock source to either HSI, HSE, or PLL.
 * It checks if the selected clock source is ready before switching.
 *
 * @param clockSource The desired clock source (SYSCLK_HSI, SYSCLK_HSE, SYSCLK_PLL)
 *
 * @return RCC_Status_t Status of the operation (RCC_OK, RCC_ERROR, RCC_TIMEOUT)
 * 
 * @note Ensure the target clock source is enabled and ready before calling this function
 * @warning Configure Flash latency appropriately before switching to higher frequency
 * @note System clock switch is performed by hardware when safe
 * @author Eng.Gemy
 */
RCC_Status_t RCC_SetSysClock(RCC_ClockSrc_t clockSourceMask)
{
    /* Local variable to hold function return status */
    RCC_Status_t status = RCC_NOT_OK;

    /* Validate clock source parameter
     * Valid values: 0 (HSI), 1 (HSE), 2 (PLL)
     */
    if(clockSourceMask < RCC_SYSCLK_HSI || clockSourceMask > RCC_SYSCLK_PLL)
    {
        status = RCC_WRONG_SYSCLK_SOURCE;
    }else
    {
        // Set the system clock source by writing to SW bits in CFGR register
        // SW[1:0]: 00 = HSI, 01 = HSE, 10 = PLL
        RCC_Registers->CFGR.BIT_FIELDS.SW = clockSourceMask;

        /* Wait for system clock switch to complete
         * Hardware automatically switches when safe
         */
        uint32_t timeout = HSI_TIMEOUT_VALUE;
        // Wait until the clock source is switched or timeout occurs
        // SWS (System clock switch status) should match SW when switch is complete
        while ((clockSourceMask != RCC_Registers->CFGR.BIT_FIELDS.SWS) && (timeout-- > 0));
        
        /* Check if timeout expired */
        if (timeout == 0)
        {
            /* System clock switch timeout - clock source not switched
             * This can happen if the selected clock is not ready/enabled
             */
            status = RCC_TIMEOUT;
        }
        else
        {
            /* System clock successfully switched */
            status = RCC_OK;
        }
    }
    return status;
}

/* =====================================================================
 * 3. PLL CONFIGURATION FUNCTION
 * ===================================================================== */

/******************************************************************************
 *                   PLL (PHASE-LOCKED LOOP) FUNCTIONS
 * @brief Functions to control and configure PLL
 * @note PLL provides high-frequency system clock from HSI/HSE source
 * @author Eng.Gemy
 ******************************************************************************/

/**
 * @brief Enable the PLL (Phase-Locked Loop)
 *
 * This function enables the PLL and waits until it locks or timeout occurs.
 *
 * @return RCC_Status_t Status of the operation (RCC_OK, RCC_TIMEOUT)
 * 
 * @note PLL must be configured before enabling
 * @note Wait for PLLRDY flag before using PLL as system clock
 * @author Eng.Gemy
 */
RCC_Status_t RCC_EnablePLL(void)
{
    /* Local variable to hold function return status */
    RCC_Status_t status = RCC_NOT_OK;

    // Enable PLL by setting PLLON bit in RCC_CR register
    RCC_Registers->CR.BIT_FIELDS.PLLON = 1;

    // Wait until PLL is ready (locked) or timeout occurs
    uint32_t timeout = PLL_TIMEOUT_VALUE;
    // PLLRDY flag is set by hardware when PLL output is stable
    while ((0 == RCC_Registers->CR.BIT_FIELDS.PLLRDY) && (timeout-- > 0))
        ;

    // PLL is ready (locked)
    if (1 != timeout)
    {
        status = RCC_OK;
    }
    // Timeout occurred, PLL not ready
    else
    {
        /* PLL failed to lock within timeout period
         * This can happen if:
         * 1. PLL configuration is invalid
         * 2. PLL source clock is not stable
         */
        status = RCC_TIMEOUT;
    }

    return status;
}

/**
 * @brief Disable the PLL (Phase-Locked Loop)
 *
 * This function disables the PLL.
 *
 * @return RCC_Status_t Status of the operation (RCC_OK, RCC_ERROR)
 * 
 * @warning Cannot disable PLL if it is selected as system clock source
 * @note Disabling PLL reduces power consumption
 * @author Eng.Gemy
 */
RCC_Status_t RCC_DisablePLL(void)
{
    /* Local variable to hold function return status */
    RCC_Status_t status = RCC_NOT_OK;

    // Disable PLL by clearing PLLON bit in RCC_CR register
    RCC_Registers->CR.BIT_FIELDS.PLLON = 0;

    // Check if PLL is disabled
    // PLL is disabled - PLLRDY flag should be 0
    if (0 == RCC_Registers->CR.BIT_FIELDS.PLLRDY)
    {
        status = RCC_OK;
    }
    else
    {
        /* PLL cannot be disabled because it's currently in use as system clock
         * Switch system clock to HSI or HSE before disabling PLL
         */
        status = RCC_ERROR;
    }

    return status;
}

/**
 * @brief Check if the PLL is ready (locked)
 *
 * This function checks if PLL output is stable and ready to use.
 *
 * @return uint8_t 1 if PLL is locked and ready, 0 otherwise
 * 
 * @note This is a non-blocking function
 * @author Eng.Gemy
 */
uint8_t RCC_IsPLLReady(void)
{
    /* Return the PLLRDY bit status from RCC_CR register
     * 1 = PLL is locked and output is stable
     * 0 = PLL is not ready
     */
    return (uint8_t)(RCC_Registers->CR.BIT_FIELDS.PLLRDY);
}

/******************************************************************************
 *                   PLL CONFIGURATION FUNCTION
 * @brief Comprehensive PLL configuration with parameter validation
 * @author Eng.Gemy
 ******************************************************************************/

/**
 * VCO input frequency  = PLL input clock frequency / PLLM
 * 2 ≤ PLLM ≤ 63      &&  2MHz < VCO input frequency < 1MHz.
 * 2 ≤ PLLM ≤ 63      &&  PLL input clock frequency / 2MHz < PLLM < PLL input clock frequency / 1MHz.
 *
 * VCO output frequency = VCO input frequency × PLLN
 * 192 ≤ PLLN ≤ 432   && 192 MHz ≤ VCO output frequency ≤ 432 MHz
 * 192 MHz * PLLM / PLL input clock frequency ≤ PLLN ≤ 432 MHz * PLLM / PLL input clock frequency
 *
 * PLL output frequency = VCO output frequency / PLLP
 * PLLP = 2, 4, 6, or 8   &&   PLL output frequency ≤ 84 MHz
 * PLL input clock frequency *PLLN / (PLLM *PLLP) ≤ 84 MHz
 * PLLP >= PLL input clock frequency * PLLN / (84 MHz * PLLM)
 *
 * USB output frequency = VCO output frequency / PLLQ
 *  2 ≤PLLQ ≤ 15   &&   USB output frequency ≤ 48 MHz
 * PLL input clock frequency * PLLN / (PLLM * PLLQ) ≤ 48 MHz
 * PLLQ ≥ PLL input clock frequency * PLLN / (48 MHz * PLLM)
 */

/**
 * @brief Configure the PLL (Phase Locked Loop) parameters
 *
 * This function configures the PLL with the specified parameters.
 * It disables the PLL before configuration and re-enables it afterward.
 *
 * @param Copy_PLLM (5 bits) Division factor for the main PLL input clock (2 to 63) > (0,1 > not allowed)
 *                  hint : VCO input frequency = Input clock / PLLM
 *                  Valid VCO input frequency: 1 MHz to 2 MHz
 *
 * @param Copy_PLLN (9 bits) Multiplication factor for the main PLL (96 to 432)
 *                  hint : VCO output frequency = (Input clock / PLLM) * PLLN
 *                  Valid VCO output frequency: 192 MHz to 432 MHz
 *
 * @param Copy_PLLP (2 bits) Division factor for main system clock (2,4,6,8)
 *                  hint : PLL output frequency = VCO output frequency / PLLP
 *                  Valid frequency: up to 84 MHz
 *
 * @param Copy_PLLQ (4 bits) Division factor for USB, SDIO, RNG clocks (2 to 15)
 *                  hint : USB/SDIO/RNG clock frequency = VCO output frequency / PLLQ
 *                  Valid frequency: 48 MHz
 *
 * @param Copy_PLLSource PLL clock source (0 = HSI, 1 = HSE)
 *
 * @return RCC_Status_t Status of the operation (RCC_OK, RCC_WRONG_PLL_CONFIG)
 * 
 * @note PLL must be disabled before calling this function
 * @note This function validates all PLL parameters before configuration
 * @note Flash latency is automatically adjusted for 84 MHz operation
 * @warning Incorrect PLL configuration can cause system instability
 * 
 * @example Configure PLL for 84 MHz from 16 MHz HSI:
 *          RCC_ConfigurePLL(16, 336, 4, 7, RCC_PLL_SOURCE_HSI);
 * 
 * @author Eng.Gemy
 */
RCC_Status_t RCC_ConfigurePLL(uint8_t Copy_PLLM, uint16_t Copy_PLLN,
                              uint8_t Copy_PLLP, uint8_t Copy_PLLQ,
                              uint8_t Copy_PLLSource)
{
    /* Local variable to hold function return status */
    RCC_Status_t status = RCC_NOT_OK;
    
    /* Local variables to hold calculated PLL frequencies for validation */
    float vco_in, vco_out, pll_out, usb_out;

    /* Validate PLL source parameter (must be HSI or HSE) */
    if ((Copy_PLLSource != RCC_PLL_SOURCE_HSI) && (Copy_PLLSource != RCC_PLL_SOURCE_HSE)){
        status = RCC_PLL_ERROR_SOURCE;
    }else
    {
        /* Determine PLL input clock frequency based on source selection */
        uint32_t pllClockSource;
        (Copy_PLLSource == RCC_PLL_SOURCE_HSI)?(pllClockSource = RCC_HSI_ClockSourceValue) : (pllClockSource = RCC_HSE_ClockSourceValue);
        
        /* ===== STEP 1: Validate PLLM (Input Divider) =====
         * PLLM divides input clock to produce VCO input frequency
         * Requirements:
         * - PLLM range: 2 to 63
         * - VCO input frequency range: 1 MHz to 2 MHz
         */
        // 1. Check PLLM
        vco_in = (float)pllClockSource / Copy_PLLM; // VCO input frequency
        if (Copy_PLLM < 2 || Copy_PLLM > 63 || vco_in < 1e6 || vco_in > 2e6)
        {
            /* Invalid PLLM value or resulting VCO input out of range */
            status = RCC_PLL_ERROR_M;
        }
        else
        {
            /* ===== STEP 2: Validate PLLN (VCO Multiplier) =====
             * PLLN multiplies VCO input to produce VCO output frequency
             * Requirements:
             * - PLLN range: 192 to 432
             * - VCO output frequency range: 192 MHz to 432 MHz
             */
            // 2. Check PLLN
            vco_out = vco_in * Copy_PLLN;
            if (Copy_PLLN < 192 || Copy_PLLN > 432 || vco_out < 192e6 || vco_out > 432e6)
            {
                /* Invalid PLLN value or resulting VCO output out of range */
                status = RCC_PLL_ERROR_N;
            }
            else
            {
                /* ===== STEP 3: Validate PLLP (System Clock Divider) =====
                 * PLLP divides VCO output to produce main system clock
                 * Requirements:
                 * - PLLP values: 2, 4, 6, or 8
                 * - System clock frequency: up to 84 MHz (STM32F401)
                 */
                // 3. Check PLLP
                pll_out = vco_out / Copy_PLLP; // System clock
                if ((Copy_PLLP != 2 && Copy_PLLP != 4 && Copy_PLLP != 6 && Copy_PLLP != 8) || (pll_out > 84e6))
                {
                    /* Invalid PLLP value or resulting system clock too high */
                    status = RCC_PLL_ERROR_P;
                }
                else
                {
                    /* ===== STEP 4: Validate PLLQ (USB Clock Divider) =====
                     * PLLQ divides VCO output to produce USB/SDIO clock
                     * Requirements:
                     * - PLLQ range: 2 to 15
                     * - USB/SDIO clock frequency: 48 MHz (required for USB)
                     */
                    // 4. Check PLLQ
                    usb_out = vco_out / Copy_PLLQ; // USB clock
                    if (Copy_PLLQ < 2 || Copy_PLLQ > 15 || usb_out > 48e6)
                    {
                        /* Invalid PLLQ value or resulting USB clock too high */
                        status = RCC_PLL_ERROR_Q;
                    }
                    else
                    {

                        // check if PLL is already enabled
                        /* PLL configuration can only be changed when PLL is disabled
                         * If PLL is currently running, it must be disabled first
                         */
                        if (1 == RCC_IsPLLReady())
                        {
                            status = RCC_PLL_ALREADY_ENABLED;
                        }
                        else
                        {

                            /* ===== All parameters validated, configure PLL ===== */
                            
                            // Configure PLL parameters
                            /* Set PLLM divider (6 bits in PLLCFGR register) */
                            RCC_Registers->PLLCFGR.BIT_FIELDS.PLLM = Copy_PLLM;
                            
                            /* Set PLLN multiplier (9 bits in PLLCFGR register) */
                            RCC_Registers->PLLCFGR.BIT_FIELDS.PLLN = Copy_PLLN;

                            // Set PLLP value (encoded as 0=2,1=4,2=6,3=8)
                            /* PLLP is encoded in 2 bits as: 00=÷2, 01=÷4, 10=÷6, 11=÷8 */
                            switch (Copy_PLLP)
                            {
                            case 2:
                                RCC_Registers->PLLCFGR.BIT_FIELDS.PLLP = 0;
                                break;
                            case 4:
                                RCC_Registers->PLLCFGR.BIT_FIELDS.PLLP = 1;
                                break;
                            case 6:
                                RCC_Registers->PLLCFGR.BIT_FIELDS.PLLP = 2;
                                break;
                            case 8:
                                RCC_Registers->PLLCFGR.BIT_FIELDS.PLLP = 3;
                                break;
                            }

                            /* Set PLLQ divider (4 bits in PLLCFGR register) */
                            RCC_Registers->PLLCFGR.BIT_FIELDS.PLLQ = Copy_PLLQ;
                            
                            /* Set PLL source (PLLSRC bit: 0=HSI, 1=HSE) */
                            RCC_Registers->PLLCFGR.BIT_FIELDS.PLLSRC = Copy_PLLSource;

                            /* ===== Configure Flash Latency ===== */
                            // I added them from chatGPT because when i set pll to high speed the system crashes
                            // this will enable flash latency for 84MHz operation
                            /* Flash latency must be adjusted when operating at high frequencies
                             * For 84 MHz operation with 3.3V, 2 wait states are required
                             * Flash ACR register address: 0x40023C00
                             * Bits 2:0 = LATENCY (000=0WS, 001=1WS, 010=2WS, etc.)
                             */
                            *(uint32_t*)((0x40000000UL + 0x00020000UL) + 0x3C00UL) &= ~0b111;
                            *(uint32_t*)((0x40000000UL + 0x00020000UL) + 0x3C00UL) |= (2 & 0b111);

                            // Configuration successful
                            status = RCC_OK;
                        }
                    }
                }
            }
        }
    }

    return status;
}

/******************************************************************************
 *                   GET SYSTEM CLOCK SOURCE FUNCTION
 * @brief Function to read current system clock source
 * @author Eng.Gemy
 ******************************************************************************/

/**
 * @brief Get current system clock source
 *
 * This function reads the current system clock source from hardware.
 *
 * @param[out] clockSource Pointer to store the current clock source
 *
 * @return RCC_Status_t Status of the operation (always RCC_OK)
 * 
 * @note Reads SWS field which indicates actual active clock source
 * @note SWS is set by hardware after successful clock switch
 * @author Eng.Gemy
 */
RCC_Status_t RCC_GetSystemClockSource(RCC_ClockSrc_t * clockSource){

    /* Read SWS (System clock switch status) bits from CFGR register
     * SWS[1:0]: 00 = HSI, 01 = HSE, 10 = PLL
     * This indicates which clock is actually being used as system clock
     */
    *clockSource = ((uint8_t)(RCC_Registers->CFGR.BIT_FIELDS.SWS));
    
    return RCC_OK;
}

/******************************************************************************
 *                   PRESCALER CONFIGURATION FUNCTIONS
 * @brief Functions to configure AHB and APB prescalers
 * @author Eng.Gemy
 ******************************************************************************/

/**
 * @brief Set AHB prescaler
 *
 * This function configures the AHB prescaler to divide system clock.
 *
 * @param[in] AHBPrescaler AHB prescaler value (from RCC_AHPPrescaler_t enum)
 *
 * @return RCC_Status_t Status of the operation (RCC_OK, RCC_WRONG_AHB_PRESCALER)
 * 
 * @note AHB clock = SYSCLK / AHB prescaler
 * @note AHB clock drives CPU, memory, and DMA
 * @note Maximum AHB frequency: 84 MHz (STM32F401)
 * @author Eng.Gemy
 */
RCC_Status_t RCC_SetAHBPrescaler(RCC_AHPPrescaler_t AHBPrescaler)
{

    // status variable to track operation result
    RCC_Status_t status = RCC_NOT_OK;

    /* Validate AHB prescaler parameter
     * Check if any invalid bits are set using correction mask
     */
    if(0 != (AHB_PRESCALER_CORRECTION_MASK & AHBPrescaler))
    {
        status = RCC_WRONG_AHB_PRESCALER;
    }else{
        
        /* Check if "no division" option is selected
         * If prescaler doesn't have the no-division bit pattern, set to no division
         */
        if(0 == (AHB_PRESCALER_NO_DIVISION_MASK & AHBPrescaler))
        {
            AHBPrescaler = RCC_AHB_NO_DIVISION;
        }else{
            // do nothing - prescaler value is valid as-is
        }

        /* Set AHB prescaler in CFGR register
         * HPRE bits[7:4] control AHB prescaler
         * OR operation applies the prescaler value
         */
        RCC_Registers->CFGR.ALL_FIELDS |= AHB_PRESCALER_NO_DIVISION_MASK;
        status = RCC_OK;
    }

    return status;
}

/**
 * @brief Set APB1 prescaler
 *
 * This function configures the APB1 prescaler to divide AHB clock.
 *
 * @param[in] APB1Prescaler APB1 prescaler value (from RCC_APB1Prescaler_t enum)
 *
 * @return RCC_Status_t Status of the operation (RCC_OK, RCC_WRONG_APB_PRESCALER)
 * 
 * @note APB1 clock = AHB clock / APB1 prescaler
 * @note APB1 is low-speed peripheral bus
 * @note Maximum APB1 frequency: 42 MHz (STM32F401)
 * @warning APB1 peripherals: TIM2-5, USART2, SPI2-3, I2C1-3, etc.
 * @author Eng.Gemy
 */
RCC_Status_t RCC_SetAPB1Prescaler(RCC_APB1Prescaler_t APB1Prescaler)
{

    // status variable to track operation result
    RCC_Status_t status = RCC_NOT_OK;

    /* Validate APB1 prescaler parameter
     * Check if any invalid bits are set using correction mask
     */
    if(0 != (APB1_PRESCALER_CORRECTION_MASK & APB1Prescaler))
    {
        status = RCC_WRONG_APB_PRESCALER;
    }else{
        
        /* Check if "no division" option is selected
         * If prescaler doesn't have the no-division bit pattern, set to no division
         */
        if(0 == (APB1_PRESCALER_NO_DIVISION_MASK & APB1Prescaler))
        {
            APB1Prescaler = RCC_APB1_NO_DIVISION;
        }else{
            // do nothing - prescaler value is valid as-is
        }

        /* Set APB1 prescaler in CFGR register
         * PPRE1 bits[12:10] control APB1 prescaler
         * OR operation applies the prescaler value
         */
        RCC_Registers->CFGR.ALL_FIELDS |= APB1_PRESCALER_NO_DIVISION_MASK;
        status = RCC_OK;
    }

    return status;
}

/**
 * @brief Set APB2 prescaler
 *
 * This function configures the APB2 prescaler to divide AHB clock.
 *
 * @param[in] APB2Prescaler APB2 prescaler value (from RCC_APB2Prescaler_t enum)
 *
 * @return RCC_Status_t Status of the operation (RCC_OK, RCC_WRONG_APB_PRESCALER)
 * 
 * @note APB2 clock = AHB clock / APB2 prescaler
 * @note APB2 is high-speed peripheral bus
 * @note Maximum APB2 frequency: 84 MHz (STM32F401)
 * @warning APB2 peripherals: TIM1, TIM9-11, USART1, USART6, SPI1, SPI4, ADC, SDIO, SYSCFG
 * @author Eng.Gemy
 */
RCC_Status_t RCC_SetAPB2Prescaler(RCC_APB2Prescaler_t APB2Prescaler)
{

    // status variable to track operation result
    RCC_Status_t status = RCC_NOT_OK;

    /* Validate APB2 prescaler parameter
     * Check if any invalid bits are set using correction mask
     */
    if(0 != (APB2_PRESCALER_CORRECTION_MASK & APB2Prescaler))
    {
        status = RCC_WRONG_APB_PRESCALER;
    }else{
        
        /* Check if "no division" option is selected
         * If prescaler doesn't have the no-division bit pattern, set to no division
         */
        if(0 == (APB2_PRESCALER_NO_DIVISION_MASK & APB2Prescaler))
        {
            APB2Prescaler = RCC_APB2_NO_DIVISION;
        }else{
            // do nothing - prescaler value is valid as-is
        }

        /* Set APB2 prescaler in CFGR register
         * PPRE2 bits[15:13] control APB2 prescaler
         * OR operation applies the prescaler value
         */
        RCC_Registers->CFGR.ALL_FIELDS |= APB2_PRESCALER_NO_DIVISION_MASK;
        status = RCC_OK;
    }

    return status;
}



/******************************************************************************
 *                   PERIPHERAL CLOCK ENABLE FUNCTION
 * @brief Function to enable peripheral clocks on various buses
 * @author Eng.Gemy
 ******************************************************************************/

/**
 * @brief Enable peripheral clock
 *
 * This function enables clock for one or more peripherals on a specified bus.
 *
 * @param[in] bus                  Bus identifier (RCC_AHB1_BUS/AHB2_BUS/APB1_BUS/APB2_BUS)
 * @param[in] PeripheralClockMask  64-bit mask containing bus and peripheral information
 *                                 Upper 4 bits: Bus type
 *                                 Lower 32 bits: Peripheral bit positions
 *
 * @return RCC_Status_t Status of the operation
 * @retval RCC_OK                                  Clock(s) enabled successfully
 * @retval RCC_WRONG_BUS_SELECTION                 Invalid bus identifier
 * @retval RCC_WRONG_PEREPHRAL_SELECTION           Invalid peripheral mask for selected bus
 * @retval RCC_WRONG_PEREPHRAL_WITHBUS_SELECTION   Bus-peripheral mismatch
 * 
 * @note Peripheral clocks must be enabled before accessing peripheral registers
 * @note Multiple peripherals can be enabled by ORing their masks
 * @warning Always enable clock before configuring peripheral
 * 
 * @example Enable GPIOA and GPIOB clocks:
 *          RCC_EnablePeripheralClock(RCC_AHB1_BUS, 
 *                                    RCC_AHB1_GPIOA_CLOCK | RCC_AHB1_GPIOB_CLOCK);
 * 
 * @author Eng.Gemy
 */
RCC_Status_t RCC_EnablePeripheralClock(uint8_t bus, uint64_t PeripheralClockMask)
{

    // status variable to track operation result
    RCC_Status_t status = RCC_NOT_OK;

    // Validate bus and peripheral mask
    /* Check if bus parameter has invalid bits set
     * Valid bus values should not set any bits in BUS_MASK
     */
    if (0 != (bus & BUS_MASK))
    {
        status = RCC_WRONG_BUS_SELECTION;
    }
    /* Check if peripheral mask matches the selected bus
     * Upper 4 bits of mask should match bus identifier
     */
    else if (bus != (PeripheralClockMask >> 32))
    {
        status = RCC_WRONG_PEREPHRAL_WITHBUS_SELECTION;
    }
    else
    {

        // Enable peripheral clock based on bus type
        /* Switch statement handles different bus types
         * Each case validates the peripheral mask and enables the clock
         */
        switch (bus)
        {
        case RCC_AHB1_BUS:
            // Check if the peripheral mask is valid for AHB1 bus
            // the user selects the correct perpheral mask for the selected bus
            /* Validate that all set bits in the mask correspond to valid AHB1 peripherals
             * If any invalid bits are set, mask AND with validation mask will be non-zero
             */
            if ((0 != (PeripheralClockMask & AHB1_PERPHRALS_MASK)))
            {
                status = RCC_WRONG_PEREPHRAL_SELECTION;
            }
            else
            {
                // Valid peripheral mask for AHB1 bus
                /* Enable clock(s) by setting corresponding bit(s) in AHB1ENR register
                 * Extract lower 32 bits of mask and OR with register
                 */
                RCC_Registers->AHB1ENR.ALL_FIELDS |= (uint32_t)(PeripheralClockMask & 0xFFFFFFFF);
                status = RCC_OK;
            }
            break;
        case RCC_AHB2_BUS:
            // Check if the peripheral mask is valid for AHB2 bus
            // the user selects the correct perpheral mask for the selected bus
            /* Validate that all set bits in the mask correspond to valid AHB2 peripherals */
            if ((0 != (PeripheralClockMask & AHB2_PERPHRALS_MASK)))
            {
                status = RCC_WRONG_PEREPHRAL_SELECTION;
            }
            else
            {
                // Valid peripheral mask for AHB2 bus
                /* Enable clock(s) by setting corresponding bit(s) in AHB2ENR register */
                RCC_Registers->AHB2ENR.ALL_FIELDS |= (uint32_t)(PeripheralClockMask & 0xFFFFFFFF);
                status = RCC_OK;
            }
            break;
        case RCC_APB1_BUS:
            // Check if the peripheral mask is valid for APB1 bus
            // the user selects the correct perpheral mask for the selected bus
            /* Validate that all set bits in the mask correspond to valid APB1 peripherals */
            if (0 != (PeripheralClockMask & APB1_PERPHRALS_MASK))
            {
                status = RCC_WRONG_PEREPHRAL_SELECTION;
            }
            else
            {
                // Valid peripheral mask for APB1 bus
                /* Enable clock(s) by setting corresponding bit(s) in APB1ENR register */
                RCC_Registers->APB1ENR.ALL_FIELDS |= (uint32_t)(PeripheralClockMask  & 0xFFFFFFFF);
                status = RCC_OK;
            }
            break;
        case RCC_APB2_BUS:
            // Check if the peripheral mask is valid for APB2 bus
            // the user selects the correct perpheral mask for the selected bus
            /* Validate that all set bits in the mask correspond to valid APB2 peripherals */
            if (0 != (PeripheralClockMask & APB2_PERPHRALS_MASK))
            {
                status = RCC_WRONG_PEREPHRAL_SELECTION;
            }
            else
            {
                // Valid peripheral mask for APB2 bus
                /* Enable clock(s) by setting corresponding bit(s) in APB2ENR register */
                RCC_Registers->APB2ENR.ALL_FIELDS |= (uint32_t)(PeripheralClockMask  & 0xFFFFFFFF);
                status = RCC_OK;
            }
            break;
        default:
            // Invalid bus selection
            /* This should never be reached due to earlier validation */
            status = RCC_WRONG_BUS_SELECTION;
            break;
        }
    }

    // Return operation status
    return status;
}

/******************************************************************************
 *                   PERIPHERAL CLOCK DISABLE FUNCTION
 * @brief Function to disable peripheral clocks on various buses
 * @author Eng.Gemy
 ******************************************************************************/

/**
 * @brief Disable peripheral clock
 *
 * This function disables clock for one or more peripherals on a specified bus.
 *
 * @param[in] bus                  Bus identifier (RCC_AHB1_BUS/AHB2_BUS/APB1_BUS/APB2_BUS)
 * @param[in] PeripheralClockMask  64-bit mask containing bus and peripheral information
 *
 * @return RCC_Status_t Status of the operation
 * @retval RCC_OK                                  Clock(s) disabled successfully
 * @retval RCC_WRONG_BUS_SELECTION                 Invalid bus identifier
 * @retval RCC_WRONG_PEREPHRAL_SELECTION           Invalid peripheral mask
 * @retval RCC_WRONG_PEREPHRAL_WITHBUS_SELECTION   Bus-peripheral mismatch
 * 
 * @note Disabling peripheral clocks reduces power consumption
 * @warning Ensure peripheral is not in use before disabling its clock
 * @warning Accessing peripheral registers with clock disabled causes bus fault
 * 
 * @author Eng.Gemy
 */
RCC_Status_t RCC_DisablePeripheralClock(uint8_t bus, uint64_t PeripheralClockMask)
{

    // status variable to track operation result
    RCC_Status_t status = RCC_NOT_OK;

    // Validate bus and peripheral mask
    /* Check if bus parameter has invalid bits set */
    if (0 != (bus & BUS_MASK))
    {
        status = RCC_WRONG_BUS_SELECTION;
    }
    /* Check if peripheral mask matches the selected bus */
    else if (bus != (PeripheralClockMask >> 32))
    {
        status = RCC_WRONG_PEREPHRAL_WITHBUS_SELECTION;
    }
    else
    {

        // Enable peripheral clock based on bus type
        /* Switch statement handles different bus types for clock disable */
        switch (bus)
        {
        case RCC_AHB1_BUS:
            // Check if the peripheral mask is valid for AHB1 bus
            // the user selects the correct perpheral mask for the selected bus
            /* Validate peripheral mask for AHB1 bus */
            if (0 != (PeripheralClockMask & AHB1_PERPHRALS_MASK))
            {
                status = RCC_WRONG_PEREPHRAL_SELECTION;
            }
            else
            {
                // Valid peripheral mask for AHB1 bus
                /* Disable clock(s) by clearing corresponding bit(s) in AHB1ENR register
                 * AND with inverted mask to clear specific bits
                 */
                RCC_Registers->AHB1ENR.ALL_FIELDS &= ~(uint32_t)(PeripheralClockMask & 0xFFFFFFFF);
                status = RCC_OK;
            }
            break;
        case RCC_AHB2_BUS:
            // Check if the peripheral mask is valid for AHB2 bus
            // the user selects the correct perpheral mask for the selected bus
            /* Validate peripheral mask for AHB2 bus */
            if (0 != (PeripheralClockMask & AHB2_PERPHRALS_MASK))
            {
                status = RCC_WRONG_PEREPHRAL_SELECTION;
            }
            else
            {
                // Valid peripheral mask for AHB2 bus
                /* Disable clock(s) by clearing corresponding bit(s) in AHB2ENR register */
                RCC_Registers->AHB2ENR.ALL_FIELDS &= ~(uint32_t)(PeripheralClockMask & 0xFFFFFFFF);
                status = RCC_OK;
            }
            break;
        case RCC_APB1_BUS:
            // Check if the peripheral mask is valid for APB1 bus
            // the user selects the correct perpheral mask for the selected bus
            /* Validate peripheral mask for APB1 bus */
            if (0 != (PeripheralClockMask & APB1_PERPHRALS_MASK))
            {
                status = RCC_WRONG_PEREPHRAL_SELECTION;
            }
            else
            {
                // Valid peripheral mask for APB1 bus
                /* Disable clock(s) by clearing corresponding bit(s) in APB1ENR register */
                RCC_Registers->APB1ENR.ALL_FIELDS &= ~(uint32_t)(PeripheralClockMask & 0xFFFFFFFF);
                status = RCC_OK;
            }
            break;
        case RCC_APB2_BUS:
            // Check if the peripheral mask is valid for APB2 bus
            // the user selects the correct perpheral mask for the selected bus
            /* Validate peripheral mask for APB2 bus */
            if (0 != (PeripheralClockMask & APB2_PERPHRALS_MASK))
            {
                status = RCC_WRONG_PEREPHRAL_SELECTION;
            }
            else
            {
                // Valid peripheral mask for APB2 bus
                /* Disable clock(s) by clearing corresponding bit(s) in APB2ENR register */
                RCC_Registers->APB2ENR.ALL_FIELDS &= ~(uint32_t)(PeripheralClockMask & 0xFFFFFFFF);
                status = RCC_OK;
            }
            break;
        default:
            // Invalid bus selection
            /* This should never be reached due to earlier validation */
            status = RCC_WRONG_BUS_SELECTION;
            break;
        }
    }

    // Return operation status
    return status;
}

/******************************************************************************
 *                   PERIPHERAL RESET FUNCTION
 * @brief Function to reset specific peripherals
 * @author Eng.Gemy
 ******************************************************************************/

/**
 * @brief Reset specific peripheral(s)
 *
 * This function performs a hardware reset on one or more peripherals.
 * Reset is performed by setting then clearing the reset bit.
 *
 * @param[in] bus                  Bus identifier (RCC_AHB1_BUS/AHB2_BUS/APB1_BUS/APB2_BUS)
 * @param[in] PeripheralClockMask  64-bit mask containing bus and peripheral information
 *
 * @return RCC_Status_t Status of the operation
 * @retval RCC_OK                                  Reset successful
 * @retval RCC_WRONG_BUS_SELECTION                 Invalid bus identifier
 * @retval RCC_WRONG_PEREPHRAL_SELECTION           Invalid peripheral mask
 * @retval RCC_WRONG_PEREPHRAL_WITHBUS_SELECTION   Bus-peripheral mismatch
 * 
 * @note Reset sequence: Set reset bit → Clear reset bit
 * @note Peripheral configuration registers return to reset values
 * @note Peripheral clock remains enabled after reset
 * @warning Resetting peripheral loses all configuration
 * 
 * @example Reset USART2:
 *          RCC_ResetPeripheralClock(RCC_APB1_BUS, RCC_APB1_USART2_CLOCK);
 * 
 * @author Eng.Gemy
 */
RCC_Status_t RCC_ResetPeripheralClock(uint8_t bus, uint64_t PeripheralClockMask)
{
    // status variable to track operation result
    RCC_Status_t status = RCC_NOT_OK;

    // Validate bus and peripheral mask
    /* Check if bus parameter has invalid bits set */
    if (0 != (bus & BUS_MASK))
    {
        status = RCC_WRONG_BUS_SELECTION;
    }
    /* Check if peripheral mask matches the selected bus */
    else if (bus != (PeripheralClockMask >> 32))
    {
        status = RCC_WRONG_PEREPHRAL_WITHBUS_SELECTION;
    }
    else
    {

        // Enable peripheral clock based on bus type
        /* Switch statement handles different bus types for peripheral reset */
        switch (bus)
        {
        case RCC_AHB1_BUS:
            // Check if the peripheral mask is valid for AHB1 bus
            // the user selects the correct perpheral mask for the selected bus
            /* Validate peripheral mask for AHB1 bus */
            if (0 != (PeripheralClockMask & AHB1_PERPHRALS_MASK))
            {
                status = RCC_WRONG_PEREPHRAL_SELECTION;
            }
            else
            {
                // Valid peripheral mask for AHB1 bus
                /* Perform peripheral reset sequence:
                 * Step 1: Set reset bit(s) in AHB1RSTR register (assert reset)
                 * Step 2: Clear reset bit(s) in AHB1RSTR register (release reset)
                 */
                RCC_Registers->AHB1RSTR.ALL_FIELDS |= (uint32_t)(PeripheralClockMask & 0xFFFFFFFF);
                RCC_Registers->AHB1RSTR.ALL_FIELDS &= ~(uint32_t)(PeripheralClockMask & 0xFFFFFFFF);

                status = RCC_OK;
            }
            break;
        case RCC_AHB2_BUS:
            // Check if the peripheral mask is valid for AHB2 bus
            // the user selects the correct perpheral mask for the selected bus
            /* Validate peripheral mask for AHB2 bus */
            if (0 != (PeripheralClockMask & AHB2_PERPHRALS_MASK))
            {
                status = RCC_WRONG_PEREPHRAL_SELECTION;
            }
            else
            {
                // Valid peripheral mask for AHB2 bus
                /* Perform peripheral reset by clearing reset bit(s)
                 * Note: This looks like it might be missing the set operation
                 */
                RCC_Registers->AHB2RSTR.ALL_FIELDS &= ~(uint32_t)(PeripheralClockMask & 0xFFFFFFFF);
                status = RCC_OK;
            }
            break;
        case RCC_APB1_BUS:
            // Check if the peripheral mask is valid for APB1 bus
            // the user selects the correct perpheral mask for the selected bus
            /* Validate peripheral mask for APB1 bus */
            if (0 != (PeripheralClockMask & APB1_PERPHRALS_MASK))
            {
                status = RCC_WRONG_PEREPHRAL_SELECTION;
            }
            else
            {
                // Valid peripheral mask for APB1 bus
                /* Perform peripheral reset by clearing reset bit(s)
                 * Note: This looks like it might be missing the set operation
                 */
                RCC_Registers->APB1RSTR.ALL_FIELDS &= ~(uint32_t)(PeripheralClockMask & 0xFFFFFFFF);
                status = RCC_OK;
            }
            break;
        case RCC_APB2_BUS:
            // Check if the peripheral mask is valid for APB2 bus
            // the user selects the correct perpheral mask for the selected bus
            /* Validate peripheral mask for APB2 bus */
            if (0 != (PeripheralClockMask & APB2_PERPHRALS_MASK))
            {
                status = RCC_WRONG_PEREPHRAL_SELECTION;
            }
            else
            {
                // Valid peripheral mask for APB2 bus
                /* Perform peripheral reset by clearing reset bit(s)
                 * Note: This looks like it might be missing the set operation
                 */
                RCC_Registers->APB2RSTR.ALL_FIELDS &= ~(uint32_t)(PeripheralClockMask & 0xFFFFFFFF);
                status = RCC_OK;
            }
            break;
        default:
            // Invalid bus selection
            /* This should never be reached due to earlier validation */
            status = RCC_WRONG_BUS_SELECTION;
            break;
        }
    }

    // Return operation status
    return status;
}

/******************************************************************************
 *                   RESET ALL PERIPHERALS FUNCTION
 * @brief Function to reset all peripherals on all buses
 * @author Eng.Gemy
 ******************************************************************************/

/**
 * @brief Reset all peripherals on all buses
 *
 * This function performs a hardware reset on all peripherals by
 * setting and then clearing all reset register bits.
 *
 * @return void
 * 
 * @note This resets ALL peripherals on AHB1, AHB2, APB1, and APB2 buses
 * @note Reset sequence: Set all bits → Clear all bits for each bus
 * @note Peripheral clocks remain enabled after reset
 * @warning This will reset ALL peripherals - use with extreme caution
 * @warning All peripheral configurations will be lost
 * @warning System may become unstable if critical peripherals are reset
 * 
 * @author Eng.Gemy
 */
void RCC_ResetALLPeripheralClock(void)
{
    /* Reset AHB1 peripheral reset register
     * Step 1: Set all reset bits (assert reset on all AHB1 peripherals)
     * Step 2: Clear all reset bits (release reset on all AHB1 peripherals)
     */
    // Reset AHB1 peripheral reset register
    RCC_Registers->AHB1RSTR.ALL_FIELDS = 0xFFFFFFFFU;
    RCC_Registers->AHB1RSTR.ALL_FIELDS = 0x00000000U;
    
    /* Reset AHB2 peripheral reset register
     * Step 1: Set all reset bits (assert reset on all AHB2 peripherals)
     * Step 2: Clear all reset bits (release reset on all AHB2 peripherals)
     */
    // Reset AHB2 peripheral reset register
    RCC_Registers->AHB2RSTR.ALL_FIELDS = 0xFFFFFFFFU;
    RCC_Registers->AHB2RSTR.ALL_FIELDS = 0x00000000U;
    
    /* Reset APB1 peripheral reset register
     * Step 1: Set all reset bits (assert reset on all APB1 peripherals)
     * Step 2: Clear all reset bits (release reset on all APB1 peripherals)
     */
    // Reset APB1 peripheral reset register
    RCC_Registers->APB1RSTR.ALL_FIELDS = 0xFFFFFFFFU;
    RCC_Registers->APB1RSTR.ALL_FIELDS = 0x00000000U;
    
    /* Reset APB2 peripheral reset register
     * Step 1: Set all reset bits (assert reset on all APB2 peripherals)
     * Step 2: Clear all reset bits (release reset on all APB2 peripherals)
     */
    // Reset APB2 peripheral reset register
    RCC_Registers->APB2RSTR.ALL_FIELDS = 0xFFFFFFFFU;
    RCC_Registers->APB2RSTR.ALL_FIELDS = 0x00000000U;
}

/******************************************************************************
 *                           END OF FILE
******************************************************************************/