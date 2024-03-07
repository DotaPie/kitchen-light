#include "display.h"
#include <stdint.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include "pinout.h"
#include "macros.h"
#include "conf.h"
#include "console.h"
#include <FastLED.h>

Adafruit_ST7789 display = Adafruit_ST7789(DISPLAY_CS_PIN, DISPLAY_DC_PIN, DISPLAY_RST_PIN);

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

    CONSOLE_CRLF("\r\nDISPLAY: COLOR HUE LOADED")

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

    CONSOLE_CRLF("\r\nDISPLAY: BRIGHTNESS LOADED")

    updateDisplayBrightness(brightness);
}

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
   
    CONSOLE_CRLF("\r\nDISPLAY: COLOR HUE UPDATED")
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

    CONSOLE_CRLF("\r\nDISPLAY: BRIGHTNESS UPDATED")
}

void updateWifiSignal(uint8_t wifiSignal)
{
    display.fillRect(display.width() - 32, 0, display.width(), 32, RGB888_TO_RGB565(0, 255, 0));
    display.setCursor(display.width() - 32, 2);
    display.setTextColor(RGB888_TO_RGB565(255, 0, 0));
    display.setTextSize(4);
    display.setTextWrap(false);
    display.print(wifiSignal);
}

void updateHour(uint8_t hour)
{
    display.fillRect(0, 60, 140, 120, RGB888_TO_RGB565(0, 0, 0));
    display.setCursor(6, 60);
    display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
    display.setTextSize(12, 17);
    display.setTextWrap(false);
    display.print(hour);    
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

void updateMinute(uint8_t minute)
{
    display.fillRect(180, 60, 140, 120, RGB888_TO_RGB565(0, 0, 0));
    display.setCursor(182, 60);
    display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
    display.setTextSize(12, 17);
    display.setTextWrap(false);
    display.print(minute);
}

void updateTemperature(float temperature)
{
    display.fillRect(0, display.height() - 26, display.width() / 2, display.height(), RGB888_TO_RGB565(0, 0, 0));
    display.setCursor(6, display.height() - 26);
    display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
    display.setTextSize(2, 3);
    display.setTextWrap(false);
    display.print((int32_t)temperature);
    display.print(" C");
}

void updateDate(uint8_t day, uint8_t month, uint16_t year)
{
    display.fillRect(0, 0, display.width() / 3 * 2, 26, RGB888_TO_RGB565(0, 0, 0));
    display.setCursor(6, 2);
    display.setTextColor(RGB888_TO_RGB565(255, 255, 255));
    display.setTextSize(2, 3);
    display.setTextWrap(false);
    display.print(day);
    display.print(' ');   
    display.print(monthNames[month]);
    display.print(' '); 
    display.print(year);        
}

void updateMainScreen(bool forceAll, uint8_t hour, uint8_t minute, uint8_t day, uint8_t month, uint16_t year, float temperature, uint8_t wifiSignal)
{
    static uint8_t prevHour = 255; 
    static uint8_t prevMinute = 255;
    static uint8_t prevDay = 255;
    static uint8_t prevMonth = 255;
    static uint16_t prevYear = 9999;
    static float prevTemperature = 273.15;  
    static uint8_t prevWifiSignal = 255;
    static bool doubledotVisible = false;

    CONSOLE_CRLF("\r\nDISPLAY: MAIN UPDATED")

    if(hour != prevHour || forceAll)
    {
        updateHour(hour);
        prevHour = hour;
        CONSOLE_CRLF("  |-- HOUR UPDATED")
    } 

    updateDoubledot(!doubledotVisible);  
    doubledotVisible = !doubledotVisible;
    CONSOLE_CRLF("  |-- DOUBLEDOT UPDATED")

    if(minute != prevMinute || forceAll)
    {
        updateMinute(minute);
        prevMinute = minute;
        CONSOLE_CRLF("  |-- MINUTE UPDATED")
    } 

    if(wifiSignal != prevWifiSignal || forceAll)
    {
        updateWifiSignal(wifiSignal);
        prevWifiSignal = wifiSignal;
        CONSOLE_CRLF("  |-- WIFI SIGNAL UPDATED")
    } 

    if(temperature != prevTemperature || forceAll)
    {
        updateTemperature(temperature);
        prevTemperature = temperature;
        CONSOLE_CRLF("  |-- TEMPERATURE UPDATED")
    }

    if(day != prevDay || month != prevMonth || year != prevYear || forceAll)
    {
        updateDate(day, month, year);
        prevDay = day;
        prevMonth = month;
        prevYear = year;
        CONSOLE_CRLF("  |-- DATE UPDATED")
    }
}

void clearDisplay()
{
    display.fillScreen(RGB888_TO_RGB565(0, 0, 0));
    CONSOLE_CRLF("\r\nDISPLAY: CLEARED")
}