# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Arduino Mega 2560 clock displaying HH:MM:SS on a 256×64 px SSD1322 OLED via the U8g2 library.

## Hardware

| Arduino Pin | SSD1322 | Note |
|---|---|---|
| 47 | CS | Chip select, active-low |
| 48 | DC | Data / command |
| 49 | RES | Reset, active-low |
| 50 | SDA | MOSI (software SPI — does NOT match hardware SPI MOSI on pin 51) |
| 51 | SCK | Clock (software SPI) |

Target MCU: ATmega2560 @ 16 MHz external crystal.

## Architecture

Single-file Arduino sketch (`ArduinoClock_SSD1322.ino`). No build system beyond the Arduino IDE / CLI.

**Timekeeping** — Timer 1 CTC interrupt fires at exactly 1 Hz:
- Prescaler 256, `OCR1A = 62499` → `16 000 000 / 256 / 62500 = 1.000 000 Hz`
- ISR increments `g_h / g_m / g_s` (all `volatile`) and sets `g_tick = true`
- `loop()` snapshots the three counters atomically with `cli()/sei()` before redrawing

**Display** — U8g2 full-framebuffer (`_F_`) software-SPI constructor:
```cpp
U8G2_SSD1322_NHD_256X64_F_4W_SW_SPI u8g2(U8G2_R0, /*SCK*/51, /*SDA*/50, /*CS*/47, /*DC*/48, /*RST*/49);
```
- Font: `u8g2_font_7Segments_26x42_mn` (capital S — lowercase causes a compile error)
- String centred with `u8g2.getStrWidth()` each refresh
- Seconds progress bar drawn as a filled box in the bottom 4 rows

**Serial time-set** — non-blocking, accumulates digits in a static buffer; accepts `HHMMSS\n` at 9600 baud.

## Key constraints

- Pins 50/51 do **not** match the Mega's hardware SPI port (MOSI=51, SCK=52), so the display must always use `4W_SW_SPI`, never `4W_HW_SPI`.
- U8g2 framebuffer for 256×64 mono = 2 048 bytes; the Mega has 8 KB SRAM total — keep allocations small.
- The only 7-segment font present in this version of U8g2 is `u8g2_font_7Segments_26x42_mn`.

## Workflow

Compile and upload via Arduino IDE or Arduino CLI:
```bash
arduino-cli compile --fqbn arduino:avr:mega ArduinoClock_SSD1322
arduino-cli upload  --fqbn arduino:avr:mega --port <COMx> ArduinoClock_SSD1322
```

After every meaningful change: commit and push so there is always a working version to revert to.
```bash
git add ArduinoClock_SSD1322.ino
git commit -m "short description"
git push
```

GitHub repository: https://github.com/Anqv/ArduinoClock_SSD1322
