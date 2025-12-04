/******************************************************************************
 * @file    NVIC_STM32F401CC.H
 * @author  Eng.Gemy
 * @brief   NVIC Driver Interface Header File for STM32F401CC Black Pill Board
 *          This file contains STM32F401CC-specific interrupt definitions,
 *          type-safe priority values, and function prototypes for NVIC control
 * @date    [Current Date]
 * @version 1.0
 * @note    This is a Black Pill (BP) specific wrapper for NVIC driver
 *          Provides STM32F401CC Black Pill board-specific IRQ numbers and formatted priorities
 ******************************************************************************/

#ifndef NVIC_STM32F401CC_H
#define NVIC_STM32F401CC_H

/******************************************************************************
 * @brief NVIC Black Pill Status Enumeration
 * @details Defines all possible return status codes for NVIC BP operations
 * @note BP = Black Pill, provides enhanced error checking for Black Pill board
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
    NVIC_BP_NOT_OK,                 /**< Operation failed */
    NVIC_BP_OK,                     /**< Operation completed successfully */
    NVIC_BP_NULL_PTR,               /**< Null pointer passed as argument */
    NVIC_BP_WRONG_IRQ,              /**< Invalid IRQ number for STM32F401CC Black Pill */
    NVIC_BP_WRONG_PRIORITY,         /**< Invalid priority value */
    NVIC_BP_WRONG_PRIORITY_GROUP,   /**< Invalid priority grouping value */

}NVIC_BP_Status_t;

/******************************************************************************
 * @brief NVIC Black Pill Pending Status Enumeration
 * @details Defines interrupt pending states for Black Pill board
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
    NVIC_BP_NOT_PENDING,    /**< Interrupt is not pending */
    NVIC_BP_PENDING         /**< Interrupt is pending (waiting to be serviced) */
}NVIC_BP_Pending_t;

/******************************************************************************
 * @brief NVIC Black Pill Active Status Enumeration
 * @details Defines interrupt active states for Black Pill board
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
    NVIC_BP_NOT_ACTIVE,     /**< Interrupt is not active */
    NVIC_BP_ACTIVE          /**< Interrupt is currently being serviced */
}NVIC_BP_Active_t;

/******************************************************************************
 * @brief STM32F401CC Black Pill IRQ Numbers Enumeration
 * @details Defines all available interrupt request numbers for STM32F401CC Black Pill
 * @note IRQ numbers map to specific peripherals in STM32F401CC (WeAct Black Pill V3.0)
 * @note Not all IRQ numbers 0-85 are used (gaps in enumeration)
 * @note Maximum IRQ number for STM32F401CC is 84
 * @note Black Pill board uses STM32F401CCU6 chip
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
    NVIC_WWDG_IRQ                 = 0,      /**< Window Watchdog interrupt */
    NVIC_EXTI16_PVD_IRQ           = 1,      /**< PVD through EXTI Line 16 detection interrupt */
    NVIC_EXTI21_TAMP_STAMP_IRQ    = 2,      /**< Tamper and TimeStamp through EXTI Line 21 */
    NVIC_EXTI22_RTC_WKUP_IRQ      = 3,      /**< RTC Wakeup through EXTI Line 22 */
    NVIC_FLASH_IRQ                = 4,      /**< Flash global interrupt */
    NVIC_RCC_IRQ                  = 5,      /**< RCC global interrupt */
    NVIC_EXTI0_IRQ                = 6,      /**< EXTI Line 0 interrupt */
    NVIC_EXTI1_IRQ                = 7,      /**< EXTI Line 1 interrupt */
    NVIC_EXTI2_IRQ                = 8,      /**< EXTI Line 2 interrupt */
    NVIC_EXTI3_IRQ                = 9,      /**< EXTI Line 3 interrupt */
    NVIC_EXTI4_IRQ                = 10,     /**< EXTI Line 4 interrupt */

    NVIC_DMA1_STREAM0_IRQ         = 11,     /**< DMA1 Stream 0 global interrupt */
    NVIC_DMA1_STREAM1_IRQ         = 12,     /**< DMA1 Stream 1 global interrupt */
    NVIC_DMA1_STREAM2_IRQ         = 13,     /**< DMA1 Stream 2 global interrupt */
    NVIC_DMA1_STREAM3_IRQ         = 14,     /**< DMA1 Stream 3 global interrupt */
    NVIC_DMA1_STREAM4_IRQ         = 15,     /**< DMA1 Stream 4 global interrupt */
    NVIC_DMA1_STREAM5_IRQ         = 16,     /**< DMA1 Stream 5 global interrupt */
    NVIC_DMA1_STREAM6_IRQ         = 17,     /**< DMA1 Stream 6 global interrupt */

    NVIC_ADC_IRQ                  = 18,     /**< ADC1 global interrupt */

    NVIC_EXTI9_5_IRQ              = 23,     /**< EXTI Lines 5-9 interrupt */

    NVIC_TIM1_BRK_TIM9_IRQ        = 24,     /**< TIM1 Break and TIM9 global interrupt */
    NVIC_TIM1_UP_TIM10_IRQ        = 25,     /**< TIM1 Update and TIM10 global interrupt */
    NVIC_TIM1_TRG_COM_TIM11_IRQ   = 26,     /**< TIM1 Trigger/Commutation and TIM11 global interrupt */
    NVIC_TIM1_CC_IRQ              = 27,     /**< TIM1 Capture Compare interrupt */

    NVIC_TIM2_IRQ                 = 28,     /**< TIM2 global interrupt */
    NVIC_TIM3_IRQ                 = 29,     /**< TIM3 global interrupt */
    NVIC_TIM4_IRQ                 = 30,     /**< TIM4 global interrupt */

    NVIC_I2C1_EV_IRQ              = 31,     /**< I2C1 Event interrupt */
    NVIC_I2C1_ER_IRQ              = 32,     /**< I2C1 Error interrupt */
    NVIC_I2C2_EV_IRQ              = 33,     /**< I2C2 Event interrupt */
    NVIC_I2C2_ER_IRQ              = 34,     /**< I2C2 Error interrupt */

    NVIC_SPI1_IRQ                 = 35,     /**< SPI1 global interrupt */
    NVIC_SPI2_IRQ                 = 36,     /**< SPI2 global interrupt */

    NVIC_USART1_IRQ               = 37,     /**< USART1 global interrupt */
    NVIC_USART2_IRQ               = 38,     /**< USART2 global interrupt */

    NVIC_EXTI15_10_IRQ            = 40,     /**< EXTI Lines 10-15 interrupt */
    NVIC_EXTI17_RTC_ALARM_IRQ     = 41,     /**< RTC Alarm through EXTI Line 17 interrupt */
    NVIC_EXTI18_OTG_FS_WKUP_IRQ   = 42,     /**< USB OTG FS Wakeup through EXTI Line 18 */

    NVIC_DMA1_STREAM7_IRQ         = 47,     /**< DMA1 Stream 7 global interrupt */

    NVIC_SDIO_IRQ                 = 49,     /**< SDIO global interrupt */
    NVIC_TIM5_IRQ                 = 50,     /**< TIM5 global interrupt */
    NVIC_SPI3_IRQ                 = 51,     /**< SPI3 global interrupt */

    NVIC_DMA2_STREAM0_IRQ         = 56,     /**< DMA2 Stream 0 global interrupt */
    NVIC_DMA2_STREAM1_IRQ         = 57,     /**< DMA2 Stream 1 global interrupt */
    NVIC_DMA2_STREAM2_IRQ         = 58,     /**< DMA2 Stream 2 global interrupt */
    NVIC_DMA2_STREAM3_IRQ         = 59,     /**< DMA2 Stream 3 global interrupt */
    NVIC_DMA2_STREAM4_IRQ         = 60,     /**< DMA2 Stream 4 global interrupt */

    NVIC_OTG_FS_IRQ               = 67,     /**< USB OTG FS global interrupt */

    NVIC_DMA2_STREAM5_IRQ         = 68,     /**< DMA2 Stream 5 global interrupt */
    NVIC_DMA2_STREAM6_IRQ         = 69,     /**< DMA2 Stream 6 global interrupt */
    NVIC_DMA2_STREAM7_IRQ         = 70,     /**< DMA2 Stream 7 global interrupt */

    NVIC_USART6_IRQ               = 71,     /**< USART6 global interrupt */

    NVIC_I2C3_EV_IRQ              = 72,     /**< I2C3 Event interrupt */
    NVIC_I2C3_ER_IRQ              = 73,     /**< I2C3 Error interrupt */

    NVIC_FPU_IRQ                  = 81,     /**< FPU global interrupt */

    NVIC_SPI4_IRQ                 = 84      /**< SPI4 global interrupt */

} NVIC_BP_IRQ_t;

/******************************************************************************
 * @brief STM32F4 Black Pill Priority Levels Enumeration
 * @details Defines pre-formatted priority values for STM32F4 (4-bit priority)
 * @note Values are pre-shifted left by 4 bits for direct use in IPR register
 * @note STM32F4 uses only upper 4 bits of priority byte (bits 7:4)
 * @note Lower priority number = Higher priority level
 * @note NVIC_PRIORITY_0 = Highest priority (0x00 = 0000 0000)
 * @note NVIC_PRIORITY_15 = Lowest priority (0xF0 = 1111 0000)
 * @note Priority interpretation depends on PRIGROUP setting
 * @note These values can be used directly without bit shifting
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
    NVIC_PRIORITY_0  = 0x00,  /**< Priority 0 (Highest) - 0000 0000 */
    NVIC_PRIORITY_1  = 0x10,  /**< Priority 1 - 0001 0000 */
    NVIC_PRIORITY_2  = 0x20,  /**< Priority 2 - 0010 0000 */
    NVIC_PRIORITY_3  = 0x30,  /**< Priority 3 - 0011 0000 */
    NVIC_PRIORITY_4  = 0x40,  /**< Priority 4 - 0100 0000 */
    NVIC_PRIORITY_5  = 0x50,  /**< Priority 5 - 0101 0000 */
    NVIC_PRIORITY_6  = 0x60,  /**< Priority 6 - 0110 0000 */
    NVIC_PRIORITY_7  = 0x70,  /**< Priority 7 - 0111 0000 */
    NVIC_PRIORITY_8  = 0x80,  /**< Priority 8 - 1000 0000 */
    NVIC_PRIORITY_9  = 0x90,  /**< Priority 9 - 1001 0000 */
    NVIC_PRIORITY_10 = 0xA0,  /**< Priority 10 - 1010 0000 */
    NVIC_PRIORITY_11 = 0xB0,  /**< Priority 11 - 1011 0000 */
    NVIC_PRIORITY_12 = 0xC0,  /**< Priority 12 - 1100 0000 */
    NVIC_PRIORITY_13 = 0xD0,  /**< Priority 13 - 1101 0000 */
    NVIC_PRIORITY_14 = 0xE0,  /**< Priority 14 - 1110 0000 */
    NVIC_PRIORITY_15 = 0xF0   /**< Priority 15 (Lowest) - 1111 0000 (lowest) */
} NVIC_BP_Priority_t;

/******************************************************************************
 * @brief Priority Grouping Configuration Enumeration for Black Pill
 * @details Defines how priority bits are split between preemption and subpriority
 * @note STM32F4 uses 4 priority bits (bits 7:4 of priority byte)
 * @note Group priority determines interrupt preemption capability
 * @note Subpriority determines order when group priority is equal
 * 
 * @note Priority Grouping Options:
 *       PRIORITYGROUP_0 (0b000): [0:4] - No preemption, 16 subpriority levels
 *       PRIORITYGROUP_1 (0b100): [1:3] - 2 preemption levels, 8 subpriority levels
 *       PRIORITYGROUP_2 (0b101): [2:2] - 4 preemption levels, 4 subpriority levels (balanced)
 *       PRIORITYGROUP_3 (0b110): [3:1] - 8 preemption levels, 2 subpriority levels
 *       PRIORITYGROUP_4 (0b111): [4:0] - 16 preemption levels, no subpriority (full preemption)
 * 
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
    NVIC_BP_PRIORITYGROUP_0 = 0b000,  /**< 0 bits preemption, 4 bits subpriority - No preemption possible */
    NVIC_BP_PRIORITYGROUP_1 = 0b100,  /**< 1 bit  preemption, 3 bits subpriority - 2 preemption levels */
    NVIC_BP_PRIORITYGROUP_2 = 0b101,  /**< 2 bits preemption, 2 bits subpriority - 4 preemption levels (balanced) */
    NVIC_BP_PRIORITYGROUP_3 = 0b110,  /**< 3 bits preemption, 1 bit  subpriority - 8 preemption levels */
    NVIC_BP_PRIORITYGROUP_4 = 0b111   /**< 4 bits preemption, 0 bits subpriority - 16 preemption levels (full) */
} NVIC_BP_PriorityGroupBits_t;



/******************************************************************************
 *                           FUNCTION PROTOTYPES
 * @brief STM32F401CC Black Pill-specific NVIC driver public API functions
 * @note These functions provide type-safe wrappers with STM32F401CC validation
 * @author Eng.Gemy
 ******************************************************************************/

/**
 * @brief Enable external interrupt in NVIC (STM32F401CC Black Pill-specific)
 * 
 * @param[in] IRQn  Interrupt request number from NVIC_BP_IRQ_t enum
 * 
 * @return NVIC_BP_Status_t Status of the operation
 * @retval NVIC_BP_OK        Interrupt enabled successfully
 * @retval NVIC_BP_WRONG_IRQ Invalid IRQ number for STM32F401CC Black Pill
 * 
 * @note This function validates IRQ number against STM32F401CC valid IRQs
 * @note Provides type safety using NVIC_BP_IRQ_t enumeration
 * 
 * @example Enable USART1 interrupt:
 *          NVIC_BP_EnableIRQ(NVIC_USART1_IRQ);
 * 
 * @author Eng.Gemy
 */
NVIC_BP_Status_t NVIC_BP_EnableIRQ(NVIC_BP_IRQ_t IRQn);

/**
 * @brief Disable external interrupt in NVIC (STM32F401CC Black Pill-specific)
 * 
 * @param[in] IRQn  Interrupt request number from NVIC_BP_IRQ_t enum
 * 
 * @return NVIC_BP_Status_t Status of the operation
 * @retval NVIC_BP_OK        Interrupt disabled successfully
 * @retval NVIC_BP_WRONG_IRQ Invalid IRQ number for STM32F401CC Black Pill
 * 
 * @note This function validates IRQ number against STM32F401CC valid IRQs
 * 
 * @example Disable USART1 interrupt:
 *          NVIC_BP_DisableIRQ(NVIC_USART1_IRQ);
 * 
 * @author Eng.Gemy
 */
NVIC_BP_Status_t NVIC_BP_DisableIRQ(NVIC_BP_IRQ_t IRQn);

/**
 * @brief Get pending status of external interrupt
 * 
 * @param[in]  IRQn       Interrupt request number from NVIC_BP_IRQ_t enum
 * @param[out] Pointer to store pending status (NVIC_BP_PENDING or NVIC_BP_NOT_PENDING)
 * 
 * @return NVIC_BP_Status_t Status of the operation
 * @retval NVIC_BP_OK        Status read successfully
 * @retval NVIC_BP_WRONG_IRQ Invalid IRQ number
 * @retval NVIC_BP_NULL_PTR  Null pointer passed
 * 
 * @note Pending means interrupt has been triggered but not yet serviced
 * 
 * @example Check if USART1 interrupt is pending:
 *          NVIC_BP_Pending_t status;
 *          NVIC_BP_GetPendingIRQ(NVIC_USART1_IRQ, &status);
 * 
 * @author Eng.Gemy
 */
NVIC_BP_Status_t NVIC_BP_GetPendingIRQ (NVIC_BP_IRQ_t IRQn,NVIC_BP_Pending_t*);

/**
 * @brief Set interrupt to pending state (software trigger)
 * 
 * @param[in] IRQn  Interrupt request number from NVIC_BP_IRQ_t enum
 * 
 * @return NVIC_BP_Status_t Status of the operation
 * @retval NVIC_BP_OK        Pending flag set successfully
 * @retval NVIC_BP_WRONG_IRQ Invalid IRQ number
 * 
 * @note Software can trigger interrupt by setting pending flag
 * @note Useful for testing ISRs or software-generated interrupts
 * 
 * @example Trigger USART1 interrupt from software:
 *          NVIC_BP_SetPendingIRQ(NVIC_USART1_IRQ);
 * 
 * @author Eng.Gemy
 */
NVIC_BP_Status_t NVIC_BP_SetPendingIRQ (NVIC_BP_IRQ_t IRQn);

/**
 * @brief Clear pending status of interrupt
 * 
 * @param[in] IRQn  Interrupt request number from NVIC_BP_IRQ_t enum
 * 
 * @return NVIC_BP_Status_t Status of the operation
 * @retval NVIC_BP_OK        Pending status cleared successfully
 * @retval NVIC_BP_WRONG_IRQ Invalid IRQ number
 * 
 * @note Clears pending flag to prevent interrupt from triggering
 * @note Useful to cancel pending interrupt before enabling
 * 
 * @example Clear pending USART1 interrupt:
 *          NVIC_BP_ClearPendingIRQ(NVIC_USART1_IRQ);
 * 
 * @author Eng.Gemy
 */
NVIC_BP_Status_t NVIC_BP_ClearPendingIRQ (NVIC_BP_IRQ_t IRQn);

/**
 * @brief Get active status of external interrupt
 * 
 * @param[in]  IRQn      Interrupt request number from NVIC_BP_IRQ_t enum
 * @param[out] Pointer to store active status (NVIC_BP_ACTIVE or NVIC_BP_NOT_ACTIVE)
 * 
 * @return NVIC_BP_Status_t Status of the operation
 * @retval NVIC_BP_OK        Status read successfully
 * @retval NVIC_BP_WRONG_IRQ Invalid IRQ number
 * @retval NVIC_BP_NULL_PTR  Null pointer passed
 * 
 * @note Active means interrupt is currently being serviced (in ISR)
 * @note Cannot be set by software (read-only status)
 * @note Useful for debugging nested interrupt scenarios
 * 
 * @example Check if USART1 interrupt is active:
 *          NVIC_BP_Active_t status;
 *          NVIC_BP_GetActive(NVIC_USART1_IRQ, &status);
 * 
 * @author Eng.Gemy
 */
NVIC_BP_Status_t NVIC_BP_GetActive (NVIC_BP_IRQ_t IRQn,NVIC_BP_Active_t*);

/**
 * @brief Set priority for external interrupt with type-safe priority value
 * 
 * @param[in] IRQn     Interrupt request number from NVIC_BP_IRQ_t enum
 * @param[in] Priority Priority value from NVIC_BP_Priority_t enum (pre-formatted)
 * 
 * @return NVIC_BP_Status_t Status of the operation
 * @retval NVIC_BP_OK              Priority set successfully
 * @retval NVIC_BP_WRONG_IRQ       Invalid IRQ number
 * @retval NVIC_BP_WRONG_PRIORITY  Invalid priority value
 * 
 * @note Uses pre-formatted priority values (already shifted left by 4)
 * @note Priority values from NVIC_BP_Priority_t are ready to use directly
 * @note Lower priority number = Higher priority level
 * @note Priority 0 is highest, Priority 15 is lowest
 * @note Priority interpretation depends on PRIGROUP setting
 * 
 * @example Set USART1 interrupt to priority 5:
 *          NVIC_BP_SetPriority(NVIC_USART1_IRQ, NVIC_PRIORITY_5);
 * 
 * @warning Configure priority grouping before setting priorities
 * 
 * @author Eng.Gemy
 */
NVIC_BP_Status_t NVIC_BP_SetPriority (NVIC_BP_IRQ_t IRQn, NVIC_BP_Priority_t );

/**
 * @brief Get priority of external interrupt
 * 
 * @param[in]  IRQn     Interrupt request number from NVIC_BP_IRQ_t enum
 * @param[out] Pointer to store priority value (NVIC_BP_Priority_t type)
 * 
 * @return NVIC_BP_Status_t Status of the operation
 * @retval NVIC_BP_OK        Priority read successfully
 * @retval NVIC_BP_WRONG_IRQ Invalid IRQ number
 * @retval NVIC_BP_NULL_PTR  Null pointer passed
 * 
 * @note Returns pre-formatted priority value from NVIC_BP_Priority_t enum
 * @note Returned value interpretation depends on priority grouping setting
 * 
 * @example Get priority of USART1 interrupt:
 *          NVIC_BP_Priority_t priority;
 *          NVIC_BP_GetPriority(NVIC_USART1_IRQ, &priority);
 * 
 * @author Eng.Gemy
 */
NVIC_BP_Status_t NVIC_BP_GetPriority (NVIC_BP_IRQ_t IRQn,NVIC_BP_Priority_t *);

/**
 * @brief Set priority grouping for interrupt priorities
 * 
 * @param[in] Priority grouping value from NVIC_BP_PriorityGroupBits_t enum
 * 
 * @return NVIC_BP_Status_t Status of the operation
 * @retval NVIC_BP_OK                    Priority grouping set successfully
 * @retval NVIC_BP_WRONG_PRIORITY_GROUP  Invalid grouping value
 * 
 * @note Priority grouping splits priority bits into:
 *       - Group priority: Determines preemption (higher group can interrupt lower)
 *       - Subpriority: Determines order when group priority is same
 * 
 * @note Grouping options for STM32F401CC Black Pill:
 *       - PRIORITYGROUP_0: No preemption, 16 subpriority levels
 *       - PRIORITYGROUP_1: 2 preemption levels, 8 subpriority levels
 *       - PRIORITYGROUP_2: 4 preemption levels, 4 subpriority levels (balanced)
 *       - PRIORITYGROUP_3: 8 preemption levels, 2 subpriority levels
 *       - PRIORITYGROUP_4: 16 preemption levels, no subpriority (full preemption)
 * 
 * @note Should be set once at system initialization
 * 
 * @example Set balanced grouping (2 bits group, 2 bits sub):
 *          NVIC_BP_SetPriorityGrouping(NVIC_BP_PRIORITYGROUP_2);
 * 
 * @warning Changing priority grouping affects interpretation of all interrupt priorities
 * @warning Must be configured before setting interrupt priorities
 * 
 * @author Eng.Gemy
 */
NVIC_BP_Status_t NVIC_BP_SetPriorityGrouping(NVIC_BP_PriorityGroupBits_t);

#endif /* NVIC_STM32F401CC_H */