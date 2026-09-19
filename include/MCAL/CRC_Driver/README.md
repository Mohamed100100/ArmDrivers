# CRC Driver — MCAL

**Location:** `include/MCAL/CRC_Driver/crc.h` | `crc_priv.h` | `src/MCAL/CRC_Driver/crc.c`  
**Hardware:** STM32F4 CRC unit — `CRC_BASE 0x40023000` (`DR, IDR, CR`) — polynomial `0x4C11DB7` (Ethernet CRC-32), init `0xFFFFFFFF`, 4×AHB cycles per 32-bit word.  
**Note:** Clock must be enabled manually: `RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN` or `RCC_EnablePeripheralClock(RCC_AHB1_BUS, RCC_AHB1_CRC_CLOCK)` (per header note `crc.h:15`).

## 1. Overview
Hardware-accelerated CRC-32 driver exposing reset, single-word accumulate, array (with/without reset), byte-array, IDR scratch, and memory-region CRC (ideal for firmware integrity checks over Flash).

## 2. Features
| Feature | API | Detail |
|---|---|---|
| **Polynomial** | — | Fixed `0x04C11DB7`, 32-bit, no config |
| **Single word** | `CRC_Accumulate(uint32_t data)` | `CRC->DR = data` → `return CRC->DR` |
| **Array (no reset)** | `CRC_Accumulate_Array(const uint32_t *pData, uint32_t len)` | Loops `DR = pData[i]` — accumulates from current value |
| **Array (reset)** | `CRC_Calculate(const uint32_t *pData, uint32_t len)` | `Reset()` then accumulate — full CRC |
| **Byte array** | `CRC_Calculate_Bytes / Accumulate_Bytes(const uint8_t*, len)` | Handles unaligned; buffers bytes into 32-bit words (padding `0x00` for tail) — still resets option |
| **Present value** | `CRC_GetValue()` | `return CRC->DR` |
| **Scratch IDR** | `CRC_SetIDR(uint8_t) / GetIDR()` | 8-bit `IDR` general-purpose reg — survives `Reset()` (`CR.RESET` only resets `DR`) |
| **Memory region** | `CRC_CalculateMemoryRegion(startAddress, len)` / `AccumulateMemoryRegion` | Reads `*(uint32_t*)addr` directly — no copy. Useful for `CRC over 0x08000000` Flash |
| **Errors** | `CRC_Error_t {OK, NULL_PTR, ZERO_LEN}` | Array helpers return `0` on error (collides with valid CRC `0` — check before) |

## 3. Registers Model (`crc_priv.h`)
| Reg | Offset | Width | Access |
|---|---|---|---|
| `DR` | `0x00` | 32b R/W | Write feeds CRC engine; read yields result |
| `IDR` | `0x04` | 8b R/W | Scratch |
| `CR` | `0x08` | 1b W | Write `1` to `CR.RESET` → `DR=0xFFFFFFFF` |

## 4. API Reference
| Function | Prototype | Description |
|---|---|---|
| `Reset` | `void CRC_Reset(void)` | `CRC->CR = 1` → `DR=0xFFFFFFFF` |
| `Accumulate` | `uint32_t CRC_Accumulate(uint32_t data)` | `DR = data; return DR` |
| `Accumulate_Array` | `uint32_t CRC_Accumulate_Array(const uint32_t*, uint32_t len)` | `if !pData\|!len return 0; for(i) Accumulate(pData[i]);` |
| `Calculate` | `uint32_t CRC_Calculate(const uint32_t*, uint32_t len)` | `Reset(); return Accumulate_Array(...)` |
| `Calculate_Bytes` | `uint32_t CRC_Calculate_Bytes(const uint8_t*, uint32_t len)` | `Reset(); Accumulate_Bytes(...)` — packs 4B → word LE |
| `Accumulate_Bytes` | `uint32_t CRC_Accumulate_Bytes(const uint8_t*, uint32_t len)` | Same without reset |
| `GetValue` | `uint32_t CRC_GetValue(void)` | `return DR` |
| `SetIDR/GetIDR` | `void CRC_SetIDR(uint8_t)` / `uint8_t CRC_GetIDR(void)` | `IDR` |
| `CalculateMemoryRegion` | `uint32_t CRC_CalculateMemoryRegion(uint32_t start, uint32_t len)` / `Accumulate...` | `len` in bytes; should be %4 for optimal. Reads word-wise. |

## 5. Usage Examples

### 5.1 Basic word array
```c
#include "MCAL/CRC_Driver/crc.h"
#include "MCAL/RCC_Driver/rcc_int.h"
RCC_EnablePeripheralClock(RCC_AHB1_BUS, RCC_AHB1_CRC_CLOCK);
uint32_t data[]={0x12345678, 0x9ABCDEF0};
uint32_t crc = CRC_Calculate(data, 2); // 0x... 
// incremental:
CRC_Reset();
CRC_Accumulate(0x12345678);
crc = CRC_Accumulate(0x9ABCDEF0); // same as above
```

### 5.2 Byte stream (string)
```c
const char *msg="Hello STM32";
uint32_t c = CRC_Calculate_Bytes((uint8_t*)msg, strlen(msg));
```

### 5.3 Flash integrity
```c
// CRC over 16KB application area at 0x08008000
uint32_t appCrc = CRC_CalculateMemoryRegion(0x08008000, 0x4000);
// store expected in Flash info sector, compare at boot
if (appCrc != expected) NVIC_SystemReset();
```

### 5.4 Using IDR scratch
```c
CRC_SetIDR(0x5A); // survive resets
CRC_Reset();
uint8_t tag = CRC_GetIDR(); // still 0x5A
```

## 6. Endianness & Padding Note
Byte helpers pack as little-endian words (`buf[0] | buf[1]<<8 | ...`). Tail <4B is padded with `0x00` — so CRC depends on length strictly; receiver must mimic same pad.

## 7. Dependencies
`LIB/stdtypes.h`, `RCC_Driver` for `CRCEN`. No NVIC/interrupts — pure compute.

## 8. Warnings
- `NULL` or `len==0` returns `0` — ambiguous; validate args before (header documents `CRC_Error_t` but API returns `uint32_t`; error code is lost).
- `CalculateMemoryRegion` dereferences `startAddress` directly — ensure address is word-readable (Flash/SRAM) and `len` does not cross invalid mapping (BusFault).
- CRC core is polynomial-fixed; for CRC-16/8-CCITT etc., use SW.

## 9. Tests
No dedicated `Tests/crcTest.c` (check `crc` usage in bootloader/`flashTest`).
