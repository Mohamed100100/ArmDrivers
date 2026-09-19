# RCC Driver — MCAL

**Location:** `include/MCAL/RCC_Driver/rcc_int.h` | `rcc_priv.h` | `rcc_cfg.h` | `src/MCAL/RCC_Driver/rcc.c`  
**Base:** `RCC_BASE_ADDRESS 0x40023800` (`rcc_priv.h:34`) — covers full `RCC_Registers_t` map (`rcc_priv.h:884`)  
**Role:** System clock tree + peripheral clock gating/reset for STM32F401CC.

## 1. Overview
Implements the whole RCC peripheral: `CR` (HSI/HSE/PLL control), `PLLCFGR`, `CFGR` (SW/SWS/HPRE/PPRE), `CIR`, `AHB1/2RSTR+ENR`, `APB1/2RSTR+ENR`, low-power `LPENR`, `BDCR`, `CSR`, `SSCGR`, `PLLI2SCFGR`, `DCKCFGR`. Wraps hardware polling with timeout loops (`HSE_TIMEOUT 100000`, `HSI 50000`, `PLL 1000000`).

## 2. Features

### 2.1 Clock Sources
- **HSI 16 MHz** internal ±1% (`RCC_EnableHSI/IsHSIReady/DisableHSI`): trimming via `CR.HSITRIM`, calibration `HSICAL`.
- **HSE 4-26 MHz** external crystal (`RCC_EnableHSE/IsHSEReady/DisableHSE`).
- **PLL** — formula in header (`rcc_int.h:286`):  
  `VCO = (Input / PLLM) * PLLN` ; `SYSCLK = VCO / PLLP` ; `USB/SDIO = VCO / PLLQ`

### 2.2 PLL Configuration & Validation (`RCC_ConfigurePLL`, rcc.c:471)
Validates per RM:
- `PLLM 2-63`, VCO_in `1-2 MHz`
- `PLLN 192-432` (code allows 50-432, runtime check 192-432), VCO_out `192-432 MHz`
- `PLLP 2/4/6/8`, `SYSCLK ≤84 MHz`
- `PLLQ 2-15`, `USB ≤48 MHz`
- Source `RCC_PLL_SOURCE_HSI/HSE` (`rcc_int.h:23`)
- Refuses if `RCC_IsPLLReady()==1` → `RCC_PLL_ALREADY_ENABLED`
- Side-effect: sets Flash latency to 2 WS for 84 MHz at `0x40023C00` (`rcc.c:603`) — hard-coded fix.

### 2.3 System Clock Switching
- `RCC_SetSysClock(RCC_SYSCLK_HSI/HSE/PLL)` → writes `CFGR.SW`, polls `CFGR.SWS` (`rcc.c:272`).
- `RCC_GetSystemClockSource()` reads `SWS`.

### 2.4 Prescalers
| Bus | Enum | Register Field | Max Freq |
|---|---|---|---|
| AHB | `RCC_AHPPrescaler_t` (`rcc_int.h:137`) — ÷1,2,4,8,16,64,128,256,512 | `CFGR.HPRE` | 84 MHz (F401) |
| APB1 | `RCC_APB1Prescaler_t` (`rcc_int.h:157`) — ÷1,2,4,8,16 | `CFGR.PPRE1` | 42 MHz |
| APB2 | `RCC_APB2Prescaler_t` (`rcc_int.h:172`) — ÷1,2,4,8,16 | `CFGR.PPRE2` | 84 MHz |

Correctness masks `AHB_PRESCALER_CORRECTION_MASK` etc. guard invalid bits (`rcc_priv.h:55`).

### 2.5 Peripheral Clock Gating
64-bit `PeripheralClockMask` encodes bus in upper 32 bits + bit position. E.g.:
```c
RCC_AHB1_GPIOA_CLOCK = 0b0001...0001ULL  // bus AHB1, bit0
```
APIs:
- `RCC_EnablePeripheralClock(bus, mask)` → `AHB1ENR/APB...ENR |= low32(mask)`
- `RCC_DisablePeripheralClock(...)` → `&= ~low32`
- `RCC_ResetPeripheralClock(...)` → `RSTR |= mask; RSTR &= ~mask` (AHB1 correct; AHB2/APB1/APB2 only clear in current code `rcc.c:1164`)
- `RCC_ResetALLPeripheralClock()` → set+clear `0xFFFFFFFF` on all RSTRs.

Bus IDs: `RCC_AHB1_BUS 0b0001`, `AHB2 0b0010`, `APB1 0b0100`, `APB2 0b1000` (`rcc_int.h:34`).

Supported peripherals (header masks): GPIOA/B/C/D/E/H, CRC, DMA1/2, OTGFS, TIM1-5/9-11, WWDG, SPI1-4, USART1/2/6, I2C1-3, ADC1, SDIO, SYSCFG, PWR etc.

### 2.6 Error Model
`RCC_Status_t` (`rcc_int.h:189`): `RCC_OK`, `TIMEOUT`, `WRONG_PLL_CONFIG`, `PLL_ALREADY_ENABLED`, `WRONG_BUS/PEREPHRAL[_WITHBUS]`, `PLL_ERROR_M/N/P/Q/SOURCE`, `WRONG_SYSCLK/AHB/APB`.

## 3. Global Clock Variables
```c
uint32_t RCC_HSI_ClockSourceValue; // set by app, e.g. 16000000
uint32_t RCC_HSE_ClockSourceValue; // e.g. 8000000
```
Used inside `RCC_ConfigurePLL` to compute `vco_in/out` (`rcc.c:488`).

## 4. API Quick Reference
| Group | Function |
|---|---|
| HSI | `RCC_EnableHSI()`, `RCC_DisableHSI()`, `RCC_IsHSIReady()` |
| HSE | `RCC_EnableHSE()`, `RCC_DisableHSE()`, `RCC_IsHSEReady()` |
| PLL | `RCC_ConfigurePLL(M,N,P,Q,Source)`, `RCC_EnablePLL()`, `RCC_DisablePLL()`, `RCC_IsPLLReady()` |
| System | `RCC_SetSysClock(RCC_ClockSrc_t)`, `RCC_GetSystemClockSource(RCC_ClockSrc_t*)`, `RCC_SetAHBPrescaler()`, `RCC_SetAPB1Prescaler()`, `RCC_SetAPB2Prescaler()` |
| Peripheral | `RCC_EnablePeripheralClock(bus,mask)`, `RCC_DisablePeripheralClock(bus,mask)`, `RCC_ResetPeripheralClock(bus,mask)`, `RCC_ResetALLPeripheralClock()` |

## 5. Typical Bring-Up Sequence (84 MHz from 16 MHz HSI)
```c
RCC_HSI_ClockSourceValue = 16000000;
RCC_HSE_ClockSourceValue = 8000000; // if used

RCC_EnableHSI();
while(!RCC_IsHSIReady());
RCC_ConfigurePLL(16, 336, 4, 7, RCC_PLL_SOURCE_HSI); // VCO=336M, SYS=84M, USB=48M
RCC_EnablePLL();
while(!RCC_IsPLLReady());
RCC_SetAHBPrescaler(RCC_AHB_NO_DIVISION);
RCC_SetAPB1Prescaler(RCC_APB1_DIVIDED_BY_2); // 42M
RCC_SetAPB2Prescaler(RCC_APB2_NO_DIVISION); // 84M
RCC_SetSysClock(RCC_SYSCLK_PLL);

RCC_EnablePeripheralClock(RCC_AHB1_BUS, RCC_AHB1_GPIOA_CLOCK | RCC_AHB1_GPIOB_CLOCK);
RCC_EnablePeripheralClock(RCC_APB2_BUS, RCC_APB2_USART1_CLOCK);
```

External crystal variant:
```c
RCC_EnableHSE(); while(!RCC_IsHSEReady());
RCC_ConfigurePLL(8, 336, 4, 7, RCC_PLL_SOURCE_HSE); // 8M *42 /4 =84M
```

## 6. Configuration
`rcc_cfg.h` is intentionally empty — all config is via runtime API / globals. Board value is set in app (often `MCU_Driver` wraps it).

## 7. Dependencies & Consumers
- `LIB/stdtypes.h` + CMSIS base
- MCU driver (`HAL/MCU_Driver`) is a facade over RCC
- GPIO, UART, SPI, DMA, CRC all require prior `RCC_EnablePeripheralClock`

## 8. Warnings & Errata
- `RCC_ResetPeripheralClock` for AHB2/APB1/APB2 only clears reset bits (missing set) — effectively no reset (`rcc.c:1164,1182,1200`). AHB1 path is correct.
- Flash latency write hard-codes address `0x40023C00` & assumes 3.3 V / 84 MHz. Adjust for other frequencies.
- Prescaler setters use `|=` (`rcc.c:696`) not `&=~mask | val` — repeated calls accumulate bits. Clear `CFGR` first or reset.
- Status typo `PEREPHRAL`.

## 9. Tests
`src/Tests/` no dedicated RCC test; exercised via `MCU_Driver` tests and indirect peripheral clock gating.
