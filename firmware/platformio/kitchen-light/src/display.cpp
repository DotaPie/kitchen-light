// core includes
#include <WiFi.h>
#include <stdint.h>
#include <math.h>

// project includes
#include "display.h"
#include "pinout.h"
#include "macros.h"
#include "conf.h"
#include "utilities.h"
#include "console.h"
#include "kelvin2RGB.h"
#include "images.h"
#include "colors.h"

// lib includes
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <FastLED.h>
#include <Preferences.h>

Adafruit_ST7789 display = Adafruit_ST7789(DISPLAY_CS_PIN, DISPLAY_DC_PIN, DISPLAY_RST_PIN);

void setupDisplay()
{
    CONSOLE("Display: ")
    
    display.init(DISPLAY_WIDTH, DISPLAY_HEIGHT);       
    display.setRotation(DISPLAY_ROTATION_DEGREE/90);
    display.invertDisplay(false); 
    display.fillScreen(COLOR_RGB565_DISPLAY_BACKGROUND);
    display.setCursor(4, 4);
    display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
    display.setTextSize(2);
    display.setTextWrap(true);
    display.print("Please wait ..."); 

    delay(500);

    displayLedControl(true, false); // enabling display LED some time after init removes all the random flickering that is going on during display setup

    CONSOLE_CRLF("OK") 
}

void loadDisplayColorTemperature(uint16_t currentColorTemperatureIndex, uint16_t previousColorTemperatureIndex)
{
    for(uint16_t i = 0; i < PICKER_WIDTH; i++)
    {
        CRGB color = kelvin2RGB_lookupTable[map(i, 0, PICKER_WIDTH - 1, 0, (KELVIN2RGB_LOOKUP_TABLE_MAX_VALUE - KELVIN2RGB_LOOKUP_TABLE_MIN_VALUE) / KELVIN2RGB_LOOKUP_TABLE_STEP)];
        display.drawFastVLine(PICKER_OFFSET_X + i, PICKER_OFFSET_Y, PICKER_HEIGHT, RGB888_TO_RGB565(color.r, color.g, color.b));    
    }

    display.setCursor(PICKER_OFFSET_X, PICKER_OFFSET_Y - 30);
    display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
    display.setTextSize(2, 3);
    display.setTextWrap(false);
    display.print("COLOR TEMPERATURE");

    CONSOLE_CRLF("DISPLAY: COLOR TEMPERATURE LOADED")

    updateDisplayColorTemperature(currentColorTemperatureIndex, previousColorTemperatureIndex);
}

void loadDisplayColorHue(uint16_t currentColorHueIndex, uint16_t previousColorHueIndex)
{
    // R -> G
    for(uint16_t i = 0; i < PICKER_WIDTH * 1/6; i++)
    {
        display.drawFastVLine(PICKER_OFFSET_X + i, PICKER_OFFSET_Y, PICKER_HEIGHT, RGB888_TO_RGB565(255, map(i, 0, PICKER_WIDTH * 1/6 - 1, 0, 255), 0));
    }

    for(uint16_t i = PICKER_WIDTH * 1/6; i < PICKER_WIDTH * 2/6; i++)
    {
        display.drawFastVLine(PICKER_OFFSET_X + i, PICKER_OFFSET_Y, PICKER_HEIGHT, RGB888_TO_RGB565(map(i - PICKER_WIDTH * 1/6, 0, PICKER_WIDTH * 1/6 - 1, 255, 0), 255, 0)); 
    }

    // G -> B
    for(uint16_t i = PICKER_WIDTH * 2/6; i < PICKER_WIDTH * 3/6; i++)
    {
        display.drawFastVLine(PICKER_OFFSET_X + i, PICKER_OFFSET_Y, PICKER_HEIGHT, RGB888_TO_RGB565(0, 255, map(i - PICKER_WIDTH * 2/6, 0, PICKER_WIDTH * 1/6 - 1, 0, 255)));
    }

    for(uint16_t i = PICKER_WIDTH * 3/6; i < PICKER_WIDTH * 4/6; i++)
    {
        display.drawFastVLine(PICKER_OFFSET_X + i, PICKER_OFFSET_Y, PICKER_HEIGHT, RGB888_TO_RGB565(0, map(i - PICKER_WIDTH * 3/6, 0, PICKER_WIDTH * 1/6 - 1, 255, 0), 255)); 
    }

    // B -> R
    for(uint16_t i = PICKER_WIDTH * 4/6; i < PICKER_WIDTH * 5/6; i++)
    {
        display.drawFastVLine(PICKER_OFFSET_X + i, PICKER_OFFSET_Y, PICKER_HEIGHT, RGB888_TO_RGB565(map(i - PICKER_WIDTH * 4/6, 0, PICKER_WIDTH * 1/6 - 1, 0, 255), 0, 255));
    }

    for(uint16_t i = PICKER_WIDTH * 5/6; i < PICKER_WIDTH; i++)
    {
        display.drawFastVLine(PICKER_OFFSET_X + i, PICKER_OFFSET_Y, PICKER_HEIGHT, RGB888_TO_RGB565(255, 0, map(i - PICKER_WIDTH * 5/6, 0, PICKER_WIDTH * 1/6 - 1, 255, 0)));  
    }

    display.setCursor(PICKER_OFFSET_X, PICKER_OFFSET_Y - 30);
    display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
    display.setTextSize(2, 3);
    display.setTextWrap(false);
    display.print("COLOR HUE");

    CONSOLE_CRLF("DISPLAY: COLOR HUE LOADED")

    updateDisplayColorHue(currentColorHueIndex, previousColorHueIndex);
}

void loadDisplayBrightness(uint8_t brightness)
{
    display.drawRect(PICKER_OFFSET_X, PICKER_OFFSET_Y, PICKER_WIDTH, PICKER_HEIGHT, COLOR_RGB565_DISPLAY_FOREGROUND);
    display.drawRect(PICKER_OFFSET_X + 1, PICKER_OFFSET_Y + 1, PICKER_WIDTH - 2, PICKER_HEIGHT - 2, COLOR_RGB565_DISPLAY_FOREGROUND);

    display.setCursor(PICKER_OFFSET_X, PICKER_OFFSET_Y - 30);
    display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
    display.setTextSize(2, 3);
    display.setTextWrap(false);
    display.print("BRIGHTNESS");

    CONSOLE_CRLF("DISPLAY: BRIGHTNESS LOADED")

    updateDisplayBrightness(brightness);
}

void loadDisplayNumberOfLeds()
{
    clearDisplay();

    display.setCursor(5, 6);
    display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
    display.setTextSize(3, 3);
    display.setTextWrap(false);
    display.print("Number of LEDs\r\n");

    display.setCursor(5, display.height() - 60);
    display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
    display.setTextSize(2, 2);
    display.setTextWrap(false);
    display.print("+10/-10 -> left knob");

    display.setCursor(5, display.height() - 40);
    display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
    display.setTextSize(2, 2);
    display.setTextWrap(false);
    display.print("+1/-1 -> right knob");

    display.setCursor(5, display.height() - 20);
    display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
    display.setTextSize(2, 2);
    display.setTextWrap(false);
    display.print("Push any knob to accept");

    CONSOLE_CRLF("DISPLAY: NUMBER OF LEDS LOADED")
}

void updateDisplayNumberOfLeds(uint16_t numberOfLeds, bool valueLocked)
{
    display.fillRect(0, 60, display.width(), display.height() - (60 + 60), COLOR_RGB565_DISPLAY_BACKGROUND);
    display.setCursor(6, 60);
    
    if(valueLocked)
    {
        display.setTextColor(COLOR_RGB565_NUMBER_OF_LEDS_CONFIRMED);
    }
    else
    {
        display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
    }

    display.setTextSize(8, 12);
    display.setTextWrap(false); 

    display.print(numberOfLeds); 
}

CRGB calculateColorTemperatureFromPickerPosition(uint16_t pickerPosition)
{
    return kelvin2RGB_lookupTable[map(pickerPosition, 0, PICKER_WIDTH - 1, 0, (KELVIN2RGB_LOOKUP_TABLE_MAX_VALUE - KELVIN2RGB_LOOKUP_TABLE_MIN_VALUE) / KELVIN2RGB_LOOKUP_TABLE_STEP)];    
}

/* Unused by prepared if needed.
 */
uint16_t calculatePickerPositionFromColorHue(CRGB color)
{
    // R -> G
    if(color.b == 0)
    {
        if(color.r == 255)
        {
            return map(color.g, 0, 255, 0, PICKER_WIDTH * 1/6 - 1);
        } 
        else if(color.g == 255)
        {
            return map(color.r, 255, 0, PICKER_WIDTH * 1/6, PICKER_WIDTH * 2/6 - 1);    
        }   
    }
    // G -> B
    else if(color.r == 0)
    {
        if(color.g == 255)
        {
            return map(color.b, 0, 255, PICKER_WIDTH * 2/6, PICKER_WIDTH * 3/6 - 1);
        } 
        else if(color.b == 255)
        {
            return map(color.g, 255, 0, PICKER_WIDTH * 3/6, PICKER_WIDTH * 4/6 - 1);    
        }     
    }
    // B -> R
    else if(color.g == 0)
    {
        if(color.b == 255)
        {
            return map(color.r, 0, 255, PICKER_WIDTH * 4/6, PICKER_WIDTH * 5/6 - 1);
        } 
        else if(color.r == 255)
        {
            return map(color.b, 255, 0, PICKER_WIDTH * 5/6, PICKER_WIDTH - 1);    
        }      
    }

    return 0; // will never get here
}

CRGB calculateColorHueFromPickerPosition(uint16_t pickerPosition)
{
    CRGB color;

    // R -> G
    if(pickerPosition >= 0 && pickerPosition < PICKER_WIDTH * 2/6) 
    {
        color.b = 0;  

        if(pickerPosition >= 0 && pickerPosition < PICKER_WIDTH * 1/6)  
        {
            color.r = 255;
            color.g = map(pickerPosition, 0, PICKER_WIDTH * 1/6 - 1, 0, 255);
        }
        else if(pickerPosition >= PICKER_WIDTH * 1/6 && pickerPosition < PICKER_WIDTH * 2/6)
        {
            color.r = map(pickerPosition, PICKER_WIDTH * 1/6, PICKER_WIDTH * 2/6 - 1, 255, 0);
            color.g = 255;    
        }
    }  
    // G -> B
    else if(pickerPosition >= PICKER_WIDTH * 2/6 && pickerPosition < PICKER_WIDTH * 4/6)
    {
        color.r = 0; 

        if(pickerPosition >= PICKER_WIDTH * 2/6 && pickerPosition < PICKER_WIDTH * 3/6)  
        {
            color.g = 255;
            color.b = map(pickerPosition, PICKER_WIDTH * 2/6, PICKER_WIDTH * 3/6 - 1, 0, 255);   
        }
        else if(pickerPosition >= PICKER_WIDTH * 3/6 && pickerPosition < PICKER_WIDTH * 4/6)
        {
            color.g = map(pickerPosition, PICKER_WIDTH * 3/6, PICKER_WIDTH * 4/6 - 1, 255, 0);
            color.b = 255;  
        }
    } 
    // B -> R
    else if(pickerPosition >= PICKER_WIDTH * 4/6 && pickerPosition < PICKER_WIDTH)
    {
        color.g = 0;

        if(pickerPosition >= PICKER_WIDTH * 4/6 && pickerPosition < PICKER_WIDTH * 5/6)  
        {
            color.b = 255;
            color.r = map(pickerPosition, PICKER_WIDTH * 4/6, PICKER_WIDTH * 5/6 - 1, 0, 255); 
        }
        else if(pickerPosition >= PICKER_WIDTH * 5/6 && pickerPosition < PICKER_WIDTH)
        {
            color.b = map(pickerPosition, PICKER_WIDTH * 5/6, PICKER_WIDTH - 1, 255, 0);
            color.r = 255; 
        } 
    }     

    return color;   
}

void updateDisplayColorHue(uint16_t currentColorHueIndex, uint16_t previousColorHueIndex)
{
    display.fillTriangle(PICKER_OFFSET_X + previousColorHueIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2, (PICKER_OFFSET_X - 8) + previousColorHueIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2 + 15, (PICKER_OFFSET_X + 8) + previousColorHueIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2 + 15, COLOR_RGB565_DISPLAY_BACKGROUND);
    display.fillTriangle(PICKER_OFFSET_X + currentColorHueIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2, (PICKER_OFFSET_X - 8) + currentColorHueIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2 + 15, (PICKER_OFFSET_X + 8) + currentColorHueIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2 + 15, COLOR_RGB565_COLOR_PICKER_ARROW);
   
    CONSOLE_CRLF("DISPLAY: COLOR HUE UPDATED")
}

void updateDisplayColorTemperature(uint16_t currentColorTemperatureIndex, uint16_t previousColorTemperatureIndex)
{
    display.fillTriangle(PICKER_OFFSET_X + previousColorTemperatureIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2, (PICKER_OFFSET_X - 8) + previousColorTemperatureIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2 + 15, (PICKER_OFFSET_X + 8) + previousColorTemperatureIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2 + 15, COLOR_RGB565_DISPLAY_BACKGROUND);
    display.fillTriangle(PICKER_OFFSET_X + currentColorTemperatureIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2, (PICKER_OFFSET_X - 8) + currentColorTemperatureIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2 + 15, (PICKER_OFFSET_X + 8) + currentColorTemperatureIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2 + 15, COLOR_RGB565_COLOR_PICKER_ARROW);
   
    CONSOLE_CRLF("DISPLAY: COLOR TEMPERATURE UPDATED")
}

void updateDisplayBrightness(uint8_t brightness)
{
    uint16_t barWidth = map(brightness, 0, 255, 0, PICKER_WIDTH - 4);

    for(uint16_t i = 0; i < barWidth; i++)
    {
        display.drawFastVLine(PICKER_OFFSET_X + 2 + i, PICKER_OFFSET_Y + 2, PICKER_HEIGHT - 4, COLOR_RGB565_DISPLAY_FOREGROUND);
    } 

    for(uint16_t i = barWidth; i < PICKER_WIDTH - 4; i++)
    {
        display.drawFastVLine(PICKER_OFFSET_X + 2 + i, PICKER_OFFSET_Y + 2, PICKER_HEIGHT - 4, COLOR_RGB565_DISPLAY_BACKGROUND);
    } 

    CONSOLE_CRLF("DISPLAY: BRIGHTNESS UPDATED")
}

void drawWifiSignalUndefined()
{
    display.fillRect(display.width() - 64, 0, 64, 39, COLOR_RGB565_DISPLAY_BACKGROUND); 
}

void drawWifiSignalDisconnected()
{
    display.fillRect(display.width() - 64, 0, 64, 39, COLOR_RGB565_DISPLAY_BACKGROUND); 

    // dot
    display.fillCircle(display.width() - 22, 28, 3, COLOR_RGB565_WIFI_SIGNAL_EMPTY_RING);

    // 1st ring
    display.drawFastVLine(display.width() - 22, 18, 3, COLOR_RGB565_WIFI_SIGNAL_EMPTY_RING);
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 8, 3, COLOR_RGB565_WIFI_SIGNAL_EMPTY_RING);
    }
    display.endWrite();

    // 2nd ring
    display.drawFastVLine(display.width() - 22, 12, 3, COLOR_RGB565_WIFI_SIGNAL_EMPTY_RING);
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 14, 3, COLOR_RGB565_WIFI_SIGNAL_EMPTY_RING);
    }
    display.endWrite();

    // 3rd ring
    display.drawFastVLine(display.width() - 22, 6, 3, COLOR_RGB565_WIFI_SIGNAL_EMPTY_RING);
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 20, 3, COLOR_RGB565_WIFI_SIGNAL_EMPTY_RING);
    }
    display.endWrite();

    // remove excessive parts of rings
    display.fillTriangle(display.width() - 54, 0, display.width() - 54, 28, display.width() - 25, 28, COLOR_RGB565_DISPLAY_BACKGROUND);
    display.fillTriangle(display.width() + 10, 0, display.width() + 10, 28, display.width() - 19, 28, COLOR_RGB565_DISPLAY_BACKGROUND); 

    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawLine(display.width() - 44, i, display.width() - 1, 30 + i, COLOR_RGB565_WIFI_SIGNAL_CROSSED);
    }
}

void drawWifiSignalBad()
{
    display.fillRect(display.width() - 64, 0, 64, 39, COLOR_RGB565_DISPLAY_BACKGROUND); 

    // dot
    display.fillCircle(display.width() - 22, 28, 3, COLOR_RGB565_COLOR_PICKER_ARROW);

    // 1st ring
    display.drawFastVLine(display.width() - 22, 18, 3, COLOR_RGB565_COLOR_PICKER_ARROW);
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 8, 3, COLOR_RGB565_COLOR_PICKER_ARROW);
    }
    display.endWrite();

    // 2nd ring
    display.drawFastVLine(display.width() - 22, 12, 3, COLOR_RGB565_WIFI_SIGNAL_EMPTY_RING);
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 14, 3, COLOR_RGB565_WIFI_SIGNAL_EMPTY_RING);
    }
    display.endWrite();

    // 3rd ring
    display.drawFastVLine(display.width() - 22, 6, 3, COLOR_RGB565_WIFI_SIGNAL_EMPTY_RING);
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 20, 3, COLOR_RGB565_WIFI_SIGNAL_EMPTY_RING);
    }
    display.endWrite();

    // remove excessive parts of rings
    display.fillTriangle(display.width() - 54, 0, display.width() - 54, 28, display.width() - 25, 28, COLOR_RGB565_DISPLAY_BACKGROUND);
    display.fillTriangle(display.width() + 10, 0, display.width() + 10, 28, display.width() - 19, 28, COLOR_RGB565_DISPLAY_BACKGROUND); 
}

void drawWifiSignalGood()
{
    display.fillRect(display.width() - 64, 0, 64, 39, COLOR_RGB565_DISPLAY_BACKGROUND); 

    // dot
    display.fillCircle(display.width() - 22, 28, 3, COLOR_RGB565_WIFI_SIGNAL_GOOD);

    // 1st ring
    display.drawFastVLine(display.width() - 22, 18, 3, COLOR_RGB565_WIFI_SIGNAL_GOOD);
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 8, 3, COLOR_RGB565_WIFI_SIGNAL_GOOD);
    }
    display.endWrite();

    // 2nd ring
    display.drawFastVLine(display.width() - 22, 12, 3, COLOR_RGB565_WIFI_SIGNAL_GOOD);
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 14, 3, COLOR_RGB565_WIFI_SIGNAL_GOOD);
    }
    display.endWrite();

    // 3rd ring
    display.drawFastVLine(display.width() - 22, 6, 3, COLOR_RGB565_WIFI_SIGNAL_EMPTY_RING);
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 20, 3, COLOR_RGB565_WIFI_SIGNAL_EMPTY_RING);
    }
    display.endWrite();

    // remove excessive parts of rings
    display.fillTriangle(display.width() - 54, 0, display.width() - 54, 28, display.width() - 25, 28, COLOR_RGB565_DISPLAY_BACKGROUND);
    display.fillTriangle(display.width() + 10, 0, display.width() + 10, 28, display.width() - 19, 28, COLOR_RGB565_DISPLAY_BACKGROUND); 
}

void drawWifiSignalExcellent()
{
    display.fillRect(display.width() - 64, 0, 64, 39, COLOR_RGB565_DISPLAY_BACKGROUND); 

    // dot
    display.fillCircle(display.width() - 22, 28, 3, COLOR_RGB565_WIFI_SIGNAL_EXCELLENT);

    // 1st ring
    display.drawFastVLine(display.width() - 22, 18, 3, COLOR_RGB565_WIFI_SIGNAL_EXCELLENT);
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 8, 3, COLOR_RGB565_WIFI_SIGNAL_EXCELLENT);
    }
    display.endWrite();

    // 2nd ring
    display.drawFastVLine(display.width() - 22, 12, 3, COLOR_RGB565_WIFI_SIGNAL_EXCELLENT);
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 14, 3, COLOR_RGB565_WIFI_SIGNAL_EXCELLENT);
    }
    display.endWrite();

    // 3rd ring
    display.drawFastVLine(display.width() - 22, 6, 3, COLOR_RGB565_WIFI_SIGNAL_EXCELLENT);
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 20, 3, COLOR_RGB565_WIFI_SIGNAL_EXCELLENT);
    }
    display.endWrite();

    // remove excessive parts of rings
    display.fillTriangle(display.width() - 54, 0, display.width() - 54, 28, display.width() - 25, 28, COLOR_RGB565_DISPLAY_BACKGROUND);
    display.fillTriangle(display.width() + 10, 0, display.width() + 10, 28, display.width() - 19, 28, COLOR_RGB565_DISPLAY_BACKGROUND);
}

void updateWifiSignal(WifiSignal wifiSignal, bool internetConnection)
{
    switch(wifiSignal)
    {
        case WifiSignal::DISCONNECTED:
            drawWifiSignalDisconnected();
            break;

        case WifiSignal::BAD:
            drawWifiSignalBad();
            break;
        
        case WifiSignal::GOOD:
            drawWifiSignalGood();
            break;
        
        case WifiSignal::EXCELLENT:
            drawWifiSignalExcellent();
            break;

        default:
            drawWifiSignalUndefined();
            break;
    }

    // cross out wifi signal as in WifiSignal::DISCONNECTED when no internet
    if(!internetConnection && (wifiSignal == WifiSignal::BAD || wifiSignal == WifiSignal::GOOD || wifiSignal == WifiSignal::EXCELLENT))
    {
        for(uint8_t i = 0; i < 3; i++)
        {
            display.drawLine(display.width() - 44, i, display.width() - 1, 30 + i, COLOR_RGB565_WIFI_SIGNAL_CROSSED);
        }      
    }
}

void updateHour(uint8_t hour, bool invalid = false)
{
    if(!invalid)
    {
        display.fillRect(0, 60, 140, 120, COLOR_RGB565_DISPLAY_BACKGROUND);
        display.setCursor(6, 60);
        display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
        display.setTextSize(12, 17);
        display.setTextWrap(false);

        if(hour < 10)
        {
            display.print('0');
        }

        display.print(hour);  
    }
}

void drawDoubledot(bool visible)
{
    display.fillRect(140, 60, 40, 120, COLOR_RGB565_DISPLAY_BACKGROUND);

    if(visible)
    {
        display.setCursor(130, 60);
        display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
        display.setTextSize(12, 17); 
        display.setTextWrap(false);
        display.print(':'); 
    }
}

void updateMinute(uint8_t minute, bool invalid = false)
{
    if(!invalid)
    {
        display.fillRect(180, 60, 140, 120, COLOR_RGB565_DISPLAY_BACKGROUND);
        display.setCursor(182, 60);
        display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
        display.setTextSize(12, 17);
        display.setTextWrap(false);

        if(minute < 10)
        {
            display.print('0');
        }

        display.print(minute);
    }
}

void updateTemperature(float temperature, bool invalid = false)
{
    display.fillRect(0, display.height() - 39, display.width() * 1/4 - 22, 39, COLOR_RGB565_DISPLAY_BACKGROUND);
    display.setCursor(5, display.height() - 26);
    display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
    display.setTextSize(3, 3);
    display.setTextWrap(false);

    if(!invalid)
    {
        display.print((int32_t)round(temperature));
    }
}

void updateDate(uint8_t day, uint8_t month, uint16_t year, bool invalid = false)
{
    display.fillRect(0, 0, display.width() - 64, 40, COLOR_RGB565_DISPLAY_BACKGROUND);
    display.setCursor(5, 6);
    display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
    display.setTextSize(3, 3);
    display.setTextWrap(false);

    if(!invalid)
    {
        display.print(day);
        display.print(' ');   
        display.print(monthNames[month]);
        display.print(' '); 
        display.print(year); 
    } 
}

void updateHumidity(uint8_t humidity, bool invalid = false)
{
    display.fillRect(display.width() * 1/4 + 1, display.height() - 39, display.width() * 1/4 - 22, 39, COLOR_RGB565_DISPLAY_BACKGROUND);
    display.setCursor(display.width() * 1/4 + 6, display.height() - 26);
    display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
    display.setTextSize(3, 3);
    display.setTextWrap(false);
    
    if(!invalid)
    {
        display.print(humidity); 
    }
}

void updateWindSpeed(float windGust, bool invalid = false)
{
    display.fillRect(display.width() * 2/4 + 1, display.height() - 39, display.width() * 1/4 - 22, 39, COLOR_RGB565_DISPLAY_BACKGROUND);
    display.setCursor(display.width() * 2/4 + 6, display.height() - 26);
    display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
    display.setTextSize(3, 3);
    display.setTextWrap(false);

    if(!invalid)
    {
        display.print((uint32_t)round(windGust));
    }
}

/* Draw RGB565 picture from uint8 *array with added option of filtering out completely one color.
 * For example, if color of pixel is black, dont draw pixel at all. 
 * This will allow custom background color defined in code.
 */
void drawRGB565_filtered(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *image, uint16_t filteredColor)
{
    display.startWrite();
    
    for (int16_t i = 0; i < h; i++, y++) 
    {
        for (int16_t j = 0; j < w; j++) 
        {
            if(image[i * w + j] != filteredColor)
            {
                display.writePixel(x + j, y, image[i * w + j]);
            }
        }
    }

    display.endWrite();  
}

void updateWeather(Weather weather, bool invalid = false)
{
    display.fillRect(display.width() * 3/4 + 1, display.height() - 39, display.width() - 1, 39, COLOR_RGB565_DISPLAY_BACKGROUND); 

    if(!invalid)
    {
        switch(weather)
        {
            case Weather::CLEAR_SKY_DAY:
                drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_01d, COLOR_RGB565_IGNORE_IN_BMP_PICTURES);
                break;

            case Weather::CLEAR_SKY_NIGHT:
                drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_01n, COLOR_RGB565_IGNORE_IN_BMP_PICTURES);
                break;
            
            case Weather::FEW_CLOUDS_DAY:
                drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_02d, COLOR_RGB565_IGNORE_IN_BMP_PICTURES);
                break;

            case Weather::FEW_CLOUDS_NIGHT:
                drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_02n, COLOR_RGB565_IGNORE_IN_BMP_PICTURES);
                break;
                
            case Weather::SCATTERED_CLOUDS_DAY:
                drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_03d, COLOR_RGB565_IGNORE_IN_BMP_PICTURES);
                break;

            case Weather::SCATTERED_CLOUDS_NIGHT:
                drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_03n, COLOR_RGB565_IGNORE_IN_BMP_PICTURES);
                break;
                
            case Weather::BROKEN_CLOUDS_DAY:
                drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_04d, COLOR_RGB565_IGNORE_IN_BMP_PICTURES);
                break;

            case Weather::BROKEN_CLOUDS_NIGHT:
                drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_04n, COLOR_RGB565_IGNORE_IN_BMP_PICTURES);
                break;
                
            case Weather::SHOWER_RAIN_DAY:
                drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_09d, COLOR_RGB565_IGNORE_IN_BMP_PICTURES);
                break;

            case Weather::SHOWER_RAIN_NIGHT:
                drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_09n, COLOR_RGB565_IGNORE_IN_BMP_PICTURES);
                break;
                
            case Weather::RAIN_DAY:
                drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_10d, COLOR_RGB565_IGNORE_IN_BMP_PICTURES);
                break;

            case Weather::RAIN_NIGHT:
                drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_10n, COLOR_RGB565_IGNORE_IN_BMP_PICTURES);
                break;
                
            case Weather::THUNDERSTORM_DAY:
                drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_11d, COLOR_RGB565_IGNORE_IN_BMP_PICTURES);
                break;

            case Weather::THUNDERSTORM_NIGHT:
                drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_11n, COLOR_RGB565_IGNORE_IN_BMP_PICTURES);
                break;
                
            case Weather::SNOW_DAY:
                drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_13d, COLOR_RGB565_IGNORE_IN_BMP_PICTURES);
                break;

            case Weather::SNOW_NIGHT:
                drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_13n, COLOR_RGB565_IGNORE_IN_BMP_PICTURES);
                break;
                
            case Weather::MIST_DAY:
                drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_50d, COLOR_RGB565_IGNORE_IN_BMP_PICTURES);
                break;

            case Weather::MIST_NIGHT:
                drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_50n, COLOR_RGB565_IGNORE_IN_BMP_PICTURES);
                break;

            default:
                break;
        }
    }
}

void drawFixedParts()
{
    display.drawFastHLine(0, display.height() - 40, display.width(), COLOR_RGB565_DISPLAY_FOREGROUND);
    display.drawFastHLine(0, 40, display.width(), COLOR_RGB565_DISPLAY_FOREGROUND);
    display.drawFastVLine(display.width() * 1/4, display.height() - 40, 40, COLOR_RGB565_DISPLAY_FOREGROUND);
    display.drawFastVLine(display.width() * 2/4, display.height() - 40, 40, COLOR_RGB565_DISPLAY_FOREGROUND);
    display.drawFastVLine(display.width() * 3/4, display.height() - 40, 40, COLOR_RGB565_DISPLAY_FOREGROUND);

    // °C
    display.setCursor(display.width() * 1/4 - 13, display.height() - 36);
    display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
    display.setTextSize(2, 2);
    display.setTextWrap(false);
    display.print("C");

    display.fillCircle(display.width() * 1/4 - 18, display.height() - 32, 2, COLOR_RGB565_DISPLAY_FOREGROUND);
    display.fillCircle(display.width() * 1/4 - 18, display.height() - 32, 1, COLOR_RGB565_DISPLAY_BACKGROUND);

    // %
    display.setCursor(display.width() * 2/4 - 13, display.height() - 36);
    display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
    display.setTextSize(2, 2);
    display.setTextWrap(false);
    display.print("%");

    // m/s
    display.setCursor(display.width() * 3/4 - 13, display.height() - 38);
    display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
    display.setTextSize(2, 2);
    display.setTextWrap(false);
    display.print("m");

    display.drawFastHLine(display.width() * 3/4 - 13, display.height() - 22, 11, COLOR_RGB565_DISPLAY_FOREGROUND);

    display.setCursor(display.width() * 3/4 - 13, display.height() - 22);
    display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
    display.setTextSize(2, 2);
    display.setTextWrap(false);
    display.print("s");
}

void drawSetupText()
{
    char buff[256] = "";
    
    display.fillRect(0, 41, display.width(), display.height() - 82, COLOR_RGB565_DISPLAY_BACKGROUND);
    display.setCursor(0, 47);
    display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
    display.setTextSize(2, 2);
    display.setTextWrap(true);

    char tempIP[32] = "";
    WiFi.softAPIP().toString().toCharArray(tempIP, 32);

    sprintf(buff, "1) Connect the the Wi-Fi:\r\n  -> %s\r\n\r\n2) Enter password:\r\n  -> %s\r\n\r\n3) Enter to browser:\r\n  -> http://%s", defaultSoftAP_ssid, defaultSoftAP_pwd, tempIP);   
    display.print(buff);        
}

void drawOfflineMode()
{
    display.fillRect(0, 41, display.width(), display.height() - 82, COLOR_RGB565_DISPLAY_BACKGROUND);
    display.setCursor(0, 47);
    display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
    display.setTextSize(2, 2);
    display.setTextWrap(true);
    display.print("Offline mode enabled.\r\n\r\n");  
    display.print("Reset device to enter\r\nconfiguration mode again.\r\n"); 
}

void loadAndExecuteFactoryReset(Preferences *preferences)
{
    display.drawRect(PICKER_OFFSET_X, PICKER_OFFSET_Y, PICKER_WIDTH, PICKER_HEIGHT, COLOR_RGB565_DISPLAY_FOREGROUND);
    display.drawRect(PICKER_OFFSET_X + 1, PICKER_OFFSET_Y + 1, PICKER_WIDTH - 2, PICKER_HEIGHT - 2, COLOR_RGB565_DISPLAY_FOREGROUND);

    display.setCursor(PICKER_OFFSET_X, PICKER_OFFSET_Y - 30);
    display.setTextColor(COLOR_RGB565_DISPLAY_FOREGROUND);
    display.setTextSize(2, 3);
    display.setTextWrap(false);
    display.print("FACTORY RESET");

    CONSOLE_CRLF("DISPLAY: FACTORY RESET LOADED")

    for(uint16_t i = 0; i < PICKER_WIDTH - 4; i++)
    {
        uint32_t timer = millis();

        // if we decide to interrupt factory reset
        if(digitalRead(RE_1_SW_PIN) == HIGH || digitalRead(RE_2_SW_PIN) == HIGH)
        {
            return;
        }

        display.drawFastVLine(PICKER_OFFSET_X + 2 + i, PICKER_OFFSET_Y + 2, PICKER_HEIGHT - 4, COLOR_RGB565_DISPLAY_FOREGROUND);

        delay((FACTORY_RESET_TIMEOUT_MS / PICKER_WIDTH) - (millis() - timer));  
    }

    clearDisplay();

    // force factory reset by modifying first run value
    uint32_t tempVal = preferences->getUInt("firstRun", DEFAULT_PREFERENCES_ID);
    tempVal += 1;
    preferences->putUInt("firstRun", tempVal);

    CONSOLE_CRLF("ESP32: RESTART") 
    ESP.restart();
}

void updateMainScreen(bool internetConnection, bool offlineMode, bool validWifiSetup, bool validWeather, bool validDateTime, bool forceAll, uint8_t hour, uint8_t minute, uint8_t day, uint8_t month, uint16_t year, float temperature, uint8_t humidity, float windSpeed, Weather weather, WifiSignal wifiSignal)
{
    // track previous values
    static uint8_t prevHour = 255; 
    static uint8_t prevMinute = 255;
    static uint8_t prevDay = 255;
    static uint8_t prevMonth = 255;
    static uint16_t prevYear = 9999;
    static float prevTemperature = -273.15;  
    static uint8_t prevHumidity = 255;
    static float prevWindSpeed = -1.0;
    static WifiSignal prevWifiSignal = WifiSignal::NONE;
    static Weather prevWeather = Weather::NONE;

    static bool previousValidWifiConnection = !validWifiSetup;
    static bool previousValidWeather = !validWeather;
    static bool previousValidDateTime = !validDateTime;
    static bool previousOfflineMode = !offlineMode;
    static bool previousInternetConnection = !internetConnection;
    
    CONSOLE_CRLF("DISPLAY: MAIN UPDATED")

    if(forceAll)
    {
        drawFixedParts();

        if(validWifiSetup && validDateTime)
        {
            drawDoubledot(true); 
        }
    }

    if((previousValidWifiConnection || forceAll) && !validWifiSetup && !offlineMode)
    {
        drawSetupText();
    }

    if((!previousOfflineMode || forceAll) && offlineMode)
    {
        drawOfflineMode();
    }
    
    if(wifiSignal != prevWifiSignal || internetConnection != previousInternetConnection || forceAll)
    {
        updateWifiSignal(wifiSignal, internetConnection);
        prevWifiSignal = wifiSignal;
        CONSOLE_CRLF("  |-- WIFI SIGNAL UPDATED")
    } 
    
    if((hour != prevHour || forceAll) && validDateTime)
    {
        updateHour(hour);
        prevHour = hour;
        CONSOLE_CRLF("  |-- HOUR UPDATED")
    } 
    else if((previousValidDateTime || forceAll) && !validDateTime)
    {
        updateHour(hour, true); 
        CONSOLE_CRLF("  |-- HOUR UPDATED (INVALID)")      
    }
    
    if((minute != prevMinute || forceAll) && validDateTime)
    {
        updateMinute(minute);
        prevMinute = minute;
        CONSOLE_CRLF("  |-- MINUTE UPDATED")
    } 
    else if((previousValidDateTime || forceAll) && !validDateTime)
    {
        updateMinute(minute, true); 
        CONSOLE_CRLF("  |-- MINUTE UPDATED (INVALID)")      
    }

    if((temperature != prevTemperature || forceAll) && validWeather)
    {
        updateTemperature(temperature);
        prevTemperature = temperature;
        CONSOLE_CRLF("  |-- TEMPERATURE UPDATED")
    }
    else if((previousValidWeather || forceAll) && !validWeather)
    {
        updateTemperature(temperature, true); 
        CONSOLE_CRLF("  |-- TEMPERATURE UPDATED (INVALID)")      
    }

    if((humidity != prevHumidity || forceAll) && validWeather)
    {
        updateHumidity(humidity);
        prevHumidity = humidity;
        CONSOLE_CRLF("  |-- HUMIDITY UPDATED")
    }
    else if((previousValidWeather || forceAll) && !validWeather)
    {
        updateHumidity(humidity, true); 
        CONSOLE_CRLF("  |-- HUMIDITY UPDATED (INVALID)")      
    }

    if((windSpeed != prevWindSpeed || forceAll) && validWeather)
    {
        updateWindSpeed(windSpeed);
        prevWindSpeed = windSpeed;
        CONSOLE_CRLF("  |-- WIND SPEED UPDATED")
    }
    else if((previousValidWeather || forceAll) && !validWeather)
    {
        updateWindSpeed(windSpeed, true); 
        CONSOLE_CRLF("  |-- WIND SPEED UPDATED (INVALID)")      
    }

    if((weather != prevWeather || forceAll) && validWeather)
    {
        updateWeather(weather);
        prevWeather = weather;
        CONSOLE_CRLF("  |-- WEATHER UPDATED")

    }
    else if((previousValidWeather || forceAll) && !validWeather)
    {
        updateWeather(weather, true);
        CONSOLE_CRLF("  |-- WEATHER UPDATED (INVALID)")      
    }

    if((day != prevDay || month != prevMonth || year != prevYear || forceAll) && validDateTime)
    {
        updateDate(day, month, year);
        prevDay = day;
        prevMonth = month;
        prevYear = year;
        CONSOLE_CRLF("  |-- DATE UPDATED")
    }
    else if((previousValidDateTime || forceAll) && !validDateTime)
    {
        updateDate(day, month, year, true); 
        CONSOLE_CRLF("  |-- DATE UPDATED (INVALID)")      
    }

    if(previousValidWifiConnection != validWifiSetup)
    {
        previousValidWifiConnection = validWifiSetup;  
    }

    if(previousValidDateTime != validDateTime)
    {
        previousValidDateTime = validDateTime;
    }

    if(previousValidWeather != validWeather)
    {
        previousValidWeather = validWeather;
    }

    if(previousOfflineMode != offlineMode)
    {
        previousOfflineMode = offlineMode;
    }

    if(previousInternetConnection != internetConnection)
    {
        previousInternetConnection = internetConnection;
    }
}

void clearDisplay()
{
    display.fillScreen(COLOR_RGB565_DISPLAY_BACKGROUND);
    CONSOLE_CRLF("DISPLAY: CLEARED")
}

void displayLedControl(bool pinVal, bool setupPin)
{
    if(setupPin)
    {
        pinMode(DISPLAY_LED_PIN, OUTPUT);
    }

    digitalWrite(DISPLAY_LED_PIN, pinVal ? HIGH : LOW);
}