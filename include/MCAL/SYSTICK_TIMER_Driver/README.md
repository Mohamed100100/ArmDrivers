# SysTick Driver — MCAL

**Location:** `include/MCAL/SYSTICK_TIMER_Driver/systick.h` | `systick_priv.h` | `src/MCAL/SYSTICK_TIMER_Driver/systick.c`  
**Hardware:** ARM Cortex-M4 SysTick — 24-bit down-counter at `0xE000E010` (`CTRL/LOAD/VAL/CALIB`)  
**Role:** Time-base for `OS/schedule` and blocking delays.

## 1. Overview
Thin wrapper over Cortex-M SysTick. Configures `CTRL` (ENABLE/TICKINT/CLKSOURCE), `LOAD` (reload), `VAL` (current) with clock-source selection (processor clock or ÷8). Provides init, callback, reload-value, start/stop, busywait `Wait_ms`, and `GetCurrentCount`. Used by the cooperative scheduler as its tick.

## 2. Features
| Feature | Detail |
|---|---|
| **Clock Sources** | `SYSTICK_NO_PRESCALLER (0b100)` = processor clock; `SYSTICK_PRESCALLER_8 (0b000)` = ÷8 (`systick.h:33`) |
| **24-bit Reload** | `LOAD` range `0x000000`–`0xFFFFFF` (16 777 215) — driver validates `SYSTICK_WRONG_STARTVALUE / ZERO_STARTVALUE` |
| **Interrupt + Callback** | `SYSTICK_SetCallBack(SYSTICK_Callback_t)` stores ISR callback; SysTick exception fires when `VAL`→0 |
| **Control States** | `SYSTICK_OFF`, `SYSTICK_EXCEPTION_OFF` distinguish disabled counter vs disabled interrupt |
| **Start/Stop** | `SYSTICK_StartCount()` sets `CTRL.ENABLE`; `SYSTICK_StopCount()` clears it |
| **Blocking Delay** | `SYSTICK_Wait_ms(uint32_t ms)` — busywait polling `COUNTFLAG`; calculates ticks from `ClockValue` saved in `SYSTICK_Init` |
| **Current Count** | `SYSTICK_GetCurrentCount(uint32_t*)` reads `VAL` |

## 3. Enums & Types
```c
typedef void (*SYSTICK_Callback_t)(void);
typedef enum { SYSTICK_NOT_OK, SYSTICK_OK, SYSTICK_WRONG_PRESCALLER,
               SYSTICK_WRONG_STARTVALUE, SYSTICK_OFF, SYSTICK_EXCEPTION_OFF,
               SYSTICK_ZERO_STARTVALUE, SYSTICK_NULL_PTR } SYSTICK_Status_t;
typedef enum { SYSTICK_NO_PRESCALLER=0b100, SYSTICK_PRESCALLER_8=0b000 } SYSTICK_Prescaller_t;
```

## 4. API Reference
| Function | Prototype | Description |
|---|---|---|
| `Init` | `SYSTICK_Status_t SYSTICK_Init(uint32_t ClockValue, SYSTICK_Prescaller_t)` | Saves `ClockValue`, configures `CTRL.CLKSOURCE` from prescaler, validates, disables initially. `ClockValue` is sysclk in Hz. |
| `SetCallBack` | `SYSTICK_Status_t SYSTICK_SetCallBack(SYSTICK_Callback_t)` | Registers ISR callback; `NULL` → `SYSTICK_NULL_PTR`. |
| `SetStartValue` | `SYSTICK_Status_t SYSTICK_SetStartValue(uint32_t)` | Writes `LOAD` (24-bit). Checks zero/overflow. |
| `StartCount` | `void SYSTICK_StartCount()` | `CTRL.ENABLE=1`, `CTRL.TICKINT=1`. |
| `StopCount` | `void SYSTICK_StopCount()` | `CTRL.ENABLE=0`. |
| `Wait_ms` | `SYSTICK_Status_t SYSTICK_Wait_ms(uint32_t ms)` | Computes `ticks = (Clock/(presc?1:8)/1000)*ms`, loads `LOAD`, polls `CTRL.COUNTFLAG` per ms. Blocking. |
| `GetCurrentCount` | `SYSTICK_Status_t SYSTICK_GetCurrentCount(uint32_t*)` | `*ptr = SYST_VAL`. |

## 5. Usage Examples

### 5.1 Scheduler tick (1 ms from 84 MHz, no prescaler)
```c
#include "MCAL/SYSTICK_TIMER_Driver/systick.h"
SYSTICK_Init(84000000, SYSTICK_NO_PRESCALLER);
SYSTICK_SetCallBack(Scheduler_Tick); // void Scheduler_Tick(void)
SYSTICK_SetStartValue(84000 - 1); // (84M/1000) -1
SYSTICK_StartCount();
```

### 5.2 Divided clock (8 MHz)
```c
SYSTICK_Init(84000000, SYSTICK_PRESCALLER_8); // tick = AHB/8 = 10.5M
SYSTICK_SetStartValue(10500 - 1); // 1ms
```

### 5.3 Blocking delay (no scheduler)
```c
SYSTICK_Init(16000000, SYSTICK_NO_PRESCALLER);
SYSTICK_Wait_ms(100); // blocks 100ms
```

### 5.4 Poll current
```c
uint32_t v; SYSTICK_GetCurrentCount(&v);
```

## 6. Configuration
No `systick_cfg.h`. `ClockValue` passed to `Init` is authoritative — mismatch yields wrong delays. `LOAD` must be pre-computed by caller or via scheduler `SCHED_enuInit(sysclk, tick_ms)`.

## 7. Dependencies
- `LIB/stdtypes.h`, `CMSIS core_cm4`
- Consumed by `OS/schedule` (required). Also free for app delays before scheduler starts.

## 8. Warnings
- `Wait_ms` is **blocking** — do not call from ISR or when scheduler expects SysTick as system tick (it will stall tasks).
- Only one callback slot — second `SetCallBack` overwrites.
- `LOAD` is `value-1` convention (writing `N` yields period `N+1` ticks).
- Enabling with `SYSTICK_PRESCALLER_8` at high sysclk may still exceed 24-bit for large `ms` in `Wait_ms` → `SYSTICK_WRONG_STARTVALUE`.

## 9. Tests
No dedicated `Tests/systickTest.c`; exercised via `OS/schedule` runnables.

## 10. Registers
`SYST_CSR (CTRL) 0xE000E010` | `RVR (LOAD) 0xE000E014` | `CVR (VAL) 0xE000E018` | `CALIB 0xE000E01C`
