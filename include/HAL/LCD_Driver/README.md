# LCD Driver — HAL (HD44780 16x2)

**Location:** `include/HAL/LCD_Driver/lcd.h` | `lcd_queue.h` | `src/HAL/LCD_Driver/lcd.c` , `lcd_queue.c` , `lcd_cfg.c`  
**Depends:** `MCAL/GPIO_Driver`, `MCAL/SYSTICK` or `OS/schedule` (async), `LIB/stdtypes`  
**Controller:** HD44780 compatible — 8-bit or 4-bit (saves 4 pins)

## 1. Overview
Full 16x2 driver with **synchronous blocking** (simple, ~50-100 ms init blocking) and **asynchronous non-blocking** (scheduler-driven queue, no delay loops) families. Supports custom chars (CGRAM 0-7), cursor/blink/display/shift controls, and queue-based string streaming.

## 2. Features

### 2.1 Bus Modes
- **8-bit** `LCD_8_BIT_OPERATION` → 11 pins: `DB0-7, RS, RW, EN` (`lcd.h:113`)
- **4-bit** `LCD_4_BIT_OPERATION` → 7 pins: `DB4-7, RS, RW, EN` — sends byte as `HIGH_NIBBLE(4)` then `LOW_NIBBLE(0)` (`lcd.h:38`)

### 2.2 Display Config (`LCD_Config_t` `lcd.h:274`)
- `BitOperation`, `LineDisplay {1/2}`, `FontSize {5x7 (8 chars) / 5x10 (4 chars, 1-line only)}`, `Display ON/OFF`, `Cursor ON/OFF`, `Blink ON/OFF`, `IncrementStatus {INCREMENT/DECREMENT}`, `DisplayShiftOperation {NO_SHIFT/AUTO_SHIFT}`

### 2.3 Pin Mapping
- `LCD_Port_t {A,B,C,D,E,H}`, `LCD_Pin_t {0..15}`
- `LCD_PinInfo_t {port,pin}`, unions `LCD_Pinout_8BitMode_t` (DB0-7+RS/RW/EN) and `LCD_Pinout_4BitMode_t` (DB4-7+RS/RW/EN) (`lcd.h:231`)

### 2.4 Sync vs Async
| Aspect | Sync (`LCD_enuSync*`) | Async (`LCD_enuAsyn*` / `LCD_Asyn*`) |
|---|---|---|
| Behavior | Blocking waits (~2 ms per cmd, 50 ms init) | Returns `LCD_OK` immediately; scheduler runnable processes queue per tick |
| Use | Simple apps, boot log | Responsive apps, RTOS, streaming |
| Callback | none | `LCD_Callback_t cb(status)` via `LCD_vdAsyncRegisterCallback` — invoked with `INIT_SUCEESSFULLY/WRITE_SUCCESSFULLY/.../GPIO_ERROR` |
| Queue | — | `lcd_queue.h` FIFO of ops (write string, create char, set pos) — automatic 16-char wrap |

### 2.5 Core Operations
- **Init:** `LCD_enuSynInit()` / `LCD_enuAsynInit()` — power-on sequence `(0x33,0x32,0x28...)`, function set, display control, entry mode, clear.
- **Character:** `SyncWriteCharacter(c)` — RS=1, pulse EN; async variant queued.
- **String:** `AsynWriteString((uint8_t*)"Hello")`, `AsynWriteStringAtPosition(str,row,col)` — row`0-1`, col`0-15` validation (`WRONG_ROW/COLUMN`).
- **Custom Char:** 8 rows × 5 bits: `SyncCreateCustomChar(loc, map[8])` writes CGRAM address (`0x40+loc*8`), then `SyncWriteCustomChar(loc)` to DDRAM; async equivalents.
- **Controls:** `SyncClearDisplay`, `SyncReturnHome`, `SyncSetDisplay(ON/OFF)`, `SyncSetCursor(ON/OFF)`, `SyncSetBlink`, `SyncSetCursorPosition(row,col)`, `SyncSetIncrementDecrementMode`, `SyncDisplayShift`, `SyncSetFontSize`, `SyncSetLineDisplay`.
- **Status:** `LCD_Status_t` 16 codes incl `NOT_INIT, BUSY, WRONG_LOCATION (0-7), WRONG_BIT_OPERATION` (`lcd.h:52`)

## 3. Configuration (`lcd_cfg.c`)
```c
LCD_Config_t LcdConf = {
    .BitOperation=LCD_4_BIT_OPERATION, .LineDisplay=LCD_2_LINE_DISPLAY,
    .FontSize=LCD_5_7_DOT_FONT, .Display=LCD_DISPLAY_ON, .Cursor=LCD_CURSOR_OFF,
    .Blink=LCD_BLINK_OFF, .IncrementStatus=LCD_INCREMENT, .DisplayShiftOperation=LCD_NO_SHIFT
};
LCD_Pinout_4BitMode_t Lcd4BitPins = {
    .RS={LCD_PORT_A, LCD_PIN_0}, .RW={LCD_PORT_A, LCD_PIN_1}, .EN={LCD_PORT_A, LCD_PIN_2},
    .DB4={LCD_PORT_A, LCD_PIN_4}, .DB5={LCD_PORT_A, LCD_PIN_5}, .DB6={LCD_PORT_A, LCD_PIN_6}, .DB7={LCD_PORT_A, LCD_PIN_7}
};
```
Set before `LCD_enu*Init`.

## 4. API Reference (abbrev)

| Group | Sync | Async |
|---|---|---|
| Init | `LCD_enuSynInit()` | `LCD_enuAsynInit()` |
| Write | `LCD_enuSyncWriteCharacter(c)`, `LCD_enuSyncClearDisplay()` | `LCD_enuAsynWriteCharacter(c)`, `LCD_enuAsynWriteString(s)`, `AsynWriteStringAtPosition(s,row,col)` |
| Cursor | `SyncSetCursorPosition`, `SyncReturnHome`, `SyncSetCursor`, `SyncSetBlink` | *(position embedded in WriteAtPosition)* |
| Display | `SyncSetDisplay`, `SyncDisplayShift`, `SyncSetIncrementDecrementMode`, `SyncSetFontSize/LineDisplay` | — |
| Custom | `SyncCreateCustomChar(loc,map)`, `SyncWriteCustomChar(loc)` | `AsynCreateCustomChar`, `AsynDisplayCustomChar`, `AsynDisplayCustomCharAtPosition` |
| Async Ctrl | — | `LCD_vdAsyncRegisterCallback(cb)` |

## 5. Usage Examples

### 5.1 Sync boot message
```c
#include "HAL/LCD_Driver/lcd.h"
LCD_enuSynInit();
LCD_enuSyncClearDisplay();
LCD_enuSyncSetCursorPosition(0,0);
for(char *p="Hello"; *p; p++) LCD_enuSyncWriteCharacter(*p);
LCD_enuSyncSetCursorPosition(1,0);
LCD_enuSyncWriteCharacter('X');
```

### 5.2 Async with scheduler (non-blocking)
```c
void LcdCb(LCD_Status_t st){
    if(st==LCD_INIT_SUCEESSFULLY) LCD_enuAsynWriteString((uint8_t*)"Ready");
    if(st==LCD_WRITE_SUCCESSFULLY) {/* next */ }
}
LCD_vdAsyncRegisterCallback(LcdCb);
LCD_enuAsynInit();
// scheduler must be running: SCHED_enuInit(84000000, 1); SCHED_enuStart();
LCD_enuAsynWriteStringAtPosition((uint8_t*)"Temp: 25C", 1, 0);
```

### 5.3 Custom smiley (5×8)
```c
uint8_t smile[8]={0b00000,0b01010,0b01010,0b00000,0b10001,0b01110,0b00000,0b00000};
LCD_enuSyncCreateCustomChar(0, smile);
LCD_enuSyncWriteCustomChar(0);
```

## 6. Dependencies
`GPIO` (all pins `OUTPUT`), `SysTick`/`Scheduler` for async timing, `LIB/stdtypes`.

## 7. Warnings
- Async requires `SCHED_Start()` — without it queue never drains → `LCD_BUSY`.
- String buffer must remain live until async write completes (queued pointer, not copy for strings).
- 4-bit mode still needs `RW` pin — tie to GND or drive via GPIO; EN pulse width depends on HD44780 (~450 ns).
- Custom chars limited 0-7 (5×8) or 0-3 (5×10).

## 8. Tests
`src/Tests/testLcd.c` (sync), `asychLcdTest.c` (async queue).
