#include "display.h"
#include <stdint.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include "pinout.h"
#include "macros.h"
#include "conf.h"
#include <FastLED.h>

Adafruit_ST7789 display = Adafruit_ST7789(DISPLAY_CS_PIN, DISPLAY_DC_PIN, DISPLAY_RST_PIN);

void loadPickerColorHue()
{
    uint8_t colorIndex;

    // R -> G
    colorIndex = 0;
    for(uint16_t i = 0; i < 48; i++)
    {
        display.drawFastVLine(RGB_PICKER_OFFSET_X + i, RGB_PICKER_OFFSET_Y, RGB_PICKER_HEIGHT, RGB888_TO_RGB565(255, colorIndex, 0));
            
        if(i % 3 != 0)
        {
            colorIndex += 8; 
        }
    }

    colorIndex = 0;
    for(uint16_t i = 48; i < 96; i++)
    {
        display.drawFastVLine(RGB_PICKER_OFFSET_X + i, RGB_PICKER_OFFSET_Y, RGB_PICKER_HEIGHT, RGB888_TO_RGB565(255 - colorIndex, 255, 0)); 

        if(i % 3 != 0)
        {
            colorIndex += 8; 
        }
    }

    // G -> B
    colorIndex = 0;
    for(uint16_t i = 96; i < 144; i++)
    {
        display.drawFastVLine(RGB_PICKER_OFFSET_X + i, RGB_PICKER_OFFSET_Y, RGB_PICKER_HEIGHT, RGB888_TO_RGB565(0, 255, colorIndex));
            
        if(i % 3 != 0)
        {
            colorIndex += 8; 
        }
    }

    colorIndex = 0;
    for(uint16_t i = 144; i < 192; i++)
    {
        display.drawFastVLine(RGB_PICKER_OFFSET_X + i, RGB_PICKER_OFFSET_Y, RGB_PICKER_HEIGHT, RGB888_TO_RGB565(0, 255 - colorIndex, 255)); 

        if(i % 3 != 0)
        {
            colorIndex += 8; 
        }
    }

    // B -> R
    colorIndex = 0;
    for(uint16_t i = 192; i < 240; i++)
    {
        display.drawFastVLine(RGB_PICKER_OFFSET_X + i, RGB_PICKER_OFFSET_Y, RGB_PICKER_HEIGHT, RGB888_TO_RGB565(colorIndex, 0, 255));
            
        if(i % 3 != 0)
        {
            colorIndex += 8; 
        }
    }

    colorIndex = 0;
    for(uint16_t i = 240; i < 288; i++)
    {
        display.drawFastVLine(RGB_PICKER_OFFSET_X + i, RGB_PICKER_OFFSET_Y, RGB_PICKER_HEIGHT, RGB888_TO_RGB565(255, 0, 255 - colorIndex));  

        if(i % 3 != 0)
        {
            colorIndex += 8; 
        }
    }
}

void loadPickerColorTemperature()
{
    // TODO
}

void loadBrightness()
{
    // TODO    
}

CRGB updateDisplayColorHue(uint16_t index, MOVEMENT_DIRECTION movementDirection)
{
    uint8_t red, green, blue;

    if(movementDirection == DIRECTION_RIGHT)
    {
        // top arrow
        display.fillTriangle(RGB_PICKER_OFFSET_X + index, RGB_PICKER_OFFSET_Y - 2, (RGB_PICKER_OFFSET_X - 8) + index, RGB_PICKER_OFFSET_Y - 2 - 15, (RGB_PICKER_OFFSET_X + 8) + index, RGB_PICKER_OFFSET_Y - 2 - 15, RGB888_TO_RGB565(255, 0, 0));
        display.drawLine((RGB_PICKER_OFFSET_X - 2) + index, RGB_PICKER_OFFSET_Y - 2, (RGB_PICKER_OFFSET_X - 10) + index, RGB_PICKER_OFFSET_Y - 2 - 15, RGB888_TO_RGB565(0, 0, 0));
        display.drawLine((RGB_PICKER_OFFSET_X - 1) + index, RGB_PICKER_OFFSET_Y - 2, (RGB_PICKER_OFFSET_X - 9) + index, RGB_PICKER_OFFSET_Y - 2 - 15, RGB888_TO_RGB565(0, 0, 0));
        
        // bottom arrow
        display.fillTriangle(RGB_PICKER_OFFSET_X + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2, (RGB_PICKER_OFFSET_X - 8) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, (RGB_PICKER_OFFSET_X + 8) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(255, 0, 0));
        display.drawLine((RGB_PICKER_OFFSET_X - 2) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2, (RGB_PICKER_OFFSET_X - 10) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(0, 0, 0));
        display.drawLine((RGB_PICKER_OFFSET_X - 1) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2, (RGB_PICKER_OFFSET_X - 9) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(0, 0, 0));

        // R -> G
        if(index >= 0 && index < 48)
        {
            red = 255;
            green = (uint8_t)((float)index * (256/48));
            blue = 0;
        }
        else if(index >= 48 && index < 96)
        {
            red = 255 - (uint8_t)((float)(index - 48) * (256/48));
            green = 255;
            blue = 0;    
        }

        // G -> B
        else if(index >= 96 && index < 144)
        {
            red = 0;
            green = 255;
            blue = (uint8_t)((float)(index - 96) * (256/48));
        }
        else if(index >= 144 && index < 192)
        {
            red = 0;
            green = 255 - (uint8_t)((float)(index - 144) * (256/48));
            blue = 255;    
        }

        // B -> R
        else if(index >= 192 && index < 240)
        {
            red = uint8_t((float)(index - 192) * (256/48));
            green = 0;
            blue = 255;
        }
        else if(index >= 240 && index < 288)
        {
            red = 255;
            green = 0;
            blue = 255 - (uint8_t)((float)(index - 240) * (256/48));    
        }
    }

    // movement to left
    if(movementDirection == DIRECTION_LEFT)
    {
        // top arrow
        display.fillTriangle(RGB_PICKER_OFFSET_X + index, RGB_PICKER_OFFSET_Y - 2, (RGB_PICKER_OFFSET_X - 8) + index, RGB_PICKER_OFFSET_Y - 2 - 15, (RGB_PICKER_OFFSET_X + 8) + index, RGB_PICKER_OFFSET_Y - 2 - 15, RGB888_TO_RGB565(255, 0, 0));
        display.drawLine((RGB_PICKER_OFFSET_X + 2) + index, RGB_PICKER_OFFSET_Y - 2, (RGB_PICKER_OFFSET_X + 10) + index, RGB_PICKER_OFFSET_Y - 2 - 15, RGB888_TO_RGB565(0, 0, 0));
        display.drawLine((RGB_PICKER_OFFSET_X + 1) + index, RGB_PICKER_OFFSET_Y - 2, (RGB_PICKER_OFFSET_X + 9) + index, RGB_PICKER_OFFSET_Y - 2 - 15, RGB888_TO_RGB565(0, 0, 0));
        
        // bottom arrow
        display.fillTriangle(RGB_PICKER_OFFSET_X + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2, (RGB_PICKER_OFFSET_X - 8) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, (RGB_PICKER_OFFSET_X + 8) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(255, 0, 0));
        display.drawLine((RGB_PICKER_OFFSET_X + 2) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2, (RGB_PICKER_OFFSET_X + 10) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(0, 0, 0));
        display.drawLine((RGB_PICKER_OFFSET_X + 1) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2, (RGB_PICKER_OFFSET_X + 9) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(0, 0, 0));

        // R -> G
        if(index >= 0 && index < 48)
        {
            red = 255;
            green = (uint8_t)((float)index * (256/48));
            blue = 0;
        }
        else if(index >= 48 && index < 96)
        {
            red = 255 - (uint8_t)((float)(index - 48) * (256/48));
            green = 255;
            blue = 0;    
        }

        // G -> B
        else if(index >= 96 && index < 144)
        {
            red = 0;
            green = 255;
            blue = (uint8_t)((float)(index - 96) * (256/48));
        }
        else if(index >= 144 && index < 192)
        {
            red = 0;
            green = 255 - (uint8_t)((float)(index - 144) * (256/48));
            blue = 255;    
        }

        // B -> R
        else if(index >= 192 && index < 240)
        {
            red = uint8_t((float)(index - 192) * (256/48));
            green = 0;
            blue = 255;
        }
        else if(index >= 240 && index < 288)
        {
            red = 255;
            green = 0;
            blue = 255 - (uint8_t)((float)(index - 240) * (256/48));    
        } 
    }

    return CRGB(red, green, blue);
}

CRGB updateDisplayColorTemperature(uint16_t index, MOVEMENT_DIRECTION movementDirection)
{
    uint8_t red, green, blue;

    if(movementDirection == DIRECTION_RIGHT)
    {
        // top arrow
        display.fillTriangle(RGB_PICKER_OFFSET_X + index, RGB_PICKER_OFFSET_Y - 2, (RGB_PICKER_OFFSET_X - 8) + index, RGB_PICKER_OFFSET_Y - 2 - 15, (RGB_PICKER_OFFSET_X + 8) + index, RGB_PICKER_OFFSET_Y - 2 - 15, RGB888_TO_RGB565(255, 0, 0));
        display.drawLine((RGB_PICKER_OFFSET_X - 2) + index, RGB_PICKER_OFFSET_Y - 2, (RGB_PICKER_OFFSET_X - 10) + index, RGB_PICKER_OFFSET_Y - 2 - 15, RGB888_TO_RGB565(0, 0, 0));
        display.drawLine((RGB_PICKER_OFFSET_X - 1) + index, RGB_PICKER_OFFSET_Y - 2, (RGB_PICKER_OFFSET_X - 9) + index, RGB_PICKER_OFFSET_Y - 2 - 15, RGB888_TO_RGB565(0, 0, 0));
        
        // bottom arrow
        display.fillTriangle(RGB_PICKER_OFFSET_X + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2, (RGB_PICKER_OFFSET_X - 8) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, (RGB_PICKER_OFFSET_X + 8) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(255, 0, 0));
        display.drawLine((RGB_PICKER_OFFSET_X - 2) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2, (RGB_PICKER_OFFSET_X - 10) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(0, 0, 0));
        display.drawLine((RGB_PICKER_OFFSET_X - 1) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2, (RGB_PICKER_OFFSET_X - 9) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(0, 0, 0));

        // TODO
    }

    // movement to left
    if(movementDirection == DIRECTION_LEFT)
    {
        // top arrow
        display.fillTriangle(RGB_PICKER_OFFSET_X + index, RGB_PICKER_OFFSET_Y - 2, (RGB_PICKER_OFFSET_X - 8) + index, RGB_PICKER_OFFSET_Y - 2 - 15, (RGB_PICKER_OFFSET_X + 8) + index, RGB_PICKER_OFFSET_Y - 2 - 15, RGB888_TO_RGB565(255, 0, 0));
        display.drawLine((RGB_PICKER_OFFSET_X + 2) + index, RGB_PICKER_OFFSET_Y - 2, (RGB_PICKER_OFFSET_X + 10) + index, RGB_PICKER_OFFSET_Y - 2 - 15, RGB888_TO_RGB565(0, 0, 0));
        display.drawLine((RGB_PICKER_OFFSET_X + 1) + index, RGB_PICKER_OFFSET_Y - 2, (RGB_PICKER_OFFSET_X + 9) + index, RGB_PICKER_OFFSET_Y - 2 - 15, RGB888_TO_RGB565(0, 0, 0));
        
        // bottom arrow
        display.fillTriangle(RGB_PICKER_OFFSET_X + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2, (RGB_PICKER_OFFSET_X - 8) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, (RGB_PICKER_OFFSET_X + 8) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(255, 0, 0));
        display.drawLine((RGB_PICKER_OFFSET_X + 2) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2, (RGB_PICKER_OFFSET_X + 10) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(0, 0, 0));
        display.drawLine((RGB_PICKER_OFFSET_X + 1) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2, (RGB_PICKER_OFFSET_X + 9) + index, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(0, 0, 0));

        // TODO
    }

    return CRGB(red, green, blue);
}

void updateBrightness()
{
    // TODO
}