# MCU Driver — HAL

**Location:** `include/HAL/MCU_Driver/mcu.h` | `mcu_cfg.h` | `src/HAL/MCU_Driver/mcu.c` , `mcu_cfg.c`  
**Depends:** `MCAL/RCC_Driver` (facade) — thin wrapper translating `MCU_*` names to `RCC_*`  
**Role:** Single-entry MCU bring-up: system clock, prescalers, PLL, and all peripheral clock enables.

## 1. Overview
Board Support Package entry. `mcu_cfg.c` defines `const MCU_Config_t MCU_Configs` consumed by `MCU_enuInit(&MCU_Configs)`. Driver sequences HSI/HSE enable, PLL configure, flash latency, prescalers, sysclock switch, then bulk peripheral clock gating — mirroring the RCC typical sequence in one call.

## 2. Features

### 2.1 Config Struct (`mcu.h:231`)
```c
typedef struct {
    uint64_t MCU_AHB1_PrephralEnable; // OR of MCU_AHB1_GPIOA_CLOCK | CRC | DMA1...
    uint64_t MCU_AHB2_PrephralEnable; // OTGFS etc
    uint64_t MCU_APB1_PrephralEnable; // USART2, SPI2/3, I2C, TIM...
    uint64_t MCU_APB2_PrephralEnable; // USART1/6, SPI1/4, ADC, SYSCFG...
    MCU_ClockSrc_t MCU_SystemClockSource; // HSI/HSE/PLL
    MCU_AHPPrescaler_t MCU_AHP_Prescaler;   // ÷1..512
    MCU_APB1Prescaler_t MCU_APB1_Prescaler; // ÷1..16
    MCU_APB2Prescaler_t MCU_APB2_Prescaler; // ÷1..16
    uint32_t MCU_HSI_ClockSource; // Hz e.g. 16000000
    uint32_t MCU_HSE_ClockSource; // Hz e.g. 8000000
    uint16_t MCU_PLLN; // 50..432
    uint8_t MCU_PLLM;  // 2..63
    uint8_t MCU_PLLP;  // 2,4,6,8
    uint8_t MCU_PLLQ;  // 2..15
    uint8_t MCU_PLLClockSource; // MCU_PLL_SOURCE_HSI/HSE
} MCU_Config_t;
```

All `MCU_AHB1_*_CLOCK`, bus IDs `MCU_AHB1_BUS`, and `MCU_PLL_SOURCE_*`, `MCU_AHP/APB*` enums are `#define` aliases to `RCC_*` equivalents (`mcu.h:46`).

### 2.2 Init Sequence (`mcu.c: MCU_enuInit`)
1. Copies `HSI/HSE` freq to `RCC_HSI_ClockSourceValue / RCC_HSE_ClockSourceValue`
2. Enables selected source (HSI/HSE) + waits ready
3. If `SystemClock == PLL` → `RCC_ConfigurePLL(M,N,P,Q,src)` then `RCC_EnablePLL`
4. `RCC_SetAHBPrescaler`, `RCC_SetAPB1Prescaler`, `RCC_SetAPB2Prescaler`
5. `RCC_SetSysClock(source)` + `RCC_GetSystemClockSource` verify
6. If `peripheralEnable != NO_PERIPHERAL (0)` → `RCC_EnablePeripheralClock(bus, mask)` per bus (up to 3 masks per bus ORed).

Any step returning `!= RCC_OK` is mapped to corresponding `MCU_Status_t`.

### 2.3 Status Mapping (`mcu.h:9`)
`MCU_Status_t {OK, NOT_OK, TIMEOUT, WRONG_PLL_CONFIG, PLL_ALREADY_ENABLED, WRONG_BUS/PEREPHRAL[_WITHBUS], PLL_ERROR_M/N/P/Q/SOURCE, WRONG_SYSCLK/AHB_APB, WRONG_CONFIG, ERROR}` — 1:1 with RCC statuses.

## 3. Configuration (`mcu_cfg.c`)
```c
const MCU_Config_t MCU_Configs = {
    .MCU_SystemClockSource = MCU_SYSCLK_PLL,
    .MCU_HSI_ClockSource = 16000000, .MCU_HSE_ClockSource = 8000000,
    .MCU_PLLM = 16, .MCU_PLLN = 336, .MCU_PLLP = 4, .MCU_PLLQ = 7,
    .MCU_PLLClockSource = MCU_PLL_SOURCE_HSI,
    .MCU_AHP_Prescaler = MCU_AHB_NO_DIVISION,
    .MCU_APB1_Prescaler = MCU_APB1_DIVIDED_BY_2, // 42M
    .MCU_APB2_Prescaler = MCU_APB2_NO_DIVISION,  // 84M
    .MCU_AHB1_PrephralEnable = MCU_AHB1_GPIOA_CLOCK | MCU_AHB1_GPIOB_CLOCK | MCU_AHB1_GPIOC_CLOCK | MCU_AHB1_DMA1_CLOCK,
    .MCU_APB1_PrephralEnable = MCU_APB1_USART2_CLOCK,
    .MCU_APB2_PrephralEnable = MCU_APB2_USART1_CLOCK | MCU_APB2_SPI1_CLOCK,
    .MCU_AHB2_PrephralEnable = MCU_AHB2_NO_PERIPHERAL
};
```
Edit this table for board crystal / target frequency / needed peripherals before `MCU_enuInit` call.

## 4. API Reference
| Function | Signature | Description |
|---|---|---|
| `MCU_enuInit` | `MCU_Status_t MCU_enuInit(MCU_Config_t *cfg)` | `NULL` → `MCU_WRONG_CONFIG`; otherwise executes sequence above; returns first error or `MCU_OK`. `mcu.h:301`. |

## 5. Usage
```c
#include "HAL/MCU_Driver/mcu.h"
extern const MCU_Config_t MCU_Configs;
int main(void){
    MCU_Status_t s = MCU_enuInit((MCU_Config_t*)&MCU_Configs);
    if(s!=MCU_OK) { while(1); } // trap PLL/clock error
    // peripherals now clocked — init GPIO/LED/UART...
}
```

### 5.1 Override at runtime
```c
MCU_Config_t cfg = MCU_Configs;
cfg.MCU_AHB1_PrephralEnable |= MCU_AHB1_GPIOD_CLOCK;
MCU_enuInit(&cfg);
```

## 6. Dependencies
`RCC_Driver` (100% of work), `LIB/stdtypes`, linker flash latency side-effect inside `RCC_ConfigurePLL`.

## 7. Warnings
- `PeripheralEnable` values are 64-bit with bus-encoded high bits — use only `MCU_*_CLOCK` macros; raw bit shifts risk `WRONG_PEREPHRAL_WITHBUS`.
- Driver ORs prescaler bits (`|=`) like RCC — calling `MCU_enuInit` twice accumulates. Reset RCC or power-cycle between re-inits.
- `MCU_AHB1_NO_PERIPHERAL` sentinel is `0` — driver skips that bus if zero (no enable call).
- `PEREPHRAL` typo preserved from RCC.

## 8. Tests
No standalone `mcuTest.c` — exercised as prerequisite for all `Tests/*Test.c` suites (each `main` calls `MCU_enuInit` first).

## 9. Related
`MCAL/RCC_Driver/README.md` for detailed clock math and bus maps.
