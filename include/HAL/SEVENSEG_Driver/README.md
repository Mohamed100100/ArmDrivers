# SEVENSEG Driver — HAL

**Location:** `include/HAL/SEVENSEG_Driver/sevenseg.h` | `sevenseg_cfg.h` | `src/HAL/SEVENSEG_Driver/sevenseg.c` , `sevenseg_cfg.c`  
**Depends:** `MCAL/GPIO_Driver`  
**Display:** Common-cathode or common-anode 7-seg (segments A-G, active HIGH/LOW)

## 1. Overview
Single-digit seven-segment driver. Each display defined by 7 GPIO pins (A-G) plus `activeState` (determines segment-on level) and `outputType`. `sevenseg_cfg.c` can hold multiple instances indexed by `SEVSEG_Name_t` (if multi-digit, multiplex externally in scheduler).

```
      AAA
     F   B
      GGG
     E   C
      DDD
```

## 2. Features
| Feature | Detail |
|---|---|
| **Active State** | `SEVSEG_ACTIVE_HIGH` (cathode: HIGH=ON) / `ACTIVE_LOW` (anode: LOW=ON) (`sevenseg.h:66`) |
| **Output Type** | `PUSH_PULL / OPEN_DRAIN` (`sevenseg.h:75`) |
| **Segment Config** | `SEVSEG_Pinout_t {port,pin}` per segment (`sevenseg.h:85`); `SEVSEG_cfg_t {PinA..PinG, activeState, outputType}` (`sevenseg.h:106`) |
| **Digit Map** | Internal `segMap[10] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F}` bits `0=A..6=G` — driver translates per `activeState` |
| **Init** | `SEVSEG_enuInit()` — enables RCC for used ports, inits 7 GPIOs per display to `OUTPUT`, drives blank |
| **Display** | `SEVSEG_enuDisplayValue(uint8_t value)` — if `value>9` → `SEVSEG_NOT_OK` (or displays hex A-F if extended), else loops segments `A..G` and `GPIO_enuSetPinVal` per bit of `segMap[value]` XOR `activeState` |

Status `SEVSEG_Status_t` (`sevenseg.h:12`): `OK, NOT_OK, NULL_PTR(2), WRONG_PORT(4), WRONG_PIN(5), WRONG_OUTPUT_TYPE(6), WRONG_SEVSEG_NAME`.

## 3. Configuration (`sevenseg_cfg.h/.c`)
```c
typedef enum { SEVSEG_1, SEVSEG_LEN } SEVSEG_Name_t;
const SEVSEG_cfg_t SEVSEG_Configs[SEVSEG_LEN] = {
    [SEVSEG_1] = {
        .PinA={SEVSEG_PORT_A, SEVSEG_PIN_0}, .PinB={SEVSEG_PORT_A, SEVSEG_PIN_1},
        .PinC={SEVSEG_PORT_A, SEVSEG_PIN_2}, .PinD={SEVSEG_PORT_A, SEVSEG_PIN_3},
        .PinE={SEVSEG_PORT_A, SEVSEG_PIN_4}, .PinF={SEVSEG_PORT_A, SEVSEG_PIN_5},
        .PinG={SEVSEG_PORT_A, SEVSEG_PIN_6},
        .activeState=SEVSEG_ACTIVE_HIGH, .outputType=SEVSEG_OUTPUT_TYPE_PUSH_PULL
    }
};
```
For 4-digit multiplex, define 4 entries + common cathode/digit-select GPIOs driven externally.

## 4. API Reference
| Function | Signature | Description |
|---|---|---|
| `SEVSEG_enuInit` | `SEVSEG_Status_t SEVSEG_enuInit(void)` | Init all segments; OFF at start. |
| `SEVSEG_enuDisplayValue` | `SEVSEG_Status_t SEVSEG_enuDisplayValue(uint8_t value)` | Shows `0-9`. Some forks support `10-15 → A-F`. Current signature lacks `name` — single instance or uses global config. |

*If multi-instance fork, signature is `SEVSEG_enuDisplayValue(SEVSEG_Name_t, uint8_t)` — check your header.* 

## 5. Usage
```c
#include "HAL/SEVENSEG_Driver/sevenseg.h"
SEVSEG_enuInit();
SEVSEG_enuDisplayValue(5); // shows "5"
for (uint8_t i=0;i<10;i++){ SEVSEG_enuDisplayValue(i); SYSTICK_Wait_ms(500); }
// multiplex 4 digits via scheduler @5ms
void Mux(void* arg){
    static uint8_t idx=0;
    DisableAllDigits();
    SEVSEG_enuDisplayValue(digits[idx]);
    EnableDigit(idx); idx=(idx+1)%4;
}
```

## 6. Dependencies
`GPIO_Driver`, `RCC` for port clock, optional `OS/schedule` for multiplex/dim.

## 7. Notes
- 7 GPIOs per digit — consider `74HC595` shift expand for many digits.
- No DP (decimal point) pin — add `PinDP` if needed in custom fork.
- `activeState` inversion applies to whole digit — mixing cathode/anode per segment not supported.
