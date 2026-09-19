# HSERIAL Driver — HAL

**Location:** `include/HAL/HSERIAL_Driver/hserial.h` | `hserial_cfg.h` | `src/HAL/HSERIAL_Driver/hserial.c` , `hserial_cfg.c`  
**Depends:** `MCAL/UART_Driver`, `MCAL/SPI_Driver`, `MCAL/DMA_Driver` (optional), `MCAL/NVIC_Driver`, `MCAL/RCC_Driver`  
**Role:** Unified serial facade over UART/SPI — channel-configured (wraps both peripherals under one `HSERIAL_Channel_t`)

## 1. Overview
High-level serial multiplexer. `hserial_cfg.c` declares `HSERIAL_Config_t` array per channel with `HSERIAL_Mode_t` selecting transport + transfer style. One app call `HSERIAL_enuTransmitBuffer(channel, buf, sz)` routes to the underlying `UART_*` or `SPI_*` sync/async/DMA API, with channel-priority interrupt config.

## 2. Features

### 2.1 Channel Modes (`hserial.h:23`)
| Mode | Transport | Method |
|---|---|---|
| `HSERIAL_MODE_UART_SYNC` | UART | Polling `SynTransmit/Receive` |
| `HSERIAL_MODE_UART_ASYNC` | UART | Interrupt `AsynTransmit/Receive` + NVIC priority |
| `HSERIAL_MODE_UART_DMA` | UART | DMA (`UART_DMA_*` + `DMA_Driver`) |
| `HSERIAL_MODE_SPI_SYNC` | SPI | Polling `Master/Slave Sync` |
| `HSERIAL_MODE_SPI_ASYNC` | SPI | Interrupt + priority |
| `HSERIAL_MODE_SPI_DMA` | SPI | (reserved — DMA suffix) |

### 2.2 Channels (`hserial_cfg.h`)
```c
typedef enum { HSERIAL_CH_UART2, HSERIAL_CH_SPI1, HSERIAL_LEN } HSERIAL_Channel_t;
```
Each `HSERIAL_Config_t` is a union:
```c
typedef struct {
    HSERIAL_Mode_t HSERIAL_Mode;
    union {
        H_UART_Sync_Config_t  UART_Sync_Config;   // periphClock, channel, baud, parity...
        H_UART_Async_Config_t UART_Async_Config;  // + Tx/Rx callbacks + interrupt priority
        H_UART_Dma_Config_t   UART_Dma_Config;    // DMA callbacks + priority
        H_SPI_Sync_Config_t   SPI_Sync_Config;    // spiChannel, commMode, cpol/cpha, nss...
        H_SPI_Async_Config_t  SPI_Async_Config;   // + callbacks + priority
    };
} HSERIAL_Config_t;
```
UART config fields mirror `UART_Config_t` (parity, overSampling, stopBits, wordLength, sample, enable) but typed `HSERIAL_Uart_*` (`hserial.h:33`). SPI fields mirror `SPI_Config_t` (commMode, nss, baudDiv, dataOrder...). Priority `HSERIAL_Priority_t 0x00..0xF0` maps to NVIC top-nibble (`hserial.h:76`).

### 2.3 Single Transport API
- `HSERIAL_enuInit(void)` — loops channels: enables RCC, calls `UART_enuInit` or `SPI_enuInit` per mode, registers NVIC priorities, prepares DMA if needed.
- `HSERIAL_enuTransmitBuffer(HSERIAL_Channel_t ch, const uint8_t *buf, uint16_t size)` — dispatches to `UART_enuSyn/Asyn/DMA` or `SPI_enuMasterSync/Asyn` based on mode; validates channel range/size/null.
- `HSERIAL_enuReceiveBuffer(ch, uint8_t *buf, uint16_t size)` — same dispatch for RX.

Callbacks for async/DMA come from `H_UART_Async_Config_t {UartTxCompleteCallback, UartRxCompleteCallback}` and `H_SPI_Async_Config_t` — invoked from ISR via UART/SPI driver → up to HSERIAL.

Status `HSERIAL_Status_t` (`hserial.h:10`): `OK, NOT_OK, ERROR_INIT_UART/DMA/SPI/NVIC, FAILED_TRANSMIT, WRONG_CHANNEL, NULL_POINTER, INVALID_SIZE`.

## 3. API Reference
| Function | Signature | Description |
|---|---|---|
| `Init` | `HSERIAL_Status_t HSERIAL_enuInit(void)` | Registers all channels from `hserial_cfg.c`. Call once after `MCU_enuInit`. |
| `Transmit` | `HSERIAL_Status_t HSERIAL_enuTransmitBuffer(HSERIAL_Channel_t, const uint8_t*, uint16_t)` | Unified TX. Returns `INVALID_SIZE` if `size==0`. |
| `Receive` | `HSERIAL_Status_t HSERIAL_enuReceiveBuffer(HSERIAL_Channel_t, uint8_t*, uint16_t)` | Unified RX. Null-ptr checked. |

## 4. Configuration Example (`hserial_cfg.c`)
```c
const HSERIAL_Config_t HSERIAL_Configs[HSERIAL_LEN] = {
    [HSERIAL_CH_UART2] = {.HSERIAL_Mode=HSERIAL_MODE_UART_ASYNC,
        .UART_Async_Config={.HSERIAL_UartChannel=HSERIAL_UART_2,
            .HSERIAL_UartPeripheralClock=42000000, .HSERIAL_UartBaudRate=115200,
            .HSERIAL_UartParity=HSERIAL_UART_PARITY_NONE, .HSERIAL_UartStopBits=HSERIAL_UART_STOPBITS_1,
            .HSERIAL_UartWordLength=HSERIAL_UART_WORDLENGTH_8B, .HSERIAL_UartEnable=HSERIAL_ENABLE_UART_BOTH,
            .HSERIAL_UartTxCompleteCallback=UartTxDone, .HSERIAL_UartRxCompleteCallback=UartRxDone,
            .HSERIAL_UartInterruptPriority=HSERIAL_PRIORITY_5}},
    [HSERIAL_CH_SPI1] = {.HSERIAL_Mode=HSERIAL_MODE_SPI_SYNC,
        .SPI_Sync_Config={.HSERIAL_SpiChannel=HSERIAL_SPI1,.HSERIAL_SpiMode=HSERIAL_SPI_MASTER,
            .HSERIAL_SpiBaudRate=HSERIAL_SPI_BAUDERATE_DIV8,.HSERIAL_SpiPolarityPhase=HSERIAL_SPI_ZERO_IDLE_FIRST_EDGE}},
};
```

## 5. Usage Examples

### 5.1 UART async string
```c
#include "HAL/HSERIAL_Driver/hserial.h"
void TxDone(void){ LED_vdToggle(BLACK_PILL_LED); }
HSERIAL_enuInit();
const uint8_t msg[]="Hello HSERIAL\r\n";
HSERIAL_enuTransmitBuffer(HSERIAL_CH_UART2, msg, sizeof(msg)-1);
uint8_t rx[32]; HSERIAL_enuReceiveBuffer(HSERIAL_CH_UART2, rx, 32);
```

### 5.2 SPI sync master transaction
```c
uint8_t tx=0x33, rx;
HSERIAL_enuTransmitBuffer(HSERIAL_CH_SPI1, &tx, 1);
HSERIAL_enuReceiveBuffer(HSERIAL_CH_SPI1, &rx, 1);
```

### 5.3 UART DMA (if mode includes DMA)
```c
// hserial_cfg sets MODE_UART_DMA + callbacks + priority + DMA stream mapping in dma_cfg
HSERIAL_enuTransmitBuffer(HSERIAL_CH_UART2, largeBuf, 256); // DMA async, callback on TC
```

## 6. Dependencies
- `UART_Driver` / `SPI_Driver` (exact mapping per mode)
- `DMA_Driver` if any channel uses `*_DMA` mode
- `NVIC_Driver` for async interrupt enable + priority
- `RCC_Driver` via underlying MCAL inits

## 7. Notes & Warnings
- Channel enum must stay sync between `hserial_cfg.h` and `hserial_cfg.c` — `HSERIAL_WRONG_CHANNEL` if out-of-range.
- UART async/DMA relies on callback preservation — don't free buffer before callback.
- SPI HSERIAL currently master-centric; slave RX uses underlying `SPI_enuSlave*` — configure correct `SpiMode`.
- Priority `0x00` is highest (maps to IPR `0x00`) — reserve 0-1 for critical IRQs.

## 8. Tests
`src/Tests/testHserial.c` (UART sync/async, SPI sync, HSERIAL unified), `uartTest.c`, `testSpi.c`
