# SPI Driver — MCAL

**Location:** `include/MCAL/SPI_Driver/spi.h` | `spi_priv.h` | `src/MCAL/SPI_Driver/spi.c`  
**Instances:** `SPI1` (APB2 `0x40013000`), `SPI2` (APB1 `0x40003800`), `SPI3` (APB1 `0x40003C00`), `SPI4` (APB2 `0x40013400`)  
**Layer:** MCAL — supports master/slave, sync/async polling vs interrupt, multi-slave SW NSS.

## 1. Overview
Full SPI abstraction over `CR1/CR2/SR/DR/CRCPR/RXCRC/TXCRC`. Supports all frame modes, baud dividers, DMA markers, CRC, TI/Motorola, and software-managed NSS for multi-slave masters (up to `SPI_MAX_SLAVES 8`). Async mode uses NVIC IRQs (`SPI1_IRQn` etc.) with per-flag callbacks.

## 2. Features

### 2.1 Communication Modes (`spi.h:71`)
- `FULL_DUPLEX` (BIDIMODE=0,RXONLY=0)
- `HALF_DUPLEX_2LINES_TX_ONLY`, `_RX_ONLY`, `1LINE_RX_ONLY`, `1LINE_TX_ONLY` — via `BIDIMODE/BIDIOE/RXONLY` bits in `CR1`.

### 2.2 Master/Slave & NSS (`spi.h:87,183`)
| Use Case | Enum | Effect |
|---|---|---|
| Single master/single slave, auto NSS | `SPI_NSS_MASTER_HW_OUTPUT (0x0004)` | `SSOE=1, SSM=0` — hardware drives NSS |
| Single master, multi-slave | `SPI_NSS_MASTER_SW (0x0300)` | `SSM=1, SSI=1` — SW GPIO CS (array `SPI_SlavesConfig_t`) |
| Multi-master | `SPI_NSS_MASTER_HW_INPUT` | `SSM=0` — NSS input detects bus conflict |
| Normal slave | `SPI_NSS_SLAVE_HW` | `SSM=0` — master drives NSS pin |
| Lone slave, save pin | `SPI_NSS_SLAVE_SW` | `SSM=1,SSI=1` — always selected |

### 2.3 Data & Order
- CRC: `CRC_DISABLED/ENABLED` (CR1.CRCEN), polynomial `0x0000-0xFFFF`
- Data length: `8_BIT / 16_BIT` (DFF)
- Bit order: `MSB_FIRST / LSB_FIRST` (LSBFIRST)
- Baud: `DIV2..DIV256` (BR[2:0]) — `PCLK/(2^(BR+1))`
- Mode CPOL/CPHA: `ZERO_IDLE_FIRST_EDGE` .. `ONE_IDLE_SECOND_EDGE` (4 combos)
- Frame: `MOTOROLA / TI_MODE` (FRF)
- DMA: `DISABLE / TX / RX / TX_RX` — sets `CR2.TXDMAEN/RXDMAEN`

### 2.4 Multi-Slave Config (`spi.h:212`)
```c
typedef struct { SPI_Port_t port; SPI_Pin_t pin; } SPI_SlaveCS_t;
typedef struct { SPI_SlaveCS_t slaves[8]; uint8_t numberOfSlaves; } SPI_SlavesConfig_t;
```
Master asserts GPIO CS via `SPI_enuMasterSelectSlave` / `DeselectSlave` (drive pin low/high via GPIO).

### 2.5 Flags & Interrupts (`spi.h:196`)
`RXNE(0), TXE(1), UNDERRUN(3), CRCERR(4), MODF(5), OVR(6), BSY(7), FRE(8)` — accessed via `SPI_u8ReadFlag`, cleared via `SPI_enuClearFlag`, callbacks per flag via `SPI_enuRegisterCallback`, IRQ enable via `Enable/DisableInterrupt`.

## 3. Config Struct (`spi.h:225`)
```c
typedef struct {
    SPI_Number_t spiNumber;          // SPI1..4
    SPI_Communication_t communicationMode;
    SPI_Mode_t mode;                 // MASTER/SLAVE
    SPI_Crc_t crcState;
    SPI_DataLength_t dataLength;
    SPI_DataOrder_t dataOrder;
    SPI_BaudRate_t baudRate;
    SPI_PolarityPhase_t polarityPhase;
    SPI_FrameFormat_t frameFormat;
    SPI_Dma_t dmaState;
    SPI_Nss_t nssManagement;
    uint16_t crcPolynomial;
    SPI_SlavesConfig_t slavesConfig; // master only
} SPI_Config_t;
```

## 4. API Reference
| Function | Description |
|---|---|
| `SPI_enuInit(SPI_Config_t*)` | Validates, enables RCC (`SPI1/2/3/4`), inits GPIO AF (SCK/MISO/MOSI+NSS), writes `CR1` (`BR,CPOL,CPHA,DFF,LSBFIRST,SSM,SSI,CRCEN,MSTR,BIDIMODE...`), `CRCPR`, `CR2` (DMA, NSS, FRACT), NVIC enable if needed. |
| `SPI_enuMasterSyncTransmitReceive(SPI_Number_t, uint16_t Tx, uint16_t *Rx)` | Poll `TXE`, write `DR`, poll `RXNE`, read `DR`. Full-duplex. |
| `SPI_enuMasterSyncTransmit(SPI_Number_t, uint16_t Tx)` | Poll `TXE`, write. |
| `SPI_enuMasterSyncReceive(SPI_Number_t, uint16_t *Rx)` | Dummy write or poll `RXNE`, read. |
| `SPI_enuMasterAsynTransmit(SPI_Number_t, uint16_t Tx, SPI_Callback_t)` | Enables `TXEIE`, stores callback, ISR sends on `TXE`, handles `TC/BSY`. |
| `SPI_enuMasterAsynReceive(SPI_Number_t, uint16_t* Rx, SPI_Callback_t)` | Enables `RXNEIE`, ISR fills `*Rx` and fires callback. |
| `SPI_enuMasterSelectSlave / DeselectSlave(SPI_Number_t, SPI_SlaveCS_t)` | GPIO CS low/high (for `MASTER_SW`). Validates `numberOfSlaves`. |
| `SPI_enuSlaveSyncTransmit(SPI_Number_t, uint16_t)` | Slave write (must be clocked by master). |
| `SPI_enuSlaveSyncReceive(SPI_Number_t, uint16_t*)` | Poll `RXNE` read. |
| `SPI_enuSlaveAsynTransmit/Receive(...)` | Slave async via IRQ. |
| `SPI_enuClearFlag(SPI_Number_t, SPI_Flag_t)` | Clears `CRCERR/MODF/OVR` per RM sequence. |
| `SPI_enuRegisterCallback(SPI_Number_t, SPI_Flag_t, SPI_Callback_t)` | Per-flag callback table. |
| `SPI_enuEnableInterrupt / DisableInterrupt(SPI_Number_t, SPI_Flag_t)` | Sets `CR2.TXEIE/RXNEIE/ERRIE`. |
| `SPI_u8ReadFlag(SPI_Number_t, SPI_Flag_t)` | `return (SR>>flag)&1`. |
| `SPI_State_t` busy flag | `SPI_BUSY / NOT_BUSY` per instance. |

Status `SPI_Status_t` (`spi.h:43`): `OK, WRONG_SPI_NUMBER/COMMUNICATION/MODE/CRC/DATA_LENGTH/ORDER/BAUDRATE/POLARITY_PHASE/FRAME_FORMAT/DMA_STATE/NSS_MANAGEMENT, NULL_POINTER, GPIO_NOT_INITIALIZED, ERROR_SELECTING_SLAVE, STATUS_IS_BUSY, WRONG_FLAG_VALUE`.

## 5. Usage Examples

### 5.1 Master 8-bit, DIV8, Mode0, SW NSS with 2 slaves
```c
#include "MCAL/SPI_Driver/spi.h"
SPI_SlaveCS_t cs1={SPI_PORT_A, SPI_PIN_4}, cs2={SPI_PORT_A, SPI_PIN_5};
SPI_Config_t cfg = {
    .spiNumber=SPI1, .communicationMode=SPI_FULL_DUPLEX, .mode=SPI_MASTER,
    .crcState=SPI_CRC_DISABLED, .dataLength=SPI_8_BIT_DATA, .dataOrder=SPI_MSB_FIRST,
    .baudRate=SPI_BAUDRATE_DIV8, .polarityPhase=SPI_ZERO_IDLE_FIRST_EDGE,
    .frameFormat=SPI_MOTOROLA, .dmaState=SPI_DISABLE_DMA,
    .nssManagement=SPI_NSS_MASTER_SW, .crcPolynomial=0x7,
    .slavesConfig={.slaves={cs1,cs2}, .numberOfSlaves=2}
};
SPI_enuInit(&cfg);
SPI_enuMasterSelectSlave(SPI1, cs1);
uint16_t rx; SPI_enuMasterSyncTransmitReceive(SPI1, 0x55, &rx);
SPI_enuMasterDeselectSlave(SPI1, cs1);
```

### 5.2 Async master TX with callback
```c
void TxCpl(void){ SPI_enuMasterDeselectSlave(SPI1, cs1); }
SPI_enuMasterSelectSlave(SPI1, cs1);
SPI_enuMasterAsynTransmit(SPI1, 0xA5, TxCpl);
```

### 5.3 Slave RX poll
```c
SPI_Config_t sCfg={.spiNumber=SPI2,.mode=SPI_SLAVE,.nssManagement=SPI_NSS_SLAVE_HW, /* ... */};
SPI_enuInit(&sCfg);
uint16_t cmd; SPI_enuSlaveSyncReceive(SPI2, &cmd);
```

## 6. Dependencies
`LIB/stdtypes.h`, `RCC_Driver` (SPI1/2/3/4 clock), `GPIO_Driver` (SCK/MISO/MOSI/NSS AF5), `NVIC_Driver` for async, `DMA_Driver` if `dmaState != DISABLE`.

## 7. Warnings
- `SPI_NSS_MASTER_SW` requires app to manage CS GPIOs — driver only drives the configured `slaves[]` pins.
- Async APIs share per-SPI busy state — second call before completion → `SPI_STATUS_IS_BUSY`.
- `SPI_enuRegisterCallback` is declared twice in header (`spi.h:260` dup) — link chooses one.
- `nssManagement` encoding packs `SSM/SSI/SSOE` into one value — ensure correct quick-guide selection (`spi.h:156`).

## 8. Tests
`src/Tests/testSpi.c`
