# UART Driver — MCAL

**Location:** `include/MCAL/UART_Driver/uart.h` | `uart_priv.h` | `src/MCAL/UART_Driver/uart.c`  
**Instances:** `UART_1` (APB2 `0x40011000`), `UART_2` (APB1 `0x40004400`), `UART_6` (APB2 `0x40011400`) — STM32F401CC  
**Layer:** MCAL — low-level USART; wrapped by `HAL/HSERIAL`

## 1. Overview
Full-duplex USART with **sync polling**, **interrupt-async** (`TXE/TC/RXNE/PE`), and **DMA** modes. Handles baud-rate (`BRR`) calculation from `PeripheralClock/OverSampling`, frame config, flag polling, and NVIC enable. DMA is activated via `UART_enuActivateDMA` which sets `CR3.DMAT/DMAR`.

## 2. Features
| Feature | Details |
|---|---|
| **Instances** | `UART_Number_t {UART_1,UART_2,UART_6}` (`uart.h:69`) |
| **Enable** | `UART_ENABLE_TRANSMITE 0b1000` (TE), `ENABLE_RECEIVE 0b0100` (RE), `DISABLE 0` — OR for both |
| **Baud** | `BaudRate` + `PeripheralClock` + `OverSampling` → `BRR = fCK/(8/16*baud)`. Oversampling `16` or `8` (`uart.h:82`) |
| **Parity** | `NONE/Even/Odd` (`uart.h:75`) — sets `CR1.PCE/PS` |
| **Stop bits** | `1, 0.5, 2, 1.5` (`uart.h:88`), mask `UART_STOPBITS_MASK` |
| **Word length** | `8B/9B` (`M` bit `uart.h:97`) |
| **Oversample** | `OVER16 / OVER8` (`OVER8` bit) |
| **Sampling** | `THREE_SAMPLE / ONE_SAMPLE` (`uart.h:102`) |
| **Interrupts** | `TXE, TC, RXNE, PE, ERROR` (`uart.h:15`) — note `INTERRUPT_TXE` is `0` at init to avoid spurious TXE IRQ (comment `uart.h:12`); enabled on-demand in async TX |
| **Sync I/O** | `UART_enuSynTransmitBuffer` / `SynReceiveBuffer` — polling `TXE/RXNE` |
| **Async I/O** | `UART_enuAsynTransmitBuffer`/`ReceiveBuffer` using `UART_AsynBuffer_t {callback, size, index, buffer}` (`uart.h:36`). TXE IRQ per byte, TC at end |
| **DMA** | `UART_enuActivateDMA(uart, UART_DMA_TRANSMIT_ENABLE/RECEIVE_ENABLE)` → `CR3` |
| **Flags** | `UART_FLAG_TXE/TC/RXNE/ORE/NOISE/FE/PE` (`uart.h:22`) with `UART_u8Read*Flag()` accessors |
| **Error Callbacks** | `UART_Callbacks_t {PE, FE, NE, ORE, TC}` via `UART_enuRegisterCallbacks` |
| **GPIO AF** | Driver inits GPIO internally (AF7 for UART1/2, AF8 for UART6) — returns `UART_GPIO_ERROR` on failure |

Config struct (`uart.h:112`):
```c
typedef struct {
    uint32_t PeripheralClock;
    UART_Number_t UART_Number;
    uint32_t BaudRate;
    UART_Parity_t Parity;
    UART_OverSampling_t OverSampling;
    UART_StopBit_t StopBits;
    UART_WordLength_t WordLength;
    UART_Sample_t Sample;
    uint32_t UartEnabled;     // mask of ENABLE_TRANSMITE/RECEIVE
    uint32_t InterruptFlags;  // OR of INTERRUPT_*
} UART_Config_t;
```

## 3. API Reference
| Function | Description |
|---|---|
| `UART_enuInit(UART_Config_t*)` | Validates, enables RCC (`USART1/2/6`), inits GPIO AF, programs `CR1/CR2/CR3`, `BRR`, NVIC if interrupts enabled. |
| `UART_enuSynTransmitBuffer(uart, txBuf, size)` | Blocking per-byte: waits `TXE`, writes `DR`, waits `TC`. |
| `UART_enuSynReceiveBuffer(uart, rxBuf, size)` | Polls `RXNE`, reads `DR`. |
| `UART_enuAsynTransmitBuffer(uart, UART_AsynBuffer_t*)` | Non-blocking: starts TXE interrupt; ISR sends byte-by-byte, fires `callback` on `TC`. Returns `UART_TX_BUSY` if busy. |
| `UART_enuAsynReceiveBuffer(uart, UART_AsynBuffer_t*)` | Enables `RXNE` interrupt, fills buffer, callback when `size` reached. |
| `UART_enuActivateDMA(uart, enableDmaFlag)` | Sets `CR3.DMAT/DMAR`. Validate `UART_WRONG_DMA_ENABLE`. |
| `UART_u8ReadTXEFlag/TCFlag/RXNEFlag/OREFlag/NoiseFlag/FEFlag/PEFlag(uart)` | `return SR & FLAG` |
| `UART_enuEnableInterrupts(uart, flags)` / `DisableInterrupts` | Set/clear `CR1` IE bits |
| `UART_enuClearFlags(uart, flags)` | Clears `SR` flags (read SR + DR sequence) |
| `UART_enuRegisterCallbacks(uart, UART_Callbacks_t*)` | Installs per-error + TC callbacks |

Status `UART_Status_t` (`uart.h:51`): `OK, NOT_OK, NULL_PTR, WRONG_UART_NUMBER/ENABLE/PARITY/OVERSAMPLING/STOPBITS/WORDLENGTH/SAMPLE/INTERRUPT_FLAGS, NOT_INIT_SUCCESSFULLY, GPIO_ERROR, TX_BUSY, WRONG_DMA_ENABLE`.

## 4. Usage Examples

### 4.1 Polling 9600 8N1 UART2 (APB1 42M)
```c
#include "MCAL/UART_Driver/uart.h"
UART_Config_t cfg = {
    .UART_Number = UART_2,
    .PeripheralClock = 42000000,
    .BaudRate = 9600,
    .Parity = UART_PARITY_NONE,
    .OverSampling = UART_OVERSAMPLING_16,
    .StopBits = UART_STOPBITS_1,
    .WordLength = UART_WORDLENGTH_8B,
    .Sample = UART_THREE_SAMPLE,
    .UartEnabled = UART_ENABLE_TRANSMITE | UART_ENABLE_RECEIVE,
    .InterruptFlags = UART_INTERRUPT_RXNE // + TC/PE etc if needed
};
UART_enuInit(&cfg);
const uint8_t msg[]="Hello\r\n";
UART_enuSynTransmitBuffer(UART_2, msg, sizeof(msg)-1);
uint8_t rx[10]; UART_enuSynReceiveBuffer(UART_2, rx, 10);
```

### 4.2 Async TX with callback
```c
void TxDone(void){ /* toggle LED */ }
UART_AsynBuffer_t ab = {.buffer=(uint8_t*)msg,.size=sizeof(msg)-1,.index=0,.callback=TxDone};
UART_enuAsynTransmitBuffer(UART_2, &ab);
// OR with DMA:
UART_enuActivateDMA(UART_2, UART_DMA_TRANSMIT_ENABLE);
// then start DMA stream that targets USART2_DR
```

### 4.3 Flags & error handling
```c
if (UART_u8ReadOREFlag(UART_2)) UART_enuClearFlags(UART_2, UART_FLAG_ORE);
UART_Callbacks_t cbs = {.ParityErrorCallback=PE_Handler,.OverrunErrorCallback=ORE_Handler,
                        .TC_Callback=TxDone,.FramingErrorCallback=FE_Handler,.NoiseErrorCallback=NE_Handler};
UART_enuRegisterCallbacks(UART_2, &cbs);
```

## 5. Configuration Notes
- `PeripheralClock` must equal actual APB1/APB2 frequency after RCC prescalers — else baud error.
- `InterruptFlags=0` at init is intentional; async API enables `TXE` internally.
- 9-bit word requires `WordLength=9B` and usually `Parity=NONE` — buffer is `uint16_t` conceptually but API uses `uint8_t*` split.

## 6. Dependencies
- `LIB/stdtypes.h`, `RCC_Driver` (enables `USART1/2/6` clock), `GPIO_Driver` (AF init), `NVIC_Driver` if interrupts, `DMA_Driver` if DMA mode, `HSERIAL` as high-level facade.

## 7. Warnings
- Enabling `TXE` interrupt before data ready causes immediate ISR storm — driver leaves it disabled.
- Sync functions are blocking — not ISR-safe for large sizes.
- Async state is per-UART with busy flag — second `AsynTransmit` before completion → `UART_TX_BUSY`.
- `uart_priv.h` register map assumes F401; reuse on other F4 requires verification.

## 8. Tests
`src/Tests/uartTest.c`, `testHserial.c`
