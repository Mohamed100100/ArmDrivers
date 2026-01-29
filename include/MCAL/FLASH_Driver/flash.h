/******************************************************************************/
/* File Name   : flash.h                                                      */
/* Author      : Bootloader Team                                              */
/* Description : Public interface for STM32F401 Flash driver                  */
/*               Provides functions for programming and erasing Flash         */
/******************************************************************************/

#ifndef FLASH_H
#define FLASH_H

#include "LIB/stdtypes.h"

/******************************************************************************/
/*                          TYPE DEFINITIONS                                   */
/******************************************************************************/

/**
 * @brief Flash operation result codes
 */
typedef enum {
    FLASH_OK                    = 0x00U,  /* Operation successful */
    FLASH_ERROR_BUSY            = 0x01U,  /* Flash is busy */
    FLASH_ERROR_WRITE_PROTECTED = 0x02U,  /* Sector is write protected */
    FLASH_ERROR_ALIGNMENT       = 0x03U,  /* Address alignment error */
    FLASH_ERROR_PARALLELISM     = 0x04U,  /* Program parallelism error */
    FLASH_ERROR_SEQUENCE        = 0x05U,  /* Programming sequence error */
    FLASH_ERROR_READ_PROTECTED  = 0x06U,  /* Read protection error */
    FLASH_ERROR_OPERATION       = 0x07U,  /* General operation error */
    FLASH_ERROR_TIMEOUT         = 0x08U,  /* Timeout during operation */
    FLASH_ERROR_INVALID_SECTOR  = 0x09U,  /* Invalid sector number */
    FLASH_ERROR_INVALID_ADDRESS = 0x0AU,  /* Address out of Flash range */
    FLASH_ERROR_LOCKED          = 0x0BU,  /* Flash is locked */
} Flash_ErrorStatus_t;

/**
 * @brief Flash sector numbers
 */
typedef enum {
    FLASH_SECTOR_0 = 0U,  /* 16 KB  : 0x08000000 - 0x08003FFF */
    FLASH_SECTOR_1 = 1U,  /* 16 KB  : 0x08004000 - 0x08007FFF */
    FLASH_SECTOR_2 = 2U,  /* 16 KB  : 0x08008000 - 0x0800BFFF */
    FLASH_SECTOR_3 = 3U,  /* 16 KB  : 0x0800C000 - 0x0800FFFF */
    FLASH_SECTOR_4 = 4U,  /* 64 KB  : 0x08010000 - 0x0801FFFF */
    FLASH_SECTOR_5 = 5U,  /* 128 KB : 0x08020000 - 0x0803FFFF */
    FLASH_SECTOR_6 = 6U,  /* 128 KB : 0x08040000 - 0x0805FFFF */
    FLASH_SECTOR_7 = 7U,  /* 128 KB : 0x08060000 - 0x0807FFFF */
} Flash_Sector_t;

/**
 * @brief Flash program data size
 */
typedef enum {
    FLASH_PROGRAM_SIZE_BYTE       = 0U,  /* 8-bit  programming (1.8V - 3.6V) */
    FLASH_PROGRAM_SIZE_HALFWORD   = 1U,  /* 16-bit programming (2.1V - 3.6V) */
    FLASH_PROGRAM_SIZE_WORD       = 2U,  /* 32-bit programming (2.7V - 3.6V) */
    FLASH_PROGRAM_SIZE_DOUBLEWORD = 3U,  /* 64-bit programming (external VPP) */
} Flash_ProgramSize_t;

/******************************************************************************/
/*                    FUNCTION PROTOTYPES                                      */
/******************************************************************************/

/**
 * @brief  Unlock Flash for programming/erasing operations
 * @note   Must be called before any write or erase operation
 * @param  None
 * @retval Flash_ErrorStatus_t: FLASH_OK if successful
 */
Flash_ErrorStatus_t Flash_Unlock(void);

/**
 * @brief  Lock Flash to prevent accidental programming/erasing
 * @note   Should be called after write/erase operations are complete
 * @param  None
 * @retval Flash_ErrorStatus_t: FLASH_OK if successful
 */
Flash_ErrorStatus_t Flash_Lock(void);

/**
 * @brief  Erase a specific Flash sector
 * @note   Sector erase takes significant time (~250ms - 1s)
 * @param  Sector: Sector number to erase (FLASH_SECTOR_0 to FLASH_SECTOR_7)
 * @retval Flash_ErrorStatus_t: FLASH_OK if successful, error code otherwise
 * 
 * @code
 *   // Example: Erase sector 5
 *   Flash_ErrorStatus_t status;
 *   status = Flash_Unlock();
 *   if (status == FLASH_OK) {
 *       status = Flash_EraseSector(FLASH_SECTOR_5);
 *       Flash_Lock();
 *   }
 * @endcode
 */
Flash_ErrorStatus_t Flash_EraseSector(Flash_Sector_t Sector);

/**
 * @brief  Erase all Flash sectors (Mass Erase)
 * @note   This erases ALL user Flash! Takes several seconds.
 * @warning This operation is irreversible!
 * @param  None
 * @retval Flash_ErrorStatus_t: FLASH_OK if successful, error code otherwise
 * 
 * @code
 *   // Example: Mass erase entire Flash
 *   Flash_ErrorStatus_t status;
 *   status = Flash_Unlock();
 *   if (status == FLASH_OK) {
 *       status = Flash_MassErase();
 *       Flash_Lock();
 *   }
 * @endcode
 */
Flash_ErrorStatus_t Flash_MassErase(void);

/**
 * @brief  Program a byte (8-bit) to Flash
 * @param  Address: Flash address to program (must be in valid Flash range)
 * @param  Data: Byte value to program
 * @retval Flash_ErrorStatus_t: FLASH_OK if successful, error code otherwise
 */
Flash_ErrorStatus_t Flash_ProgramByte(uint32_t Address, uint8_t Data);

/**
 * @brief  Program a half-word (16-bit) to Flash
 * @param  Address: Flash address to program (must be 2-byte aligned)
 * @param  Data: Half-word value to program
 * @retval Flash_ErrorStatus_t: FLASH_OK if successful, error code otherwise
 */
Flash_ErrorStatus_t Flash_ProgramHalfWord(uint32_t Address, uint16_t Data);

/**
 * @brief  Program a word (32-bit) to Flash
 * @param  Address: Flash address to program (must be 4-byte aligned)
 * @param  Data: Word value to program
 * @retval Flash_ErrorStatus_t: FLASH_OK if successful, error code otherwise
 * 
 * @code
 *   // Example: Program a word to address 0x08020000
 *   Flash_ErrorStatus_t status;
 *   status = Flash_Unlock();
 *   if (status == FLASH_OK) {
 *       status = Flash_ProgramWord(0x08020000, 0x12345678);
 *       Flash_Lock();
 *   }
 * @endcode
 */
Flash_ErrorStatus_t Flash_ProgramWord(uint32_t Address, uint32_t Data);

/**
 * @brief  Program a double-word (64-bit) to Flash
 * @note   Requires external VPP voltage - rarely used
 * @param  Address: Flash address to program (must be 8-byte aligned)
 * @param  Data: Double-word value to program
 * @retval Flash_ErrorStatus_t: FLASH_OK if successful, error code otherwise
 */
Flash_ErrorStatus_t Flash_ProgramDoubleWord(uint32_t Address, uint64_t Data);

/**
 * @brief  Program an array of bytes to Flash
 * @param  Address: Starting Flash address
 * @param  Data: Pointer to data buffer
 * @param  Length: Number of bytes to program
 * @retval Flash_ErrorStatus_t: FLASH_OK if successful, error code otherwise
 * 
 * @code
 *   // Example: Program an array of data
 *   uint8_t myData[] = {0x01, 0x02, 0x03, 0x04};
 *   Flash_ErrorStatus_t status;
 *   
 *   status = Flash_Unlock();
 *   if (status == FLASH_OK) {
 *       status = Flash_ProgramBuffer(0x08020000, myData, sizeof(myData));
 *       Flash_Lock();
 *   }
 * @endcode
 */
Flash_ErrorStatus_t Flash_ProgramBuffer(uint32_t Address, const uint8_t* Data, uint32_t Length);

/**
 * @brief  Clear all error flags in Flash status register
 * @param  None
 * @retval None
 */
void Flash_ClearErrors(void);

/**
 * @brief  Get current Flash error status
 * @param  None
 * @retval Flash_ErrorStatus_t: Current error code or FLASH_OK if no errors
 */
Flash_ErrorStatus_t Flash_GetError(void);

/**
 * @brief  Check if Flash is busy with an operation
 * @param  None
 * @retval uint8_t: 1 if busy, 0 if not busy
 */
uint8_t Flash_IsBusy(void);

/**
 * @brief  Check if Flash is locked
 * @param  None
 * @retval uint8_t: 1 if locked, 0 if unlocked
 */
uint8_t Flash_IsLocked(void);

/**
 * @brief  Get sector number from Flash address
 * @param  Address: Flash address
 * @param  Sector: Pointer to store sector number
 * @retval Flash_ErrorStatus_t: FLASH_OK if valid address, FLASH_ERROR_INVALID_ADDRESS otherwise
 */
Flash_ErrorStatus_t Flash_GetSectorFromAddress(uint32_t Address, Flash_Sector_t* Sector);

/**
 * @brief  Get base address of a sector
 * @param  Sector: Sector number
 * @param  Address: Pointer to store base address
 * @retval Flash_ErrorStatus_t: FLASH_OK if valid sector, FLASH_ERROR_INVALID_SECTOR otherwise
 */
Flash_ErrorStatus_t Flash_GetSectorBaseAddress(Flash_Sector_t Sector, uint32_t* Address);

/**
 * @brief  Get size of a sector in bytes
 * @param  Sector: Sector number
 * @param  Size: Pointer to store size
 * @retval Flash_ErrorStatus_t: FLASH_OK if valid sector, FLASH_ERROR_INVALID_SECTOR otherwise
 */
Flash_ErrorStatus_t Flash_GetSectorSize(Flash_Sector_t Sector, uint32_t* Size);

#endif /* FLASH_H */