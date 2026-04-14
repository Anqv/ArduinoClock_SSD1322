/*
 ******************************************************************************
 *  SSD1322 OLED Clock  –  Arduino Mega 2560
 *  Library  : U8g2  (install via Library Manager: "U8g2 by olikraus")
 *  Display  : 256 × 64 px, monochrome
 *  Timing   : Timer 1 CTC interrupt, exactly 1 Hz  (16 MHz crystal)
 *
 *  Wiring
 *  ──────────────────────────────────────────────
 *  Arduino Pin   SSD1322    Function
 *       47         CS       Chip Select  (active-low)
 *       48         DC       Data / Command
 *       49         RES      Reset        (active-low)
 *       50         SDA      Serial Data  (MOSI)
 *       51         SCK      Serial Clock
 *       GND        GND
 *       3.3 V      VCC      (verify your module's supply voltage)
 *  ──────────────────────────────────────────────
 *
 *  Screen layout
 *  ──────────────────────────────────────────────
 *    ┌────────────────────────────────────┐
 *    │                                    │
 *    │         HH : MM : SS               │  ← large 38 px font, centred
 *    │                                    │
 *    │ ████████░░░░░░░░░░░░░░░░░░░░░░░░░░ │  ← 4 px seconds progress bar
 *    └────────────────────────────────────┘
 *
 *  Optional time-set via Serial (9600 baud):
 *    Send 6 ASCII digits + Enter, e.g.  "133045"  → 13:30:45
 ******************************************************************************
 */

#include <U8g2lib.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// ─── U8g2 constructor ────────────────────────────────────────────────────────
// 4-wire software SPI: (rotation, SCK, SDA/MOSI, CS, DC, RST)
// Pins 50/51 do not match the Mega's hardware SPI port, so SW_SPI is used.
U8G2_SSD1322_NHD_256X64_F_4W_SW_SPI u8g2(
    U8G2_R0,
    /* SCK */ 51,
    /* SDA */ 50,
    /* CS  */ 47,
    /* DC  */ 48,
    /* RST */ 49
);

// ─── Time state (updated by ISR) ─────────────────────────────────────────────
volatile uint8_t g_h    = 12;
volatile uint8_t g_m    =  0;
volatile uint8_t g_s    =  0;
volatile bool    g_tick = false;

// ─── Timer 1 CTC – exactly 1 Hz ──────────────────────────────────────────────
// F_CPU=16 MHz, prescaler=256, OCR1A=62499  → 16e6/256/62500 = 1.000000 Hz
ISR(TIMER1_COMPA_vect)
{
    if (++g_s >= 60) { g_s = 0;
        if (++g_m >= 60) { g_m = 0;
            if (++g_h >= 24) g_h = 0; } }
    g_tick = true;
}

static void timerInit()
{
    TCCR1A = 0;
    TCCR1B = _BV(WGM12) | _BV(CS12);   // CTC, prescaler /256
    OCR1A  = 62499;
    TIMSK1 = _BV(OCIE1A);
}

// ─── Render clock face and push to display ────────────────────────────────────
static void drawClock(uint8_t h, uint8_t m, uint8_t s)
{
    char buf[9];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);

    u8g2.clearBuffer();

    // ── Time string – 7-segment style font (26 × 42 px per digit) ──
    u8g2.setFont(u8g2_font_7Segments_26x42_mn);
    int16_t strW = u8g2.getStrWidth(buf);
    int16_t x    = (256 - strW) / 2;   // centre horizontally
    u8g2.drawStr(x, 52, buf);          // baseline at y=52 → top ≈ y=14

    // ── Seconds progress bar (bottom 4 rows) ──
    // Fills left-to-right: 1 full width at s=59
    uint16_t barW = (uint16_t)(s + 1) * 256 / 60;
    u8g2.drawBox(0, 60, barW, 4);

    u8g2.sendBuffer();
}

// ─── Non-blocking serial time-set ────────────────────────────────────────────
// Protocol: type HHMMSS and press Enter, e.g. "130045" → 13:00:45
static void checkSerial()
{
    static char    buf[6];
    static uint8_t pos = 0;

    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (pos == 6) {
                uint8_t nh = (buf[0]-'0')*10 + (buf[1]-'0');
                uint8_t nm = (buf[2]-'0')*10 + (buf[3]-'0');
                uint8_t ns = (buf[4]-'0')*10 + (buf[5]-'0');
                if (nh < 24 && nm < 60 && ns < 60) {
                    cli(); g_h = nh; g_m = nm; g_s = ns; sei();
                    Serial.print(F("Time set to "));
                    if (nh < 10) Serial.print('0'); Serial.print(nh); Serial.print(':');
                    if (nm < 10) Serial.print('0'); Serial.print(nm); Serial.print(':');
                    if (ns < 10) Serial.print('0'); Serial.println(ns);
                } else {
                    Serial.println(F("Invalid – use HHMMSS, e.g. 133045"));
                }
            }
            pos = 0;
        } else if (c >= '0' && c <= '9' && pos < 6) {
            buf[pos++] = c;
        } else {
            pos = 0;   // unexpected character – reset buffer
        }
    }
}

// ─── Arduino entry points ─────────────────────────────────────────────────────
void setup()
{
    Serial.begin(9600);
    Serial.println(F("SSD1322 Clock  –  send HHMMSS<Enter> to set time"));

    u8g2.begin();
    timerInit();

    drawClock(g_h, g_m, g_s);
}

void loop()
{
    checkSerial();

    if (g_tick) {
        g_tick = false;
        cli();
        uint8_t h = g_h, m = g_m, s = g_s;
        sei();
        drawClock(h, m, s);
    }
}
