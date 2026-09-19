# ITI-ARM_Drivers-Playground — STM32F401CC Bare-Metal Drivers (HAL / MCAL / OS / LIB)

> **ECU firmware layer for STM32F401CC (Cortex-M4, 84 MHz, 256 KB Flash, 64 KB SRAM)** — BlackPill + Kit shield. Built with **PlatformIO + CMSIS + Custom Linker Script**. Authored by **Eng.Gemy / Bootloader Team / ITI Intake46**.

[![Platform](https://img.shields.io/badge/platform-ststm32-blue)]() [![Board](https://img.shields.io/badge/board-genericSTM32F401CC-green)]() [![Framework](https://img.shields.io/badge/framework-CMSIS-orange)]() [![MCU](https://img.shields.io/badge/MCU-STM32F401CC-red)]() [![Lang](https://img.shields.io/badge/lang-C99-lightgrey)]()

---

## Table of Contents
- [1. Project Overview](#1-project-overview)
- [2. Hardware & Toolchain](#2-hardware--toolchain)
- [3. Repository Structure](#3-repository-structure)
- [4. Memory Layout & Linker Script](#4-memory-layout--linker-script)
- [5. Architecture Layers](#5-architecture-layers)
- [6. Clock Tree & BSP (MCU / RCC)](#6-clock-tree--bsp-mcu--rcc)
- [7. Drivers — Complete Catalog](#7-drivers--complete-catalog)
  - [7.1 LIB](#71-lib--stdtypesh)
  - [7.2 MCAL](#72-mcal)
  - [7.3 HAL](#73-hal)
  - [7.4 OS](#74-os--scheduleh)
- [8. Quick Start & Bring-Up Sequence](#8-quick-start--bring-up-sequence)
- [9. Build / Upload / Debug](#9-build--upload--debug)
- [10. Tests (`src/Tests`)](#10-tests-srctests)
- [11. Per-Driver READMEs (Deep Dives)](#11-per-driver-readmes-deep-dives)
- [12. Usage Examples (Sync / Async / DMA)](#12-usage-examples-sync--async--dma)
- [13. Configuration Files](#13-configuration-files)
- [14. Dependencies Between Drivers](#14-dependencies-between-drivers)
- [15. Known Issues & Warnings](#15-known-issues--warnings)
- [16. Troubleshooting](#16-troubleshooting)
- [17. Roadmap](#17-roadmap)
- [18. Contributing & Style](#18-contributing--style)
- [19. License & Credits](#19-license--credits)

---

## 1. Project Overview

ArmDrivers is a **from-scratch bare-metal driver stack** for **STM32F401CC** — no HAL, no SPL, no STM32Cube — every register is hand-mapped and wrapped in a type-safe, validated API. It demonstrates:

- **9 MCAL drivers:** GPIO, RCC, NVIC, SysTick, UART (USART1/2/6), SPI (1-4), DMA (1/2), FLASH, CRC
- **6 HAL drivers:** LED, SWITCH, 7-SEG, LCD HD44780 (sync+async), HSERIAL (UART/SPI mux), MCU (single-struct BSP)
- **1 OS primitive:** Cooperative periodic scheduler on SysTick
- **1 LIB:** `stdtypes.h` (`uint8-64_t`, `bool_t`, `NULL`) — foundation for every header
- **12 tests** in `src/Tests/` + `test.h` entry dispatcher
- **Custom linker** `LinkerScript/CustomLinkerScript.ld:10` with `LED_CFG` RAM/FLASH sections (`0x20000800`/`0x08008000`)
- **PlatformIO** `platformio.ini:11` env `genericSTM32F401CC`, `ststm32`, `framework=cmsis`, ST-Link, `-O0` debug, `cppcheck`

**Design goals:** type safety (enums vs raw ints), exhaustive parameter validation (mask checks), detailed `Status_t` returns, zero dynamic allocation, MISRA-friendly unions for registers, and scheduler-friendly async paths.

---

## 2. Hardware & Toolchain

| Item | Value |
|------|-------|
| **MCU** | STM32F401CCU6 — Cortex-M4F, 84 MHz, 256 KB Flash (`0x08000000`), 64 KB SRAM (`0x20000000`), 64-pin LQFP |
| **Boards** | WeAct BlackPill (PC13 inverted LED) + Kit shield (8 LEDs, switches, 7-seg, 16×2 LCD, UART/SPI headers) |
| **Core peripherals** | GPIOA/B/C/D/E/H, RCC, NVIC (84 IRQs), SysTick 24-bit, USART1/2/6, SPI1-4, DMA1/2 (8 streams ×8 ch), FLASH controller `0x40023C00`, CRC `0x40023000` |
| **Toolchain** | `arm-none-eabi-gcc` via PlatformIO `platform = ststm32`, CMSIS headers, `arm-none-eabi-gdb` |
| **Upload/Debug** | ST-Link (`upload_protocol=stlink`, `debug_tool=stlink`) — SWD, `board_build.ldscript = LinkerScript/CustomLinkerScript.ld` |
| **Host** | Linux (validated at `/media/gemy/Gemy/ITI_Intake46/ARM_Workspace/ArmDrivers`) — `platformio.ini:18` uses `-O0 -ggdb3` |

---

## 3. Repository Structure

```
ITI-ARM_Drivers-Playground/
├── platformio.ini                 # env:genericSTM32F401CC, ststm32, cmsis, stlink, CustomLinkerScript.ld
├── LinkerScript/
│   └── CustomLinkerScript.ld      # RAM 64K @0x20000000, FLASH 256K @0x08000000 + LED_CFG sections
├── include/
│   ├── LIB/
│   │   └── stdtypes.h             # uint8-64_t, sint*, float*, bool_t, NULL  → see LIB/README.md
│   ├── MCAL/
│   │   ├── GPIO_Driver/           # gpio_int.h, gpio_priv.h, gpio_cfg.h
│   │   ├── RCC_Driver/            # rcc_int.h, rcc_priv.h, rcc_cfg.h
│   │   ├── NVIC_Driver/           # nvic.h, nvic_priv.h, nvic_stm32f401cc.h
│   │   ├── SYSTICK_TIMER_Driver/  # systick.h, systick_priv.h
│   │   ├── UART_Driver/           # uart.h, uart_priv.h
│   │   ├── SPI_Driver/            # spi.h, spi_priv.h
│   │   ├── DMA_Driver/            # dma.h, dma_priv.h
│   │   ├── FLASH_Driver/          # flash.h, flash_priv.h
│   │   └── CRC_Driver/            # crc.h, crc_priv.h
│   ├── HAL/
│   │   ├── LED_Driver/            # led.h, led_cfg.h
│   │   ├── SWITCH_Driver/         # switch.h, switch_cfg.h
│   │   ├── SEVENSEG_Driver/       # sevenseg.h, sevenseg_cfg.h
│   │   ├── LCD_Driver/            # lcd.h, lcd_queue.h
│   │   ├── HSERIAL_Driver/        # hserial.h, hserial_cfg.h
│   │   └── MCU_Driver/            # mcu.h, mcu_cfg.h
│   ├── OS/
│   │   ├── schedule.h             # cooperative scheduler
│   │   └── schedule_cfg.h
│   ├── test.h                     # dispatcher for src/Tests/*
│   └── README                     # (legacy PlatformIO template)
├── src/
│   ├── main.c                     # entry, currently calls test_SPI_PollingTransmitReceive()
│   ├── MCAL/                      # gpio.c, rcc.c, nvic.c, systick.c, uart.c, spi.c, dma.c, flash.c, crc.c
│   ├── HAL/                       # led.c/.cfg.c, switch.c/.cfg.c, sevenseg.c/.cfg.c, lcd.c/queue.c/.cfg.c, hserial.c/.cfg.c, mcu.c/.cfg.c
│   ├── OS/                        # schedule.c
│   └── Tests/                     # 12 tests: gpiotest.c, nvicTest.c, uartTest.c, testSpi.c, dmaTest.c, flashTest.c, sevsegTest.c, switchTest.c, testLcd.c, asychLcdTest.c, testHserial.c, SectionAdd_ld.c
├── lib/                           # (empty, reserved)
├── .vscode/                       # c_cpp_properties, launch.json, settings
├── .pio/build/genericSTM32F401CC/ # build artifacts (firmware.elf/bin/hex)
├── firmware.elf / .bin / .hex / app.bin
└── README.md                      # ← this file (home main README)
```

**Generated detailed per-driver docs:** each `include/*/*/README.md` (16 files + root) — see §11.

---

## 4. Memory Layout & Linker Script

`LinkerScript/CustomLinkerScript.ld:10` — `ENTRY(Reset_Handler)`, `_estack = ORIGIN(RAM)+LENGTH(RAM)`, `_Min_Heap_Size 0x200`, `_Min_Stack_Size 0x400`.

| Region | Attr | Origin | Length | Content |
|--------|------|--------|--------|---------|
| `RAM` | `xrw` | `0x20000000` | 64 K | `.data` (copied from Flash), `.bss`, heap ↑, stack ↓ from `_estack` |
| `FLASH` | `rx` | `0x08000000` | 256 K | `.isr_vector` (must be first), `.text`, `.rodata`, `.ARM.*`, `.preinit/init/fini_array`, `_sidata` |
| `LED_CFG_RAM` | `xrw` | `0x20000800` | 10 | `.led_cfg_ram_section (NOLOAD)` |
| `LED_CFG_FLASH` | `xrw` | `0x08008000` | 10 | `.led_cfg_flash_section` |

**Sections flow:** `.isr_vector` → `.text` → `.rodata` → `.ARM.*` → `.preinit/init/fini_array` → `_sidata = LOADADDR(.data)` → `.data >RAM AT>FLASH` (`_sdata.._edata`) → `.bss` (`_sbss.._ebss`) → `.led_cfg_flash_section` → `._user_heap_stack` (reserve `0x200+0x400`, `end/_end`) → `.led_cfg_ram_section (NOLOAD)` → `/DISCARD/ libc/libm/libgcc`.

**Startup (CMSIS `startup_stm32f401xc.s`):** load SP from `0x08000000`, jump `Reset_Handler` → copy `.data` from `_sidata`→`_sdata`..`_edata` → zero `.bss` `_sbss`..`_ebss` → call `.preinit_array` → `.init_array` (C++ ctors) → `main()` → on return `.fini_array`.

> Why `LED_CFG` split? `0x20000800` NOLOAD lets `LED_Configs` live in RAM (mutable at runtime) while flash copy at `0x08008000` seeds initial values — `SectionAdd_ld.c` test validates.

---

## 5. Architecture Layers

```
┌─────────────────────────────────────────────────────┐
│  App : main.c / src/Tests/*  (test_*, Example_*)   │
├─────────────────────────────────────────────────────┤
│  HAL : LED / SWITCH / SEVENSEG / LCD / HSERIAL / MCU│  ← config tables in *_cfg.c, enums for names
├─────────────────────────────────────────────────────┤
│  OS  : schedule (SCHED_Runnable_t, tick 1ms)        │  ← SysTick ISR → tick → dispatch loop
├─────────────────────────────────────────────────────┤
│ MCAL : GPIO / RCC / NVIC / SysTick / UART / SPI / DMA / FLASH / CRC │
│        └─ gpio_priv.h unions map regs precisely      │
├─────────────────────────────────────────────────────┤
│  LIB : stdtypes.h (uint*_t, bool_t)                 │
├─────────────────────────────────────────────────────┤
│ CMSIS : core_cm4.h, system_stm32f4xx.c, startup.s  │
└─────────────────────────────────────────────────────┘
  LinkerScript        PlatformIO (genericSTM32F401CC)
```

**Rules:** MCAL never includes HAL; HAL aggregates MCAL (e.g., LED→GPIO+RCC); OS only depends on SysTick+LIB; MCU wraps RCC for one-call init.

---

## 6. Clock Tree & BSP (MCU / RCC)

**RCC base** `0x40023800` (`rcc_priv.h:34`) — registers `CR, PLLCFGR, CFGR, CIR, AHB1/2RSTR+ENR, APB1/2RSTR+ENR, LPENR, BDCR, CSR, SSCGR, PLLI2SCFGR, DCKCFGR` (`RCC_Registers_t:884`).

**Sources:** HSI 16 MHz (internal, ±1%, trimmed via `CR.HSITRIM`), HSE 4-26 MHz crystal, PLL.

**PLL math** (`rcc_int.h:286`): `VCO = (Input / PLLM) * PLLN`; `SYSCLK = VCO / PLLP`; `USB = VCO / PLLQ`

**Validation in `RCC_ConfigurePLL` (`rcc.c:471`):** `PLLM 2-63` → `VCO_in 1-2 MHz`, `PLLN 192-432` → `VCO_out 192-432 MHz`, `PLLP {2,4,6,8}` → `SYSCLK ≤84 MHz`, `PLLQ 2-15` → `USB ≤48 MHz`, source `HSI/HSE`, refuse if `PLL_RDY` → `PLL_ALREADY_ENABLED`, sets Flash latency 2 WS at `0x40023C00`.

**Prescalers:**

| Bus | Enum | Field | Max |
|-----|------|-------|-----|
| AHB | `RCC_AHPPrescaler_t` ÷1,2,4,8,16,64,128,256,512 | `CFGR.HPRE` | 84 MHz |
| APB1 | `RCC_APB1Prescaler_t` ÷1,2,4,8,16 | `CFGR.PPRE1` | 42 MHz |
| APB2 | same | `CFGR.PPRE2` | 84 MHz |

**Peripheral gating:** 64-bit mask `[busID:32][bit:32]` e.g. `RCC_AHB1_GPIOA_CLOCK 0b0001…0001ULL`. APIs: `RCC_EnablePeripheralClock(bus,mask)` → `AHB1ENR |= low32`, `Disable`, `ResetPeripheralClock` (`RSTR|=mask; RSTR&=~mask` for AHB1, else only clear — errata), `ResetALLPeripheralClock` (set+clear `0xFFFFFFFF`).

**MCU BSP** `mcu.h:231` (`MCU_Config_t` with `AHB1/2/APB1/2_PrephralEnable` 64-bit masks + `SystemClockSource, AHPPrescaler, APB1/2, HSI/HSE freq, PLLM/N/P/Q, PLL source`). `MCU_enuInit` (`mcu.c`) sequences: copy freq globals → enable HSI/HSE → if PLL → `ConfigurePLL`+`EnablePLL` → `SetAHB/APB1/APB2` → `SetSysClock` → enable per bus if `!=0`. Maps `RCC_Status_t` → `MCU_Status_t`.

**Typical preset** (`mcu_cfg.c`): `HSI 16M, PLLM16, PLLN336, PLLP4 → 84 MHz`, `AHB÷1, APB1÷2→42M, APB2÷1, PLLQ7→48M USB`, `GPIOA/B/C+DMA1 + USART1/2` enabled.

---

## 7. Drivers — Complete Catalog

### 7.1 LIB — `stdtypes.h`

| Typedef | Base |
|---------|------|
| `uint8_t/16/32/64_t` | `unsigned char/short int/int/long long` |
| `sint8/16/32/64_t` | signed variants |
| `float32/64_t` | `float/double` |
| `bool_t {FALSE=0, TRUE=1}` | enum |
| `NULL ((void*)0)` | guarded |

Leaf — included by all headers. No `<stdint.h>` to stay freestanding.

---

### 7.2 MCAL

#### GPIO — `gpio_int.h` / `gpio.c` / `gpio_priv.h`

- **Ports:** `GPIO_Port_t {A,B,C,D,E,H}` bases `0x40020000..0x40021C00`
- **Pins:** `GPIO_Pin_t 0..15`
- **Modes** `GPIO_Mode_t {INPUT, OUTPUT, ALTERNATE_FUNCTION, ANALOG}`
- **Types** `GPIO_OutputType_t {PUSH_PULL, OPEN_DRAIN}`
- **Speed** `GPIO_Speed_t {LOW(8M), MEDIUM(50M), HIGH(100M), VERY_HIGH(180M)}`
- **Pull** `GPIO_Pull_t {NO_PULL, PULL_UP, PULL_DOWN}`
- **AF** `GPIO_AlternateFunction_t AF0..15` → auto `AFRL` (0-7) / `AFRH` (8-15)
- **Config** `GPIO_cfg_t {port,pin,mode,outputType,speed,pull,alternateFunction}`
- **Status** 11 codes `GPIO_OK, NULL_PTR, WRONG_MODE/PORT/PIN/OUTPUT_TYPE/PULL/ALTARNATIVE/SPEED/VALUE`

**Regs** `gpio_priv.h:426` `MODER 0x00, OTYPER 0x04, OSPEEDR 0x08, PUPDR 0x0C, IDR 0x10, ODR 0x14, BSRR 0x18, LCKR 0x1C, AFRL 0x20, AFRH 0x24` — unions per bit.

**APIs:** `GPIO_enuInit(cfg*)` (ORs `MODER/OTYPER/PUPDR/OSPEEDR/AFRx`), `GPIO_enuSetPinVal(port,pin, GPIO_HIGH=0/LOW=16)` via `BSRR ((1<<pin)<<val)`, `GPIO_enuFlipPinVal` (`ODR^=1<<pin`), `GPIO_enuSetPinMode/AltFunc/OutType/Pull/Speed`, `GPIO_enuReadPinVal(port,pin,uint8_t*)`.

> Note `GPIO_enuInit` ORs — reset port first if re-init.

#### RCC — see §6

**Extra enums:** `RCC_ClockSrc_t {HSI,HSE,PLL}`, bus IDs `AHB1 0b0001, AHB2 0010, APB1 0100, APB2 1000`, masks `RCC_AHB1_GPIOA_CLOCK … DMA2_CLOCK, AHB2_OTGFS, APB1/TIM2..PWR, APB2/TIM1..TIM11`.

**APIs:** `RCC_EnableHSI/IsHSIReady/DisableHSI`, `EnableHSE/IsHSEReady/DisableHSE`, `ConfigurePLL(M,N,P,Q,Source)`, `EnablePLL/DisablePLL/IsPLLReady`, `SetSysClock/GetSystemClockSource`, `SetAHB/APB1/APB2Prescaler`, `Enable/Disable/ResetPeripheralClock(bus,mask)`, `ResetALLPeripheralClock`.

#### NVIC — `nvic.h`

- **IRQs** `NVIC_IRQ_t 0..239` (`NVIC_IRQ37 = USART1`)
- **Status** `NVIC_Status_t {NOT_OK, OK, NULL_PTR}`, `Pending {NOT_PENDING,PENDING}`, `Active {NOT_ACTIVE,ACTIVE}`
- **Priority** 0 highest; F401 implements 4 bits (values top-nibbled `0x00..0xF0`)
- **Grouping** `NVIC_SetPriorityGrouping` → `SCB->AIRCR.PRIGROUP` (0x00→4 sub / 0x07→4 group)

**APIs:** `NVIC_EnableIRQ`, `DisableIRQ`, `GetPendingIRQ(IRQ, Pending*)`, `SetPendingIRQ`, `ClearPendingIRQ`, `GetActiveIRQ`, `SetPriority(IRQ,prio)`, `GetPriority`, `SystemReset` (`AIRCR.SYSRESETREQ VECTKEY 0x05FA`), `SetPriorityGrouping`.

**Regs:** `ISER/ICER/ISPR/ICPR/IABR[0..7], IPR[0..59], SCB_AIRCR`.

#### SysTick — `systick.h`

- **Prescaler** `SYSTICK_NO_PRESCALLER 0b100` (÷1) / `PRESCALLER_8 0b000` (÷8)
- **Status** `SYSTICK_Status_t {NOT_OK,OK,WRONG_PRESCALLER/STARTVALUE,OFF,EXCEPTION_OFF,ZERO_STARTVALUE,NULL_PTR}`
- **Cfg** `SYSTICK_Callback_t void(*)(void)`

**APIs:** `SYSTICK_Init(ClockValue, Prescaler)`, `SetCallBack(cb)`, `SetStartValue(uint32_t 0..0xFFFFFF)`, `StartCount()`, `StopCount()`, `Wait_ms(ms)` (blocking ticks calc), `GetCurrentCount(uint32_t*)`.

**Regs:** `CTRL 0xE000E010, LOAD 0xE000E014, VAL 0xE000E018, CALIB 0xE000E01C`.

#### UART — `uart.h`

- **Instances** `UART_Number_t {UART_1 (APB2 0x40011000), UART_2 (APB1), UART_6 (APB2)}`
- **Enables** `UART_ENABLE_TRANSMITE 0b1000 (TE), ENABLE_RECEIVE 0b0100 (RE)`
- **Cfg** `UART_Config_t {PeripheralClock, UART_Number, BaudRate, Parity {NONE/EVEN/ODD}, OverSampling {16/8}, StopBits {1,0.5,2,1.5}, WordLength {8B/9B}, Sample {THREE/ONE}, UartEnabled mask, InterruptFlags}`
- **Interrupts** `UART_INTERRUPT_TXE(0 at init), ERROR, RXNE, TC, PE` — TXE disabled at init to avoid storm
- **Flags** `TXE/TC/RXNE/ORE/NOISE/FE/PE`
- **DMA** `UART_DMA_TRANSMIT/RECEIVE_ENABLE` → `CR3.DMAT/DMAR`
- **Buffers** `UART_AsynBuffer_t {callback,size,index,buffer}`, `UART_Callbacks_t {PE,FE,NE,ORE,TC}`

**APIs:** `UART_enuInit(cfg*)`, `SynTransmitBuffer/ReceiveBuffer(uart,buf,size)` (poll TXE/RXNE), `AsynTransmit/Receive(uart, AsynBuffer*)` (TXE/ TC, RXNE), `ActivateDMA(uart, flag)`, `u8ReadTXE/TC/RXNE/ORE/Noise/FE/PEFlag(uart)`, `Enable/DisableInterrupts(uart,flags)`, `ClearFlags`, `RegisterCallbacks(uart,Callbacks*)`.

**Status:** 14 codes `OK, WRONG_UART_NUMBER/ENABLE/PARITY/OVERSAMPLING/STOPBITS/WORDLENGTH/SAMPLE/INTERRUPT_FLAGS, NOT_INIT, GPIO_ERROR, TX_BUSY, WRONG_DMA_ENABLE`.

#### SPI — `spi.h`

- **Instances** `SPI1 APB2, SPI2/3 APB1, SPI4 APB2`
- **Comm** `FULL_DUPLEX, HALF_DUPLEX_2LINES_TX/RX_ONLY, 1LINE_RX/TX_ONLY` (`CR1 BIDIMODE/RXONLY/BIDIOE`)
- **Mode** `MASTER/SLAVE` (`MSTR`), **CRC** `DISABLED/ENABLED + polynomial`, **Length** `8/16b`, **Order** `MSB/LSB`, **Baud** `DIV2..256`, **CPOL/CPHA** `4 combos`, **Frame** `MOTOROLA/TI`, **DMA** `DISABLE/TX/RX/TX_RX`, **NSS** `MASTER_HW_OUTPUT (0x0004), MASTER_HW_INPUT, MASTER_SW (0x0300, 8 slaves), SLAVE_HW, SLAVE_SW`

**Multi-slave:** `SPI_SlaveCS_t {port,pin}`, `SPI_SlavesConfig_t {slaves[8], numberOfSlaves}` → GPIO CS via `MasterSelect/DeselectSlave`.

**Flags** `RXNE,TXE,UNDERRUN,CRCERR,MODF,OVR,BSY,FRE` + `SPI_u8ReadFlag`, `ClearFlag`, `RegisterCallback`, `Enable/DisableInterrupt`.

**APIs:** `SPI_enuInit(SPI_Config_t*)`, `MasterSyncTransmitReceive/SyncTransmit/SyncReceive(Tx, Rx*)`, `MasterAsynTransmit/Receive(Tx, callback)`, `MasterSelect/DeselectSlave(slave)`, `SlaveSync/AsynTransmit/Receive`, `ClearFlag/RegisterCallback`, `Enable/DisableInterrupt`, `u8ReadFlag`.

#### DMA — `dma.h`

- **Matrix:** `DMA_Controller_t {DMA1,DMA2}`, `Stream 0..7`, `Channel 0..7 (CHSEL)`
- **Cfg enums:** `Direction P2M/M2P/M2M`, `MINC/PINC FIXED/AUTO`, `MSIZE/PSIZE BYTE/HALFWORD/WORD`, `Priority LOW..VERY_HIGH`, `Circular DISABLE/ENABLE`, `MBurst/PBurst SINGLE/INCR4/8/16`, `DoubleBuffer DISABLE/ENABLE (CT)`, `FlowCtrl DMA/PERIPHERAL`, `Mode DIRECT/FIFO`, `FifoThreshold QUARTER/HALF/3Q/FULL`
- **Interrupts** masks `TC 0x10, HT 0x08, TE 0x04, DME 0x02, FE 0x80` → `CR/FCR`

**Config** `DMA_Config_t {DMAx,Streamx,Channel,Direction,PeripheralFlowCtrl,Mode,Priority,MSize,PSize,MemoryInc,PeripheralInc,CircularMode,MBurst,PBurst,DoubleBuffer,FifoThreshold,PeripheralAddress,Memory0/1Address,Interrupts,NumberOfData}`.

**APIs:** `DMA_enuInit(Config*)`, `StartTransfer(CTRL,Stream)`, `StopTransfer`, `SetMemoryAddress`, `SetNumberOfData`, `RegisterCallback(CTRL,Stream,Interrupt,cb)`, `u8ReadFlag`, `ClearFlag` — ISRs `DMAx_Streamy_IRQHandler` dispatch callbacks.

#### FLASH — `flash.h`

- **Sectors:** `0 16K 0x08000000, 1 16K, 2 16K, 3 16K, 4 64K 0x08010000, 5 128K 0x08020000, 6 128K, 7 128K`
- **Sizes** `BYTE 8b (1.8V)/HALFWORD 16b (2.1V)/WORD 32b (2.7V)/DOUBLEWORD 64b+VPP` → `CR.PSIZE`

**APIs:** `Flash_Unlock()` (KEYR `0x45670123/0xCDEF89AB`), `Lock()`, `EraseSector(Sector)` (≈250ms-1s), `MassErase()` (irreversible), `ProgramByte/HalfWord/Word/DoubleWord(addr,data)` (align check), `ProgramBuffer(addr, *data, len)`, `ClearErrors()`, `GetError()`, `IsBusy()`, `IsLocked()`, `GetSectorFromAddress/GetSectorBaseAddress/GetSectorSize`.

**Status** 12 `FLASH_OK, BUSY, WRITE_PROTECTED, ALIGNMENT, PARALLELISM, SEQUENCE, READ_PROTECTED, OPERATION, TIMEOUT, INVALID_SECTOR/ADDRESS, LOCKED`.

#### CRC — `crc.h`

- **Poly** `0x4C11DB7` Ethernet, init `0xFFFFFFFF`, 4 cycles/word, unit `0x40023000` (`DR, IDR 8b, CR.RESET`)

**APIs:** `CRC_Reset()` (`CR.RESET→DR=0xFFFFFFFF`), `Accumulate(uint32_t)`, `Accumulate_Array(ptr,len)`, `Calculate(ptr,len)` (reset+array), `Calculate/Accumulate_Bytes(uint8_t,len)` (packs LE, pads 0x00), `GetValue()`, `SetIDR/GetIDR()`, `Calculate/AccumulateMemoryRegion(start,len)` (word-read Flash without copy).

> Enable clock manually `RCC_EnablePeripheralClock(RCC_AHB1_BUS, RCC_AHB1_CRC_CLOCK)`.

---

### 7.3 HAL

#### LED — `led.h` / `led_cfg.h`

- **Names** `BLACK_PILL_LED (PC13 inverted), KIT_LED_1..8, LED_LEN=9`
- **Types** `LED_Port_t A/H`, `PIN 0..15`, `ActiveState {LOW (PC13), HIGH}`, `OutputType PUSH_PULL/OPEN_DRAIN`
- **Config** `LED_cfg_t {port,pin,activeState,outputType}` array `LED_Configs[LED_LEN]` in `led_cfg.c`
- **APIs:** `LED_vdInit()` (RCC+`GPIO_enuInit` per LED, OFF initial), `LED_vdTurnON(name)` (respects activeState `HIGH→GPIO_HIGH else LOW`), `LED_vdTurnOFF`, `LED_vdToggle` (flip).

Status aligned to GPIO codes + `WRONG_LED_NAME`.

#### SWITCH — `switch.h` / `switch_cfg.h`

- **Names** user-defined `SWITCH_Name_t` (e.g., `KIT_SW_1`) + `SWITCH_LEN`
- **Ports/Pins** `SWITCH_PORT_A..H`, `PIN 0..15`
- **Connection** `INTERNAL_PULLUP 0b0001, PULLDOWN 0010, EXTERNAL_PULLUP 0100/PULLDOWN 1000`
- **State** `PUSHED/RELEASED` — pull-up `LOW→PUSHED`

**APIs:** `SWITCH_enuInit()` (GPIO `INPUT`+pull), `SWITCH_enuReadVAl(name, SWITCH_State_t*)` (debounced, typo preserved). Status `WRONG_NAME, ERROR_READ, ERROR_SCHEDUALE`.

#### SEVENSEG — `sevenseg.h`

- **Map** `  AAA / F B / GGG / E C / DDD `, `segMap[10] {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F}` bits `0=A..6=G`
- **Cfg** `SEVSEG_Pinout_t {port,pin}`, `SEVSEG_cfg_t {PinA..G, activeState HIGH/LOW, outputType}`

**APIs:** `SEVSEG_enuInit()` (7 GPIOs OUTPUT, blank), `SEVSEG_enuDisplayValue(uint8_t 0..9)` (validates, loops `GPIO_enuSetPinVal` per bit XOR activeState). Multi-digit multiplex via scheduler.

#### LCD — `lcd.h` / `lcd_queue.h`

- **Controller** HD44780 16×2, **Modes** `8_BIT 11 pins DB0-7+RS/RW/EN` / `4_BIT 7 pins DB4-7+RS/RW/EN` (byte as `HIGH_NIBBLE 4` + `LOW_NIBBLE 0`)
- **Config** `LCD_Config_t {BitOperation, LineDisplay 1/2, FontSize 5x7/5x10, Display ON/OFF, Cursor ON/OFF, Blink ON/OFF, Increment INCREMENT/DECREMENT, DisplayShift NO_SHIFT/AUTO_SHIFT}`
- **Pins** `LCD_Port_t/Pin_t`, `PinInfo {port,pin}`, unions `Pinout_8BitMode_t`/`4BitMode_t`
- **Sync vs Async:** Sync blocks 2 ms/cmd, 50 ms init; Async queue (`lcd_queue.h`) via `SCHED` tick, callback `LCD_Callback_t(status)` with `INIT_SUCEESSFULLY/WRITE_SUCCESSFULLY/CREATE_CUSTOM_CHAR/GPIO_ERROR`

**Sync APIs:** `LCD_enuSynInit`, `SyncWriteCharacter(c)`, `SyncClearDisplay`, `SyncSetDisplay/RoturnHome/SetCursor/SetBlink/SetCursorPosition(row 0-1,col0-15)/SetIncrementDecrement/DisplayShift/SetFontSize/SetLineDisplay`, `SyncCreateCustomChar(loc0-7,map[8])`, `SyncWriteCustomChar(loc)`.

**Async APIs:** `LCD_enuAsynInit`, `AsynWriteCharacter/WriteString/WriteStringAtPosition(str,row,col)`, `AsynCreateCustomChar`, `AsynDisplayCustomChar(+AtPosition)`, `LCD_vdAsyncRegisterCallback(cb)`.

#### HSERIAL — `hserial.h` / `hserial_cfg.h`

- **Modes** `HSERIAL_MODE_UART_SYNC/ASYNC/DMA, SPI_SYNC/ASYNC/DMA`
- **Channel** `HSERIAL_Channel_t {BOOTLOADER_HSERIAL_CHANNEL=0, ... HSERIAL_CHANNEL_LENGTH}` (currently 1)
- **Union config** `HSERIAL_Config_t {Mode, union {H_UART_Sync/Async/Dma_Config, H_SPI_Sync/Async_Config}}` — fields mirror UART/SPI but `HSERIAL_Uart_*` types, priority `0x00..0xF0`

**APIs:** `HSERIAL_enuInit()` (loops channels → `UART/SPI_Init` + NVIC priority + DMA), `HSERIAL_enuTransmitBuffer(ch, buf, size)`, `HSERIAL_enuReceiveBuffer(ch, buf, size)` — dispatches by mode. Async callbacks from `H_UART_Async_Config.{Tx,Rx}CompleteCallback`.

#### MCU — `mcu.h` / `mcu_cfg.h`

- **Alias defines:** `MCU_AHB1_GPIOA_CLOCK = RCC_AHB1_GPIOA_CLOCK` etc., `MCU_AHB1_BUS`, `MCU_PLL_SOURCE_HSI`
- **Config** `MCU_Config_t` (see §6) — single struct `MCU_Configs` in `mcu_cfg.c`
- **API:** `MCU_Status_t MCU_enuInit(MCU_Config_t*)` — validates null → `WRONG_CONFIG`, else full sequence (§6). Maps `RCC_*` statuses to `MCU_*`.

---

### 7.4 OS — `schedule.h`

- **Runnable** `SCHED_Runnable_t {CBF void(*)(void*), Periodicity_ms, FirstDalay_ms (typo), Args void*, Priority}` — priority high → earlier in ready sort
- **Cfg** `SCHED_MAX_RUNNABLES` (schedule_cfg.h, ~10)

**APIs:** `SCHED_enuInit(sysclk_Hz, tick_ms)` (SysTick calc `LOAD = sysclk/presc/1000*tick_ms -1`), `SCHED_enuRegisterRunnable(ptr)` (`NULL→NULL_PTR`, dup→`ERROR_RUNNABLE_STORED_BEFORE`), `SCHED_enuRemoveRunnable(ptr)`, `SCHED_enuStart()` infinite loop `{ tick≥firstDelay && (tick-firstDelay)%period==0 → ready; sort by Priority desc; execute }` — never returns; uses `WFI` between ticks.

> Cooperative: runnables must be short < tick; no preemption; periods must be multiples of tick_ms.

---

## 8. Quick Start & Bring-Up Sequence

**Prereq:** ST-Link driver, `arm-none-eabi-gcc`, `PlatformIO CLI` (`pip install platformio`).

```c
// 1. MCU clocks + peripherals
#include "HAL/MCU_Driver/mcu.h"
extern const MCU_Config_t MCU_Configs; // mcu_cfg.c — edit target freq/ports
MCU_enuInit((MCU_Config_t*)&MCU_Configs); // 84M PLL, GPIOA/B/C, DMA1, USART1/2

// 2. HAL
#include "HAL/LED_Driver/led.h"
#include "HAL/SWITCH_Driver/switch.h"
LED_vdInit(); SWITCH_enuInit();

// 3. Scheduler + runnables
#include "OS/schedule.h"
void Blink(void* a){ LED_vdToggle(BLACK_PILL_LED); }
void Poll(void* a){ SWITCH_State_t s; SWITCH_enuReadVAl(0,&s); }

SCHED_Runnable_t rBlink={.CBF=Blink,.Periodicity_ms=500,.FirstDalay_ms=0,.Args=NULL,.Priority=1};
SCHED_Runnable_t rPoll={.CBF=Poll,  .Periodicity_ms=10, .FirstDalay_ms=5,  .Args=NULL,.Priority=2};

SCHED_enuInit(84000000, 1);
SCHED_enuRegisterRunnable(&rBlink);
SCHED_enuRegisterRunnable(&rPoll);
SCHED_enuStart(); // cooperative forever
```

**Async LCD example:**
```c
void LcdCb(LCD_Status_t st){ if(st==LCD_INIT_SUCEESSFULLY) LCD_enuAsynWriteString((uint8_t*)"Ready"); }
LCD_vdAsyncRegisterCallback(LcdCb);
LCD_enuAsynInit(); // needs SCHED running
LCD_enuAsynWriteStringAtPosition((uint8_t*)"Temp:25C",1,0);
```

**UART HSERIAL:**
```c
#include "HAL/HSERIAL_Driver/hserial.h"
HSERIAL_enuInit();
uint8_t msg[]="Hello DMA";
HSERIAL_enuTransmitBuffer(BOOTLOADER_HSERIAL_CHANNEL, msg, sizeof(msg)-1);
```

**SPI master (SW NSS, 2 slaves):**
```c
SPI_SlaveCS_t cs1={SPI_PORT_A,SPI_PIN_4}, cs2={SPI_PORT_A,SPI_PIN_5};
SPI_Config_t cfg={.spiNumber=SPI1,.mode=SPI_MASTER,.nssManagement=SPI_NSS_MASTER_SW,
  .communicationMode=SPI_FULL_DUPLEX,.baudRate=SPI_BAUDRATE_DIV8,.polarityPhase=SPI_ZERO_IDLE_FIRST_EDGE,
  .slavesConfig={.slaves={cs1,cs2},.numberOfSlaves=2}};
SPI_enuInit(&cfg);
SPI_enuMasterSelectSlave(SPI1,cs1);
uint16_t rx; SPI_enuMasterSyncTransmitReceive(SPI1,0x55,&rx);
SPI_enuMasterDeselectSlave(SPI1,cs1);
```

---

## 9. Build / Upload / Debug

```bash
# build
pio run -e genericSTM32F401CC
# verbose
pio run -e genericSTM32F401CC -v
# static check
pio check --checkTool cppcheck

# upload via ST-Link (SWD)
pio run -e genericSTM32F401CC -t upload

# debug
pio debug -e genericSTM32F401CC   # launches openocd + gdb TUI
# or
arm-none-eabi-gdb .pio/build/genericSTM32F401CC/firmware.elf -ex "target extended-remote localhost:3333"

# outputs
ls -lh .pio/build/genericSTM32F401CC/firmware.{elf,bin,hex}
```

**Build confirms:** `-O0`, `-ggdb3`, `-g3` (`platformio.ini:18`), no `Os`.

**Manual DFU fallback:** `dfu-util -a 0 -s 0x08000000:leave -D firmware.bin`

---

## 10. Tests (`src/Tests`)

All declared in `include/test.h:5`, invoked from `src/main.c:22` (currently `test_SPI_PollingTransmitReceive()`):

| Test File | Function | Covers |
|-----------|----------|--------|
| `gpiotest.c` | `gpioTest` | MODER/BSRR/IDR per-port |
| `nvicTest.c` | `nvicTest` | Enable/SetPending/Active/Priority/Reset |
| `uartTest.c` | `uartTest` | UART sync TX/RX, async, TC/ORE flags |
| `testSpi.c` | `test_SPI_PollingTransmitReceive`, `Async` | SPI1 full-duplex SW NSS, 2-slave sel |
| `testHserial.c` | `Test_Hserial_Sync/ ASync/ Dma_Uart` | HSERIAL unified API |
| `dmaTest.c` | `DMA_Test_Transmit/Receive` | DMA1 M2P FIFO, TC/HT |
| `flashTest.c` | `Example_ProgramData/MassErase/ProgramBuffer` | Sector5 erase+Word program, verify |
| `sevsegTest.c` | `sevsegTest` | 0-9 sweep |
| `switchTest.c` | `SwitchTest` | Debounced read |
| `testLcd.c` | `LcdTest` | Sync 4-bit HD44780 |
| `asychLcdTest.c` | `AsynchLcdTest` | Async queue + callback |
| `SectionAdd_ld.c` | `testLinkerScript` | LED_CFG RAM vs FLASH sections (`0x20000800` vs `0x08008000`) |

**Run:** uncomment desired in `src/main.c:9`, `pio run -t upload`, observe via SWV/UART or LEDs.

---

## 11. Per-Driver READMEs (Deep Dives)

| Layer | README | Contents |
|-------|--------|----------|
| LIB | [include/LIB/README.md](include/LIB/README.md) | `stdtypes.h` widths, `bool_t`, `NULL`, include guards |
| MCAL GPIO | [include/MCAL/GPIO_Driver/README.md](include/MCAL/GPIO_Driver/README.md) | 6 ports, 4 modes, AF0-15, BSRR, FLIP, per-pin re-config, OR warnings |
| RCC | [include/MCAL/RCC_Driver/README.md](include/MCAL/RCC_Driver/README.md) | HSI/HSE/PLL math & validation, prescalers, 64-bit masks, RSTR errata |
| NVIC | [include/MCAL/NVIC_Driver/README.md](include/MCAL/NVIC_Driver/README.md) | 240 IRQs, grouping 2+2, SystemReset, ISER/IPR regs |
| SysTick | [include/MCAL/SYSTICK_TIMER_Driver/README.md](include/MCAL/SYSTICK_TIMER_Driver/README.md) | 24-bit, ÷1/8, Wait_ms calc, callback |
| UART | [include/MCAL/UART_Driver/README.md](include/MCAL/UART_Driver/README.md) | USART1/2/6, 8N1, sync/async/DMA, flag helpers |
| SPI | [include/MCAL/SPI_Driver/README.md](include/MCAL/SPI_Driver/README.md) | 4 instances, SW NSS 8 slaves, TI/Motorola, DMA markers |
| DMA | [include/MCAL/DMA_Driver/README.md](include/MCAL/DMA_Driver/README.md) | 2×8×8 matrix, FIFO/Direct, burst, DBM, FTH |
| FLASH | [include/MCAL/FLASH_Driver/README.md](include/MCAL/FLASH_Driver/README.md) | 8 sectors, PSIZE, KEYR sequence, alignment |
| CRC | [include/MCAL/CRC_Driver/README.md](include/MCAL/CRC_Driver/README.md) | poly 0x4C11DB7, memory-region CRC |
| HAL LED | [include/HAL/LED_Driver/README.md](include/HAL/LED_Driver/README.md) | 9 LEDs, active LOW/HIGH, OFF init |
| SWITCH | [include/HAL/SWITCH_Driver/README.md](include/HAL/SWITCH_Driver/README.md) | pull mapping, VAl typo |
| SEVENSEG | [include/HAL/SEVENSEG_Driver/README.md](include/HAL/SEVENSEG_Driver/README.md) | 7 pins, segMap, multiplex |
| LCD | [include/HAL/LCD_Driver/README.md](include/HAL/LCD_Driver/README.md) | HD44780 4/8-bit, sync+async queue, CGRAM |
| HSERIAL | [include/HAL/HSERIAL_Driver/README.md](include/HAL/HSERIAL_Driver/README.md) | UART/SPI mux, channel union, priorities |
| MCU | [include/HAL/MCU_Driver/README.md](include/HAL/MCU_Driver/README.md) | `MCU_Configs` single-struct BSP |
| OS | [include/OS/README.md](include/OS/README.md) | cooperative, Priority ordering, FirstDalay_ms |

---

## 12. Usage Examples (Sync / Async / DMA)

### UART polling (USART2 115200 8N1 @ 42M, `PeripheralClock=42000000`)
```c
UART_Config_t c={.UART_Number=UART_2,.PeripheralClock=42000000,.BaudRate=115200,.Parity=UART_PARITY_NONE,
  .OverSampling=UART_OVERSAMPLING_16,.StopBits=UART_STOPBITS_1,.WordLength=UART_WORDLENGTH_8B,
  .Sample=UART_THREE_SAMPLE,.UartEnabled=UART_ENABLE_TRANSMITE|UART_ENABLE_RECEIVE};
UART_enuInit(&c);
const uint8_t t[]="Hello\r\n"; UART_enuSynTransmitBuffer(UART_2,t,sizeof(t)-1);
uint8_t r[10]; UART_enuSynReceiveBuffer(UART_2,r,10);
```

### UART async + DMA
```c
void TxDone(void){ }
UART_AsynBuffer_t a={.buffer=t,.size=6,.index=0,.callback=TxDone};
UART_enuAsynTransmitBuffer(UART_2,&a); // TXE ISR
// or DMA:
DMA_Config_t d={.DMAx=DMA1,.Streamx=DMA_STREAM6,.Channel=DMA_CHANNEL4,.Direction=DMA_DIRECTION_M2P,
  .MSize=DMA_MSIZE_BYTE,.PSize=DMA_PSIZE_BYTE,.MemoryInc=DMA_MINC_AUTO_INCREMENT,
  .PeripheralAddress=(uint32_t)&USART2->DR,.Memory0Address=(uint32_t)t,.NumberOfData=6,
  .Interrupts=DMA_INTERRUPT_TRANSFER_COMPLETE_ENABLE};
DMA_enuInit(&d); UART_enuActivateDMA(UART_2,UART_DMA_TRANSMIT_ENABLE); DMA_enuStartTransfer(DMA1,DMA_STREAM6);
```

### CRC Flash check
```c
RCC_EnablePeripheralClock(RCC_AHB1_BUS,RCC_AHB1_CRC_CLOCK);
uint32_t crc = CRC_CalculateMemoryRegion(0x08008000,0x4000);
if(crc!=EXPECTED) NVIC_SystemReset();
```

### FLASH program word
```c
Flash_Unlock();
Flash_EraseSector(FLASH_SECTOR_5);
Flash_ProgramWord(0x08020000,0x12345678);
Flash_Lock();
```

---

## 13. Configuration Files

- `rcc_cfg.h` — intentionally empty (runtime API only) — `include/MCAL/RCC_Driver/rcc_cfg.h:1`
- `gpio_cfg.h` — empty, per-pin struct passed at init — `gpio_cfg.h:1`
- `led_cfg.h / .c` — `LED_Name_t`, `LED_Configs[LED_LEN]`
- `switch_cfg.h / .c` — `SWITCH_Name_t`, `SWITCH_Configs[]` with `SWITCH_Connection_t`
- `sevenseg_cfg.h / .c` — `SEVSEG_cfg_t` A-G pins
- `lcd_cfg.c` — `LcdConf` + `Lcd4BitPins`
- `hserial_cfg.h/.c` — `HSERIAL_Channel_t`, `HSERIAL_Configs[]` union
- `mcu_cfg.h/.c` — `MCU_Configs` master table (edit this for board)
- `schedule_cfg.h` — `SCHED_MAX_RUNNABLES`, systick prescaler

> Edit `*_cfg.c` not `*_cfg.h` for board adaptation — `LED_LEN` must stay last sentinel.

---

## 14. Dependencies Between Drivers

```
stdtypes ← RCC ← GPIO ← LED/SWITCH/SEVENSEG/LCD/HSERIAL (via GPIO)
         ← RCC ← UART/SPI/DMA/CRC/FLASH (clock)
 NVIC ← UART/SPI/DMA/SysTick (IRQs) ← HSERIAL (priority)
 SysTick → schedule → LCD-async/HSERIAL-async (tick source)
 MCU wraps RCC; FLASH touches ACR latency set in RCC_ConfigurePLL
 CRC requires explicit RCC CRCEN
```

**Init order:** `MCU_enuInit` → `GPIO/RCC/NVIC` implied → `LED/SWITCH/SEVENSEG/LCD-UART-SPI` → `SCHED_enuInit` → `SCHED_enuStart`.

---

## 15. Known Issues & Warnings

| Area | Issue | Ref |
|------|-------|-----|
| GPIO | `GPIO_enuInit` does `MODER\|=`, `AFRx\|=` — re-init accumulates w/o reset; call `RCC_ResetPeripheralClock` first | `gpio.c:100` |
| GPIO | Only `SetPinVal` is atomic (BSRR); `Flip/SetPinMode` are `\|=` RMW — not ISR-safe for same port | `gpio.c:220` |
| GPIO | Typo `GPIO_WRONG_ALTARNATIVE` | `gpio_int.h:26` |
| RCC | `RCC_ResetPeripheralClock` AHB2/APB1/APB2 only clears `RSTR` (missing set) — no reset | `rcc.c:1164,1182,1200` |
| RCC | Prescaler setters use `\|=` not `&=~mask\|val` — accumulates | `rcc.c:696` |
| RCC | Flash latency hard-codes `0x40023C00` WS2 for 84M @3.3V | `rcc.c:603` |
| RCC | Typo `PEREPHRAL` in enums | `rcc_int.h:189` |
| NVIC | Enum lists 240 IRQs regardless of F401 (≈84 impl) — out-of-range is nop | `nvic.h:73` |
| SysTick | `Wait_ms` blocks — stalls scheduler if used after `SCHED_Start` | `systick.h` |
| SWITCH | API `SWITCH_enuReadVAl` capital/lower typo preserved | `switch.h` |
| SCHED | Field `FirstDalay_ms` typo; priority tie → order by array index | `schedule.h:32` |
| LCD | Async string pointer not copied — keep buffer live until callback | `lcd.h` |
| LCD | Async needs `SCHED_Start` — else `BUSY` | `lcd.h` |
| SPI | Declares `SPI_enuRegisterCallback` twice | `spi.h:260` |
| SPI | `MASTER_SW` demands manual CS GPIO handling | `spi.h:156` |
| FLASH | Erase blocks 0.25-1s — jitter if not suspended | `flash.h` |
| FLASH | 1→0 only; Word program req 2.7V supply | `flash.h` |
| CRC | Error returns `0` ambiguous with valid CRC 0 | `crc.h` |
| MCU | Re-init accumulates prescaler bits — reset first | `mcu.h` |

---

## 16. Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| HardFault on first `GPIO_enuInit` | Peripheral clock not enabled | Call `MCU_enuInit` or `RCC_EnablePeripheralClock(RCC_AHB1_BUS,RCC_AHB1_GPIOA_CLOCK)` first |
| UART baud error | `PeripheralClock` field ≠ actual APB1/2 after prescaler | Set `UART_Config.PeripheralClock` to real APB freq (42M/84M) per `MCU_Configs` |
| TXE ISR storm | TXE enabled before data | Keep `InterruptFlags=0` at init; async API enables on demand (header comment) |
| PLL `PLL_ALREADY_ENABLED` | `ConfigurePLL` called while `PLLON=1` | `RCC_DisablePLL()` + wait `!IsPLLReady()` before re-config |
| Flash `ALIGNMENT` | Address not %4 for Word | Align: `(addr&0x3)==0` else use Byte/HalfWord |
| LCD async never completes | Scheduler not running | `SCHED_enuInit` + `SCHED_enuStart` before `LCD_enuAsynInit` |
| DMA `WRONG_ZERO_NUMBER_OF_DATA` | `NumberOfData==0` | Set NDTR >0 |
| `Scheduling SCHED_NULL_PTR` | Null runnable pointer | Ensure static `SCHED_Runnable_t` alive (not stack temp) |
| `pio check` cppcheck warns | Raw register unions | Suppress or annotate `// cppcheck-suppress` per driver |

---

## 17. Roadmap

- [ ] Add I2C MCAL (I2C1-3) + HSERIAL I2C mode
- [ ] Add ADC1 driver + DMA interleaved
- [ ] Add TIM2-5 PWM/InputCapture + sched integration
- [ ] Fix RCC `RSTR` errata for AHB2/APB buses
- [ ] Make prescaler setters `&=~mask` before `|=val`
- [ ] Add unit-test runner on host (`native` env with mocked regs)
- [ ] CI: PlatformIO build + cppcheck on PR

---

## 18. Contributing & Style

- **Style:** `@brief` doxygen on every enum/func, `Copy_` param prefix, `enu/u8/vd` naming (enum/uint8/void), `ALL_FIELDS` vs `BIT_FIELDS` union access.
- **Validation-first:** validate mask before any register write; return specific `*_WRONG_*`.
- **Docs:** update per-driver `README.md` + root `README.md` TOC when adding enum/API.
- **Commits:** `feat(gpio): add SetPull`, `fix(rcc): clear before set prescaler`, `docs(lcd): async queue`.

```bash
git status && git diff
pio check
pio run -e genericSTM32F401CC
```

---

## 19. License & Credits

- **Course:** ITI Intake46, ARM Workspace — student project, not Yet distributed as OSS.
- **Authors:** `Eng.Gemy` (GPIO, RCC, NVIC, UART/SPI/DMA, HAL, OS), `Bootloader Team` (FLASH), `Your Name` placeholder (CRC).
- **Upstream:** CMSIS 5, PlatformIO `ststm32` package, `cppcheck`.
- **Hardware ref:** `STM32F401CC datasheet`, `RM0368 Reference Manual` (register offsets, PLL math), `PM0214 Cortex-M4`.

---

**Questions?** Open an issue with `[driver][api][status]` tag, include `rcc_priv.h` base + `MCU_Configs` dump. Happy hacking! 🚀

