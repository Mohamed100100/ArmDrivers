# OS Scheduler — Cooperative

**Location:** `include/OS/schedule.h` | `schedule_cfg.h` | `src/OS/schedule.c`  
**Depends:** `MCAL/SYSTICK_TIMER_Driver`, `LIB/stdtypes`  
**Model:** Cooperative, time-triggered, periodic runnables — tick-driven by SysTick.

## 1. Overview
Lightweight non-preemptive scheduler. `SysTick_Handler` increments tick; `SCHED_enuStart()` loops, checks ready tasks (elapsed ≥ periodicity accounting for `FirstDalay_ms`), sorts ready by `Priority`, executes callbacks `CBF(void* Args)`. Periodicities must be multiples of tick period.

## 2. Features
| Feature | Detail |
|---|---|
| **Runnable Struct** | `SCHED_Runnable_t {RunnableCallback_t CBF, uint32_t Periodicity_ms, FirstDalay_ms, void* Args, uint32_t Priority}` (`schedule.h:32`) |
| **Init** | `SCHED_enuInit(uint32_t sysclk_Hz, uint32_t tick_ms)` — computes `SYSTICK_LOAD = (sysclk/presc /1000)*tick_ms -1`, `SYSTICK_Init`, `SetCallBack(Tick)`, `SetStartValue`, `StartCount`. Validates `SYSTICK_WRONG_STARTVALUE`. |
| **Register** | `SCHED_enuRegisterRunnable(SCHED_Runnable_t*)` — `NULL→NULL_PTR`, duplicate pointer → `ERROR_RUNNABLE_STORED_BEFORE`, stores in `RunnableList[]` (capacity `SCHED_MAX_RUNNABLES` in `schedule_cfg.h`, typically 10-20) |
| **Remove** | `SCHED_enuRemoveRunnable(ptr)` — clears slot, compact |
| **Start** | `SCHED_enuStart(void)` — `while(1){ for each r : if (tick >= firstDelay && (tick-firstDelay)%periodicity==0) push ready; sort ready by Priority desc; execute; }` — never returns; uses `WFI` or spin until next tick flag |
| **Callback** | `RunnableCallback_t(void* Args)` — `Args` is passthrough `void*`; cast inside runnable |
| **Priority** | Higher `Priority` value = earlier execution when multiple ready same tick |

Status `SCHED_Status_t` (`schedule.h:18`): `NOT_OK, OK, SYSTICK_ERROR, SYSTICK_WRONG_START_VALUE, FAILED_TO_SET_CALLBACK, NULL_PTR, ERROR_RUNNABLE_STORED_BEFORE`.

## 3. Configuration (`schedule_cfg.h`)
```c
#define SCHED_MAX_RUNNABLES  10  // max concurrent runnables
#define SCHED_SYSTICK_PRESCALER SYSTICK_NO_PRESCALLER
```

## 4. API Reference
| Function | Signature | Description |
|---|---|---|
| `SCHED_enuInit` | `SCHED_Status_t SCHED_enuInit(uint32_t sysclk, uint32_t tick_ms)` | Calls SysTick driver; tick_ms usually `1` or `2`. |
| `SCHED_enuRegisterRunnable` | `SCHED_Status_t SCHED_enuRegisterRunnable(SCHED_Runnable_t*)` | Add task before `Start`. |
| `SCHED_enuRemoveRunnable` | `SCHED_Status_t SCHED_enuRemoveRunnable(SCHED_Runnable_t*)` | Remove by address at runtime. |
| `SCHED_enuStart` | `void SCHED_enuStart(void)` | Infinite dispatch loop. |

## 5. Usage Examples

### 5.1 Basic 1 ms tick, 2 tasks
```c
#include "OS/schedule.h"
void Blink(void* arg){ LED_vdToggle(BLACK_PILL_LED); }
void PollSw(void* arg){ SWITCH_State_t s; SWITCH_enuReadVAl(KIT_SW_1,&s); }

SCHED_Runnable_t r1={.CBF=Blink,.Periodicity_ms=500,.FirstDalay_ms=0,.Args=NULL,.Priority=1};
SCHED_Runnable_t r2={.CBF=PollSw,.Periodicity_ms=10,.FirstDalay_ms=5,.Args=NULL,.Priority=2};

SCHED_enuInit(84000000, 1); // 84M, 1ms tick
SCHED_enuRegisterRunnable(&r1);
SCHED_enuRegisterRunnable(&r2);
SCHED_enuStart(); // never returns
```

### 5.2 LCD async dependency
```c
// LCD async needs scheduler running
SCHED_enuInit(84000000, 2);
SCHED_Runnable_t lcdSrv={.CBF=LCD_SchedulerTick,.Periodicity_ms=2,.FirstDalay_ms=0,.Priority=10};
SCHED_enuRegisterRunnable(&lcdSrv);
```

### 5.3 Dynamic remove
```c
SCHED_enuRemoveRunnable(&r1); // stop blinking
```

## 6. Constraints & Warnings
- **Cooperative:** Runnable must be short (< tick period). Blocking on `SYSTICK_Wait_ms` or Flash erase stalls all tasks.
- **Period multiples:** Periodicity must be multiple of `tick_ms` — `10ms task` with `1ms tick` OK; `15ms` with `2ms tick` will jitter.
- **Duplicate check** is pointer equality — two structs with same callback but different addresses are distinct.
- **No preemption** — higher priority only orders same-tick ready set, does not interrupt running task.
- **ISR use:** Do not call `SCHED_enuRegister` from ISR — not ISR-safe for list mutation.

## 7. Dependencies
`SYSTICK_TIMER_Driver` (sole time base), `LIB/stdtypes`.

## 8. Tests
No dedicated test — exercised by all async drivers (`LCD asychLcdTest.c`, `HSERIAL`), manual `schedule.c` validation.
