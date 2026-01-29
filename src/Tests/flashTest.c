



#include "MCAL/FLASH_Driver/flash.h"

#include "test.h"

void Example_ProgramData(void)
{
    Flash_ErrorStatus_t status;
    uint32_t address = 0x08020000;  /* Sector 5 base address */
    uint32_t data[] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    uint32_t i;
    
    /*
     * Step 1: Unlock Flash
     */
    status = Flash_Unlock();
    if (status != FLASH_OK)
    {
        /* Handle error */
        return;
    }
    
    /*
     * Step 2: Erase sector (MUST erase before programming!)
     */
    status = Flash_EraseSector(FLASH_SECTOR_5);
    if (status != FLASH_OK)
    {
        Flash_Lock();
        /* Handle error */
        return;
    }
    
    /*
     * Step 3: Program data
     */
    for (i = 0; i < 4; i++)
    {
        status = Flash_ProgramWord(address + (i * 4), data[i]);
        if (status != FLASH_OK)
        {
            Flash_Lock();
            /* Handle error */
            return;
        }
    }
    
    /*
     * Step 4: Lock Flash (always lock when done!)
     */
    Flash_Lock();
    
    /*
     * Step 5: Verify data (optional but recommended)
     */
    for (i = 0; i < 4; i++)
    {
        if (*((volatile uint32_t*)(address + (i * 4))) != data[i])
        {
            /* Verification failed! */
            return;
        }
    }
    
    /* Success! */
}


void Example_MassErase(void)
{
    Flash_ErrorStatus_t status;
    
    /*
     * ⚠️ WARNING: This erases EVERYTHING including this code!
     * Only use from bootloader that's in protected sector or RAM
     */
    
    status = Flash_Unlock();
    if (status != FLASH_OK) return;
    
    status = Flash_MassErase();
    if (status != FLASH_OK)
    {
        Flash_Lock();
        return;
    }
    
    Flash_Lock();
    
    /* After mass erase, all Flash = 0xFF */
    /* Device will likely hang if running from Flash! */
}

void Example_ProgramBuffer(void)
{
    Flash_ErrorStatus_t status;
    uint32_t address = 0x08020000;  /* Sector 5 */
    
    /* Data to program (could be firmware image) */
    uint8_t firmware[] = {
        0x00, 0x10, 0x00, 0x20,  /* Initial SP */
        0x09, 0x00, 0x02, 0x08,  /* Reset vector */
        /* ... more data ... */
    };
    
    /* Unlock */
    status = Flash_Unlock();
    if (status != FLASH_OK) return;
    
    /* Erase */
    status = Flash_EraseSector(FLASH_SECTOR_5);
    if (status != FLASH_OK)
    {
        Flash_Lock();
        return;
    }
    
    /* Program buffer */
    status = Flash_ProgramBuffer(address, firmware, sizeof(firmware));
    if (status != FLASH_OK)
    {
        Flash_Lock();
        return;
    }
    
    /* Lock */
    Flash_Lock();
}