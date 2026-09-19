# NVIC Driver — MCAL

**Location:** `include/MCAL/NVIC_Driver/nvic.h` | `nvic_priv.h` | `nvic_stm32f401cc.h` | `src/MCAL/NVIC_Driver/nvic.c` , `nvic_stm32f401cc.c`  
**Core:** ARM Cortex-M4 NVIC (System Control Block + NVIC regs `0xE000E100` series)  
**Layer:** MCAL — arbitration for all peripheral interrupts (used by UART, SPI, DMA, Systick, HSERIAL).

## 1. Overview
Full NVIC abstraction: enable/disable, pending/active status, software trigger, priority set/get, priority grouping, and system reset via `AIRCR`. Validates IRQ `0-239` (NVIC supports up to 240 externals; F401 implements ~84). Maps directly to `ISER/ICER/ISPR/ICPR/IABR/IPR` and `SCB->AIRCR`.

## 2. Features
| Feature | Details |
|---|---|
| **240 IRQs** | `NVIC_IRQ_t` enum `NVIC_IRQ0..239` (`nvic.h:73`) |
| **Enable/Disable** | `NVIC_EnableIRQ` → `ISER[IRQ/32] |= 1<<(IRQ%32)`; `Disable` → `ICER` |
| **Pending** | `GetPendingIRQ` (reads `ISPR`), `SetPendingIRQ` (`ISPR`), `ClearPendingIRQ` (`ICPR`) |
| **Active** | `GetActiveIRQ` reads `IABR` — read-only, true while ISR executing (`nvic.h:446`) |
| **Priority** | `SetPriority(IRQ, 0..15)` writes `IPR[IRQ]` shifted to implemented bits (F401=4 bits → values `0x00..0xF0` upper-nibble) |
| **Priority Grouping** | `NVIC_SetPriorityGrouping(group)` → `AIRCR.PRIGROUP` ( `0x00` no preempt .. `0x07` full preempt) — split group/subpriority |
| **System Reset** | `NVIC_SystemReset()` → `AIRCR.SYSRESETREQ` with VECTKEY `0x05FA` — never returns |
| **Status Enums** | `NVIC_Status_t {NOT_OK, OK, NULL_PTR}`, `NVIC_Pending_t`, `NVIC_Active_t` |

## 3. API Reference
| Function | Signature | Description |
|---|---|---|
| `EnableIRQ` | `NVIC_Status_t NVIC_EnableIRQ(NVIC_IRQ_t)` | Enables IRQ in NVIC (`nvic.h:343`) |
| `DisableIRQ` | `NVIC_Status_t NVIC_DisableIRQ(NVIC_IRQ_t)` | Disables IRQ |
| `GetPendingIRQ` | `NVIC_Status_t NVIC_GetPendingIRQ(NVIC_IRQ_t, NVIC_Pending_t*)` | `ISPR` polling |
| `SetPendingIRQ` | `NVIC_Status_t NVIC_SetPendingIRQ(NVIC_IRQ_t)` | SW trigger — if enabled, ISR fires immediately |
| `ClearPendingIRQ` | `NVIC_Status_t NVIC_ClearPendingIRQ(NVIC_IRQ_t)` | `ICPR` |
| `GetActiveIRQ` | `NVIC_Status_t NVIC_GetActiveIRQ(NVIC_IRQ_t, NVIC_Active_t*)` | Reads `IABR` |
| `SetPriority` | `NVIC_Status_t NVIC_SetPriority(NVIC_IRQ_t, uint8_t priority)` | Lower value = higher priority |
| `GetPriority` | `NVIC_Status_t NVIC_GetPriority(NVIC_IRQ_t, uint8_t*)` | Reads `IPR` |
| `SystemReset` | `NVIC_Status_t NVIC_SystemReset(void)` | Triggers reset |
| `SetPriorityGrouping` | `NVIC_Status_t NVIC_SetPriorityGrouping(uint32_t)` | Sets `AIRCR.PRIGROUP` |

STM32F4-specific names in `nvic_stm32f401cc.h` map IRQs to peripherals (e.g., `NVIC_IRQ37 = USART1`).

## 4. Priority Grouping Models (4 bits, F4)
| AIRCR.PRIGROUP | Group bits | Sub bits | Behavior |
|---|---|---|---|
| 0x00 | 0 | 4 | No preemption, order by sub |
| 0x04 | 1 | 3 | 2 preempt levels |
| 0x05 | 2 | 2 | 4 preempt levels |
| 0x06 | 3 | 1 | 8 preempt levels |
| 0x07 | 4 | 0 | Full preemption |

Call once at boot before setting priorities.

## 5. Usage Examples

### 5.1 Enable USART1 interrupt at priority 5
```c
#include "MCAL/NVIC_Driver/nvic.h"
NVIC_SetPriorityGrouping(0x05); // 2+2 split
NVIC_SetPriority(NVIC_IRQ37, 5);
NVIC_EnableIRQ(NVIC_IRQ37);
// ensure USART1 CR1 RXNEIE also enabled in UART driver
```

### 5.2 Poll vs. SW trigger
```c
NVIC_Pending_t p; NVIC_GetPendingIRQ(NVIC_IRQ37, &p);
if (p == NVIC_PENDING) { /* handle */ }
NVIC_SetPendingIRQ(NVIC_IRQ38); // SW trigger USART2
NVIC_ClearPendingIRQ(NVIC_IRQ38);
```

### 5.3 Check active in nested ISR
```c
NVIC_Active_t a; NVIC_GetActiveIRQ(NVIC_IRQ56, &a); // DMA2 Stream0
```

### 5.4 System reset
```c
NVIC_SystemReset(); // does not return
```

## 6. Configuration
No `nvic_cfg.h` — grouping set at runtime. STM32F401 mapping via `nvic_stm32f401cc.h`.  

## 7. Dependencies
- `LIB/stdtypes.h`, CMSIS `core_cm4.h` for `SCB/NVIC` bases
- Used by: UART (TXE/TC/RXNE), SPI (TXE/RXNE/ERR), DMA (TC/HT/TE), SysTick (exception), HSERIAL

## 8. Warnings
- Enums list 0..239 unconditionally — validate against F401 datasheet (only ~84 implemented; writing beyond is harmless but does nothing).
- Disable critical IRQs (SysTick, DMA, UART) cautiously — can stall scheduler/IO.
- Priority values on F401 are top-aligned: writing `5` may become `0x50` in `IPR`.

## 9. Tests
`src/Tests/nvicTest.c`

## 10. Registers Touched
`NVIC_ISER[0..7], ICER, ISPR, ICPR, IABR, IPR[0..59], SCB_AIRCR`
