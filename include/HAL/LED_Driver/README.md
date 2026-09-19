# LED Driver — HAL

**Location:** `include/HAL/LED_Driver/led.h` | `led_cfg.h` | `src/HAL/LED_Driver/led.c` , `led_cfg.c`  
**Depends:** `MCAL/GPIO_Driver`, `MCAL/RCC_Driver`  
**Board:** BlackPill (`PC13` inverted) + Kit 8 LEDs (config in `led_cfg.c`)

## 1. Overview
Configuration-driven LED abstraction. `led_cfg.c` declares `LED_cfg_t` array indexed by `LED_Name_t` ( `BLACK_PILL_LED, KIT_LED_1..8` in `led_cfg.h:9`). Driver translates `TurnON/OFF/Toggle` to GPIO writes honoring `activeState` (common via `activeHigh/activeLow`).

## 2. Features
| Feature | Detail |
|---|---|
| **Names** | `LED_Name_t` enum `BLACK_PILL_LED (0)` … `KIT_LED_8_LED (8)`, sentinel `LED_LEN=9` (`led_cfg.h:21`) |
| **Active State** | `LED_ACTIVE_HIGH` (GPIO HIGH → ON) / `LED_ACTIVE_LOW` (GPIO LOW → ON) — handles inverted BlackPill LED (`led.h:66`) |
| **Output Type** | `PUSH_PULL` / `OPEN_DRAIN` (`led.h:75`) passed to GPIO |
| **Config Struct** | `LED_cfg_t {LED_Port_t port, LED_Pin_t pin, activeState, outputType}` (`led.h:84`) — port/pin enums map 1:1 to GPIO |
| **Init** | `LED_vdInit()` — enables RCC for used ports, calls `GPIO_enuInit` per LED, drives all OFF (writes inactive level) |
| **Control** | `LED_vdTurnON(name)`, `LED_vdTurnOFF(name)`, `LED_vdToggle(name)` — each validates `name < LED_LEN`, translates active logic via `GPIO_enuSetPinVal` / `Flip` |

Status `LED_Status_t` (`led.h:12`): `OK, NOT_OK, NULL_PTR(2), WRONG_PORT(4), WRONG_PIN(5), WRONG_OUTPUT_TYPE(6), WRONG_LED_NAME` — aligned to GPIO codes.

## 3. Configuration (`led_cfg.c`)
```c
const LED_cfg_t LED_Configs[LED_LEN] = {
    [BLACK_PILL_LED] = {.port=PORT_C,.pin=PIN_13,.activeState=LED_ACTIVE_LOW,.outputType=LED_OUTPUT_TYPE_PUSH_PULL},
    [KIT_LED_1_LED]  = {.port=PORT_A,.pin=PIN_0, .activeState=LED_ACTIVE_HIGH,.outputType=LED_OUTPUT_TYPE_PUSH_PULL},
    // ...
};
```
Edit this table for custom board — no rebuild of `led.h`.

## 4. API Reference
| Function | Signature | Description |
|---|---|---|
| `LED_vdInit` | `LED_Status_t LED_vdInit(void)` | Iterates `LED_Configs`, enables RCC, calls `GPIO_enuInit`, sets OFF. Call first. |
| `LED_vdTurnON` | `LED_Status_t LED_vdTurnON(LED_Name_t)` | If `activeState==HIGH` → `GPIO_HIGH` else `LOW`. |
| `LED_vdTurnOFF` | `LED_Status_t LED_vdTurnOFF(LED_Name_t)` | Opposite of ON. |
| `LED_vdToggle` | `LED_Status_t LED_vdToggle(LED_Name_t)` | `GPIO_enuFlipPinVal`. |

## 5. Usage
```c
#include "HAL/LED_Driver/led.h"
LED_vdInit();
LED_vdTurnON(BLACK_PILL_LED);   // drives PC13 low
LED_vdTurnOFF(KIT_LED_1_LED);
LED_vdToggle(KIT_LED_2_LED);    // blink
// scheduler runnable
void Blink(void* arg){ LED_vdToggle(BLACK_PILL_LED); }
```

## 6. Dependencies
`GPIO_Driver` (all I/O), `RCC_Driver` for port clock. Optionally `OS/schedule` for blinking.

## 7. Notes
- `LED_vd*` naming: `vd` historic — returns `LED_Status_t` not `void` despite prefix.
- Validate `name` before call — out-of-range → `LED_WRONG_LED_NAME`.
- For new LEDs: extend `LED_Name_t` enum and `LED_Configs[]` — keep `LED_LEN` last.
