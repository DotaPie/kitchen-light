#include <Arduino.h>
#include <Preferences.h>

// project includes
#include "console.h"
#include "pinout.h"
#include "conf.h"
#include "macros.h"

// libs
#include <RotaryEncoder.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <FastLED.h>

// enums
enum STATE {STATE_NONE, STATE_MAIN};
enum COLOR_PICKER_TYPE {CPT_NONE, CPT_COLOR_TEMPERATURE, CPT_COLOR_HUE};

// globals
STATE state, previousState;
COLOR_PICKER_TYPE current_CPT, previous_CPT;
CRGB currentColor, previousColor;

const char* stateString[] = {"STATE_NONE", "STATE_MAIN"};
const char* CPT_String[] = {"CPT_NONE", "CPT_COLOR_TEMPERATURE", "CPT_COLOR_HUE"};

CRGB LED_stripArray[LED_STRIP_COUNT];
RotaryEncoder encoder_1 = RotaryEncoder(RE_1_IN1_PIN, RE_1_IN2_PIN, RotaryEncoder::LatchMode::TWO03);
RotaryEncoder encoder_2 = RotaryEncoder(RE_2_IN1_PIN, RE_2_IN2_PIN, RotaryEncoder::LatchMode::TWO03);
Adafruit_ST7789 display = Adafruit_ST7789(DISPLAY_CS_PIN, DISPLAY_DC_PIN, DISPLAY_RST_PIN);

Preferences preferences;

// function definitions

void loadPreferences()
{
    state = STATE_MAIN;
    previousState = STATE_NONE;

    CONSOLE("Loading preferences: ")

    bool firstTimeRun;

    if(preferences.getUInt("firstRun", 0) == 0)
    {
        firstTimeRun = true;
        preferences.putUInt("firstRun", 100);

        preferences.putUChar("CPT", (uint8_t)CPT_COLOR_TEMPERATURE);

        preferences.putUChar("color-R", 255); // TODO .. neutral K 
        preferences.putUChar("color-G", 255); // TODO .. neutral K 
        preferences.putUChar("color-B", 255); // TODO .. neutral K 
    }
    else
    {
        firstTimeRun = false;
    }

    current_CPT = (COLOR_PICKER_TYPE)preferences.getUChar("CPT", (uint8_t)CPT_NONE);  
    currentColor = CRGB(preferences.getUChar("color-R", 255), preferences.getUChar("color-G", 255), preferences.getUChar("color-B", 255)); // TODO .. neutral K  
    
    previous_CPT = CPT_NONE;  
    previousColor = CRGB(0, 0, 0);

    CONSOLE_CRLF("OK")

    CONSOLE("  |-- first time run: ") 
    CONSOLE_CRLF(firstTimeRun ? "Yes" : "No")

    CONSOLE("  |-- current color picker type: ") 
    CONSOLE_CRLF(CPT_String[(uint8_t)current_CPT])

    CONSOLE("  |-- current color: ") 
    CONSOLE("[R:") 
    CONSOLE(currentColor.r)
    CONSOLE(" | G:") 
    CONSOLE(currentColor.g)
    CONSOLE(" | B:") 
    CONSOLE(currentColor.b)
    CONSOLE_CRLF("]") 
}

void setup_LED_strip()
{
    CONSOLE("LED strip: ")
    FastLED.addLeds<LED_STRIP_TYPE, LED_STRIP_PIN, COLOR_ORDER>(LED_stripArray, LED_STRIP_COUNT).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(DEFAULT_BRIGHTNESS);
    
    for(uint16_t i = 0; i < LED_STRIP_COUNT; i++)
    {
        for(uint16_t j = 0; j < i; j++)
        {
            LED_stripArray[j] = ColorFromPalette(RainbowColors_p, (uint8_t)(j/2), DEFAULT_BRIGHTNESS, COLOR_BLENDING);   
        }

        for(uint16_t j = i; j < LED_STRIP_COUNT; j++)
        {
            LED_stripArray[j] = CRGB(0, 0, 0);   
        }  

        FastLED.show();
        delay(6);
    }

    CONSOLE_CRLF("OK")
}

void checkRotaryEncoderPosition_1()
{
    encoder_1.tick();
}

void checkRotaryEncoderPosition_2()
{
    encoder_2.tick();
}

void setupRotaryEncoders()
{
    CONSOLE("Rotary encoder #1: ")
    attachInterrupt(digitalPinToInterrupt(RE_1_IN1_PIN), checkRotaryEncoderPosition_1, CHANGE);
    attachInterrupt(digitalPinToInterrupt(RE_1_IN2_PIN), checkRotaryEncoderPosition_1, CHANGE);
    CONSOLE_CRLF("OK")

    CONSOLE("Rotary encoder #2: ")
    attachInterrupt(digitalPinToInterrupt(RE_2_IN1_PIN), checkRotaryEncoderPosition_2, CHANGE);
    attachInterrupt(digitalPinToInterrupt(RE_2_IN2_PIN), checkRotaryEncoderPosition_2, CHANGE);
    CONSOLE_CRLF("OK")
}

void setupSwitches()
{
    CONSOLE("Switches: ")   
    pinMode(SWITCH_1_PIN, INPUT_PULLUP);
    pinMode(SWITCH_2_PIN, INPUT_PULLUP);
    CONSOLE_CRLF("OK") 

    CONSOLE("  |-- Switch state #1: ") 
    CONSOLE_CRLF(digitalRead(SWITCH_1_PIN) == LOW ? "ON" : "OFF")

    CONSOLE("  |-- Switch state #2: ") 
    CONSOLE_CRLF(digitalRead(SWITCH_2_PIN) == LOW ? "ON" : "OFF")
}

void setupDisplay()
{
    CONSOLE("Display: ")
    display.init(DISPLAY_WIDTH, DISPLAY_HEIGHT);       
    display.setRotation(DISPLAY_ROTATION_DEGREE/90);
    display.invertDisplay(false); 
    display.fillScreen(ST77XX_BLACK);
    display.setCursor(4, 4);
    display.setTextColor(ST77XX_WHITE);
    display.setTextSize(2);
    display.setTextWrap(true);
    display.print("Booting ...");

    /*uint8_t colorIndex;

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
    }*/

    CONSOLE_CRLF("OK") 
}

void setupPreferences()
{
    preferences.begin("app", false);
}

void setup()
{
    delay(DELAY_BEFORE_STARTUP_MS);

    CONSOLE_SERIAL.begin(CONSOLE_BAUDRATE);

    CONSOLE_CRLF()
    CONSOLE_CRLF("~~~ SETUP ~~~")
    CONSOLE_CRLF()
    setupPreferences();
    loadPreferences();
    setupDisplay();
    setupRotaryEncoders();
    setupSwitches();
    setup_LED_strip();
    CONSOLE_CRLF()

    /*while(1)
    {
        // movement to right
        for(int16_t i = 0; i < 48 * 2 * 3; i++)
        {
            // top arrow
            display.fillTriangle(RGB_PICKER_OFFSET_X + i, RGB_PICKER_OFFSET_Y - 2, (RGB_PICKER_OFFSET_X - 8) + i, RGB_PICKER_OFFSET_Y - 2 - 15, (RGB_PICKER_OFFSET_X + 8) + i, RGB_PICKER_OFFSET_Y - 2 - 15, RGB888_TO_RGB565(255, 0, 0));
            display.drawLine((RGB_PICKER_OFFSET_X - 2) + i, RGB_PICKER_OFFSET_Y - 2, (RGB_PICKER_OFFSET_X - 10) + i, RGB_PICKER_OFFSET_Y - 2 - 15, RGB888_TO_RGB565(0, 0, 0));
            display.drawLine((RGB_PICKER_OFFSET_X - 1) + i, RGB_PICKER_OFFSET_Y - 2, (RGB_PICKER_OFFSET_X - 9) + i, RGB_PICKER_OFFSET_Y - 2 - 15, RGB888_TO_RGB565(0, 0, 0));
            
            // bottom arrow
            display.fillTriangle(RGB_PICKER_OFFSET_X + i, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2, (RGB_PICKER_OFFSET_X - 8) + i, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, (RGB_PICKER_OFFSET_X + 8) + i, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(255, 0, 0));
            display.drawLine((RGB_PICKER_OFFSET_X - 2) + i, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2, (RGB_PICKER_OFFSET_X - 10) + i, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(0, 0, 0));
            display.drawLine((RGB_PICKER_OFFSET_X - 1) + i, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2, (RGB_PICKER_OFFSET_X - 9) + i, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(0, 0, 0));

            uint8_t red, green, blue;

            // R -> G
            if(i >= 0 && i < 48)
            {
                red = 255;
                green = (uint8_t)((float)i * (256/48));
                blue = 0;
            }
            else if(i >= 48 && i < 96)
            {
                red = 255 - (uint8_t)((float)(i - 48) * (256/48));
                green = 255;
                blue = 0;    
            }

            // G -> B
            else if(i >= 96 && i < 144)
            {
                red = 0;
                green = 255;
                blue = (uint8_t)((float)(i - 96) * (256/48));
            }
            else if(i >= 144 && i < 192)
            {
                red = 0;
                green = 255 - (uint8_t)((float)(i - 144) * (256/48));
                blue = 255;    
            }

            // B -> R
            else if(i >= 192 && i < 240)
            {
                red = uint8_t((float)(i - 192) * (256/48));
                green = 0;
                blue = 255;
            }
            else if(i >= 240 && i < 288)
            {
                red = 255;
                green = 0;
                blue = 255 - (uint8_t)((float)(i - 240) * (256/48));    
            }

            LED_stripArray[0] = CRGB(red, green, blue);
            LED_stripArray[1] = CRGB(red, green, blue);
            LED_stripArray[2] = CRGB(red, green, blue);

            FastLED.show();
        }

        // movement to left
        for(int16_t i = 48 * 2 * 3 - 1; i >= 0; i--)
        {
            // top arrow
            display.fillTriangle(RGB_PICKER_OFFSET_X + i, RGB_PICKER_OFFSET_Y - 2, (RGB_PICKER_OFFSET_X - 8) + i, RGB_PICKER_OFFSET_Y - 2 - 15, (RGB_PICKER_OFFSET_X + 8) + i, RGB_PICKER_OFFSET_Y - 2 - 15, RGB888_TO_RGB565(255, 0, 0));
            display.drawLine((RGB_PICKER_OFFSET_X + 2) + i, RGB_PICKER_OFFSET_Y - 2, (RGB_PICKER_OFFSET_X + 10) + i, RGB_PICKER_OFFSET_Y - 2 - 15, RGB888_TO_RGB565(0, 0, 0));
            display.drawLine((RGB_PICKER_OFFSET_X + 1) + i, RGB_PICKER_OFFSET_Y - 2, (RGB_PICKER_OFFSET_X + 9) + i, RGB_PICKER_OFFSET_Y - 2 - 15, RGB888_TO_RGB565(0, 0, 0));
            
            // bottom arrow
            display.fillTriangle(RGB_PICKER_OFFSET_X + i, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2, (RGB_PICKER_OFFSET_X - 8) + i, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, (RGB_PICKER_OFFSET_X + 8) + i, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(255, 0, 0));
            display.drawLine((RGB_PICKER_OFFSET_X + 2) + i, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2, (RGB_PICKER_OFFSET_X + 10) + i, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(0, 0, 0));
            display.drawLine((RGB_PICKER_OFFSET_X + 1) + i, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2, (RGB_PICKER_OFFSET_X + 9) + i, RGB_PICKER_OFFSET_Y + RGB_PICKER_HEIGHT + 2 + 15, RGB888_TO_RGB565(0, 0, 0));
            
            uint8_t red, green, blue;

            // R -> G
            if(i >= 0 && i < 48)
            {
                red = 255;
                green = (uint8_t)((float)i * (256/48));
                blue = 0;
            }
            else if(i >= 48 && i < 96)
            {
                red = 255 - (uint8_t)((float)(i - 48) * (256/48));
                green = 255;
                blue = 0;    
            }

            // G -> B
            else if(i >= 96 && i < 144)
            {
                red = 0;
                green = 255;
                blue = (uint8_t)((float)(i - 96) * (256/48));
            }
            else if(i >= 144 && i < 192)
            {
                red = 0;
                green = 255 - (uint8_t)((float)(i - 144) * (256/48));
                blue = 255;    
            }

            // B -> R
            else if(i >= 192 && i < 240)
            {
                red = uint8_t((float)(i - 192) * (256/48));
                green = 0;
                blue = 255;
            }
            else if(i >= 240 && i < 288)
            {
                red = 255;
                green = 0;
                blue = 255 - (uint8_t)((float)(i - 240) * (256/48));    
            }

            LED_stripArray[0] = CRGB(red, green, blue);
            LED_stripArray[1] = CRGB(red, green, blue);
            LED_stripArray[2] = CRGB(red, green, blue);

            FastLED.show();   
        }
    }*/

    state = STATE_MAIN;

    CONSOLE_CRLF()
    CONSOLE_CRLF("~~~ LOOP ~~~")
    CONSOLE_CRLF()
}

void loop() 
{
    
}