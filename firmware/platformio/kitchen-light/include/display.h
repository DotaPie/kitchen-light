#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <FastLED.h>
#include "utilities.h"
#include <Preferences.h>

void setupDisplay();
void showPleaseWaitOnDisplay();
void loadDisplayNumberOfLeds();
void updateDisplayNumberOfLeds(uint16_t numberOfLeds, bool valueLocked);
CRGB calculateColorTemperatureFromPickerPosition(uint16_t pickerPosition);
CRGB calculateColorHueFromPickerPosition(uint16_t pickerPosition);
uint16_t calculatePickerPositionFromColorHue(CRGB color);
void loadDisplayBrightness(uint8_t brightness);
void updateDisplayBrightness(uint8_t brightness);
void loadDisplayColorHue(uint16_t currentColorHueIndex, uint16_t previousColorHueIndex);
void updateDisplayColorHue(uint16_t currentColorHueIndex, uint16_t previousColorHueIndex);
void loadDisplayColorTemperature(uint16_t currentColorTemperatureIndex, uint16_t previousColorTemperatureIndex);
void updateDisplayColorTemperature(uint16_t currentColorTemperatureIndex, uint16_t previousColorTemperatureIndex);
void loadAndExecuteFactoryReset(Preferences *preferences);
void updateMainScreen(bool internetConnection, bool offlineMode, bool validWifiSetup, bool validWeather, bool validDateTime, bool forceAll, uint8_t hour, uint8_t minute, uint8_t day, uint8_t month, uint16_t year, float temperature, uint8_t humidity, float windSpeed, Weather weather, WifiSignal wifiSingal);
void clearDisplay();
void displayLedControl(bool pinVal, bool setupPin);

#endif