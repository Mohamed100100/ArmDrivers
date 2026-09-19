# GPIO Driver — MCAL

**Location:** `include/MCAL/GPIO_Driver/gpio_int.h` | `include/MCAL/GPIO_Driver/gpio_priv.h` | `src/MCAL/GPIO_Driver/gpio.c`  
**Target:** STM32F401CC (Cortex-M4) — Ports A, B, C, D, E, H (base `0x40020000` .. `0x40021C00` in `gpio_priv.h:21`)  
**Layer:** MCAL — Basis for all HAL drivers (LED, SWITCH, SEVENSEG, LCD, HSERIAL)

## 1. Overview
Full-featured GPIO abstraction that maps STM32 registers (`MODER`, `OTYPER`, `OSPEEDR`, `PUPDR`, `IDR`, `ODR`, `BSRR`, `LCKR`, `AFRL/AFRH` at `gpio_priv.h:426`) to a type-safe API. All functions perform bit-mask validation (`GPIO_MODE_MASK_CHECK` etc. at `gpio_priv.h:35`) and return detailed `GPIO_Status_t`.

`GPIO_Base_Addreses[]` in `gpio.c:24` holds the 6 port bases indexed by `GPIO_Port_t`.

## 2. Features
| Feature | Details |
|---|---|
| **4 Modes** | `GPIO_MODE_INPUT`, `OUTPUT`, `ALTERNATE_FUNCTION`, `ANALOG` (`gpio_int.h:37`) |
| **Output Types** | Push-Pull / Open-Drain (`gpio_int.h:50`) — valid in OUTPUT/AF modes |
| **Speeds** | LOW (8 MHz) / MEDIUM (50 MHz) / HIGH (100 MHz) / VERY_HIGH (180 MHz) (`gpio_int.h:61`) |
| **Pull** | NO_PULL / PULL_UP / PULL_DOWN (`gpio_int.h:74`) |
| **AF Mapping** | AF0–AF15, auto-routed to `AFRL` (pins 0-7) or `AFRH` (8-15) (`gpio_int.h:124`, `gpio.c:112`) |
| **Atomic Set/Reset** | `GPIO_enuSetPinVal` uses `BSRR` (no read-modify-write) — `GPIO_HIGH=0`, `GPIO_LOW=16` (`gpio_int.h:165`) |
| **Toggle & Read** | `GPIO_enuFlipPinVal` (XOR `ODR`), `GPIO_enuReadPinVal` (reads `IDR`) |
| **Dynamic Re-config** | Per-pin `SetPinMode`, `SetAltFunc`, `SetPinOutType`, `SetPinPull`, `SetPinSpeed` |
| **Robust Errors** | 11 status codes: `GPIO_NULL_PTR`, `WRONG_MODE/PORT/PIN/OUTPUT_TYPE/PULL/ALTARNATIVE/SPEED/VALUE` (`gpio_int.h:17`) |

## 3. Configuration Type

```c
typedef struct {
    GPIO_Port_t port;                           // A,B,C,D,E,H
    GPIO_Pin_t pin;                             // 0-15
    GPIO_Mode_t mode;                           // INPUT/OUTPUT/AF/ANALOG
    GPIO_OutputType_t outputType;               // PUSH_PULL / OPEN_DRAIN
    GPIO_Speed_t speed;                         // LOW .. VERY_HIGH
    GPIO_Pull_t pull;                           // NO_PULL / PULL_UP / PULL_DOWN
    GPIO_AlternateFunction_t alternateFunction; // AF0..AF15
} GPIO_cfg_t; // gpio_int.h:148
```

## 4. API Reference

| Function | Prototype | Description |
|---|---|---|
| **Init** | `GPIO_Status_t GPIO_enuInit(GPIO_cfg_t *Copy_pstGPIOCfg)` | Validates all fields, programs `MODER/OTYPER/PUPDR/OSPEEDR/AFRx`. Clock must be enabled before call (`gpio.c:60`). |
| **Set** | `GPIO_enuSetPinVal(GPIO_Port_t, GPIO_Pin_t, GPIO_Val_t)` | Atomic write via `BSRR`: `(1<<pin)<<val` (`gpio.c:177`). |
| **Flip** | `GPIO_enuFlipPinVal(GPIO_Port_t, GPIO_Pin_t)` | `ODR ^= (1<<pin)` (`gpio.c:220`). Not atomic. |
| **Mode** | `GPIO_enuSetPinMode(...)` | ORs `mode << (pin<<1)` into `MODER` (`gpio.c:323`). |
| **AF** | `GPIO_enuSetAltFunc(...)` | `AFRL/H << (pin<<2)` (`gpio.c:376`). |
| **OutType** | `GPIO_enuSetPinOutType(...)` | Sets `OTYPER` bit (`gpio.c:435`). |
| **Pull** | `GPIO_enuSetPinPull(...)` | `PUPDR |= pull << (pin<<1)` (`gpio.c:488`). |
| **Speed** | `GPIO_enuSetPinSpeed(...)` | `OSPEEDR |= speed << (pin<<1)` (`gpio.c:546`). |
| **Read** | `GPIO_enuReadPinVal(GPIO_Port_t, GPIO_Pin_t, uint8_t *val)` | `*val = (IDR>>pin)&1` (`gpio.c:273`). |

## 5. Usage Examples

### 5.1 LED output init + set
```c
#include "MCAL/GPIO_Driver/gpio_int.h"
#include "MCAL/RCC_Driver/rcc_int.h"

RCC_EnablePeripheralClock(RCC_AHB1_BUS, RCC_AHB1_GPIOA_CLOCK);
GPIO_cfg_t led = {
    .port = GPIO_PORT_A, .pin = GPIO_PIN_5,
    .mode = GPIO_MODE_OUTPUT,
    .outputType = GPIO_OUTPUT_TYPE_PUSH_PULL,
    .speed = GPIO_SPEED_LOW,
    .pull = GPIO_NO_PULL,
    .alternateFunction = GPIO_AF0
};
GPIO_enuInit(&led);
GPIO_enuSetPinVal(GPIO_PORT_A, GPIO_PIN_5, GPIO_HIGH);
GPIO_enuFlipPinVal(GPIO_PORT_A, GPIO_PIN_5);
uint8_t v; GPIO_enuReadPinVal(GPIO_PORT_A, GPIO_PIN_5, &v);
```

### 5.2 USART1 AF (PA9/PA10)
```c
GPIO_cfg_t tx = {.port=GPIO_PORT_A,.pin=GPIO_PIN_9,.mode=GPIO_MODE_ALTERNATE_FUNCTION,
                 .alternateFunction=GPIO_AF7,.outputType=GPIO_OUTPUT_TYPE_PUSH_PULL,
                 .speed=GPIO_SPEED_HIGH,.pull=GPIO_NO_PULL};
GPIO_cfg_t rx = {.port=GPIO_PORT_A,.pin=GPIO_PIN_10,.mode=GPIO_MODE_ALTERNATE_FUNCTION,
                 .alternateFunction=GPIO_AF7,.outputType=GPIO_OUTPUT_TYPE_PUSH_PULL,
                 .speed=GPIO_SPEED_HIGH,.pull=GPIO_PULL_UP};
GPIO_enuInit(&tx); GPIO_enuInit(&rx);
```

### 5.3 Re-configure at runtime
```c
GPIO_enuSetPinMode(GPIO_PORT_B, GPIO_PIN_6, GPIO_MODE_OUTPUT);
GPIO_enuSetPinSpeed(GPIO_PORT_B, GPIO_PIN_6, GPIO_SPEED_VERY_HIGH);
```

## 6. Dependencies
- `LIB/stdtypes.h`
- `RCC_Driver` — enable `RCC_AHB1_GPIOx_CLOCK` before `GPIO_enuInit`
- Consumed by: LED, SWITCH, SEVENSEG, LCD, HSERIAL, SPI

## 7. Design Notes & Warnings
- **`GPIO_enuInit` ORs registers** (`|=`) at `gpio.c:100` — re-init without reset accumulates bits. Call `RCC_ResetPeripheralClock()` first if re-configuring whole port.
- Only `GPIO_enuSetPinVal` is atomic (BSRR). `Flip`/`SetPinMode` etc. are read-modify-write `|=` — not ISR-safe for same port.
- Typo in enum: `GPIO_WRONG_ALTARNATIVE` (gpio_int.h:26).
- Analog mode ignores `outputType/speed/pull/AF` but driver still programs them.

## 8. Registers Map (`gpio_priv.h:426`)
`MODER 0x00` | `OTYPER 0x04` | `OSPEEDR 0x08` | `PUPDR 0x0C` | `IDR 0x10` | `ODR 0x14` | `BSRR 0x18` | `LCKR 0x1C` | `AFRL 0x20` | `AFRH 0x24`

## 9. Tests
`src/Tests/gpiotest.c`
