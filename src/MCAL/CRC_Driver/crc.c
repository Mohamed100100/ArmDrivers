/**
 * @file    crc.c
 * @author  Eng.Gemy (adapted from Your Name template)
 * @brief   CRC Driver Implementation — STM32F4 Hardware CRC (0x4C11DB7)
 *          Provides reset, single-word accumulate, array/bytes helpers (with/
 *          without reset), IDR scratch, and memory-region CRC via direct
 *          word reads. Clock must be enabled before use (RCC_AHB1ENR_CRCEN).
 * @date    2024
 * @version 1.0
 * @note    Polynomial fixed 0x04C11DB7, init 0xFFFFFFFF, 4 AHB cycles/word.
 *          Byte helpers pad tail with 0xFF to word boundary.
 */

/*============================================================================
 *                              INCLUDES
 *===========================================================================*/
#include "MCAL/CRC_Driver/crc.h"
#include "MCAL/CRC_Driver/crc_priv.h"

/*============================================================================
 *                              PUBLIC FUNCTIONS
 *===========================================================================*/

/**
 * @brief   Reset CRC data register to 0xFFFFFFFF
 * @details Writes CRC->CR.RESET (auto-cleared by HW). Does NOT affect IDR.
 * @param   None
 * @return  None
 */
void CRC_Reset(void)
{
    /* Set RESET bit - automatically cleared by hardware */
    CRC->CR = CRC_CR_RESET;
}

/**
 * @brief   Accumulate single 32-bit word into CRC
 * @details Writes DR → HW computes 4 cycles → read DR. Accumulates from
 *          current value (no reset).
 * @param[in] data 32-bit word to feed (e.g., 0x12345678)
 * @return  uint32_t Current CRC value after accumulation (CRC->DR)
 */
uint32_t CRC_Accumulate(uint32_t data)
{
    /* Write data to DR register - computation takes 4 AHB cycles */
    CRC->DR = data;
    
    /* Read and return result */
    return CRC->DR;
}

/**
 * @brief   Accumulate array of 32-bit words (no reset)
 * @details Loops DR = pData[i]. Preserves current CRC seed.
 * @param[in] pData Pointer to 32-bit aligned array (must not be NULL)
 * @param[in] length Number of words (0 → return current DR)
 * @return  uint32_t Final CRC (0 if pData==NULL per driver convention)
 * @note    Zero length returns CRC->DR unchanged (not 0xFFFFFFFF).
 */
uint32_t CRC_Accumulate_Array(const uint32_t *pData, uint32_t length)
{
    /* Validate parameters */
    if (pData == NULL) {
        return 0;
    }
    
    if (length == 0) {
        return CRC->DR;
    }
    
    /* Process each 32-bit word */
    for (uint32_t i = 0; i < length; i++) {
        CRC->DR = pData[i];
    }
    
    /* Return final CRC value */
    return CRC->DR;
}

/**
 * @brief   Calculate CRC of word array with reset (full computation)
 * @details Does CRC_Reset() then Accumulate_Array. Equivalent to fresh CRC.
 * @param[in] pData Pointer to 32-bit array (NULL → 0)
 * @param[in] length Number of words (0 → 0xFFFFFFFF init value)
 * @return  uint32_t CRC value (0 on NULL, 0xFFFFFFFF if length==0)
 */
uint32_t CRC_Calculate(const uint32_t *pData, uint32_t length)
{
    /* Validate parameters */
    if (pData == NULL) {
        return 0;
    }
    
    if (length == 0) {
        return CRC_INITIAL_VALUE;
    }
    
    /* Reset CRC calculator */
    CRC->CR = CRC_CR_RESET;
    
    /* Process each 32-bit word */
    for (uint32_t i = 0; i < length; i++) {
        CRC->DR = pData[i];
    }
    
    /* Return final CRC value */
    return CRC->DR;
}

/**
 * @brief   Calculate CRC of byte array with reset (handles unaligned)
 * @details Resets, then packs bytes little-endian into 32-bit words.
 *          Aligned fast-path casts to uint32_t*; unaligned assembles
 *          word via shifts. Tail <4 bytes padded with 0xFF (HW requires
 *          word write). Use this for strings / firmware blobs.
 * @param[in] pData Pointer to byte array (NULL → 0)
 * @param[in] length Number of bytes (0 → 0xFFFFFFFF)
 * @return  uint32_t Final CRC
 */
uint32_t CRC_Calculate_Bytes(const uint8_t *pData, uint32_t length)
{
    /* Validate parameters */
    if (pData == NULL) {
        return 0;
    }
    
    if (length == 0) {
        return CRC_INITIAL_VALUE;
    }
    
    /* Reset CRC calculator */
    CRC->CR = CRC_CR_RESET;
    
    /* Calculate number of complete 32-bit words */
    uint32_t word_count = length / 4;
    uint32_t remaining_bytes = length % 4;
    
    /* Check if data is 32-bit aligned */
    if (((uint32_t)pData & 0x03U) == 0) {
        /* Aligned access - fast path */
        const uint32_t *pData32 = (const uint32_t *)pData;
        for (uint32_t i = 0; i < word_count; i++) {
            CRC->DR = pData32[i];
        }
    } else {
        /* Unaligned access - byte by byte assembly */
        for (uint32_t i = 0; i < word_count; i++) {
            uint32_t offset = i * 4;
            uint32_t word = ((uint32_t)pData[offset]) |
                           ((uint32_t)pData[offset + 1] << 8) |
                           ((uint32_t)pData[offset + 2] << 16) |
                           ((uint32_t)pData[offset + 3] << 24);
            CRC->DR = word;
        }
    }
    
    /* Process remaining bytes (if any) */
    if (remaining_bytes > 0) {
        uint32_t last_word = 0xFFFFFFFFUL;  /* Pad with 0xFF */
        uint32_t offset = word_count * 4;
        
        for (uint32_t i = 0; i < remaining_bytes; i++) {
            ((uint8_t *)&last_word)[i] = pData[offset + i];
        }
        
        CRC->DR = last_word;
    }
    
    /* Return final CRC value */
    return CRC->DR;
}

/**
 * @brief   Accumulate byte array without reset
 * @details Same packing as Calculate_Bytes but preserves current CRC seed.
 * @param[in] pData Pointer to byte array (NULL → 0)
 * @param[in] length Number of bytes (0 → current DR)
 * @return  uint32_t Final CRC after accumulation
 */
uint32_t CRC_Accumulate_Bytes(const uint8_t *pData, uint32_t length)
{
    /* Validate parameters */
    if (pData == NULL) {
        return 0;
    }
    
    if (length == 0) {
        return CRC->DR;
    }
    
    /* Calculate number of complete 32-bit words */
    uint32_t word_count = length / 4;
    uint32_t remaining_bytes = length % 4;
    
    /* Check if data is 32-bit aligned */
    if (((uint32_t)pData & 0x03U) == 0) {
        /* Aligned access - fast path */
        const uint32_t *pData32 = (const uint32_t *)pData;
        for (uint32_t i = 0; i < word_count; i++) {
            CRC->DR = pData32[i];
        }
    } else {
        /* Unaligned access - byte by byte assembly */
        for (uint32_t i = 0; i < word_count; i++) {
            uint32_t offset = i * 4;
            uint32_t word = ((uint32_t)pData[offset]) |
                           ((uint32_t)pData[offset + 1] << 8) |
                           ((uint32_t)pData[offset + 2] << 16) |
                           ((uint32_t)pData[offset + 3] << 24);
            CRC->DR = word;
        }
    }
    
    /* Process remaining bytes (if any) */
    if (remaining_bytes > 0) {
        uint32_t last_word = 0xFFFFFFFFUL;  /* Pad with 0xFF */
        uint32_t offset = word_count * 4;
        
        for (uint32_t i = 0; i < remaining_bytes; i++) {
            ((uint8_t *)&last_word)[i] = pData[offset + i];
        }
        
        CRC->DR = last_word;
    }
    
    /* Return final CRC value */
    return CRC->DR;
}

/**
 * @brief   Get current CRC data register value
 * @param   None
 * @return  uint32_t CRC->DR (0xFFFFFFFF if just reset)
 */
uint32_t CRC_GetValue(void)
{
    return CRC->DR;
}

/**
 * @brief   Set 8-bit Independent Data Register (IDR) scratch
 * @details IDR survives CRC reset (only DR reset). Use for tag storage.
 * @param[in] data 8-bit value to write
 * @return  None
 */
void CRC_SetIDR(uint8_t data)
{
    CRC->IDR = (uint32_t)data;
}

/**
 * @brief   Get IDR scratch value
 * @param   None
 * @return  uint8_t IDR & 0xFF
 */
uint8_t CRC_GetIDR(void)
{
    return (uint8_t)(CRC->IDR & CRC_IDR_IDR_MSK);
}

/**
 * @brief   Calculate CRC over memory region with reset
 * @details Direct word reads from startAddress (volatile). Useful for
 *          firmware integrity: CRC over Flash 0x08000000. Tail padded 0xFF.
 * @param[in] startAddress Start address (should be word-aligned for speed)
 * @param[in] length Bytes (0 → 0xFFFFFFFF); word_count = len/4
 * @return  uint32_t Final CRC
 * @warning Ensure region is readable (no BusFault); length covers valid map.
 */
uint32_t CRC_CalculateMemoryRegion(uint32_t startAddress, uint32_t length)
{
    /* Validate parameters */
    if (length == 0) {
        return CRC_INITIAL_VALUE;
    }
    
    /* Reset CRC calculator */
    CRC->CR = CRC_CR_RESET;
    
    /* Calculate number of complete 32-bit words */
    uint32_t word_count = length / 4;
    uint32_t remaining_bytes = length % 4;
    
    /* Process complete 32-bit words */
    volatile uint32_t *pMem = (volatile uint32_t *)startAddress;
    
    for (uint32_t i = 0; i < word_count; i++) {
        CRC->DR = pMem[i];
    }
    
    /* Process remaining bytes (if any) */
    if (remaining_bytes > 0) {
        uint32_t last_word = 0xFFFFFFFFUL;  /* Pad with 0xFF */
        volatile uint8_t *pMemBytes = (volatile uint8_t *)(startAddress + (word_count * 4));
        
        for (uint32_t i = 0; i < remaining_bytes; i++) {
            ((uint8_t *)&last_word)[i] = pMemBytes[i];
        }
        
        CRC->DR = last_word;
    }
    
    /* Return final CRC value */
    return CRC->DR;
}

/**
 * @brief   Accumulate CRC over memory region without reset
 * @details Same as CalculateMemoryRegion but preserves current DR seed.
 * @param[in] startAddress Start address
 * @param[in] length Bytes
 * @return  uint32_t Final CRC (current DR if length==0)
 */
uint32_t CRC_AccumulateMemoryRegion(uint32_t startAddress, uint32_t length)
{
    /* Validate parameters */
    if (length == 0) {
        return CRC->DR;
    }
    
    /* Calculate number of complete 32-bit words */
    uint32_t word_count = length / 4;
    uint32_t remaining_bytes = length % 4;
    
    /* Process complete 32-bit words */
    volatile uint32_t *pMem = (volatile uint32_t *)startAddress;
    
    for (uint32_t i = 0; i < word_count; i++) {
        CRC->DR = pMem[i];
    }
    
    /* Process remaining bytes (if any) */
    if (remaining_bytes > 0) {
        uint32_t last_word = 0xFFFFFFFFUL;  /* Pad with 0xFF */
        volatile uint8_t *pMemBytes = (volatile uint8_t *)(startAddress + (word_count * 4));
        
        for (uint32_t i = 0; i < remaining_bytes; i++) {
            ((uint8_t *)&last_word)[i] = pMemBytes[i];
        }
        
        CRC->DR = last_word;
    }
    
    /* Return final CRC value */
    return CRC->DR;
}
