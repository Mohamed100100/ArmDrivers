/******************************************************************************
 * @file    NVIC_STM32F401CC.C
 * @author  Eng.Gemy
 * @brief   NVIC Driver Implementation File for STM32F401CC Black Pill Board
 *          This file contains the implementation of Black Pill-specific NVIC
 *          functions with input validation for STM32F401CC
 * @date    [Current Date]
 * @version 1.0
 * @note    BP = Black Pill - provides type-safe wrapper with validation
 ******************************************************************************/

#include "LIB/stdtypes.h"

#include "MCAL/NVIC_Driver/nvic.h"

#include "MCAL/NVIC_Driver/nvic_stm32f401cc.h"

/******************************************************************************
 *                        PRIORITY VALIDATION MASK
 * @brief Mask to validate priority values
 * @details STM32F4 uses upper 4 bits for priority (bits 7:4)
 *          Lower 4 bits (bits 3:0) should always be 0
 *          This mask checks bits 3:0 to ensure they are clear
 * @author Eng.Gemy
 ******************************************************************************/
#define PRIORITY_MASK_CHECK         (0x0FUL)  /**< Mask for bits 3:0 - should be 0 for valid priority */

/******************************************************************************
 *                        STATIC FUNCTION PROTOTYPES
 * @brief Private helper functions for validation
 * @author Eng.Gemy
 ******************************************************************************/

/**
 * @brief Validate IRQ number for STM32F401CC Black Pill
 * 
 * @param[in] IRQ number to validate
 * 
 * @return bool_t Validation result
 * @retval TRUE  IRQ number is valid for STM32F401CC
 * @retval FALSE IRQ number is invalid
 * 
 * @note Checks IRQ against list of valid interrupts for STM32F401CC
 * @note Static function - internal use only
 * 
 * @author Eng.Gemy
 */
static bool_t IsValidIRQ(NVIC_BP_IRQ_t );

/**
 * @brief Validate priority value format
 * 
 * @param[in] Priority value to validate
 * 
 * @return bool_t Validation result
 * @retval TRUE  Priority is properly formatted (lower 4 bits are 0)
 * @retval FALSE Priority has invalid format
 * 
 * @note Ensures priority value has lower 4 bits clear (bits 3:0 = 0)
 * @note STM32F4 uses only upper 4 bits for priority
 * @note Static function - internal use only
 * 
 * @author Eng.Gemy
 */
static bool_t IsValidPriority(NVIC_BP_Priority_t);


/******************************************************************************
 * @brief Enable external interrupt in NVIC (Black Pill wrapper)
 * 
 * @details This function validates the IRQ number against STM32F401CC valid
 *          interrupts before calling the generic NVIC enable function.
 * 
 * @param[in] IRQn  Interrupt request number from NVIC_BP_IRQ_t enum
 * 
 * @return NVIC_BP_Status_t Status of the operation
 * @retval NVIC_BP_OK        Interrupt enabled successfully
 * @retval NVIC_BP_WRONG_IRQ Invalid IRQ number for STM32F401CC
 * 
 * @note Provides input validation before calling generic NVIC driver
 * @note Ensures only valid STM32F401CC IRQs are enabled
 * 
 * @author Eng.Gemy
 ******************************************************************************/
NVIC_BP_Status_t NVIC_BP_EnableIRQ(NVIC_BP_IRQ_t IRQn){
    
    /* Local variable to hold function return status */
    NVIC_BP_Status_t status = NVIC_BP_NOT_OK;

    /* Validate IRQ number against STM32F401CC valid interrupts */
    if(FALSE == IsValidIRQ(IRQn)){
        /* IRQ number is not valid for STM32F401CC Black Pill */
        status = NVIC_BP_WRONG_IRQ;
    }else{
        /* IRQ is valid - call generic NVIC driver function
         * Cast return status to Black Pill status type
         */
        status = (NVIC_BP_Status_t)NVIC_EnableIRQ(IRQn);
    }
    return status;
}

/******************************************************************************
 * @brief Disable external interrupt in NVIC (Black Pill wrapper)
 * 
 * @details This function validates the IRQ number against STM32F401CC valid
 *          interrupts before calling the generic NVIC disable function.
 * 
 * @param[in] IRQn  Interrupt request number from NVIC_BP_IRQ_t enum
 * 
 * @return NVIC_BP_Status_t Status of the operation
 * @retval NVIC_BP_OK        Interrupt disabled successfully
 * @retval NVIC_BP_WRONG_IRQ Invalid IRQ number for STM32F401CC
 * 
 * @note Provides input validation before calling generic NVIC driver
 * 
 * @author Eng.Gemy
 ******************************************************************************/
NVIC_BP_Status_t NVIC_BP_DisableIRQ(NVIC_BP_IRQ_t IRQn){
    /* Local variable to hold function return status */
    NVIC_BP_Status_t status = NVIC_BP_NOT_OK;

    /* Validate IRQ number against STM32F401CC valid interrupts */
    if(FALSE == IsValidIRQ(IRQn)){
        /* IRQ number is not valid for STM32F401CC Black Pill */
        status = NVIC_BP_WRONG_IRQ;
    }else{
        /* IRQ is valid - call generic NVIC driver function
         * Cast return status to Black Pill status type
         */
        status = (NVIC_BP_Status_t)NVIC_DisableIRQ(IRQn);
    }
    return status;    
}

/******************************************************************************
 * @brief Get pending status of external interrupt (Black Pill wrapper)
 * 
 * @details This function validates the IRQ number before reading pending status
 * 
 * @param[in]  IRQn    Interrupt request number from NVIC_BP_IRQ_t enum
 * @param[out] pending Pointer to store pending status
 * 
 * @return NVIC_BP_Status_t Status of the operation
 * @retval NVIC_BP_OK        Status read successfully
 * @retval NVIC_BP_WRONG_IRQ Invalid IRQ number
 * @retval NVIC_BP_NULL_PTR  Null pointer passed (checked in generic driver)
 * 
 * @note Validates IRQ before calling generic driver
 * @note Pointer validation performed by generic NVIC driver
 * 
 * @author Eng.Gemy
 ******************************************************************************/
NVIC_BP_Status_t NVIC_BP_GetPendingIRQ (NVIC_BP_IRQ_t IRQn,NVIC_BP_Pending_t* pending){
    /* Local variable to hold function return status */
    NVIC_BP_Status_t status = NVIC_BP_NOT_OK;

    /* Validate IRQ number against STM32F401CC valid interrupts */
    if(FALSE == IsValidIRQ(IRQn)){
        /* IRQ number is not valid for STM32F401CC Black Pill */
        status = NVIC_BP_WRONG_IRQ;
    }else{
        /* IRQ is valid - call generic NVIC driver function
         * Cast pending pointer to generic type
         * Cast return status to Black Pill status type
         */
        status = (NVIC_BP_Status_t)NVIC_GetPendingIRQ(IRQn,(NVIC_Pending_t *)pending);
    }
    return status;
}

/******************************************************************************
 * @brief Set interrupt to pending state (Black Pill wrapper)
 * 
 * @details This function validates the IRQ number before setting pending flag
 * 
 * @param[in] IRQn  Interrupt request number from NVIC_BP_IRQ_t enum
 * 
 * @return NVIC_BP_Status_t Status of the operation
 * @retval NVIC_BP_OK        Pending flag set successfully
 * @retval NVIC_BP_WRONG_IRQ Invalid IRQ number
 * 
 * @note Software can trigger interrupt by setting pending flag
 * 
 * @author Eng.Gemy
 ******************************************************************************/
NVIC_BP_Status_t NVIC_BP_SetPendingIRQ (NVIC_BP_IRQ_t IRQn){
    /* Local variable to hold function return status */
    NVIC_BP_Status_t status = NVIC_BP_NOT_OK;

    /* Validate IRQ number against STM32F401CC valid interrupts */
    if(FALSE == IsValidIRQ(IRQn)){
        /* IRQ number is not valid for STM32F401CC Black Pill */
        status = NVIC_BP_WRONG_IRQ;
    }else{
        /* IRQ is valid - call generic NVIC driver function
         * Cast return status to Black Pill status type
         */
        status = (NVIC_BP_Status_t)NVIC_SetPendingIRQ(IRQn);
    }
    return status;   
}

/******************************************************************************
 * @brief Clear pending status of interrupt (Black Pill wrapper)
 * 
 * @details This function validates the IRQ number before clearing pending flag
 * 
 * @param[in] IRQn  Interrupt request number from NVIC_BP_IRQ_t enum
 * 
 * @return NVIC_BP_Status_t Status of the operation
 * @retval NVIC_BP_OK        Pending status cleared successfully
 * @retval NVIC_BP_WRONG_IRQ Invalid IRQ number
 * 
 * @note Clears pending flag to prevent interrupt from triggering
 * 
 * @author Eng.Gemy
 ******************************************************************************/
NVIC_BP_Status_t NVIC_BP_ClearPendingIRQ (NVIC_BP_IRQ_t IRQn){
    /* Local variable to hold function return status */
    NVIC_BP_Status_t status = NVIC_BP_NOT_OK;

    /* Validate IRQ number against STM32F401CC valid interrupts */
    if(FALSE == IsValidIRQ(IRQn)){
        /* IRQ number is not valid for STM32F401CC Black Pill */
        status = NVIC_BP_WRONG_IRQ;
    }else{
        /* IRQ is valid - call generic NVIC driver function
         * Cast return status to Black Pill status type
         */
        status = (NVIC_BP_Status_t)NVIC_ClearPendingIRQ(IRQn);
    }
    return status;    
}

/******************************************************************************
 * @brief Get active status of external interrupt (Black Pill wrapper)
 * 
 * @details This function validates the IRQ number before reading active status
 * 
 * @param[in]  IRQn   Interrupt request number from NVIC_BP_IRQ_t enum
 * @param[out] active Pointer to store active status
 * 
 * @return NVIC_BP_Status_t Status of the operation
 * @retval NVIC_BP_OK        Status read successfully
 * @retval NVIC_BP_WRONG_IRQ Invalid IRQ number
 * @retval NVIC_BP_NULL_PTR  Null pointer passed (checked in generic driver)
 * 
 * @note Active means interrupt is currently being serviced (in ISR)
 * 
 * @author Eng.Gemy
 ******************************************************************************/
NVIC_BP_Status_t NVIC_BP_GetActiveIRQ (NVIC_BP_IRQ_t IRQn,NVIC_BP_Active_t* active){
    /* Local variable to hold function return status */
    NVIC_BP_Status_t status = NVIC_BP_NOT_OK;

    /* Validate IRQ number against STM32F401CC valid interrupts */
    if(FALSE == IsValidIRQ(IRQn)){
        /* IRQ number is not valid for STM32F401CC Black Pill */
        status = NVIC_BP_WRONG_IRQ;
    }else{
        /* IRQ is valid - call generic NVIC driver function
         * Cast active pointer to generic type
         * Cast return status to Black Pill status type
         */
        status = (NVIC_BP_Status_t)NVIC_GetActiveIRQ(IRQn,(NVIC_Active_t *)active);
    }
    return status; 
}


/******************************************************************************
 * @brief Set priority for external interrupt (Black Pill wrapper)
 * 
 * @details This function validates both IRQ number and priority value format
 *          before setting the priority
 * 
 * @param[in] IRQn     Interrupt request number from NVIC_BP_IRQ_t enum
 * @param[in] priority Priority value from NVIC_BP_Priority_t enum
 * 
 * @return NVIC_BP_Status_t Status of the operation
 * @retval NVIC_BP_OK              Priority set successfully
 * @retval NVIC_BP_WRONG_IRQ       Invalid IRQ number
 * @retval NVIC_BP_WRONG_PRIORITY  Invalid priority format
 * 
 * @note Validates both IRQ and priority value format
 * @note Priority must have lower 4 bits clear (properly formatted)
 * 
 * @author Eng.Gemy
 ******************************************************************************/
NVIC_BP_Status_t NVIC_BP_SetPriority (NVIC_BP_IRQ_t IRQn, NVIC_BP_Priority_t priority){
    /* Local variable to hold function return status */
    NVIC_BP_Status_t status = NVIC_BP_NOT_OK;

    /* Validate IRQ number against STM32F401CC valid interrupts */
    if(FALSE == IsValidIRQ(IRQn)){
        /* IRQ number is not valid for STM32F401CC Black Pill */
        status = NVIC_BP_WRONG_IRQ;
    }else{
        
        /* Validate priority value format (lower 4 bits must be 0) */
        if(FALSE == IsValidPriority(priority)){
            /* Priority value is not properly formatted */
            status = NVIC_BP_WRONG_PRIORITY;
        }else{
            /* Both IRQ and priority are valid - call generic NVIC driver function
             * Cast return status to Black Pill status type
             */
            status = (NVIC_BP_Status_t)NVIC_SetPriority(IRQn,priority);
        }   
    }
    return status;    
}

/******************************************************************************
 * @brief Get priority of external interrupt (Black Pill wrapper)
 * 
 * @details This function validates the IRQ number before reading priority
 * 
 * @param[in]  IRQn     Interrupt request number from NVIC_BP_IRQ_t enum
 * @param[out] priority Pointer to store priority value
 * 
 * @return NVIC_BP_Status_t Status of the operation
 * @retval NVIC_BP_OK        Priority read successfully
 * @retval NVIC_BP_WRONG_IRQ Invalid IRQ number
 * @retval NVIC_BP_NULL_PTR  Null pointer passed (checked in generic driver)
 * 
 * @note Returns pre-formatted priority value
 * 
 * @author Eng.Gemy
 ******************************************************************************/
NVIC_BP_Status_t NVIC_BP_GetPriority (NVIC_BP_IRQ_t IRQn,NVIC_BP_Priority_t *priority){
    /* Local variable to hold function return status */
    NVIC_BP_Status_t status = NVIC_BP_NOT_OK;

    /* Validate IRQ number against STM32F401CC valid interrupts */
    if(FALSE == IsValidIRQ(IRQn)){
        /* IRQ number is not valid for STM32F401CC Black Pill */
        status = NVIC_BP_WRONG_IRQ;
    }else{
        /* IRQ is valid - call generic NVIC driver function
         * Cast return status to Black Pill status type
         * Priority pointer is already compatible type
         */
        status = (NVIC_BP_Status_t)NVIC_GetPriority(IRQn,priority);
    }
    return status; 
}

/******************************************************************************
 * @brief Set priority grouping for interrupt priorities (Black Pill wrapper)
 * 
 * @details This function validates and normalizes the priority grouping value
 *          before setting it in the AIRCR register
 * 
 * @param[in] groupBits Priority grouping value from NVIC_BP_PriorityGroupBits_t enum
 * 
 * @return NVIC_BP_Status_t Status of the operation
 * @retval NVIC_BP_OK                    Priority grouping set successfully
 * @retval NVIC_BP_WRONG_PRIORITY_GROUP  Invalid grouping value
 * 
 * @note Validates grouping value range (0-4)
 * @note Normalizes PRIORITYGROUP_0 if value is less than PRIORITYGROUP_1
 * @note Should be called once at system initialization
 * 
 * @author Eng.Gemy
 ******************************************************************************/
NVIC_BP_Status_t NVIC_BP_SetPriorityGrouping(NVIC_BP_PriorityGroupBits_t groupBits){
    /* Local variable to hold function return status */
    NVIC_BP_Status_t status = NVIC_BP_NOT_OK;

    /* Validate priority grouping value (must be 0-4) */
    if(groupBits > NVIC_BP_PRIORITYGROUP_4){
        /* Priority grouping value is out of valid range */
        status = NVIC_BP_WRONG_PRIORITY_GROUP;
    }else{
        /* Normalize grouping value for edge case
         * If grouping is less than PRIORITYGROUP_1 (0b100)
         * Force it to PRIORITYGROUP_0 (0b000)
         */
        if(groupBits < NVIC_BP_PRIORITYGROUP_1){
            groupBits = NVIC_BP_PRIORITYGROUP_0;
        }else{
            // do nothing - grouping value is already valid
        }
        /* Grouping value is valid - call generic NVIC driver function
         * Cast return status to Black Pill status type
         */
        status = (NVIC_BP_Status_t)NVIC_SetPriorityGrouping(groupBits);
    }
    return status;
}

/******************************************************************************
 * @brief Validate IRQ number for STM32F401CC Black Pill
 * 
 * @details This static function checks if the provided IRQ number is one of
 *          the valid interrupts available on STM32F401CC microcontroller.
 *          Uses a switch-case to validate against all 56 valid IRQs.
 * 
 * @param[in] IRQn  Interrupt request number to validate
 * 
 * @return bool_t Validation result
 * @retval TRUE  IRQ number is valid for STM32F401CC
 * @retval FALSE IRQ number is invalid
 * 
 * @note STM32F401CC has 85 possible IRQ positions (0-84) but only 56 are implemented
 * @note Gaps in IRQ numbering: 19-22, 39, 43-46, 48, 52-55, 61-66, 74-80, 82-83
 * @note This function is called by all Black Pill API functions for validation
 * @note Static function - internal use only, not exposed in header
 * 
 * @author Eng.Gemy
 ******************************************************************************/
static bool_t IsValidIRQ(NVIC_BP_IRQ_t IRQn){

    bool_t retstat = FALSE;
    switch (IRQn)
    {
        case NVIC_WWDG_IRQ:
        case NVIC_EXTI16_PVD_IRQ:
        case NVIC_EXTI21_TAMP_STAMP_IRQ:
        case NVIC_EXTI22_RTC_WKUP_IRQ:
        case NVIC_FLASH_IRQ:
        case NVIC_RCC_IRQ:
        case NVIC_EXTI0_IRQ:
        case NVIC_EXTI1_IRQ:
        case NVIC_EXTI2_IRQ:
        case NVIC_EXTI3_IRQ:
        case NVIC_EXTI4_IRQ:
        case NVIC_DMA1_STREAM0_IRQ:
        case NVIC_DMA1_STREAM1_IRQ:
        case NVIC_DMA1_STREAM2_IRQ:
        case NVIC_DMA1_STREAM3_IRQ:
        case NVIC_DMA1_STREAM4_IRQ:
        case NVIC_DMA1_STREAM5_IRQ:
        case NVIC_DMA1_STREAM6_IRQ:
        case NVIC_ADC_IRQ:
        case NVIC_EXTI9_5_IRQ:
        case NVIC_TIM1_BRK_TIM9_IRQ:
        case NVIC_TIM1_UP_TIM10_IRQ:
        case NVIC_TIM1_TRG_COM_TIM11_IRQ:
        case NVIC_TIM1_CC_IRQ:
        case NVIC_TIM2_IRQ:
        case NVIC_TIM3_IRQ:
        case NVIC_TIM4_IRQ:
        case NVIC_I2C1_EV_IRQ:
        case NVIC_I2C1_ER_IRQ:
        case NVIC_I2C2_EV_IRQ:
        case NVIC_I2C2_ER_IRQ:
        case NVIC_SPI1_IRQ:
        case NVIC_SPI2_IRQ:
        case NVIC_USART1_IRQ:
        case NVIC_USART2_IRQ:
        case NVIC_EXTI15_10_IRQ:
        case NVIC_EXTI17_RTC_ALARM_IRQ:
        case NVIC_EXTI18_OTG_FS_WKUP_IRQ:
        case NVIC_DMA1_STREAM7_IRQ:
        case NVIC_SDIO_IRQ:
        case NVIC_TIM5_IRQ:
        case NVIC_SPI3_IRQ:
        case NVIC_DMA2_STREAM0_IRQ:
        case NVIC_DMA2_STREAM1_IRQ:
        case NVIC_DMA2_STREAM2_IRQ:
        case NVIC_DMA2_STREAM3_IRQ:
        case NVIC_DMA2_STREAM4_IRQ:
        case NVIC_OTG_FS_IRQ:
        case NVIC_DMA2_STREAM5_IRQ:
        case NVIC_DMA2_STREAM6_IRQ:
        case NVIC_DMA2_STREAM7_IRQ:
        case NVIC_USART6_IRQ:
        case NVIC_I2C3_EV_IRQ:
        case NVIC_I2C3_ER_IRQ:
        case NVIC_FPU_IRQ:
        case NVIC_SPI4_IRQ:
            retstat = TRUE;
            break;

        /**
         * If the IRQ does not belong to any valid interrupt line,
         * it will fall into default case, thus stays FALSE.
         */
        default:
            retstat = FALSE;
            break;
    }
    return retstat;
}

/******************************************************************************
 * @brief Validate priority configuration against valid priority mask
 *
 * @details
 * Each priority value contains upper unused bits depending on priority grouping.
 *
 * PRIORITY_MASK_CHECK is used to remove any invalid bits beyond supported range.
 *
 * Example:
 *   Allowed priority bits: x x 0 0  (valid bits in lower nibble)
 *   If priority contains forbidden bits, e.g. 1011xxxx → function returns FALSE
 *
 * @param[in] priority : Priority to validate from type NVIC_BP_Priority_t
 *
 * @return bool_t  TRUE  → priority fits allowed bit-mask
 *                 FALSE → priority contains illegal bits
 ******************************************************************************/
static bool_t IsValidPriority(NVIC_BP_Priority_t priority){

    /* Start assuming priority is invalid */
    bool_t retstat = FALSE;

    /**
     * PRIORITY_MASK_CHECK is a predefined mask that isolates unsupported bits
     * If result after masking is zero → priority is valid
     */
    if(0 == (priority & PRIORITY_MASK_CHECK)){
        /* Meaning all masked bits are zero → priority valid */
        retstat = TRUE;
    }else{
        /* Some illegal bits exist */
        retstat = FALSE;
    }
    
    return retstat;
}

