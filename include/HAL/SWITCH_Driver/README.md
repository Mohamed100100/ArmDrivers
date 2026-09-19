# SWITCH Driver — HAL

**Location:** `include/HAL/SWITCH_Driver/switch.h` | `switch_cfg.h` | `switch_cfg.c` | `src/HAL/SWITCH_Driver/switch.c`  
**Depends:** `MCAL/GPIO_Driver` (input mode + pull)  
**Board:** Kit switches (typically `PBx`, `PA0` etc. with pull-up/down per `switch_cfg.c`)

## 1. Overview
Debounced switch abstraction. Each switch declared in `switch_cfg.c` as `SWITCH_cfg_t` indexed by `SWITCH_Name_t` (user-defined, e.g., `SWITCH1_ON_KIT`). Driver maps GPIO IDR value to logical `SWITCH_PUSHED/RELEASED` respecting pull configuration and optional debouncing via scheduler tick or busy delay.

## 2. Features
| Feature | Detail |
|---|---|
| **Names** | `SWITCH_Name_t` enum defined in `switch_cfg.h` (app-specific, e.g., `KIT_SW_1`, sentinel `SWITCH_LEN`) |
| **Ports/Pins** | `SWITCH_Port_t {A,B,C,D,E,H}`, `SWITCH_Pin_t {0..15}` (`switch.h:27`) — 1:1 to GPIO |
| **Connection** | `SWITCH_Connection_t` bit-encoded (`switch.h:65`): `INTERNAL_PULLUP (0b0001)`, `INTERNAL_PULLDOWN (0010)`, `EXTERNAL_PULLUP (0100)`, `EXTERNAL_PULLDOWN (1000)` — maps to `GPIO_PULL` + mode `INPUT` |
| **Config Struct** | `SWITCH_cfg_t {port,pin,connection}` (`switch.h:76`) |
| **State Enum** | `SWITCH_State_t {PUSHED, RELEASED}` (`switch.h:89`) — logical, not electrical |
| **Pull Logic** | Pull-up: `IDR==0 → PUSHED`, `1 → RELEASED`; Pull-down opposite; Floating depends on external circuit |
| **Debounce** | `SWITCH_enuReadVAl` implements simple counter/time filter (5 samples @ 5ms or ~20ms busy) — driver-internal, no extra API. Returns `SWITCH_ERROR_READ` / `SCHEDUALE` on timing fail |
| **Init** | `SWITCH_enuInit()` — enables RCC, programs `GPIO_MODE_INPUT` + appropriate `PULL` per `connection` |

Status `SWITCH_Status_t` (`switch.h:12`): `OK, NOT_OK, NULL_PTR, ERROR, WRONG_NAME, ERROR_READ, ERROR_SCHEDUALE`.

## 3. Configuration (`switch_cfg.h/.c`)
```c
// switch_cfg.h
typedef enum { KIT_SW_1, KIT_SW_2, SWITCH_LEN } SWITCH_Name_t;
// switch_cfg.c
const SWITCH_cfg_t SWITCH_Configs[SWITCH_LEN] = {
    [KIT_SW_1] = {.port=SWITCH_PORT_B,.pin=SWITCH_PIN_4,.connection=SWITCH_INTERNAL_PULLUP},
};
```
Add new switches by extending enum + array.

## 4. API Reference
| Function | Signature | Description |
|---|---|---|
| `SWITCH_enuInit` | `SWITCH_Status_t SWITCH_enuInit(void)` | Registers GPIO inputs; must call before read. |
| `SWITCH_enuReadVAl` | `SWITCH_Status_t SWITCH_enuReadVAl(SWITCH_Name_t name, SWITCH_State_t *state)` | Validates `name`, null-ptr, reads `IDR` via `GPIO_enuReadPinVal`, applies pull inversion + debounce, writes `*state`. Note spelling `VAl` (capital A/l). |

## 5. Usage
```c
#include "HAL/SWITCH_Driver/switch.h"
SWITCH_enuInit();
SWITCH_State_t s;
if (SWITCH_enuReadVAl(KIT_SW_1, &s)==SWITCH_OK) {
    if (s==SWITCH_PUSHED) LED_vdTurnON(KIT_LED_1_LED);
}
// poll in scheduler runnable @10ms
void PollSw(void* arg){
    SWITCH_State_t st; SWITCH_enuReadVAl(KIT_SW_1, &st);
}
```

## 6. Electrical Mapping
| Connection | GPIO Pull | Idle IDR | Pressed IDR | Logical |
|---|---|---|---|---|
| `INTERNAL_PULLUP` | `PULL_UP` | `1` | `0` | `1→RELEASED, 0→PUSHED` |
| `INTERNAL_PULLDOWN` | `PULL_DOWN` | `0` | `1` | `0→RELEASED, 1→PUSHED` |
| `EXTERNAL_*` | `NO_PULL` (floating) | external | external | same inversion as above |

## 7. Dependencies
`GPIO_Driver`, `RCC` for port clock. Optionally `OS/schedule` if debounce uses scheduler delay.

## 8. Notes
- API name typo `SWITCH_enuReadVAl` preserved for compatibility.
- `EXTERNAL_*` still requires GPIO `INPUT` — ensure PCB has physical pull.
- Extend `SWITCH_Name_t` carefully — `SWITCH_LEN` must stay last for bounds.
