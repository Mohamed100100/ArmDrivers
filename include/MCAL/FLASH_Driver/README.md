# FLASH Driver — MCAL

**Location:** `include/MCAL/FLASH_Driver/flash.h` | `flash_priv.h` | `src/MCAL/FLASH_Driver/flash.c`  
**Hardware:** STM32F401CC embedded Flash — 256 KB (or 512 KB variant) across 8 sectors, `0x08000000-0x0807FFFF`, controller at `0x40023C00` (`ACR, KEYR, OPTKEYR, SR, CR, OPTCR`).  
**Role:** Code storage re-programming, bootloader, EEPROM emulation, IAP.

## 1. Overview
HAL for unlock/lock, sector/mass erase, program (byte/half-word/word/double-word), and metadata queries. Implements required unlock key sequence (`0x45670123, 0xCDEF89AB`), busy-wait, parallelism config (`PSIZE`), alignment checks, and error-flag handling per `FLASH_SR`/`CR`.

## 2. Features
| Feature | Detail |
|---|---|
| **Unlock/Lock** | `Flash_Unlock()` writes KEYR sequence; `Flash_Lock()` sets `CR.LOCK`. All write/erase return `FLASH_ERROR_LOCKED` if locked. |
| **Erase** | `Flash_EraseSector(Sector)` — sets `CR.SER+SNB`, `STRT`, polls `BSY`; latency `~250ms-1s`. `Flash_MassErase()` sets `CR.MER`. Both return `FLASH_ERROR_INVALID_SECTOR`, `BUSY`, `WRITE_PROTECTED`, `TIMEOUT`. |
| **Program Sizes** | `FLASH_PROGRAM_SIZE_BYTE (8b @ 1.8V-3.6V)` / `HALFWORD (16b @2.1V)` / `WORD (32b @2.7V)` / `DOUBLEWORD (64b + VPP)` (`flash.h:52`). API pre-sets `CR.PSIZE` accordingly. |
| **Program APIs** | Per-width: `ProgramByte/ HalfWord/ Word/ DoubleWord(address, data)`, plus `ProgramBuffer(addr, *data, len)` (loops byte-wise). Validates alignment (`_HALFWORD` even, `_WORD` %4, `_DOUBLEWORD` %8) → `FLASH_ERROR_ALIGNMENT`. |
| **Error Handling** | `Flash_ErrorStatus_t` 12 codes: `BUSY, WRITE_PROTECTED, ALIGNMENT, PARALLELISM, SEQUENCE, READ_PROTECTED, OPERATION, TIMEOUT, INVALID_SECTOR/ADDRESS, LOCKED` (`flash.h:20`). `Flash_ClearErrors()` clears `OPERR/RDERR/PGAERR/PGPERR/PGSERR`; `Flash_GetError()` reads `SR`. |
| **Status** | `Flash_IsBusy()` polls `SR.BSY`; `Flash_IsLocked()` reads `CR.LOCK`. |
| **Geometry Helpers** | `Flash_GetSectorFromAddress(addr → sector)`, `GetSectorBaseAddress(sector → addr)`, `GetSectorSize(sector → size)`. Sector sizes per F401: S0-3 16 KB, S4 64 KB, S5-7 128 KB (`flash.h:38`). |
| **Flash ACR** | Driver touches `ACR.LATENCY` in RCC PLL path; flash itself uses `ACR` for wait states, prefetch, I/D cache. |

## 3. Addressing & Sectors
| Sector | Size | Range |
|---|---|---|
| 0 | 16 KB | `0x08000000-0x08003FFF` |
| 1 | 16 KB | `0x08004000-0x08007FFF` |
| 2 | 16 KB | `0x08008000-0x0800BFFF` |
| 3 | 16 KB | `0x0800C000-0x0800FFFF` |
| 4 | 64 KB | `0x08010000-0x0801FFFF` |
| 5 | 128 KB | `0x08020000-0x0803FFFF` |
| 6 | 128 KB | `0x08040000-0x0805FFFF` |
| 7 | 128 KB | `0x08060000-0x0807FFFF` |

Writes outside → `FLASH_ERROR_INVALID_ADDRESS`. Erasing write-protected sectors → `FLASH_ERROR_WRITE_PROTECTED` (option bytes).

## 4. API Reference
| Function | Signature | Notes |
|---|---|---|
| `Unlock` | `Flash_ErrorStatus_t Flash_Unlock(void)` | Must precede any erase/program. |
| `Lock` | `Flash_ErrorStatus_t Flash_Lock(void)` | Call after. |
| `EraseSector` | `Flash_ErrorStatus_t Flash_EraseSector(Flash_Sector_t)` | ~500 ms. Polls `BSY` with timeout. |
| `MassErase` | `Flash_ErrorStatus_t Flash_MassErase(void)` | Irreversible >3 s. `warning` in docs (`flash.h:98`). |
| `ProgramByte` | `Flash_ErrorStatus_t Flash_ProgramByte(uint32_t Addr, uint8_t Data)` | `CR.PG=1, CR.PSIZE=00`. |
| `ProgramHalfWord` | `Flash_ErrorStatus_t Flash_ProgramHalfWord(uint32_t Addr, uint16_t Data)` | Align 2, `PSIZE=01`. |
| `ProgramWord` | `Flash_ErrorStatus_t Flash_ProgramWord(uint32_t Addr, uint32_t Data)` | Align 4, `PSIZE=10`. Example at `flash.h:138`. |
| `ProgramDoubleWord` | `Flash_ErrorStatus_t Flash_ProgramDoubleWord(uint32_t Addr, uint64_t Data)` | Align 8, needs VPP. |
| `ProgramBuffer` | `Flash_ErrorStatus_t Flash_ProgramBuffer(uint32_t Addr, const uint8_t* Data, uint32_t Len)` | Loops `ProgramByte` (or optimized `Word` if aligned). |
| `ClearErrors` | `void Flash_ClearErrors(void)` | Writes `1` to clear `SR` error bits. |
| `GetError` | `Flash_ErrorStatus_t Flash_GetError(void)` | Maps `SR` flags → enum. |
| `IsBusy/IsLocked` | `uint8_t Flash_IsBusy/IsLocked(void)` | Boolean. |
| `GetSectorFromAddress/BaseAddress/GetSectorSize` | helpers | Geometry. |

## 5. Usage Examples

### 5.1 Word program at 0x08020000 (sector 5)
```c
#include "MCAL/FLASH_Driver/flash.h"
Flash_ErrorStatus_t s = Flash_Unlock();
if (s==FLASH_OK) {
    s = Flash_EraseSector(FLASH_SECTOR_5);
    if (s==FLASH_OK) s = Flash_ProgramWord(0x08020000, 0x12345678);
    Flash_Lock();
}
```

### 5.2 Buffer program
```c
uint8_t myData[]={0x01,0x02,0x03,0x04};
Flash_Unlock();
Flash_ProgramBuffer(0x08020000, myData, sizeof(myData));
Flash_Lock();
```

### 5.3 Check errors
```c
if (Flash_GetError()!=FLASH_OK) Flash_ClearErrors();
while(Flash_IsBusy());
```

### 5.4 Geometry
```c
Flash_Sector_t sec; Flash_GetSectorFromAddress(0x08010000, &sec); // → 4
uint32_t base, sz; Flash_GetSectorBaseAddress(FLASH_SECTOR_5, &base);
Flash_GetSectorSize(FLASH_SECTOR_5, &sz); // 131072
```

## 6. Configuration
No `flash_cfg.h` — relies on linker `CustomLinkerScript.ld` to reserve application/Flash area. Ensure `APPNOT` does not overlap bootloader.

## 7. Dependencies
`LIB/stdtypes.h`, `RCC` for Flash clock (`AHB1ENR.FLITFEN` conceptually), no NVIC. Used by bootloader. Optional `DMA` not used.

## 8. Warnings
- **Erase is blocking 0.25-1 s** — suspend scheduler/IRQ or tolerate jitter.
- **1→0 only** — cannot program `0→1` without erase. Detect by read-verify.
- Voltage-dependent PSIZE: at <2.7 V only Byte/HalfWord allowed; supply 3.3 V for Word program.
- No internal double-buffer — do not execute from same bank while programming (stalls bus).

## 9. Tests
`src/Tests/flashTest.c` — erases sector 5, programs words, verifies, locks.
