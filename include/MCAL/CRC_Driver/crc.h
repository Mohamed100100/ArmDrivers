/**
 * @file    crc.h
 * @brief   CRC Driver Header File
 * @author  Your Name
 * @date    2024
 * 
 * @details This driver provides functions to use the hardware CRC calculation
 *          unit on STM32F4 microcontrollers.
 * 
 *          Features:
 *          - CRC-32 (Ethernet) polynomial: 0x4C11DB7
 *          - Hardware accelerated (4 AHB clock cycles per 32-bit word)
 *          - Initial value: 0xFFFFFFFF
 * 
 * @note    The CRC peripheral clock must be enabled manually before use.
 *          Enable: RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;
 */

#ifndef CRC_H
#define CRC_H

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 *                              INCLUDES
 *===========================================================================*/
#include "LIB/stdtypes.h"

/*============================================================================
 *                              TYPEDEFS
 *===========================================================================*/

/**
 * @brief CRC Error Codes
 */
typedef enum {
    CRC_OK              = 0x00U,    /**< Operation successful */
    CRC_ERROR_NULL_PTR  = 0x01U,    /**< Null pointer passed */
    CRC_ERROR_ZERO_LEN  = 0x02U,    /**< Zero length data */
} CRC_Error_t;

/*============================================================================
 *                              FUNCTION PROTOTYPES
 *===========================================================================*/

/**
 * @brief   Reset CRC calculation unit
 * @details Resets the CRC data register to initial value (0xFFFFFFFF).
 * @return  None
 */
void CRC_Reset(void);

/**
 * @brief   Calculate CRC of a single 32-bit word
 * @details Accumulates CRC calculation with a new 32-bit value.
 * @param   data - 32-bit data word
 * @return  uint32_t - Current CRC value
 */
uint32_t CRC_Accumulate(uint32_t data);

/**
 * @brief   Calculate CRC of 32-bit word array (accumulate, no reset)
 * @details Calculates CRC over an array of 32-bit words.
 *          Does NOT reset CRC before calculation (accumulates).
 * @param   pData   - Pointer to data array (must be 32-bit aligned)
 * @param   length  - Number of 32-bit words
 * @return  uint32_t - Calculated CRC value (0 on error)
 */
uint32_t CRC_Accumulate_Array(const uint32_t *pData, uint32_t length);

/**
 * @brief   Calculate CRC of 32-bit word array (with reset)
 * @details Resets CRC and calculates over an array of 32-bit words.
 * @param   pData   - Pointer to data array (must be 32-bit aligned)
 * @param   length  - Number of 32-bit words
 * @return  uint32_t - Calculated CRC value (0 on error)
 */
uint32_t CRC_Calculate(const uint32_t *pData, uint32_t length);

/**
 * @brief   Calculate CRC of byte array (with reset)
 * @details Calculates CRC over a byte array (handles non-aligned data).
 *          Resets CRC before calculation.
 * @param   pData   - Pointer to byte array
 * @param   length  - Number of bytes
 * @return  uint32_t - Calculated CRC value (0 on error)
 */
uint32_t CRC_Calculate_Bytes(const uint8_t *pData, uint32_t length);

/**
 * @brief   Calculate CRC of byte array (accumulate, no reset)
 * @details Accumulates CRC calculation with a byte array.
 *          Does NOT reset CRC before calculation.
 * @param   pData   - Pointer to byte array
 * @param   length  - Number of bytes
 * @return  uint32_t - Calculated CRC value (0 on error)
 */
uint32_t CRC_Accumulate_Bytes(const uint8_t *pData, uint32_t length);

/**
 * @brief   Get current CRC value
 * @details Returns the current value in the CRC data register.
 * @return  uint32_t - Current CRC value
 */
uint32_t CRC_GetValue(void);

/**
 * @brief   Set Independent Data Register
 * @details Writes a value to the 8-bit IDR register.
 *          Can be used for temporary storage.
 * @param   data - 8-bit data value
 */
void CRC_SetIDR(uint8_t data);

/**
 * @brief   Get Independent Data Register
 * @details Reads the 8-bit IDR register value.
 * @return  uint8_t - IDR register value
 */
uint8_t CRC_GetIDR(void);

/**
 * @brief   Calculate CRC of memory region (with reset)
 * @details Calculates CRC-32 of a memory region (e.g., Flash memory).
 *          Useful for firmware integrity checking.
 * @param   startAddress - Start address of memory region
 * @param   length       - Length in bytes (should be multiple of 4)
 * @return  uint32_t - Calculated CRC value
 */
uint32_t CRC_CalculateMemoryRegion(uint32_t startAddress, uint32_t length);

/**
 * @brief   Calculate CRC of memory region (accumulate, no reset)
 * @details Accumulates CRC-32 over a memory region.
 *          Does NOT reset CRC before calculation.
 * @param   startAddress - Start address of memory region
 * @param   length       - Length in bytes (should be multiple of 4)
 * @return  uint32_t - Calculated CRC value
 */
uint32_t CRC_AccumulateMemoryRegion(uint32_t startAddress, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* CRC_H */