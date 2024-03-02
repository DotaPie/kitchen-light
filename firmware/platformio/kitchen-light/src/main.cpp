// includes
#include <Arduino.h>
#include "console.h"
#include "pinout.h"
#include "conf.h"
#include <RotaryEncoder.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <FastLED.h>

// enums
enum STATE {ERROR, SETUP};

// globals
STATE state = SETUP;
const char* state_string[] = {"ERROR", "SETUP"};
CRGB LED_strip_array[LED_STRIP_COUNT];

// function definitions
void setup_LED_strip();

void setup()
{
    delay(DELAY_BEFORE_STARTUP_MS);
    CONSOLE_CRLF("SETUP")

    setup_LED_strip();
}

void loop() 
{
    // TODO
}

void setup_LED_strip()
{
    CONSOLE("  |-- LED strip: ")
    FastLED.addLeds<LED_STRIP_TYPE, LED_STRIP_PIN, COLOR_ORDER>(LED_strip_array, LED_STRIP_COUNT).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(DEFAULT_BRIGHTNESS);
    CONSOLE_CRLF("OK")
}