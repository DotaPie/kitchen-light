#include "display.h"
#include <stdint.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include "pinout.h"
#include "macros.h"
#include "conf.h"
#include "utilities.h"
#include "console.h"
#include <FastLED.h>
#include <math.h>
#include <WiFi.h>
#include <Preferences.h>
#include "kelvin2RGB.h"
#include "images.h"

Adafruit_ST7789 display = Adafruit_ST7789(DISPLAY_CS_PIN, DISPLAY_DC_PIN, DISPLAY_RST_PIN);

void setupDisplay()
{
    CONSOLE("Display: ")
    display.init(DISPLAY_WIDTH, DISPLAY_HEIGHT);       
    display.setRotation(DISPLAY_ROTATION_DEGREE/90);
    display.invertDisplay(false); 
    display.fillScreen(ST77XX_BLACK);
    
    CONSOLE_CRLF("OK") 
}

void showPleaseWaitOnDisplay()
{
    clearDisplay();

    display.setCursor(4, 4);
    display.setTextColor(ST77XX_WHITE);
    display.setTextSize(2);
    display.setTextWrap(true);
    display.print("Please wait ...");    
}

void loadDisplayColorTemperature(uint16_t currentColorTemperatureIndex, uint16_t previousColorTemperatureIndex)
{
    for(uint16_t i = 0; i < PICKER_WIDTH; i++)
    {
        CRGB color = kelvin2RGB_lookupTable[map(i, 0, PICKER_WIDTH - 1, 0, (KELVIN2RGB_LOOKUP_TABLE_MAX_VALUE - KELVIN2RGB_LOOKUP_TABLE_MIN_VALUE) / KELVIN2RGB_LOOKUP_TABLE_STEP)];
        display.drawFastVLine(PICKER_OFFSET_X + i, PICKER_OFFSET_Y, PICKER_HEIGHT, RGB888_TO_RGB565(color.r, color.g, color.b));    
    }

    display.setCursor(PICKER_OFFSET_X, PICKER_OFFSET_Y - 30);
    display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
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
    display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
    display.setTextSize(2, 3);
    display.setTextWrap(false);
    display.print("COLOR HUE");

    CONSOLE_CRLF("DISPLAY: COLOR HUE LOADED")

    updateDisplayColorHue(currentColorHueIndex, previousColorHueIndex);
}

void loadDisplayBrightness(uint8_t brightness)
{
    display.drawRect(PICKER_OFFSET_X, PICKER_OFFSET_Y, PICKER_WIDTH, PICKER_HEIGHT, RGB888_TO_RGB565(255,255,255));
    display.drawRect(PICKER_OFFSET_X + 1, PICKER_OFFSET_Y + 1, PICKER_WIDTH - 2, PICKER_HEIGHT - 2, RGB888_TO_RGB565(255,255,255));

    display.setCursor(PICKER_OFFSET_X, PICKER_OFFSET_Y - 30);
    display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
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
    display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
    display.setTextSize(3, 3);
    display.setTextWrap(false);
    display.print("Number of LEDs\r\n");

    display.setCursor(5, display.height() - 40);
    display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
    display.setTextSize(2, 2);
    display.setTextWrap(false);
    display.print("Change -> rotate any knob");

    display.setCursor(5, display.height() - 20);
    display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
    display.setTextSize(2, 2);
    display.setTextWrap(false);
    display.print("Accept -> push any knob");

    CONSOLE_CRLF("DISPLAY: NUMBER OF LEDS LOADED")
}

void updateDisplayNumberOfLeds(uint16_t numberOfLeds, bool valueLocked)
{
    display.fillRect(0, 60, display.width(), display.height() - (60 + 40), RGB888_TO_RGB565(0, 0, 0));
    display.setCursor(6, 60);
    
    if(valueLocked)
    {
        display.setTextColor(RGB888_TO_RGB565(0, 255, 0));
    }
    else
    {
        display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
    }

    display.setTextSize(12, 17);
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
    display.fillTriangle(PICKER_OFFSET_X + previousColorHueIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2, (PICKER_OFFSET_X - 8) + previousColorHueIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2 + 15, (PICKER_OFFSET_X + 8) + previousColorHueIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(0, 0, 0));
    display.fillTriangle(PICKER_OFFSET_X + currentColorHueIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2, (PICKER_OFFSET_X - 8) + currentColorHueIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2 + 15, (PICKER_OFFSET_X + 8) + currentColorHueIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(255, 0, 0));
   
    CONSOLE_CRLF("DISPLAY: COLOR HUE UPDATED")
}

void updateDisplayColorTemperature(uint16_t currentColorTemperatureIndex, uint16_t previousColorTemperatureIndex)
{
    display.fillTriangle(PICKER_OFFSET_X + previousColorTemperatureIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2, (PICKER_OFFSET_X - 8) + previousColorTemperatureIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2 + 15, (PICKER_OFFSET_X + 8) + previousColorTemperatureIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(0, 0, 0));
    display.fillTriangle(PICKER_OFFSET_X + currentColorTemperatureIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2, (PICKER_OFFSET_X - 8) + currentColorTemperatureIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2 + 15, (PICKER_OFFSET_X + 8) + currentColorTemperatureIndex, PICKER_OFFSET_Y + PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(255, 0, 0));
   
    CONSOLE_CRLF("DISPLAY: COLOR TEMPERATURE UPDATED")
}

void updateDisplayBrightness(uint8_t brightness)
{
    uint16_t barWidth = map(brightness, 0, 255, 0, PICKER_WIDTH - 4);

    for(uint16_t i = 0; i < barWidth; i++)
    {
        display.drawFastVLine(PICKER_OFFSET_X + 2 + i, PICKER_OFFSET_Y + 2, PICKER_HEIGHT - 4, RGB888_TO_RGB565(255, 255, 255));
    } 

    for(uint16_t i = barWidth; i < PICKER_WIDTH - 4; i++)
    {
        display.drawFastVLine(PICKER_OFFSET_X + 2 + i, PICKER_OFFSET_Y + 2, PICKER_HEIGHT - 4, RGB888_TO_RGB565(0, 0, 0));
    } 

    CONSOLE_CRLF("DISPLAY: BRIGHTNESS UPDATED")
}

void drawWifiSignalUndefined()
{
    display.fillRect(display.width() - 64, 0, 64, 32, RGB888_TO_RGB565(0, 0, 0)); 
}

void drawWifiSignalDisconnected()
{
    display.fillRect(display.width() - 64, 0, 64, 32, RGB888_TO_RGB565(0, 0, 0)); 

    // dot
    display.fillCircle(display.width() - 22, 28, 3, RGB888_TO_RGB565(128, 128, 128));

    // 1st ring
    display.drawFastVLine(display.width() - 22, 18, 3, RGB888_TO_RGB565(128, 128, 128));
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 8, 3, RGB888_TO_RGB565(128, 128, 128));
    }
    display.endWrite();

    // 2nd ring
    display.drawFastVLine(display.width() - 22, 12, 3, RGB888_TO_RGB565(128, 128, 128));
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 14, 3, RGB888_TO_RGB565(128, 128, 128));
    }
    display.endWrite();

    // 3rd ring
    display.drawFastVLine(display.width() - 22, 6, 3, RGB888_TO_RGB565(128, 128, 128));
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 20, 3, RGB888_TO_RGB565(128, 128, 128));
    }
    display.endWrite();

    // remove excessive parts of rings
    display.fillTriangle(display.width() - 54, 0, display.width() - 54, 28, display.width() - 25, 28, RGB888_TO_RGB565(0, 0, 0));
    display.fillTriangle(display.width() + 10, 0, display.width() + 10, 28, display.width() - 19, 28, RGB888_TO_RGB565(0, 0, 0)); 

    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawLine(display.width() - 44, i, display.width() - 1, 30 + i, RGB888_TO_RGB565(255, 0, 0));
    }
}

void drawWifiSignalBad()
{
    display.fillRect(display.width() - 64, 0, 64, 32, RGB888_TO_RGB565(0, 0, 0)); 

    // dot
    display.fillCircle(display.width() - 22, 28, 3, RGB888_TO_RGB565(255, 0, 0));

    // 1st ring
    display.drawFastVLine(display.width() - 22, 18, 3, RGB888_TO_RGB565(255, 0, 0));
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 8, 3, RGB888_TO_RGB565(255, 0, 0));
    }
    display.endWrite();

    // 2nd ring
    display.drawFastVLine(display.width() - 22, 12, 3, RGB888_TO_RGB565(128, 128, 128));
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 14, 3, RGB888_TO_RGB565(128, 128, 128));
    }
    display.endWrite();

    // 3rd ring
    display.drawFastVLine(display.width() - 22, 6, 3, RGB888_TO_RGB565(128, 128, 128));
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 20, 3, RGB888_TO_RGB565(128, 128, 128));
    }
    display.endWrite();

    // remove excessive parts of rings
    display.fillTriangle(display.width() - 54, 0, display.width() - 54, 28, display.width() - 25, 28, RGB888_TO_RGB565(0, 0, 0));
    display.fillTriangle(display.width() + 10, 0, display.width() + 10, 28, display.width() - 19, 28, RGB888_TO_RGB565(0, 0, 0)); 
}

void drawWifiSignalGood()
{
    display.fillRect(display.width() - 64, 0, 64, 32, RGB888_TO_RGB565(0, 0, 0)); 

    // dot
    display.fillCircle(display.width() - 22, 28, 3, RGB888_TO_RGB565(255, 255, 0));

    // 1st ring
    display.drawFastVLine(display.width() - 22, 18, 3, RGB888_TO_RGB565(255, 255, 0));
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 8, 3, RGB888_TO_RGB565(255, 255, 0));
    }
    display.endWrite();

    // 2nd ring
    display.drawFastVLine(display.width() - 22, 12, 3, RGB888_TO_RGB565(255, 255, 0));
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 14, 3, RGB888_TO_RGB565(255, 255, 0));
    }
    display.endWrite();

    // 3rd ring
    display.drawFastVLine(display.width() - 22, 6, 3, RGB888_TO_RGB565(128, 128, 128));
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 20, 3, RGB888_TO_RGB565(128, 128, 128));
    }
    display.endWrite();

    // remove excessive parts of rings
    display.fillTriangle(display.width() - 54, 0, display.width() - 54, 28, display.width() - 25, 28, RGB888_TO_RGB565(0, 0, 0));
    display.fillTriangle(display.width() + 10, 0, display.width() + 10, 28, display.width() - 19, 28, RGB888_TO_RGB565(0, 0, 0)); 
}

void drawWifiSignalExcellent()
{
    display.fillRect(display.width() - 64, 0, 64, 32, RGB888_TO_RGB565(0, 0, 0)); 

    // dot
    display.fillCircle(display.width() - 22, 28, 3, RGB888_TO_RGB565(0, 255, 0));

    // 1st ring
    display.drawFastVLine(display.width() - 22, 18, 3, RGB888_TO_RGB565(0, 255, 0));
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 8, 3, RGB888_TO_RGB565(0, 255, 0));
    }
    display.endWrite();

    // 2nd ring
    display.drawFastVLine(display.width() - 22, 12, 3, RGB888_TO_RGB565(0, 255, 0));
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 14, 3, RGB888_TO_RGB565(0, 255, 0));
    }
    display.endWrite();

    // 3rd ring
    display.drawFastVLine(display.width() - 22, 6, 3, RGB888_TO_RGB565(0, 255, 0));
    display.startWrite();
    for(uint8_t i = 0; i < 3; i++)
    {
        display.drawCircleHelper(display.width() - 22, 28, i + 20, 3, RGB888_TO_RGB565(0, 255, 0));
    }
    display.endWrite();

    // remove excessive parts of rings
    display.fillTriangle(display.width() - 54, 0, display.width() - 54, 28, display.width() - 25, 28, RGB888_TO_RGB565(0, 0, 0));
    display.fillTriangle(display.width() + 10, 0, display.width() + 10, 28, display.width() - 19, 28, RGB888_TO_RGB565(0, 0, 0));
}

void updateWifiSignal(WIFI_SIGNAL wifiSignal)
{
    switch(wifiSignal)
    {
        case WIFI_SIGNAL_DISCONNECTED:
            drawWifiSignalDisconnected();
            break;

        case WIFI_SIGNAL_BAD:
            drawWifiSignalBad();
            break;
        
        case WIFI_SIGNAL_GOOD:
            drawWifiSignalGood();
            break;
        
        case WIFI_SIGNAL_EXCELLENT:
            drawWifiSignalExcellent();
            break;

        default:
            drawWifiSignalUndefined();
            break;
    }
}

void updateHour(uint8_t hour, bool invalid = false)
{
    if(!invalid)
    {
        display.fillRect(0, 60, 140, 120, RGB888_TO_RGB565(0, 0, 0));
        display.setCursor(6, 60);
        display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
        display.setTextSize(12, 17);
        display.setTextWrap(false);

        if(hour < 10)
        {
            display.print('0');
        }

        display.print(hour);  
    }
}

void updateDoubledot(bool visible)
{
    display.fillRect(140, 60, 40, 120, RGB888_TO_RGB565(0, 0, 0));

    if(visible)
    {
        display.setCursor(130, 60);
        display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
        display.setTextSize(12, 17); 
        display.setTextWrap(false);
        display.print(':'); 
    }
}

void updateMinute(uint8_t minute, bool invalid = false)
{
    if(!invalid)
    {
        display.fillRect(180, 60, 140, 120, RGB888_TO_RGB565(0, 0, 0));
        display.setCursor(182, 60);
        display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
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
    display.fillRect(0, display.height() - 39, display.width() * 1/4 - 22, 39, RGB888_TO_RGB565(0, 0, 0));
    display.setCursor(5, display.height() - 26);
    display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
    display.setTextSize(3, 3);
    display.setTextWrap(false);

    if(!invalid)
    {
        display.print((int32_t)round(temperature));
    }
}

void updateDate(uint8_t day, uint8_t month, uint16_t year, bool invalid = false)
{
    display.fillRect(0, 0, display.width() - 64, 40, RGB888_TO_RGB565(0, 0, 0));
    display.setCursor(5, 6);
    display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
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
    display.fillRect(display.width() * 1/4 + 1, display.height() - 39, display.width() * 1/4 - 22, 39, RGB888_TO_RGB565(0, 0, 0));
    display.setCursor(display.width() * 1/4 + 6, display.height() - 26);
    display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
    display.setTextSize(3, 3);
    display.setTextWrap(false);
    
    if(!invalid)
    {
        display.print(humidity); 
    }
}

void updateWindSpeed(float windGust, bool invalid = false)
{
    display.fillRect(display.width() * 2/4 + 1, display.height() - 39, display.width() * 1/4 - 22, 39, RGB888_TO_RGB565(0, 0, 0));
    display.setCursor(display.width() * 2/4 + 6, display.height() - 26);
    display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
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

void updateWeather(WEATHER weather)
{
    display.fillRect(display.width() * 3/4 + 1, display.height() - 39, display.width() - 1, 39, RGB888_TO_RGB565(0, 0, 0)); 

    switch(weather)
    {
        case WEATHER_CLEAR_SKY_DAY:
            drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_01d, RGB888_TO_RGB565(0, 0, 0));
            break;

        case WEATHER_CLEAR_SKY_NIGHT:
            drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_01n, RGB888_TO_RGB565(0, 0, 0));
            break;
        
        case WEATHER_FEW_CLOUDS_DAY:
            drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_02d, RGB888_TO_RGB565(0, 0, 0));
            break;

        case WEATHER_FEW_CLOUDS_NIGHT:
            drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_02n, RGB888_TO_RGB565(0, 0, 0));
            break;
            
        case WEATHER_SCATTERED_CLOUDS_DAY:
            drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_03d, RGB888_TO_RGB565(0, 0, 0));
            break;

        case WEATHER_SCATTERED_CLOUDS_NIGHT:
            drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_03n, RGB888_TO_RGB565(0, 0, 0));
            break;
            
        case WEATHER_BROKEN_CLOUDS_DAY:
            drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_04d, RGB888_TO_RGB565(0, 0, 0));
            break;

        case WEATHER_BROKEN_CLOUDS_NIGHT:
            drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_04n, RGB888_TO_RGB565(0, 0, 0));
            break;
            
        case WEATHER_SHOWER_RAIN_DAY:
            drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_09d, RGB888_TO_RGB565(0, 0, 0));
            break;

        case WEATHER_SHOWER_RAIN_NIGHT:
            drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_09n, RGB888_TO_RGB565(0, 0, 0));
            break;
            
        case WEATHER_RAIN_DAY:
            drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_10d, RGB888_TO_RGB565(0, 0, 0));
            break;

        case WEATHER_RAIN_NIGHT:
            drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_10n, RGB888_TO_RGB565(0, 0, 0));
            break;
            
        case WEATHER_THUNDERSTORM_DAY:
            drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_11d, RGB888_TO_RGB565(0, 0, 0));
            break;

        case WEATHER_THUNDERSTORM_NIGHT:
            drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_11n, RGB888_TO_RGB565(0, 0, 0));
            break;
            
        case WEATHER_SNOW_DAY:
            drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_13d, RGB888_TO_RGB565(0, 0, 0));
            break;

        case WEATHER_SNOW_NIGHT:
            drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_13n, RGB888_TO_RGB565(0, 0, 0));
            break;
            
        case WEATHER_MIST_DAY:
            drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_50d, RGB888_TO_RGB565(0, 0, 0));
            break;

        case WEATHER_MIST_NIGHT:
            drawRGB565_filtered(display.width() * 3/4 + 9, display.height() - 35, IMAGE_WIDTH, IMAGE_HEIGHT, image_50n, RGB888_TO_RGB565(0, 0, 0));
            break;

        default:
            break;
    }
}

void drawFixedParts()
{
    display.drawFastHLine(0, display.height() - 40, display.width(), RGB888_TO_RGB565(255, 255, 255));
    display.drawFastHLine(0, 40, display.width(), RGB888_TO_RGB565(255, 255, 255));
    display.drawFastVLine(display.width() * 1/4, display.height() - 40, 40, RGB888_TO_RGB565(255, 255, 255));
    display.drawFastVLine(display.width() * 2/4, display.height() - 40, 40, RGB888_TO_RGB565(255, 255, 255));
    display.drawFastVLine(display.width() * 3/4, display.height() - 40, 40, RGB888_TO_RGB565(255, 255, 255));

    // °C
    display.setCursor(display.width() * 1/4 - 13, display.height() - 36);
    display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
    display.setTextSize(2, 2);
    display.setTextWrap(false);
    display.print("C");

    display.fillCircle(display.width() * 1/4 - 18, display.height() - 32, 2, RGB888_TO_RGB565(255, 255, 255));
    display.fillCircle(display.width() * 1/4 - 18, display.height() - 32, 1, RGB888_TO_RGB565(0, 0, 0));

    // %
    display.setCursor(display.width() * 2/4 - 13, display.height() - 36);
    display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
    display.setTextSize(2, 2);
    display.setTextWrap(false);
    display.print("%");

    // m/s
    display.setCursor(display.width() * 3/4 - 13, display.height() - 38);
    display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
    display.setTextSize(2, 2);
    display.setTextWrap(false);
    display.print("m");

    display.drawFastHLine(display.width() * 3/4 - 13, display.height() - 22, 11, RGB888_TO_RGB565(255, 255, 255));

    display.setCursor(display.width() * 3/4 - 13, display.height() - 22);
    display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
    display.setTextSize(2, 2);
    display.setTextWrap(false);
    display.print("s");
}

void drawSetupText()
{
    char buff[256] = "";
    
    display.fillRect(0, 41, display.width(), display.height() - 82, RGB888_TO_RGB565(0, 0, 0));
    display.setCursor(0, 47);
    display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
    display.setTextSize(2, 2);
    display.setTextWrap(true);

    char tempIP[32] = "";
    WiFi.softAPIP().toString().toCharArray(tempIP, 32);

    sprintf(buff, "1) Connect the the Wi-Fi:\r\n  -> %s\r\n\r\n2) Enter password:\r\n  -> %s\r\n\r\n3) Enter to browser:\r\n  -> http://%s", defaultSoftAP_ssid, defaultSoftAP_pwd, tempIP);   
    display.print(buff);        
}

void drawOfflineMode()
{
    display.fillRect(0, 41, display.width(), display.height() - 82, RGB888_TO_RGB565(0, 0, 0));
    display.setCursor(0, 47);
    display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
    display.setTextSize(2, 2);
    display.setTextWrap(true);
    display.print("Offline mode enabled.\r\n\r\n");  
    display.print("Reset device to enter\r\nconfiguration mode again.\r\n"); 
}

void loadAndExecuteFactoryReset(Preferences *preferences)
{
    display.drawRect(PICKER_OFFSET_X, PICKER_OFFSET_Y, PICKER_WIDTH, PICKER_HEIGHT, RGB888_TO_RGB565(255,255,255));
    display.drawRect(PICKER_OFFSET_X + 1, PICKER_OFFSET_Y + 1, PICKER_WIDTH - 2, PICKER_HEIGHT - 2, RGB888_TO_RGB565(255,255,255));

    display.setCursor(PICKER_OFFSET_X, PICKER_OFFSET_Y - 30);
    display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
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

        display.drawFastVLine(PICKER_OFFSET_X + 2 + i, PICKER_OFFSET_Y + 2, PICKER_HEIGHT - 4, RGB888_TO_RGB565(255, 255, 255));

        delay((FACTORY_RESET_TIMEOUT_MS / PICKER_WIDTH) - (millis() - timer));  
    }

    clearDisplay();

    // force factory reset by modifying first run value
    uint32_t tempVal = preferences->getUInt("firstRun", DEFAULT_ID);
    tempVal += 1;
    preferences->putUInt("firstRun", tempVal);

    CONSOLE_CRLF("ESP32: RESTART") 
    ESP.restart();
}

void updateMainScreen(bool offlineMode, bool validWifiConnection, bool validWeather, bool validDateTime, bool forceAll, uint8_t hour, uint8_t minute, uint8_t day, uint8_t month, uint16_t year, float temperature, uint8_t humidity, float windSpeed, WEATHER weather, WIFI_SIGNAL wifiSignal)
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
    static WIFI_SIGNAL prevWifiSignal = WIFI_SIGNAL_NONE;
    static WEATHER prevWeather = WEATHER_NONE;

    static bool doubledotVisible = false;

    static bool previousValidWifiConnection = !validWifiConnection;
    static bool previousValidWeather = !validWeather;
    static bool previousValidDateTime = !validDateTime;
    static bool previousOfflineMode = !offlineMode;
    
    CONSOLE_CRLF("DISPLAY: MAIN UPDATED")

    if(forceAll)
    {
        drawFixedParts();
    }

    if((previousValidWifiConnection || forceAll) && !validWifiConnection && !offlineMode)
    {
        drawSetupText();
    }

    if((!previousOfflineMode || forceAll) && offlineMode)
    {
        drawOfflineMode();
    }

    // update on each updateMainScreen() which happens once a second
    if(validWifiConnection && validDateTime)
    {
        updateDoubledot(!doubledotVisible);  
        doubledotVisible = !doubledotVisible;
        CONSOLE_CRLF("  |-- DOUBLEDOT UPDATED")
    }
    
    if(wifiSignal != prevWifiSignal || forceAll)
    {
        updateWifiSignal(wifiSignal);
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
        updateWindSpeed(windSpeed, true); 
        CONSOLE_CRLF("  |-- WIND SPEED UPDATED (INVALID)")      
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

    if(previousValidWifiConnection != validWifiConnection)
    {
        previousValidWifiConnection = validWifiConnection;  
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
}

void clearDisplay()
{
    display.fillScreen(RGB888_TO_RGB565(0, 0, 0));
    CONSOLE_CRLF("DISPLAY: CLEARED")
}