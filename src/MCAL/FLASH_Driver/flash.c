/******************************************************************************/
/* File Name   : flash.c                                                      */
/* Author      : Bootloader Team                                              */
/* Description : Implementation of STM32F401 Flash driver                     */
/******************************************************************************/

#include "MCAL/FLASH_Driver/flash.h"
#include "MCAL/FLASH_Driver/flash_priv.h"

/******************************************************************************/
/*                          PRIVATE MACROS                                     */
/******************************************************************************/
#define FLASH               (FLASH_BASE_ADDR)

/******************************************************************************/
/*                          PRIVATE VARIABLES                                  */
/******************************************************************************/

/* Sector base addresses lookup table */
static const uint32_t Flash_SectorBaseAddress[FLASH_TOTAL_SECTORS] = {
    FLASH_SECTOR_0_BASE,
    FLASH_SECTOR_1_BASE,
    FLASH_SECTOR_2_BASE,
    FLASH_SECTOR_3_BASE,
    FLASH_SECTOR_4_BASE,
    FLASH_SECTOR_5_BASE,
    FLASH_SECTOR_6_BASE,
    FLASH_SECTOR_7_BASE
};

/* Sector sizes lookup table */
static const uint32_t Flash_SectorSize[FLASH_TOTAL_SECTORS] = {
    FLASH_SECTOR_0_SIZE,
    FLASH_SECTOR_1_SIZE,
    FLASH_SECTOR_2_SIZE,
    FLASH_SECTOR_3_SIZE,
    FLASH_SECTOR_4_SIZE,
    FLASH_SECTOR_5_SIZE,
    FLASH_SECTOR_6_SIZE,
    FLASH_SECTOR_7_SIZE
};

/* Sector number register values lookup table */
static const uint32_t Flash_SectorNumber[FLASH_TOTAL_SECTORS] = {
    FLASH_CR_SNB_0,
    FLASH_CR_SNB_1,
    FLASH_CR_SNB_2,
    FLASH_CR_SNB_3,
    FLASH_CR_SNB_4,
    FLASH_CR_SNB_5,
    FLASH_CR_SNB_6,
    FLASH_CR_SNB_7
};

/******************************************************************************/
/*                    PRIVATE FUNCTION PROTOTYPES                              */
/******************************************************************************/
static Flash_ErrorStatus_t Flash_WaitForOperation(uint32_t Timeout);
static Flash_ErrorStatus_t Flash_CheckErrors(void);
static uint8_t Flash_IsValidAddress(uint32_t Address);

/******************************************************************************/
/*                    PRIVATE FUNCTION IMPLEMENTATIONS                         */
/******************************************************************************/

/**
 * @brief  Wait for Flash operation to complete
 * @param  Timeout: Maximum number of iterations to wait
 * @retval Flash_ErrorStatus_t: FLASH_OK if operation completed, error otherwise
 */
static Flash_ErrorStatus_t Flash_WaitForOperation(uint32_t Timeout)
{
    uint32_t tickstart = Timeout;
    
    /*
     * ┌─────────────────────────────────────────────────────────────┐
     * │                    WAIT FOR BSY = 0                         │
     * │                                                             │
     * │   Flash operations set BSY = 1 when started                 │
     * │   BSY is cleared by hardware when operation completes       │
     * │                                                             │
     * │   We poll BSY until:                                        │
     * │     - BSY = 0 (operation complete)                          │
     * │     - Timeout occurs (something went wrong)                 │
     * │                                                             │
     * └─────────────────────────────────────────────────────────────┘
     */
    
    while ((FLASH->SR & FLASH_SR_BSY) != 0U)
    {
        if (tickstart == 0U)
        {
            return FLASH_ERROR_TIMEOUT;
        }
        tickstart--;
    }
    
    /* Check for errors after operation */
    return Flash_CheckErrors();
}

/**
 * @brief  Check for errors in Flash status register
 * @param  None
 * @retval Flash_ErrorStatus_t: Error code or FLASH_OK
 */
static Flash_ErrorStatus_t Flash_CheckErrors(void)
{
    uint32_t sr = FLASH->SR;
    
    /*
     * ┌─────────────────────────────────────────────────────────────┐
     * │                    CHECK ERROR FLAGS                        │
     * │                                                             │
     * │   After any Flash operation, check these flags:             │
     * │                                                             │
     * │   WRPERR  - Tried to write/erase protected sector           │
     * │   PGAERR  - Address not properly aligned                    │
     * │   PGPERR  - Data size doesn't match PSIZE                   │
     * │   PGSERR  - Wrong programming sequence                      │
     * │   RDERR   - Tried to read protected area                    │
     * │   OPERR   - General operation error                         │
     * │                                                             │
     * └─────────────────────────────────────────────────────────────┘
     */
    
    if ((sr & FLASH_SR_WRPERR) != 0U)
    {
        return FLASH_ERROR_WRITE_PROTECTED;
    }
    
    if ((sr & FLASH_SR_PGAERR) != 0U)
    {
        return FLASH_ERROR_ALIGNMENT;
    }
    
    if ((sr & FLASH_SR_PGPERR) != 0U)
    {
        return FLASH_ERROR_PARALLELISM;
    }
    
    if ((sr & FLASH_SR_PGSERR) != 0U)
    {
        return FLASH_ERROR_SEQUENCE;
    }
    
    if ((sr & FLASH_SR_RDERR) != 0U)
    {
        return FLASH_ERROR_READ_PROTECTED;
    }
    
    if ((sr & FLASH_SR_OPERR) != 0U)
    {
        return FLASH_ERROR_OPERATION;
    }
    
    return FLASH_OK;
}

/**
 * @brief  Check if address is within valid Flash range
 * @param  Address: Address to check
 * @retval uint8_t: 1 if valid, 0 if invalid
 */
static uint8_t Flash_IsValidAddress(uint32_t Address)
{
    return ((Address >= FLASH_BASE) && (Address <= FLASH_END));
}

/******************************************************************************/
/*                    PUBLIC FUNCTION IMPLEMENTATIONS                          */
/******************************************************************************/

/**
 * @brief  Unlock Flash for programming/erasing operations
 */
Flash_ErrorStatus_t Flash_Unlock(void)
{
    Flash_ErrorStatus_t status = FLASH_OK;
    
    /*
     * ┌─────────────────────────────────────────────────────────────┐
     * │                    FLASH UNLOCK SEQUENCE                    │
     * │                                                             │
     * │   Flash is locked by default to prevent accidental writes   │
     * │                                                             │
     * │   To unlock:                                                │
     * │     1. Check if already unlocked (LOCK bit = 0)             │
     * │     2. Write KEY1 (0x45670123) to FLASH_KEYR                 │
     * │     3. Write KEY2 (0xCDEF89AB) to FLASH_KEYR                 │
     * │     4. Hardware clears LOCK bit if keys correct             │
     * │                                                             │
     * │   If wrong keys: Flash stays locked until reset!            │
     * │                                                             │
     * └─────────────────────────────────────────────────────────────┘
     */
    
    /* Check if Flash is already unlocked */
    if ((FLASH->CR & FLASH_CR_LOCK) != 0U)
    {
        /* Write unlock sequence */
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
        
        /* Verify unlock was successful */
        if ((FLASH->CR & FLASH_CR_LOCK) != 0U)
        {
            status = FLASH_ERROR_LOCKED;
        }
    }
    
    return status;
}

/**
 * @brief  Lock Flash to prevent accidental programming/erasing
 */
Flash_ErrorStatus_t Flash_Lock(void)
{
    /*
     * ┌─────────────────────────────────────────────────────────────┐
     * │                    FLASH LOCK                               │
     * │                                                             │
     * │   To lock Flash:                                            │
     * │     - Write 1 to LOCK bit in FLASH_CR                       │
     * │                                                             │
     * │   After locking:                                            │
     * │     - Programming is disabled                               │
     * │     - Erasing is disabled                                   │
     * │     - Must unlock again to write/erase                      │
     * │                                                             │
     * └─────────────────────────────────────────────────────────────┘
     */
    
    /* Set LOCK bit */
    FLASH->CR |= FLASH_CR_LOCK;
    
    return FLASH_OK;
}

/**
 * @brief  Erase a specific Flash sector
 */
Flash_ErrorStatus_t Flash_EraseSector(Flash_Sector_t Sector)
{
    Flash_ErrorStatus_t status = FLASH_OK;
    
    /*
     * ┌─────────────────────────────────────────────────────────────┐
     * │                  SECTOR ERASE PROCEDURE                     │
     * │                                                             │
     * │   1. Check sector is valid (0-7)                            │
     * │   2. Wait for any ongoing operation (BSY = 0)               │
     * │   3. Clear any previous errors                              │
     * │   4. Set SER bit (Sector Erase mode)                        │
     * │   5. Set SNB bits (Sector Number)                           │
     * │   6. Set STRT bit (Start erase)                             │
     * │   7. Wait for completion (BSY = 0)                          │
     * │   8. Clear SER bit                                          │
     * │                                                             │
     * │   ⚠️ Erase time: 250ms - 1s depending on sector size        │
     * │                                                             │
     * └─────────────────────────────────────────────────────────────┘
     */
    
    /* Step 1: Validate sector number */
    if (Sector >= FLASH_TOTAL_SECTORS)
    {
        return FLASH_ERROR_INVALID_SECTOR;
    }
    
    /* Check if Flash is locked */
    if ((FLASH->CR & FLASH_CR_LOCK) != 0U)
    {
        return FLASH_ERROR_LOCKED;
    }
    
    /* Step 2: Wait for any ongoing Flash operation */
    status = Flash_WaitForOperation(FLASH_TIMEOUT_VALUE);
    if (status != FLASH_OK)
    {
        return status;
    }
    
    /* Step 3: Clear all error flags */
    Flash_ClearErrors();
    
    /* Step 4 & 5: Configure for sector erase */
    /* Clear SER, MER, SNB, and PSIZE bits first */
    FLASH->CR &= (FLASH_CR_SER_CLR_MASK & FLASH_CR_MER_CLR_MASK & FLASH_CR_SNB_CLR_MASK);
    
    /* Set SER bit and sector number */
    FLASH->CR |= FLASH_CR_SER;                      /* Enable sector erase */
    FLASH->CR |= Flash_SectorNumber[Sector];        /* Set sector number */
    
    /* Step 6: Start erase by setting STRT bit */
    FLASH->CR |= FLASH_CR_STRT;
    
    /* Step 7: Wait for erase to complete */
    status = Flash_WaitForOperation(FLASH_ERASE_TIMEOUT_VALUE);
    
    /* Step 8: Clear SER bit */
    FLASH->CR &= FLASH_CR_SER_CLR_MASK;
    
    return status;
}

/**
 * @brief  Erase all Flash sectors (Mass Erase)
 */
Flash_ErrorStatus_t Flash_MassErase(void)
{
    Flash_ErrorStatus_t status = FLASH_OK;
    
    /*
     * ┌─────────────────────────────────────────────────────────────┐
     * │                   MASS ERASE PROCEDURE                      │
     * │                                                             │
     * │   ⚠️⚠️⚠️ WARNING: THIS ERASES ALL USER FLASH! ⚠️⚠️⚠️       │
     * │                                                             │
     * │   1. Wait for any ongoing operation (BSY = 0)               │
     * │   2. Clear any previous errors                              │
     * │   3. Set MER bit (Mass Erase mode)                          │
     * │   4. Set STRT bit (Start erase)                             │
     * │   5. Wait for completion (BSY = 0) - takes several seconds! │
     * │   6. Clear MER bit                                          │
     * │                                                             │
     * │   After mass erase:                                         │
     * │     - All sectors 0-7 contain 0xFF                          │
     * │     - Your program code is GONE!                            │
     * │                                                             │
     * └─────────────────────────────────────────────────────────────┘
     */
    
    /* Check if Flash is locked */
    if ((FLASH->CR & FLASH_CR_LOCK) != 0U)
    {
        return FLASH_ERROR_LOCKED;
    }
    
    /* Step 1: Wait for any ongoing Flash operation */
    status = Flash_WaitForOperation(FLASH_TIMEOUT_VALUE);
    if (status != FLASH_OK)
    {
        return status;
    }
    
    /* Step 2: Clear all error flags */
    Flash_ClearErrors();
    
    /* Step 3: Configure for mass erase */
    /* Clear SER, MER, SNB bits first */
    FLASH->CR &= (FLASH_CR_SER_CLR_MASK & FLASH_CR_MER_CLR_MASK & FLASH_CR_SNB_CLR_MASK);
    
    /* Set MER bit */
    FLASH->CR |= FLASH_CR_MER;
    
    /* Step 4: Start erase by setting STRT bit */
    FLASH->CR |= FLASH_CR_STRT;
    
    /* Step 5: Wait for erase to complete (this takes a long time!) */
    status = Flash_WaitForOperation(FLASH_MASS_ERASE_TIMEOUT_VALUE);
    
    /* Step 6: Clear MER bit */
    FLASH->CR &= FLASH_CR_MER_CLR_MASK;
    
    return status;
}

/**
 * @brief  Program a byte (8-bit) to Flash
 */
Flash_ErrorStatus_t Flash_ProgramByte(uint32_t Address, uint8_t Data)
{
    Flash_ErrorStatus_t status = FLASH_OK;
    
    /*
     * ┌─────────────────────────────────────────────────────────────┐
     * │                BYTE PROGRAMMING PROCEDURE                   │
     * │                                                             │
     * │   1. Validate address is in Flash range                     │
     * │   2. Wait for any ongoing operation (BSY = 0)               │
     * │   3. Clear any previous errors                              │
     * │   4. Set PSIZE = 00 (byte programming)                      │
     * │   5. Set PG bit (Programming mode)                          │
     * │   6. Write byte to Flash address                            │
     * │   7. Wait for completion (BSY = 0)                          │
     * │   8. Clear PG bit                                           │
     * │                                                             │
     * │   Note: Any address alignment is OK for byte programming    │
     * │                                                             │
     * └─────────────────────────────────────────────────────────────┘
     */
    
    /* Step 1: Validate address */
    if (!Flash_IsValidAddress(Address))
    {
        return FLASH_ERROR_INVALID_ADDRESS;
    }
    
    /* Check if Flash is locked */
    if ((FLASH->CR & FLASH_CR_LOCK) != 0U)
    {
        return FLASH_ERROR_LOCKED;
    }
    
    /* Step 2: Wait for any ongoing Flash operation */
    status = Flash_WaitForOperation(FLASH_TIMEOUT_VALUE);
    if (status != FLASH_OK)
    {
        return status;
    }
    
    /* Step 3: Clear all error flags */
    Flash_ClearErrors();
    
    /* Step 4: Set PSIZE to byte (x8) */
    FLASH->CR &= FLASH_CR_PSIZE_CLR_MASK;
    FLASH->CR |= FLASH_CR_PSIZE_BYTE;
    
    /* Step 5: Enable programming */
    FLASH->CR |= FLASH_CR_PG;
    
    /* Step 6: Write byte to Flash address */
    *((volatile uint8_t*)Address) = Data;
    
    /* Step 7: Wait for programming to complete */
    status = Flash_WaitForOperation(FLASH_TIMEOUT_VALUE);
    
    /* Step 8: Disable programming */
    FLASH->CR &= FLASH_CR_PG_CLR_MASK;
    
    return status;
}

/**
 * @brief  Program a half-word (16-bit) to Flash
 */
Flash_ErrorStatus_t Flash_ProgramHalfWord(uint32_t Address, uint16_t Data)
{
    Flash_ErrorStatus_t status = FLASH_OK;
    
    /*
     * ┌─────────────────────────────────────────────────────────────┐
     * │              HALF-WORD PROGRAMMING PROCEDURE                │
     * │                                                             │
     * │   Same as byte programming, but:                            │
     * │     - Address must be 2-byte aligned (divisible by 2)       │
     * │     - PSIZE = 01 (half-word)                                │
     * │     - Write as uint16_t                                     │
     * │                                                             │
     * └─────────────────────────────────────────────────────────────┘
     */
    
    /* Validate address */
    if (!Flash_IsValidAddress(Address))
    {
        return FLASH_ERROR_INVALID_ADDRESS;
    }
    
    /* Check alignment (must be 2-byte aligned) */
    if ((Address & 0x01U) != 0U)
    {
        return FLASH_ERROR_ALIGNMENT;
    }
    
    /* Check if Flash is locked */
    if ((FLASH->CR & FLASH_CR_LOCK) != 0U)
    {
        return FLASH_ERROR_LOCKED;
    }
    
    /* Wait for any ongoing Flash operation */
    status = Flash_WaitForOperation(FLASH_TIMEOUT_VALUE);
    if (status != FLASH_OK)
    {
        return status;
    }
    
    /* Clear all error flags */
    Flash_ClearErrors();
    
    /* Set PSIZE to half-word (x16) */
    FLASH->CR &= FLASH_CR_PSIZE_CLR_MASK;
    FLASH->CR |= FLASH_CR_PSIZE_HALFWORD;
    
    /* Enable programming */
    FLASH->CR |= FLASH_CR_PG;
    
    /* Write half-word to Flash address */
    *((volatile uint16_t*)Address) = Data;
    
    /* Wait for programming to complete */
    status = Flash_WaitForOperation(FLASH_TIMEOUT_VALUE);
    
    /* Disable programming */
    FLASH->CR &= FLASH_CR_PG_CLR_MASK;
    
    return status;
}

/**
 * @brief  Program a word (32-bit) to Flash
 */
Flash_ErrorStatus_t Flash_ProgramWord(uint32_t Address, uint32_t Data)
{
    Flash_ErrorStatus_t status = FLASH_OK;
    
    /*
     * ┌─────────────────────────────────────────────────────────────┐
     * │                WORD PROGRAMMING PROCEDURE                   │
     * │                                                             │
     * │   1. Validate address (must be 4-byte aligned)              │
     * │   2. Wait for any ongoing operation (BSY = 0)               │
     * │   3. Clear any previous errors                              │
     * │   4. Set PSIZE = 10 (word programming)                      │
     * │   5. Set PG bit (Programming mode)                          │
     * │   6. Write word to Flash address                            │
     * │   7. Wait for completion (BSY = 0)                          │
     * │   8. Clear PG bit                                           │
     * │                                                             │
     * │   This is the RECOMMENDED method for 3.3V operation         │
     * │   Fastest programming speed at this voltage                 │
     * │                                                             │
     * └─────────────────────────────────────────────────────────────┘
     */
    
    /* Validate address */
    if (!Flash_IsValidAddress(Address))
    {
        return FLASH_ERROR_INVALID_ADDRESS;
    }
    
    /* Check alignment (must be 4-byte aligned) */
    if ((Address & 0x03U) != 0U)
    {
        return FLASH_ERROR_ALIGNMENT;
    }
    
    /* Check if Flash is locked */
    if ((FLASH->CR & FLASH_CR_LOCK) != 0U)
    {
        return FLASH_ERROR_LOCKED;
    }
    
    /* Wait for any ongoing Flash operation */
    status = Flash_WaitForOperation(FLASH_TIMEOUT_VALUE);
    if (status != FLASH_OK)
    {
        return status;
    }
    
    /* Clear all error flags */
    Flash_ClearErrors();
    
    /* Set PSIZE to word (x32) */
    FLASH->CR &= FLASH_CR_PSIZE_CLR_MASK;
    FLASH->CR |= FLASH_CR_PSIZE_WORD;
    
    /* Enable programming */
    FLASH->CR |= FLASH_CR_PG;
    
    /* Write word to Flash address */
    *((volatile uint32_t*)Address) = Data;
    
    /* Wait for programming to complete */
    status = Flash_WaitForOperation(FLASH_TIMEOUT_VALUE);
    
    /* Disable programming */
    FLASH->CR &= FLASH_CR_PG_CLR_MASK;
    
    return status;
}

/**
 * @brief  Program a double-word (64-bit) to Flash
 */
Flash_ErrorStatus_t Flash_ProgramDoubleWord(uint32_t Address, uint64_t Data)
{
    Flash_ErrorStatus_t status = FLASH_OK;
    
    /*
     * ┌─────────────────────────────────────────────────────────────┐
     * │            DOUBLE-WORD PROGRAMMING PROCEDURE                │
     * │                                                             │
     * │   ⚠️ NOTE: Requires external VPP voltage!                   │
     * │   Rarely used in typical applications                       │
     * │                                                             │
     * │   Address must be 8-byte aligned                            │
     * │   PSIZE = 11 (double-word)                                  │
     * │                                                             │
     * └─────────────────────────────────────────────────────────────┘
     */
    
    /* Validate address */
    if (!Flash_IsValidAddress(Address))
    {
        return FLASH_ERROR_INVALID_ADDRESS;
    }
    
    /* Check alignment (must be 8-byte aligned) */
    if ((Address & 0x07U) != 0U)
    {
        return FLASH_ERROR_ALIGNMENT;
    }
    
    /* Check if Flash is locked */
    if ((FLASH->CR & FLASH_CR_LOCK) != 0U)
    {
        return FLASH_ERROR_LOCKED;
    }
    
    /* Wait for any ongoing Flash operation */
    status = Flash_WaitForOperation(FLASH_TIMEOUT_VALUE);
    if (status != FLASH_OK)
    {
        return status;
    }
    
    /* Clear all error flags */
    Flash_ClearErrors();
    
    /* Set PSIZE to double-word (x64) */
    FLASH->CR &= FLASH_CR_PSIZE_CLR_MASK;
    FLASH->CR |= FLASH_CR_PSIZE_DOUBLEWORD;
    
    /* Enable programming */
    FLASH->CR |= FLASH_CR_PG;
    
    /* Write double-word to Flash address */
    *((volatile uint64_t*)Address) = Data;
    
    /* Wait for programming to complete */
    status = Flash_WaitForOperation(FLASH_TIMEOUT_VALUE);
    
    /* Disable programming */
    FLASH->CR &= FLASH_CR_PG_CLR_MASK;
    
    return status;
}

/**
 * @brief  Program an array of bytes to Flash
 */
Flash_ErrorStatus_t Flash_ProgramBuffer(uint32_t Address, const uint8_t* Data, uint32_t Length)
{
    Flash_ErrorStatus_t status = FLASH_OK;
    uint32_t currentAddress = Address;
    uint32_t remainingBytes = Length;
    uint32_t index = 0U;
    
    /*
     * ┌─────────────────────────────────────────────────────────────┐
     * │               BUFFER PROGRAMMING PROCEDURE                  │
     * │                                                             │
     * │   Strategy for efficient programming:                       │
     * │                                                             │
     * │   1. If address is 4-byte aligned and ≥4 bytes remain:      │
     * │      → Program as words (32-bit) for speed                  │
     * │                                                             │
     * │   2. If address is 2-byte aligned and ≥2 bytes remain:      │
     * │      → Program as half-words (16-bit)                       │
     * │                                                             │
     * │   3. Otherwise:                                             │
     * │      → Program as bytes (8-bit)                             │
     * │                                                             │
     * │   This maximizes programming speed while handling           │
     * │   any address alignment and data length                     │
     * │                                                             │
     * └─────────────────────────────────────────────────────────────┘
     */
    
    /* Validate parameters */
    if (Data == NULL)
    {
        return FLASH_ERROR_OPERATION;
    }
    
    if (!Flash_IsValidAddress(Address) || !Flash_IsValidAddress(Address + Length - 1U))
    {
        return FLASH_ERROR_INVALID_ADDRESS;
    }
    
    /* Check if Flash is locked */
    if ((FLASH->CR & FLASH_CR_LOCK) != 0U)
    {
        return FLASH_ERROR_LOCKED;
    }
    
    /* Program data */
    while (remainingBytes > 0U)
    {
        /* Try to program as words (32-bit) when possible */
        if (((currentAddress & 0x03U) == 0U) && (remainingBytes >= 4U))
        {
            /* Address is 4-byte aligned and we have at least 4 bytes */
            uint32_t wordData = (uint32_t)Data[index] |
                               ((uint32_t)Data[index + 1U] << 8U) |
                               ((uint32_t)Data[index + 2U] << 16U) |
                               ((uint32_t)Data[index + 3U] << 24U);
            
            status = Flash_ProgramWord(currentAddress, wordData);
            if (status != FLASH_OK)
            {
                return status;
            }
            
            currentAddress += 4U;
            index += 4U;
            remainingBytes -= 4U;
        }
        /* Try to program as half-words (16-bit) when possible */
        else if (((currentAddress & 0x01U) == 0U) && (remainingBytes >= 2U))
        {
            /* Address is 2-byte aligned and we have at least 2 bytes */
            uint16_t halfWordData = (uint16_t)Data[index] |
                                   ((uint16_t)Data[index + 1U] << 8U);
            
            status = Flash_ProgramHalfWord(currentAddress, halfWordData);
            if (status != FLASH_OK)
            {
                return status;
            }
            
            currentAddress += 2U;
            index += 2U;
            remainingBytes -= 2U;
        }
        /* Program as bytes */
        else
        {
            status = Flash_ProgramByte(currentAddress, Data[index]);
            if (status != FLASH_OK)
            {
                return status;
            }
            
            currentAddress += 1U;
            index += 1U;
            remainingBytes -= 1U;
        }
    }
    
    return FLASH_OK;
}

/**
 * @brief  Clear all error flags in Flash status register
 */
void Flash_ClearErrors(void)
{
    /*
     * ┌─────────────────────────────────────────────────────────────┐
     * │                    CLEAR ERROR FLAGS                        │
     * │                                                             │
     * │   Error flags are cleared by writing 1 to them              │
     * │   (This is called "rc_w1" - read/clear write 1)             │
     * │                                                             │
     * │   Clear all error flags at once:                            │
     * │     - EOP (End of Operation)                                │
     * │     - OPERR (Operation Error)                               │
     * │     - WRPERR (Write Protection Error)                       │
     * │     - PGAERR (Alignment Error)                              │
     * │     - PGPERR (Parallelism Error)                            │
     * │     - PGSERR (Sequence Error)                               │
     * │     - RDERR (Read Error)                                    │
     * │                                                             │
     * └─────────────────────────────────────────────────────────────┘
     */
    
    /* Write 1 to clear all error flags */
    FLASH->SR = FLASH_SR_EOP    |
                FLASH_SR_OPERR  |
                FLASH_SR_WRPERR |
                FLASH_SR_PGAERR |
                FLASH_SR_PGPERR |
                FLASH_SR_PGSERR |
                FLASH_SR_RDERR;
}

/**
 * @brief  Get current Flash error status
 */
Flash_ErrorStatus_t Flash_GetError(void)
{
    /*
     * ┌─────────────────────────────────────────────────────────────┐
     * │                    GET ERROR STATUS                         │
     * │                                                             │
     * │   Read FLASH_SR and return appropriate error code           │
     * │   Priority: Check most specific errors first                │
     * │                                                             │
     * └─────────────────────────────────────────────────────────────┘
     */
    
    return Flash_CheckErrors();
}

/**
 * @brief  Check if Flash is busy with an operation
 */
uint8_t Flash_IsBusy(void)
{
    /*
     * ┌─────────────────────────────────────────────────────────────┐
     * │                    CHECK BUSY STATUS                        │
     * │                                                             │
     * │   BSY bit (bit 16) in FLASH_SR:                             │
     * │     0 = Flash is idle, ready for operation                  │
     * │     1 = Flash is busy with program/erase                    │
     * │                                                             │
     * └─────────────────────────────────────────────────────────────┘
     */
    
    return ((FLASH->SR & FLASH_SR_BSY) != 0U) ? 1U : 0U;
}

/**
 * @brief  Check if Flash is locked
 */
uint8_t Flash_IsLocked(void)
{
    /*
     * ┌─────────────────────────────────────────────────────────────┐
     * │                    CHECK LOCK STATUS                        │
     * │                                                             │
     * │   LOCK bit (bit 31) in FLASH_CR:                            │
     * │     0 = Flash is unlocked, can program/erase                │
     * │     1 = Flash is locked, programming/erasing blocked        │
     * │                                                             │
     * └─────────────────────────────────────────────────────────────┘
     */
    
    return ((FLASH->CR & FLASH_CR_LOCK) != 0U) ? 1U : 0U;
}

/**
 * @brief  Get sector number from Flash address
 */
Flash_ErrorStatus_t Flash_GetSectorFromAddress(uint32_t Address, Flash_Sector_t* Sector)
{
    /*
     * ┌─────────────────────────────────────────────────────────────┐
     * │              GET SECTOR FROM ADDRESS                        │
     * │                                                             │
     * │   Flash Memory Map:                                         │
     * │   ─────────────────────────────────────────────────────     │
     * │   Sector 0: 0x08000000 - 0x08003FFF (16 KB)                 │
     * │   Sector 1: 0x08004000 - 0x08007FFF (16 KB)                 │
     * │   Sector 2: 0x08008000 - 0x0800BFFF (16 KB)                 │
     * │   Sector 3: 0x0800C000 - 0x0800FFFF (16 KB)                 │
     * │   Sector 4: 0x08010000 - 0x0801FFFF (64 KB)                 │
     * │   Sector 5: 0x08020000 - 0x0803FFFF (128 KB)                │
     * │   Sector 6: 0x08040000 - 0x0805FFFF (128 KB)                │
     * │   Sector 7: 0x08060000 - 0x0807FFFF (128 KB)                │
     * │                                                             │
     * └─────────────────────────────────────────────────────────────┘
     */
    
    /* Validate parameters */
    if (Sector == NULL)
    {
        return FLASH_ERROR_OPERATION;
    }
    
    /* Check if address is in valid Flash range */
    if (!Flash_IsValidAddress(Address))
    {
        return FLASH_ERROR_INVALID_ADDRESS;
    }
    
    /* Determine sector based on address */
    if (Address < FLASH_SECTOR_1_BASE)
    {
        *Sector = FLASH_SECTOR_0;
    }
    else if (Address < FLASH_SECTOR_2_BASE)
    {
        *Sector = FLASH_SECTOR_1;
    }
    else if (Address < FLASH_SECTOR_3_BASE)
    {
        *Sector = FLASH_SECTOR_2;
    }
    else if (Address < FLASH_SECTOR_4_BASE)
    {
        *Sector = FLASH_SECTOR_3;
    }
    else if (Address < FLASH_SECTOR_5_BASE)
    {
        *Sector = FLASH_SECTOR_4;
    }
    else if (Address < FLASH_SECTOR_6_BASE)
    {
        *Sector = FLASH_SECTOR_5;
    }
    else if (Address < FLASH_SECTOR_7_BASE)
    {
        *Sector = FLASH_SECTOR_6;
    }
    else
    {
        *Sector = FLASH_SECTOR_7;
    }
    
    return FLASH_OK;
}

/**
 * @brief  Get base address of a sector
 */
Flash_ErrorStatus_t Flash_GetSectorBaseAddress(Flash_Sector_t Sector, uint32_t* Address)
{
    /*
     * ┌─────────────────────────────────────────────────────────────┐
     * │              GET SECTOR BASE ADDRESS                        │
     * │                                                             │
     * │   Returns the starting address of the specified sector      │
     * │   Uses lookup table for efficiency                          │
     * │                                                             │
     * └─────────────────────────────────────────────────────────────┘
     */
    
    /* Validate parameters */
    if (Address == NULL)
    {
        return FLASH_ERROR_OPERATION;
    }
    
    /* Validate sector number */
    if (Sector >= FLASH_TOTAL_SECTORS)
    {
        return FLASH_ERROR_INVALID_SECTOR;
    }
    
    /* Get base address from lookup table */
    *Address = Flash_SectorBaseAddress[Sector];
    
    return FLASH_OK;
}

/**
 * @brief  Get size of a sector in bytes
 */
Flash_ErrorStatus_t Flash_GetSectorSize(Flash_Sector_t Sector, uint32_t* Size)
{
    /*
     * ┌─────────────────────────────────────────────────────────────┐
     * │                  GET SECTOR SIZE                            │
     * │                                                             │
     * │   STM32F401 has non-uniform sector sizes:                   │
     * │                                                             │
     * │   Sectors 0-3: 16 KB each  (small sectors)                  │
     * │   Sector 4:    64 KB       (medium sector)                  │
     * │   Sectors 5-7: 128 KB each (large sectors)                  │
     * │                                                             │
     * │   Total: 4×16 + 64 + 3×128 = 512 KB                         │
     * │                                                             │
     * └─────────────────────────────────────────────────────────────┘
     */
    
    /* Validate parameters */
    if (Size == NULL)
    {
        return FLASH_ERROR_OPERATION;
    }
    
    /* Validate sector number */
    if (Sector >= FLASH_TOTAL_SECTORS)
    {
        return FLASH_ERROR_INVALID_SECTOR;
    }
    
    /* Get size from lookup table */
    *Size = Flash_SectorSize[Sector];
    
    return FLASH_OK;
}

/******************************************************************************/
/*                         END OF FILE                                         */
/******************************************************************************/