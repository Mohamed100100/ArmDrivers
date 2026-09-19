# LIB — Standard Types & Utilities

**Location:** `include/LIB/stdtypes.h` + `src/LIB/` (if any)  
**Layer:** Foundation — included by every MCAL/HAL/OS header.

## 1. Overview
Zero-dependency standard typedefs and boolean/null definitions, isolated to `LIB/` so target headers never pull toolchain `<stdint.h>` directly (portable to bare-metal without newlib).

## 2. Features

### 2.1 Integer Types (`stdtypes.h:11`)
| Alias | Base | Width |
|---|---|---|
| `uint8_t` | `unsigned char` | 8 |
| `uint16_t` | `unsigned short int` | 16 |
| `uint32_t` | `unsigned int` | 32 |
| `uint64_t` | `unsigned long long` | 64 |
| `sint8_t` | `signed char` | 8 |
| `sint16_t` | `signed short int` | 16 |
| `sint32_t` | `signed int` | 32 |
| `sint64_t` | `signed long long` | 64 |
| `float32_t` | `float` | 32 |
| `float64_t` | `double` | 64 |

### 2.2 Boolean (`stdtypes.h:27`)
```c
typedef enum { FALSE=0, TRUE=1 } bool_t;
```

### 2.3 NULL Guard (`stdtypes.h:33`)
```c
#ifndef NULL
#define NULL ((void*)0)
#endif
```
Safe with free-standing toolchain where `<stddef.h>` not implicitly included.

## 3. Usage
```c
#include "LIB/stdtypes.h"
uint32_t sysclk = 84000000;
bool_t ok = TRUE;
if (ptr == NULL) { }
float32_t v = 3.14f;
```

## 4. Design Notes
- No `<stdint.h>` include — avoids dependency on host `stdint` layout; re-defining in project scope. When mixing CMSIS headers which typedef `uint32_t`, guard via include order (`LIB` first) or CMSIS `#ifndef`.
- `sint` prefix (not `int8_t`) is project convention — maps directly to signed qualifier.
- Single header, no `.c` — pure typedefs, zero footprint.

## 5. Dependencies
None. Leaf for all drivers.

## 6. Consumers
Every driver: `GPIO, RCC, NVIC, SysTick, UART, SPI, DMA, FLASH, CRC, LED, SWITCH, SEVENSEG, LCD, HSERIAL, MCU, Scheduler`.

## 7. Warnings
- Including both `LIB/stdtypes.h` and `<stdint.h>` in same TU causes typedef redefinition warning if toolchain defines same names — either remove `LIB` include for CMSIS-heavy files or guard `stdtypes.h` with `__STDINT_H`.
- `bool_t` is not `stdbool.h bool` — do not mix `true/false` vs `TRUE/FALSE`.
