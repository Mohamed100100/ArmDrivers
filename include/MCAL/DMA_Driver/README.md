# DMA Driver — MCAL

**Location:** `include/MCAL/DMA_Driver/dma.h` | `dma_priv.h` | `src/MCAL/DMA_Driver/dma.c`  
**Controllers:** `DMA1` (AHB1 `0x40026000`, streams 0-7, NVIC 47-57) & `DMA2` ( `0x40026400`, streams 0-7, NVIC 56-68) — STM32F401CC has 8 streams × 8 channels each.  
**Layer:** MCAL — memory-to-memory/peripheral acceleration for UART/SPI/ADC etc.

## 1. Overview
Implements STM32F4 DMA: each stream has `CR, NDTR, PAR, M0AR, M1AR, FCR` plus shared `LISR/HISR, LIFCR/HIFCR`. Supports FIFO vs Direct, double-buffer, burst, flow-controller, and 5 interrupt sources.

## 2. Features

### 2.1 Stream/Channel Matrix (`dma.h:26`)
- **Controllers:** `DMA1, DMA2`
- **Streams:** `DMA_STREAM0..7`
- **Channels:** `DMA_CHANNEL0..7` ( `CHSEL[2:0]` in `CR[27:25]` = `channel<<25` ). Channel maps to request source (e.g., DMA2-Stream3-Ch4 = SPI1_TX).

### 2.2 Transfer Settings
| Field | Enum (CR bits) |
|---|---|
| Direction | `P2M (00), M2P (01), M2M (10)` — `DIR[7:6]` |
| Peripheral Inc | `FIXED / AUTO_INCREMENT` — `PINC` |
| Memory Inc | `FIXED / AUTO_INCREMENT` — `MINC` |
| Peripheral Size | `BYTE/HALFWORD/WORD` — `PSIZE[12:11]` |
| Memory Size | `BYTE/HALFWORD/WORD` — `MSIZE[14:13]` |
| Priority | `LOW/MEDIUM/HIGH/VERY_HIGH` — `PL[17:16]` |
| Circular | `DISABLE/ENABLE` — `CIRC` |
| Burst | `MBurst SINGLE/INCR4/8/16` (`MBURST[24:23]`), `PBurst SINGLE/INCR4/8/16` |
| Double Buffer | `DISABLE/ENABLE` — `DBM (18)` + `CT` bit auto-toggled; `Memory1Address` used as `M1AR` |
| Flow Control | `DMA_FLOW_CONTROL_USING_DMA / PERIPHERAL` — `PFCTRL` (when `1`, peripheral drives `NDTR`) |
| Mode | `DIRECT (0) / FIFO (1)` — `FCR.DMDIS` = `1` means FIFO. FIFO threshold `QUARTER/HALF/3Q/FULL` (`FCR.FTH[1:0]`) |

### 2.3 Interrupts (`dma.h:8` masks, `20` types)
Enable masks ORed into `Config.Interrupts`:  
`TRANSFER_COMPLETE (TCIE 0x10)`, `HALF_TRANSFER (HTIE 0x08)`, `TRANSFER_ERROR (TEIE 0x04)`, `DIRECT_MODE_ERROR (DMEIE 0x02)`, `FIFO_ERROR (FTHIE 0x80)` — mapped to `CR` bits [4,3,2,1] + `FCR.FEIE`. Driver validates `WRONG_INTERRUPTS`.

## 3. Config Struct (`dma.h:170`)
```c
typedef struct {
    DMA_Controller_t DMAx; DMA_Stream_t Streamx; DMA_Channel_t Channel;
    DMA_Direction_t Direction; DMA_PeripheralFlowCtrl_t PeripheralFlowCtrl;
    DMA_Mode_t Mode; DMA_Priority_t Priority; DMA_MSize_t MSize; DMA_PSize_t PSize;
    DMA_MemoryInc_t MemoryInc; DMA_PerihperalInc_t PeripheralInc;
    DMA_CircularMode_t CircularMode; DMA_MBurst_t MBurst; DMA_PBurst_t PBurst;
    DMA_DoubleBuffer_t DoubleBuffer; DMA_FifoThreshold_t FifoThreshold;
    uint32_t PeripheralAddress; uint32_t Memory0Address; uint32_t Memory1Address;
    uint32_t Interrupts; // OR of DMA_INTERRUPT_*_ENABLE
    uint16_t NumberOfData; // NDTR (0 → error WRONG_ZERO_NUMBER_OF_DATA)
} DMA_Config_t;
```

## 4. API Reference
| Function | Description |
|---|---|
| `DMA_enuInit(const DMA_Config_t*)` | Validates all enums + non-zero `NumberOfData`, checks stream not enabled (`CR.EN==0`), disables `CR.EN`, clears flags (`LIFCR/HIFCR`), programs `CR` OR-accumulated from enums, `PAR, M0AR, M1AR, NDTR, FCR (DMDIS, FTH, FEIE)`, enables IRQs in `CR/FCR`, optionally via `NVIC_EnableIRQ` for stream IRQs. Returns `DMA_OK` or `DMA_WRONG_*`. |
| `DMA_enuStartTransfer(DMAx, Streamx)` | `CR.EN = 1` (if not already). For `M2M`, triggers immediately; for peripheral, waits for request. |
| `DMA_enuStopTransfer(DMAx, Streamx)` | `CR.EN = 0`, polling until cleared. |
| `DMA_enuSetMemoryAddress(DMAx, Streamx, addr)` | Writes `M0AR` (or toggles `CT` for double-buffer) — only when disabled. |
| `DMA_enuSetNumberOfData(DMAx, Streamx, n)` | Writes `NDTR`. |
| `DMA_enuRegisterCallback(DMAx, Streamx, DMA_Interrupts_t, DMA_CallBack_t)` | Stores per-stream/per-flag callback table (5×16 entries). |
| `DMA_u8ReadFlag(DMAx, Streamx, Interrupt)` | Reads `LISR/HISR` flags: `TCIF, HTIF, TEIF, DMEIF, FEIF` per stream position (shift 0/6/16/22). |
| `DMA_enuClearFlag(DMAx, Streamx, Interrupt)` | Writes `1` to `LIFCR/HIFCR` bit. |

Interrupt ISRs in `dma.c` (e.g., `DMA1_Stream0_IRQHandler`) check `LISR` flags, call registered callbacks, clear via `LIFCR`.

## 5. Usage Examples

### 5.1 UART2 TX via DMA (poll started, interrupt on TC)
```c
#include "MCAL/DMA_Driver/dma.h"
#include "MCAL/UART_Driver/uart.h"
uint8_t msg[]="Hello DMA";
DMA_Config_t cfg={
    .DMAx=DMA1, .Streamx=DMA_STREAM6, .Channel=DMA_CHANNEL4, // DMA1 Stream6 Ch4 = USART2_TX
    .Direction=DMA_DIRECTION_M2P, .MemoryInc=DMA_MINC_AUTO_INCREMENT, .PeripheralInc=DMA_PINC_FIXED,
    .MSize=DMA_MSIZE_BYTE, .PSize=DMA_PSIZE_BYTE, .Priority=DMA_PRIORITY_HIGH,
    .CircularMode=DMA_CIRCULAR_MODE_DISABLE, .Mode=DMA_MODE_FIFO, .FifoThreshold=DMA_FIFO_THRESHOLD_FULL,
    .PeripheralFlowCtrl=DMA_FLOW_CONTROL_USING_DMA, .DoubleBuffer=DMA_DISABLE_DOUBLE_BUFFER,
    .PeripheralAddress=(uint32_t)&USART2->DR, .Memory0Address=(uint32_t)msg,
    .NumberOfData=sizeof(msg)-1, .Interrupts=DMA_INTERRUPT_TRANSFER_COMPLETE_ENABLE
};
DMA_enuInit(&cfg);
DMA_enuRegisterCallback(DMA1, DMA_STREAM6, DMA_INTERRUPT_TRANSMISSION_COMPLETE, TxDone);
UART_enuActivateDMA(UART_2, UART_DMA_TRANSMIT_ENABLE);
DMA_enuStartTransfer(DMA1, DMA_STREAM6);
```

### 5.2 Memory-to-memory burst (FIFO)
```c
uint32_t src[64], dst[64];
DMA_Config_t m2m={
    .DMAx=DMA2, .Streamx=DMA_STREAM0, .Channel=DMA_CHANNEL0,
    .Direction=DMA_DIRECTION_M2M, .MemoryInc=DMA_MINC_AUTO_INCREMENT,
    .PeripheralInc=DMA_PINC_AUTO_INCREMENT, // M2M uses PAR as source mem
    .MSize=DMA_MSIZE_WORD, .PSize=DMA_PSIZE_WORD, .MBurst=DMA_MBurst_INCR4, .PBurst=DMA_PBurst_INCR4,
    .Mode=DMA_MODE_FIFO, .FifoThreshold=DMA_FIFO_THRESHOLD_FULL, .NumberOfData=64,
    .PeripheralAddress=(uint32_t)src, .Memory0Address=(uint32_t)dst, .Interrupts=DMA_INTERRUPT_TRANSFER_COMPLETE_ENABLE
};
DMA_enuInit(&m2m); DMA_enuStartTransfer(DMA2, DMA_STREAM0);
```

## 6. Dependencies
`LIB/stdtypes.h`, `RCC_Driver` (enable `DMA1/2` clock), `NVIC_Driver` for interrupts, `UART/SPI/ADC` for request sources, `dma_priv.h` holds stream/channel↔NVIC mapping.

## 7. Warnings
- `NDTR==0` → `DMA_WRONG_ZERO_NUMBER_OF_DATA`.
- Change `CR` / addresses only when `EN==0` — driver enforces; `Start` while enabled is no-op.
- Double-buffer: ISR toggles `CT` bit; reading `M0AR` vs `M1AR` depends on `CT`.
- FIFO `FEIF` only in FIFO mode; `DMEIF` only in direct mode.
- Channel mapping differs DMA1 vs DMA2 — consult F401 RM Table 28/29 (driver does not validate channel-stream-peripheral legality).

## 8. Tests
`src/Tests/dmaTest.c` (mem2mem, USART TX, FIFO vs Direct).
