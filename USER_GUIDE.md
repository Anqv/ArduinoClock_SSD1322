# User Guide – ArduinoClock SSD1322

A digital clock for Arduino Mega 2560 with a 256×64 OLED display.

---

## What you need

| Item | Details |
|---|---|
| Microcontroller | Arduino Mega 2560 |
| Display | 256×64 OLED, SSD1322 controller, 4-wire SPI |
| Library | **U8g2** by olikraus (install via Arduino Library Manager) |
| Power | 5 V USB or DC barrel jack for the Arduino; check your display module for its supply voltage (usually 3.3 V) |

---

## Wiring

| Arduino Pin | Display Pin | Signal |
|---|---|---|
| 47 | CS | Chip Select |
| 48 | DC | Data / Command |
| 49 | RES | Reset |
| 50 | SDA | Data (MOSI) |
| 51 | SCK | Clock |
| GND | GND | Ground |
| 3.3 V* | VCC | Power |

*Verify your specific display module's supply voltage before connecting.

---

## Installing the library

1. Open Arduino IDE.
2. Go to **Sketch → Include Library → Manage Libraries…**
3. Search for **U8g2**.
4. Install **U8g2 by olikraus**.

---

## Uploading the sketch

1. Open `ArduinoClock_SSD1322.ino` in the Arduino IDE.
2. Select **Tools → Board → Arduino Mega or Mega 2560**.
3. Select the correct **Port** under Tools.
4. Click **Upload**.

The clock starts at **12:00:00** and begins counting immediately.

---

## Screen layout

```
┌─────────────────────────────────────┐
│                                     │
│          12 : 00 : 45               │  ← large 7-segment digits
│                                     │
│ ███████████░░░░░░░░░░░░░░░░░░░░░░░░ │  ← seconds progress bar
└─────────────────────────────────────┘
```

- **Digits** — hours, minutes, seconds in 7-segment style, centred on screen.
- **Progress bar** — fills left-to-right once per minute; full width = second 59.

---

## Setting the time

The clock does not have a battery-backed RTC, so the time resets to 12:00:00 on every power cycle. You can set the correct time over USB:

1. Open **Tools → Serial Monitor** in the Arduino IDE.
2. Set baud rate to **9600**.
3. Type the time as six digits — **HHMMSS** — and press **Enter**.

**Examples**

| You type | Clock is set to |
|---|---|
| `130000` | 13:00:00 |
| `083015` | 08:30:15 |
| `235959` | 23:59:59 |

The Serial Monitor will confirm with a message such as `Time set to 13:00:00`.

---

## Accuracy

Timekeeping uses the Arduino's 16 MHz crystal oscillator via Timer 1.  
Typical crystal accuracy is ±20–50 ppm, which equals roughly ±1–4 seconds per day.  
For higher accuracy, add a DS3231 RTC module and read the time from it on startup.

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| Display stays blank | Wrong wiring or supply voltage | Re-check the wiring table above |
| Display shows garbage | SPI clock too fast or wrong phase | Ensure you are using software SPI (pins 50/51 as wired) |
| Sketch won't compile | U8g2 not installed | Install via Library Manager |
| Time drifts noticeably | Crystal tolerance | Calibrate OCR1A or add an RTC module |
