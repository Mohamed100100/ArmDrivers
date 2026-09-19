/**
 * @file    crc_priv.h
 * @brief   CRC Private Definitions (Register Level)
 * @author  Your Name
 * @date    2024
 */

#ifndef CRC_PRIV_H
#define CRC_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 *                              INCLUDES
 *===========================================================================*/
#include "LIB/stdtypes.h"

/*============================================================================
 *                              BASE ADDRESS
 *===========================================================================*/

/**
 * @brief CRC Peripheral Base Address
 */
#define CRC_BASE_ADDRESS            (0x40023000UL)

/*============================================================================
 *                              REGISTER OFFSETS
 *===========================================================================*/

#define CRC_DR_OFFSET               (0x00UL)    /* Data Register */
#define CRC_IDR_OFFSET              (0x04UL)    /* Independent Data Register */
#define CRC_CR_OFFSET               (0x08UL)    /* Control Register */

/*============================================================================
 *                              REGISTER STRUCTURE
 *===========================================================================*/

/**
 * @brief CRC Register Map Structure
 */
typedef struct {
    volatile uint32_t DR;       /**< Data Register,              Address offset: 0x00 */
    volatile uint32_t IDR;      /**< Independent Data Register,  Address offset: 0x04 */
    volatile uint32_t CR;       /**< Control Register,           Address offset: 0x08 */
} CRC_RegDef_t;

/*============================================================================
 *                              PERIPHERAL POINTER
 *===========================================================================*/

/**
 * @brief CRC Peripheral Pointer
 */
#define CRC                         ((CRC_RegDef_t*)CRC_BASE_ADDRESS)

/*============================================================================
 *                              REGISTER BIT DEFINITIONS
 *===========================================================================*/

/*---------------------------------------------------------------------------
 * CRC_DR - Data Register (Offset: 0x00)
 * Reset Value: 0xFFFF FFFF
 *---------------------------------------------------------------------------*/
#define CRC_DR_DR_POS               (0U)
#define CRC_DR_DR_MSK               (0xFFFFFFFFUL << CRC_DR_DR_POS)
#define CRC_DR_DR                   CRC_DR_DR_MSK

/*---------------------------------------------------------------------------
 * CRC_IDR - Independent Data Register (Offset: 0x04)
 * Reset Value: 0x0000 0000
 *---------------------------------------------------------------------------*/
#define CRC_IDR_IDR_POS             (0U)
#define CRC_IDR_IDR_MSK             (0xFFUL << CRC_IDR_IDR_POS)
#define CRC_IDR_IDR                 CRC_IDR_IDR_MSK

/*---------------------------------------------------------------------------
 * CRC_CR - Control Register (Offset: 0x08)
 * Reset Value: 0x0000 0000
 *---------------------------------------------------------------------------*/
#define CRC_CR_RESET_POS            (0U)
#define CRC_CR_RESET_MSK            (0x1UL << CRC_CR_RESET_POS)
#define CRC_CR_RESET                CRC_CR_RESET_MSK

/*============================================================================
 *                              CRC POLYNOMIAL INFO
 *===========================================================================*/

/**
 * @brief CRC-32 Ethernet Polynomial
 *        X^32 + X^26 + X^23 + X^22 + X^16 + X^12 + X^11 + X^10 + X^8 + X^7 + X^5 + X^4 + X^2 + X + 1
 */
#define CRC_POLYNOMIAL              (0x4C11DB7UL)

/**
 * @brief CRC Initial Value (after reset)
 */
#define CRC_INITIAL_VALUE           (0xFFFFFFFFUL)

/**
 * @brief CRC Computation Cycles
 */
#define CRC_COMPUTATION_CYCLES      (4U)    /* 4 AHB clock cycles per word */

#ifdef __cplusplus
}
#endif

#endif /* CRC_PRIV_H */