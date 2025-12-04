/******************************************************************************
 * @file    NVIC.C
 * @author  Eng.Gemy
 * @brief   NVIC (Nested Vectored Interrupt Controller) Driver Implementation File
 *          This file contains the implementation of all NVIC driver functions
 *          for ARM Cortex-M interrupt management
 * @date    [Current Date]
 * @version 1.0
 ******************************************************************************/

#include "LIB/stdtypes.h"

#include "MCAL/NVIC_Driver/nvic_priv.h"
#include "MCAL/NVIC_Driver/nvic.h"



/******************************************************************************
 * @brief Enable external interrupt in NVIC
 * 
 * @details This function enables the specified interrupt by setting the 
 *          corresponding bit in the ISER (Interrupt Set-Enable Register).
 *          The function calculates which ISER register to use and which
 *          bit position based on the IRQ number.
 * 
 * @param[in] IRQn  Interrupt request number (0-239)
 * 
 * @return NVIC_Status_t Status of the operation
 * @retval NVIC_OK  Interrupt enabled successfully
 * 
 * @note Each ISER register handles 32 interrupts (32 bits per register)
 * @note Register index = IRQn / 32, Bit position = IRQn % 32
 * @note Writing 1 to ISER bit enables the interrupt
 * @note Interrupt must also be configured and enabled in peripheral
 * 
 * @example Enable interrupt 37 (USART1 in STM32F4):
 *          NVIC_EnableIRQ(NVIC_IRQ37);
 *          // Uses ISER[1] bit 5 (37/32 = 1, 37%32 = 5)
 * 
 * @author Eng.Gemy
 ******************************************************************************/
NVIC_Status_t NVIC_EnableIRQ(NVIC_IRQ_t IRQn){
    /* Enable interrupt by setting corresponding bit in ISER register
     * Calculate register index: IRQn / 32 (which ISER register)
     * Calculate bit position: IRQn % 32 (which bit in that register)
     * OR operation sets the bit without affecting other bits
     */
    NVIC_Registers->ISER[(uint32_t)(IRQn/NUM_OF_PREPHRALS_IN_ISER_REG)] |= (1 << (IRQn%NUM_OF_PREPHRALS_IN_ISER_REG));
    return NVIC_OK;
}

/******************************************************************************
 * @brief Disable external interrupt in NVIC
 * 
 * @details This function disables the specified interrupt by setting the 
 *          corresponding bit in the ICER (Interrupt Clear-Enable Register).
 *          The function calculates which ICER register to use and which
 *          bit position based on the IRQ number.
 * 
 * @param[in] IRQn  Interrupt request number (0-239)
 * 
 * @return NVIC_Status_t Status of the operation
 * @retval NVIC_OK  Interrupt disabled successfully
 * 
 * @note Each ICER register handles 32 interrupts (32 bits per register)
 * @note Register index = IRQn / 32, Bit position = IRQn % 32
 * @note Writing 1 to ICER bit disables the interrupt
 * @note Pending interrupt will not be serviced while disabled
 * 
 * @example Disable interrupt 37 (USART1 in STM32F4):
 *          NVIC_DisableIRQ(NVIC_IRQ37);
 *          // Uses ICER[1] bit 5 (37/32 = 1, 37%32 = 5)
 * 
 * @author Eng.Gemy
 ******************************************************************************/
NVIC_Status_t NVIC_DisableIRQ(NVIC_IRQ_t IRQn){
    /* Disable interrupt by setting corresponding bit in ICER register
     * Calculate register index: IRQn / 32 (which ICER register)
     * Calculate bit position: IRQn % 32 (which bit in that register)
     * OR operation writes 1 to disable, 0 has no effect (ICER behavior)
     */
    NVIC_Registers->ICER[(uint32_t)(IRQn/NUM_OF_PREPHRALS_IN_ICER_REG)] |= (1 << (IRQn%NUM_OF_PREPHRALS_IN_ICER_REG));
    return NVIC_OK;
}

/******************************************************************************
 * @brief Set interrupt to pending state
 * 
 * @details This function sets the specified interrupt to pending state by 
 *          setting the corresponding bit in the ISPR (Interrupt Set-Pending 
 *          Register). This allows software to trigger an interrupt.
 * 
 * @param[in] IRQn  Interrupt request number (0-239)
 * 
 * @return NVIC_Status_t Status of the operation
 * @retval NVIC_OK  Pending flag set successfully
 * 
 * @note Each ISPR register handles 32 interrupts (32 bits per register)
 * @note Register index = IRQn / 32, Bit position = IRQn % 32
 * @note Writing 1 to ISPR bit sets interrupt to pending
 * @note If interrupt is enabled, it will be serviced
 * @note Useful for software-triggered interrupts or testing ISRs
 * 
 * @example Trigger interrupt 37 from software:
 *          NVIC_SetPendingIRQ(NVIC_IRQ37);
 *          // Uses ISPR[1] bit 5 (37/32 = 1, 37%32 = 5)
 * 
 * @author Eng.Gemy
 ******************************************************************************/
NVIC_Status_t NVIC_SetPendingIRQ(NVIC_IRQ_t IRQn){
    /* Set interrupt to pending by setting corresponding bit in ISPR register
     * Calculate register index: IRQn / 32 (which ISPR register)
     * Calculate bit position: IRQn % 32 (which bit in that register)
     * OR operation sets the bit without affecting other bits
     */
    NVIC_Registers->ISPR[(uint32_t)(IRQn/NUM_OF_PREPHRALS_IN_ISPR_REG)] |= (1 << (IRQn%NUM_OF_PREPHRALS_IN_ISPR_REG));
    return NVIC_OK;
}

/******************************************************************************
 * @brief Clear pending status of interrupt
 * 
 * @details This function clears the pending state of the specified interrupt 
 *          by setting the corresponding bit in the ICPR (Interrupt Clear-Pending 
 *          Register). This prevents the interrupt from being serviced.
 * 
 * @param[in] IRQn  Interrupt request number (0-239)
 * 
 * @return NVIC_Status_t Status of the operation
 * @retval NVIC_OK  Pending flag cleared successfully
 * 
 * @note Each ICPR register handles 32 interrupts (32 bits per register)
 * @note Register index = IRQn / 32, Bit position = IRQn % 32
 * @note Writing 1 to ICPR bit clears pending status
 * @note Useful to cancel pending interrupt before enabling
 * @note Does not affect currently active interrupt
 * 
 * @example Clear pending flag for interrupt 37:
 *          NVIC_ClearPendingIRQ(NVIC_IRQ37);
 *          // Uses ICPR[1] bit 5 (37/32 = 1, 37%32 = 5)
 * 
 * @author Eng.Gemy
 ******************************************************************************/
NVIC_Status_t NVIC_ClearPendingIRQ(NVIC_IRQ_t IRQn){
    /* Clear pending status by setting corresponding bit in ICPR register
     * Calculate register index: IRQn / 32 (which ICPR register)
     * Calculate bit position: IRQn % 32 (which bit in that register)
     * OR operation writes 1 to clear pending, 0 has no effect (ICPR behavior)
     */
    NVIC_Registers->ICPR[(uint32_t)(IRQn/NUM_OF_PREPHRALS_IN_ICPR_REG)] |= (1 << (IRQn%NUM_OF_PREPHRALS_IN_ICPR_REG));
    return NVIC_OK;
}

/******************************************************************************
 * @brief Set priority for external interrupt
 * 
 * @details This function sets the priority level for the specified interrupt
 *          by writing to the IPR (Interrupt Priority Register). Each interrupt
 *          has one byte in the IPR array.
 * 
 * @param[in] IRQn     Interrupt request number (0-239)
 * @param[in] priorit  Priority value (0 = highest priority)
 * 
 * @return NVIC_Status_t Status of the operation
 * @retval NVIC_OK  Priority set successfully
 * 
 * @note Each interrupt has 1 byte (8 bits) for priority
 * @note Only upper bits are implemented (typically 4 bits in STM32)
 * @note Lower priority number = Higher priority level
 * @note Priority 0 is the highest priority
 * @note Actual priority bits used depend on implementation (check datasheet)
 * @note Priority grouping (PRIGROUP) affects interpretation
 * 
 * @example Set interrupt 37 to priority 5:
 *          NVIC_SetPriority(NVIC_IRQ37, 5);
 * 
 * @warning For STM32, shift priority left by 4: (priority << 4)
 *          This function writes raw value, ensure proper formatting
 * 
 * @author Eng.Gemy
 ******************************************************************************/
NVIC_Status_t NVIC_SetPriority(NVIC_IRQ_t IRQn,uint8_t priorit){
    /* Set interrupt priority by writing to IPR register
     * IPR is byte-addressable array, one byte per interrupt
     * Direct indexing: IPR[IRQn] accesses priority byte for interrupt IRQn
     * Write priority value directly (caller should format appropriately)
     */
    NVIC_Registers->IPR[IRQn] = priorit;
    return NVIC_OK;
}

/******************************************************************************
 * @brief Get priority of external interrupt
 * 
 * @details This function reads the current priority level for the specified 
 *          interrupt from the IPR (Interrupt Priority Register).
 * 
 * @param[in]  IRQn        Interrupt request number (0-239)
 * @param[out] RetPriorit  Pointer to store the priority value
 * 
 * @return NVIC_Status_t Status of the operation
 * @retval NVIC_OK        Priority read successfully
 * @retval NVIC_NULL_PTR  Null pointer passed
 * 
 * @note Returns raw priority byte from IPR register
 * @note Interpretation depends on how priority was set and PRIGROUP setting
 * @note Only implemented priority bits contain valid data
 * 
 * @example Get priority of interrupt 37:
 *          uint8_t priority;
 *          NVIC_GetPriority(NVIC_IRQ37, &priority);
 * 
 * @author Eng.Gemy
 ******************************************************************************/
NVIC_Status_t NVIC_GetPriority(NVIC_IRQ_t IRQn,uint8_t* RetPriorit){
    /* Local variable to hold function return status */
    NVIC_Status_t status = NVIC_NOT_OK;

    /* Validate output pointer */
    if(NULL == RetPriorit){
        /* Null pointer passed - cannot return priority value */
        status = NVIC_NULL_PTR;
    }else{
        /* Read priority from IPR register
         * IPR is byte-addressable array, one byte per interrupt
         * Direct indexing: IPR[IRQn] reads priority byte for interrupt IRQn
         */
        *RetPriorit = NVIC_Registers->IPR[IRQn];
        status = NVIC_OK;
    }
    return status;
}

/******************************************************************************
 * @brief Get active status of external interrupt
 * 
 * @details This function reads the active status of the specified interrupt
 *          from the IABR (Interrupt Active Bit Register). Active means the
 *          interrupt is currently being serviced (in ISR).
 * 
 * @param[in]  IRQn       Interrupt request number (0-239)
 * @param[out] RetActive  Pointer to store active status (NVIC_ACTIVE or NVIC_NOT_ACTIVE)
 * 
 * @return NVIC_Status_t Status of the operation
 * @retval NVIC_OK        Status read successfully
 * @retval NVIC_NULL_PTR  Null pointer passed
 * 
 * @note IABR is read-only, cannot be modified by software
 * @note Active = 1 means interrupt is currently being serviced
 * @note Active = 0 means interrupt is not currently active
 * @note Each IABR register handles 32 interrupts (32 bits per register)
 * @note Useful for debugging nested interrupt scenarios
 * 
 * @example Check if interrupt 37 is active:
 *          NVIC_Active_t active;
 *          NVIC_GetActiveIRQ(NVIC_IRQ37, &active);
 *          // Uses IABR[1] bit 5 (37/32 = 1, 37%32 = 5)
 * 
 * @author Eng.Gemy
 ******************************************************************************/
NVIC_Status_t NVIC_GetActiveIRQ (NVIC_IRQ_t IRQn,NVIC_Active_t* RetActive){
        
    /* Local variable to hold function return status */
    NVIC_Status_t status = NVIC_NOT_OK;

    /* Validate output pointer */
    if(NULL == RetActive){
        /* Null pointer passed - cannot return active status */
        status = NVIC_NULL_PTR;
    }else{
        /* Read active status from IABR register
         * Calculate register index: IRQn / 32 (which IABR register)
         * Calculate bit position: IRQn % 32 (which bit in that register)
         * Right shift to move target bit to LSB position
         * AND with 0b1 to extract only the LSB (active status bit)
         */
        *RetActive = ((NVIC_Registers->IABR[(uint32_t)(IRQn/NUM_OF_PREPHRALS_IN_ICPR_REG)] >> (IRQn%NUM_OF_PREPHRALS_IN_ICPR_REG))&0b1);
        status = NVIC_OK;
    }
    return status;
}

/******************************************************************************
 * @brief Get pending status of external interrupt
 * 
 * @details This function reads the pending status of the specified interrupt
 *          from the ISPR (Interrupt Set-Pending Register). Pending means the
 *          interrupt has been triggered but not yet serviced.
 * 
 * @param[in]  IRQn        Interrupt request number (0-239)
 * @param[out] RetPending  Pointer to store pending status (NVIC_PENDING or NVIC_NOT_PENDING)
 * 
 * @return NVIC_Status_t Status of the operation
 * @retval NVIC_OK        Status read successfully
 * @retval NVIC_NULL_PTR  Null pointer passed
 * 
 * @note Pending = 1 means interrupt is waiting to be serviced
 * @note Pending = 0 means interrupt is not pending
 * @note Each ISPR register handles 32 interrupts (32 bits per register)
 * @note Useful for polling interrupt status without enabling interrupt
 * @note Reading ISPR shows pending status without modifying it
 * 
 * @example Check if interrupt 37 is pending:
 *          NVIC_Pending_t pending;
 *          NVIC_GetPendingIRQ(NVIC_IRQ37, &pending);
 *          // Uses ISPR[1] bit 5 (37/32 = 1, 37%32 = 5)
 * 
 * @author Eng.Gemy
 ******************************************************************************/
NVIC_Status_t NVIC_GetPendingIRQ (NVIC_IRQ_t IRQn,NVIC_Pending_t* RetPending){
    /* Local variable to hold function return status */
    NVIC_Status_t status = NVIC_NOT_OK;

    /* Validate output pointer */
    if(NULL == RetPending){
        /* Null pointer passed - cannot return pending status */
        status = NVIC_NULL_PTR;
    }else{
        /* Read pending status from ISPR register
         * Calculate register index: IRQn / 32 (which ISPR register)
         * Calculate bit position: IRQn % 32 (which bit in that register)
         * Right shift to move target bit to LSB position
         * AND with 0b1 to extract only the LSB (pending status bit)
         */
        *RetPending = ((NVIC_Registers->ISPR[(uint32_t)(IRQn/NUM_OF_PREPHRALS_IN_ISPR_REG)] >> (IRQn%NUM_OF_PREPHRALS_IN_ISPR_REG))&0b1);
        status = NVIC_OK;
    }
    return status;
}

/******************************************************************************
/******************************************************************************
 * @brief Set priority grouping for interrupt priorities
 * 
 * @details This function configures the priority grouping by writing to the
 *          SCB AIRCR (Application Interrupt and Reset Control Register).
 *          Priority grouping determines how priority bits are split between
 *          group priority (preemption) and subpriority.
 * 
 * @param[in] priority_grouping  Priority grouping value (3-7 typical)
 *                                3 = [7:4] group, 0 bits sub (full preemption)
 *                                4 = [7:5] group, [4] sub
 *                                5 = [7:6] group, [5:4] sub (balanced)
 *                                6 = [7] group, [6:4] sub
 *                                7 = 0 bits group, [7:4] sub (no preemption)
 * 
 * @return NVIC_Status_t Status of the operation
 * @retval NVIC_OK  Priority grouping set successfully
 * 
 * @note AIRCR requires VECTKEY (0x05FA) in bits 31:16 for write access
 * @note PRIGROUP field is in bits 10:8 of AIRCR register
 * @note Group priority determines if interrupt can preempt another
 * @note Subpriority determines order when group priority is same
 * @note Should be set once at system initialization
 * 
 * @example Set balanced grouping (2 bits group, 2 bits sub):
 *          NVIC_SetPriorityGrouping(5);
 *          // AIRCR = 0x05FA0000 | (5 << 8)
 *          // Results in 4 preemption levels, 4 sublevels each
 * 
 * @warning Changing priority grouping affects interpretation of all interrupt priorities
 * @warning Must be set before configuring interrupt priorities
 * 
 * @author Eng.Gemy
 ******************************************************************************/
NVIC_Status_t NVIC_SetPriorityGrouping(uint32_t priority_grouping){

    /* Write to AIRCR register with VECTKEY and priority grouping
     * AIRCR_VECTKEY_MASK (0x05FA0000): Required key for register write access
     *                                   Without this key, writes are ignored
     * priority_grouping << 8: Shift grouping value to bits 10:8 (PRIGROUP field)
     * OR operation combines VECTKEY and PRIGROUP value
     * This sets how priority bits are divided between group and subpriority
     */
    // Write VECTKEY (0x5FA) + PRIGROUP field
    SCB_AIRCR = AIRCR_VECTKEY_MASK | (priority_grouping << 8);
    return NVIC_OK;
}