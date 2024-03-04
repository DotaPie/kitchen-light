#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <FastLED.h>

enum MOVEMENT_DIRECTION {DIRECTION_LEFT, DIRECTION_RIGHT};

extern Adafruit_ST7789 display;

void loadPickerColorHue();
void loadPickerColorTemperature();
void loadBrightness();
CRGB updateDisplayColorHue(uint16_t index, MOVEMENT_DIRECTION movementDirection);
CRGB updateDisplayColorTemperature(uint16_t index, MOVEMENT_DIRECTION movementDirection);
void updateBrightness();

#endif