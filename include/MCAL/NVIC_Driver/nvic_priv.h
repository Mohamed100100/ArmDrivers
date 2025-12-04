/******************************************************************************
 * @file    NVIC_PRIV.H
 * @author  Eng.Gemy
 * @brief   NVIC (Nested Vectored Interrupt Controller) Driver Private Header File
 *          This file contains the hardware register definitions, base addresses,
 *          and internal structures for the NVIC peripheral
 * @note    This file should NOT be included by application code
 *          It contains low-level hardware definitions for NVIC driver implementation
 * @note    NVIC is part of ARM Cortex-M core architecture
 ******************************************************************************/

#ifndef MCAL_NVIC_PRIV_DRIVER_NVIC_H
#define MCAL_NVIC_PRIV_DRIVER_NVIC_H

#include "LIB/stdtypes.h"

/******************************************************************************
 *                        REGISTER CAPACITY DEFINITIONS
 * @brief Number of interrupts that can be handled per register
 * @details Each register is 32-bit, so each bit controls one interrupt
 * @author Eng.Gemy
 ******************************************************************************/
#define NUM_OF_PREPHRALS_IN_ISER_REG    (32UL)  /**< Number of interrupts per ISER (Interrupt Set-Enable Register) - 32 bits */
#define NUM_OF_PREPHRALS_IN_ICER_REG    (32UL)  /**< Number of interrupts per ICER (Interrupt Clear-Enable Register) - 32 bits */
#define NUM_OF_PREPHRALS_IN_ISPR_REG    (32UL)  /**< Number of interrupts per ISPR (Interrupt Set-Pending Register) - 32 bits */
#define NUM_OF_PREPHRALS_IN_ICPR_REG    (32UL)  /**< Number of interrupts per ICPR (Interrupt Clear-Pending Register) - 32 bits */

/******************************************************************************
 *                        NVIC BASE ADDRESS
 * @brief Memory-mapped base address for NVIC peripheral
 * @details NVIC base = 0xE000E000 (System Control Space) + 0x100 offset
 * @note NVIC is part of System Control Space (SCS) in ARM Cortex-M
 * @author Eng.Gemy
 ******************************************************************************/
#define NVIC_BASE_ADDRESS   (0xE000E000 + 0x100)  /**< NVIC base address = 0xE000E100 */

/******************************************************************************
 *                        SCB AIRCR REGISTER AND MASKS
 * @brief System Control Block - Application Interrupt and Reset Control Register
 * @details AIRCR controls priority grouping and system reset
 * @note AIRCR is part of SCB (System Control Block) at address 0xE000ED0C
 * @author Eng.Gemy
 ******************************************************************************/
#define SCB_AIRCR   (*(volatile uint32_t *)0xE000ED0C)  /**< SCB Application Interrupt and Reset Control Register */
#define AIRCR_VECTKEY_MASK    0x05FA0000                /**< VECTKEY field - must write 0x05FA for register writes to take effect (bits 31:16) */
#define AIRCR_PRIGROUP_MASK   0x00000700                /**< PRIGROUP field - Priority grouping configuration (bits 10:8) */

/******************************************************************************
 *                        NVIC REGISTERS STRUCTURE
 * @brief Complete NVIC peripheral register map
 * @details This structure defines all NVIC registers with proper alignment
 * @note Register offsets from NVIC base address (0xE000E100):
 *       - ISER:  0x000 - 0x01C  (8 registers)
 *       - ICER:  0x080 - 0x09C  (8 registers)
 *       - ISPR:  0x100 - 0x11C  (8 registers)
 *       - ICPR:  0x180 - 0x19C  (8 registers)
 *       - IABR:  0x200 - 0x21C  (8 registers)
 *       - IPR:   0x300 - 0x3EF  (240 bytes)
 *       - STIR:  0xE00           (1 register)
 * @author Eng.Gemy
 ******************************************************************************/
typedef struct 
{
    /**
     * @brief Interrupt Set-Enable Registers (ISER)
     * @details Write 1 to enable interrupt, write 0 has no effect
     * @note ISER[0] controls IRQ 0-31, ISER[1] controls IRQ 32-63, etc.
     * @note Each bit corresponds to one external interrupt
     * @note Offset: 0x000 - 0x01C (8 x 32-bit registers)
     */
    uint32_t ISER[8];
    
    /**
     * @brief Reserved space between ISER and ICER
     * @details Maintains proper memory alignment (0x020 - 0x07C)
     * @note 24 x 32-bit = 96 bytes reserved
     */
    uint32_t reserved0[24];
    
    /**
     * @brief Interrupt Clear-Enable Registers (ICER)
     * @details Write 1 to disable interrupt, write 0 has no effect
     * @note ICER[0] controls IRQ 0-31, ICER[1] controls IRQ 32-63, etc.
     * @note Each bit corresponds to one external interrupt
     * @note Offset: 0x080 - 0x09C (8 x 32-bit registers)
     */
    uint32_t ICER[8];
    
    /**
     * @brief Reserved space between ICER and ISPR
     * @details Maintains proper memory alignment (0x0A0 - 0x0FC)
     * @note 24 x 32-bit = 96 bytes reserved
     */
    uint32_t reserved1[24];
    
    /**
     * @brief Interrupt Set-Pending Registers (ISPR)
     * @details Write 1 to set interrupt to pending, write 0 has no effect
     * @note ISPR[0] controls IRQ 0-31, ISPR[1] controls IRQ 32-63, etc.
     * @note Each bit corresponds to one external interrupt
     * @note Software can trigger interrupt by setting pending flag
     * @note Offset: 0x100 - 0x11C (8 x 32-bit registers)
     */
    uint32_t ISPR[8];
    
    /**
     * @brief Reserved space between ISPR and ICPR
     * @details Maintains proper memory alignment (0x120 - 0x17C)
     * @note 24 x 32-bit = 96 bytes reserved
     */
    uint32_t reserved2[24];
    
    /**
     * @brief Interrupt Clear-Pending Registers (ICPR)
     * @details Write 1 to clear pending flag, write 0 has no effect
     * @note ICPR[0] controls IRQ 0-31, ICPR[1] controls IRQ 32-63, etc.
     * @note Each bit corresponds to one external interrupt
     * @note Clears software or hardware triggered pending status
     * @note Offset: 0x180 - 0x19C (8 x 32-bit registers)
     */
    uint32_t ICPR[8];
    
    /**
     * @brief Reserved space between ICPR and IABR
     * @details Maintains proper memory alignment (0x1A0 - 0x1FC)
     * @note 24 x 32-bit = 96 bytes reserved
     */
    uint32_t reserved3[24];
    
    /**
     * @brief Interrupt Active Bit Registers (IABR)
     * @details Read-only registers showing which interrupts are active
     * @note IABR[0] shows IRQ 0-31, IABR[1] shows IRQ 32-63, etc.
     * @note Bit = 1 means interrupt is currently being serviced
     * @note Bit = 0 means interrupt is not active
     * @note Cannot be written by software (read-only)
     * @note Offset: 0x200 - 0x21C (8 x 32-bit registers)
     */
    uint32_t IABR[8];
    
    /**
     * @brief Reserved space between IABR and IPR
     * @details Maintains proper memory alignment (0x220 - 0x2FC)
     * @note 56 x 32-bit = 224 bytes reserved
     */
    uint32_t reserved4[56];
    
    /**
     * @brief Interrupt Priority Registers (IPR)
     * @details Sets priority level for each interrupt (0-239)
     * @note 240 interrupts x 1 byte each = 240 bytes
     * @note Each byte controls priority for one interrupt
     * @note Only upper bits are used (typically 4 bits in STM32)
     * @note Lower priority number = Higher priority level (0 = highest)
     * @note Priority interpretation depends on PRIGROUP in SCB_AIRCR
     * @note Offset: 0x300 - 0x3EF (240 x 8-bit registers)
     * @note Equivalent to: uint32_t IPR[60] (60 x 32-bit, 4 interrupts per word)
     */
    uint8_t IPR[240];           // === uint32_t IPR[60]; 
    
    /**
     * @brief Reserved space between IPR and STIR
     * @details Maintains proper memory alignment (0x3F0 - 0xDFC)
     * @note 644 x 32-bit = 2576 bytes reserved
     * @note Large gap in register map before STIR
     */
    uint32_t reserved5[644];
    
    /**
     * @brief Software Trigger Interrupt Register (STIR)
     * @details Write-only register to trigger interrupt via software
     * @note Write interrupt number (0-239) to trigger that interrupt
     * @note Only works if interrupt is enabled in ISER
     * @note Useful for testing ISRs or software-generated interrupts
     * @note Offset: 0xE00 (1 x 32-bit register)
     * @warning Access may be restricted based on processor mode
     */
    uint32_t STIR;    
}NVIC_Regs_t;

/******************************************************************************
 *                        NVIC PERIPHERAL POINTER DEFINITION
 * @brief Pointer to NVIC peripheral registers
 * @details This pointer provides direct access to NVIC hardware registers
 *          by casting the NVIC base address to the NVIC_Regs_t structure
 * @warning Do not modify this pointer - it points to hardware registers
 * @note Used by driver functions to access NVIC registers
 * 
 * @example Enable IRQ 37 (USART1 in STM32F4):
 *          NVIC_Registers->ISER[37/32] = (1 << (37%32));
 *          // Same as: NVIC_Registers->ISER[1] = (1 << 5);
 * 
 * @example Set priority for IRQ 37:
 *          NVIC_Registers->IPR[37] = (priority << 4);
 * 
 * @author Eng.Gemy
 ******************************************************************************/
NVIC_Regs_t *NVIC_Registers = (NVIC_Regs_t *)NVIC_BASE_ADDRESS; // Base address of RCC peripheral


#endif /* MCAL_NVIC_PRIV_DRIVER_NVIC_H */
