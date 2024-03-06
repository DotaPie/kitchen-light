#include <Arduino.h>
#include <Preferences.h>

// project includes
#include "console.h"
#include "pinout.h"
#include "conf.h"
#include "macros.h"
#include "display.h"

// libs
#include <RotaryEncoder.h>
#include <FastLED.h>

// enums
enum STATE {STATE_NONE, STATE_MAIN, STATE_BRIGHTNESS, STATE_COLOR};
enum COLOR_PICKER_TYPE {CPT_NONE, CPT_COLOR_TEMPERATURE, CPT_COLOR_HUE};

// globals
STATE state, previousState;

Preferences preferences;
COLOR_PICKER_TYPE current_CPT;
CRGB currentColor;
uint8_t currentBrightness;

uint8_t previousSwitch_1, previousSwitch_2;
long previous_encoder_1_position, previous_encoder_2_position;

const char* stateString[] = {"NONE", "MAIN", "BRIGHTNESS", "COLOR"};
const char* CPT_String[] = {"NONE", "COLOR_TEMPERATURE", "COLOR_HUE"};

CRGB LED_stripArray[LED_STRIP_LED_COUNT];
RotaryEncoder encoder_1 = RotaryEncoder(RE_1_IN1_PIN, RE_1_IN2_PIN, RotaryEncoder::LatchMode::TWO03);
RotaryEncoder encoder_2 = RotaryEncoder(RE_2_IN1_PIN, RE_2_IN2_PIN, RotaryEncoder::LatchMode::TWO03);

void loadPreferences()
{
    bool firstTimeRun = false;

    CONSOLE("\r\nLoading preferences: ")
    preferences.begin("app", false);

    if(preferences.getUInt("firstRun", 0) != DEFAULT_ID)
    {
        firstTimeRun = true;
        preferences.putUInt("firstRun", DEFAULT_ID);

        preferences.putUChar("CPT", (uint8_t)CPT_COLOR_TEMPERATURE);

        preferences.putUChar("color-R", 255); // TODO .. neutral K 
        preferences.putUChar("color-G", 255); // TODO .. neutral K 
        preferences.putUChar("color-B", 255); // TODO .. neutral K 

        preferences.putUChar("brightness", DEFAULT_BRIGHTNESS);
    }

    current_CPT = (COLOR_PICKER_TYPE)preferences.getUChar("CPT", (uint8_t)CPT_NONE);  
    currentColor = CRGB(preferences.getUChar("color-R", 255), preferences.getUChar("color-G", 255), preferences.getUChar("color-B", 255)); // TODO .. neutral K  
    currentBrightness = preferences.getUChar("brightness", DEFAULT_BRIGHTNESS);    
    
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

void update_LED_strip()
{
    for(uint16_t i = 0; i < LED_STRIP_LED_COUNT; i++)
    {
        LED_stripArray[i] = CRGB(0, 0, 0);    
    }

    // zone 1
    if(previousSwitch_1 == LOW)
    {
        for(uint16_t i = 0; i < ZONE_1_LED_COUNT; i++)
        {
            LED_stripArray[i] = currentColor;    
        }
    }

    // zone 2
    if(previousSwitch_2 == LOW)
    {
        for(uint16_t i = ZONE_1_LED_COUNT; i < LED_STRIP_LED_COUNT; i++)
        {
            LED_stripArray[i] = currentColor;    
        }
    }

    FastLED.show();
}

void LED_strip_load_animation()
{
    for(uint16_t i = 0; i < LED_STRIP_LED_COUNT; i++)
    {
        for(uint16_t j = 0; j < i; j++)
        {
            LED_stripArray[j] = ColorFromPalette(RainbowColors_p, (uint8_t)(j/2), DEFAULT_BRIGHTNESS, COLOR_BLENDING);   
        }

        for(uint16_t j = i; j < LED_STRIP_LED_COUNT; j++)
        {
            LED_stripArray[j] = CRGB(0, 0, 0);   
        }  

        FastLED.show();
        delay(6);
    }
}   

void setup_LED_strip()
{
    CONSOLE("\r\nLED strip: ")
    FastLED.addLeds<LED_STRIP_TYPE, LED_STRIP_PIN, COLOR_ORDER>(LED_stripArray, LED_STRIP_LED_COUNT).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(DEFAULT_BRIGHTNESS);
    
    LED_strip_load_animation();
    update_LED_strip();

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
    CONSOLE("\r\nRotary encoder #1: ")
    attachInterrupt(digitalPinToInterrupt(RE_1_IN1_PIN), checkRotaryEncoderPosition_1, CHANGE);
    attachInterrupt(digitalPinToInterrupt(RE_1_IN2_PIN), checkRotaryEncoderPosition_1, CHANGE);
    pinMode(RE_1_SW_PIN, INPUT_PULLUP);
    CONSOLE_CRLF("OK")

    CONSOLE("Rotary encoder #2: ")
    attachInterrupt(digitalPinToInterrupt(RE_2_IN1_PIN), checkRotaryEncoderPosition_2, CHANGE);
    attachInterrupt(digitalPinToInterrupt(RE_2_IN2_PIN), checkRotaryEncoderPosition_2, CHANGE);
    pinMode(RE_2_SW_PIN, INPUT_PULLUP);
    CONSOLE_CRLF("OK")
}

void setupSwitches()
{
    CONSOLE("\r\nSwitches: ")   
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
    CONSOLE("\r\nDisplay: ")
    display.init(DISPLAY_WIDTH, DISPLAY_HEIGHT);       
    display.setRotation(DISPLAY_ROTATION_DEGREE/90);
    display.invertDisplay(false); 
    display.fillScreen(ST77XX_BLACK);
    display.setCursor(4, 4);
    display.setTextColor(ST77XX_WHITE);
    display.setTextSize(2);
    display.setTextWrap(true);
    display.print("Booting ...");

    CONSOLE_CRLF("OK") 
}

void loadDefaultValues()
{
    CONSOLE("\r\nLoading default values: ")
    state = STATE_MAIN;
    previousState = STATE_NONE;
    previousSwitch_1 = digitalRead(SWITCH_1_PIN);
    previousSwitch_2 = digitalRead(SWITCH_2_PIN);
    previous_encoder_1_position = 0;
    previous_encoder_2_position = 0;
    CONSOLE_CRLF("OK")
}

void checkSwitches()
{
    bool switch_1 = digitalRead(SWITCH_1_PIN);
    bool switch_2 = digitalRead(SWITCH_2_PIN);
    static uint32_t switch_1_debounce_timer = 0;
    static uint32_t switch_2_debounce_timer = 0;

    if(switch_1 != previousSwitch_1 && millis() - switch_1_debounce_timer > SWITCH_DEBOUNCE_TIMER_MS)
    {
        switch_1_debounce_timer = millis();
        previousSwitch_1 = switch_1;

        update_LED_strip();

        CONSOLE("\r\nSWITCH_1 STATE CHANGE: ");
        CONSOLE_CRLF(switch_1 == LOW ? "ON" : "OFF");
    }

    if(switch_2 != previousSwitch_2 && millis() - switch_2_debounce_timer > SWITCH_DEBOUNCE_TIMER_MS)
    {
        switch_2_debounce_timer = millis();
        previousSwitch_2 = switch_2;

        update_LED_strip();

        CONSOLE("\r\nSWITCH_2 STATE CHANGE: ");
        CONSOLE_CRLF(switch_2 == LOW ? "ON" : "OFF");
    }
}

void updateBrightness(int direction)
{
    uint8_t prevBrightness = currentBrightness;
    int32_t tempBrightness = currentBrightness + (direction * BRIGHTNESS_STEP);

    if(tempBrightness < 0)
    {
        currentBrightness = 0;
    }
    else if(tempBrightness > 255)
    {
        currentBrightness = 255;
    }
    else 
    {
        currentBrightness = (uint8_t)tempBrightness;     
    }

    CONSOLE_CRLF("\r\nBRIGHTNESS UPDATE")
    CONSOLE("  |-- previous value: ")
    CONSOLE_CRLF(prevBrightness)
    CONSOLE("  |-- new value: ")
    CONSOLE_CRLF(currentBrightness)

    FastLED.setBrightness(currentBrightness);
    FastLED.show();

    updateBrightnessDisplay(currentBrightness);
}

void checkRotaryEncoders(uint32_t *rotary_encoder_timer)
{
    int8_t encoder_1_direction, encoder_2_direction;

    encoder_1.tick();
    encoder_2.tick();

    long encoder_1_position = encoder_1.getPosition();
    long encoder_2_position = encoder_2.getPosition();

    uint8_t encoder_1_switch = digitalRead(RE_1_SW_PIN);
    uint8_t encoder_2_switch = digitalRead(RE_2_SW_PIN);

    static uint32_t encoder_1_switch_debounce_timer = 0;
    static uint32_t encoder_2_switch_debounce_timer = 0;

    if( previous_encoder_1_position != encoder_1_position)
    {
        *rotary_encoder_timer = millis();
        previous_encoder_1_position = encoder_1_position;
        encoder_1_direction = (int)(encoder_1.getDirection());

        CONSOLE_CRLF("\r\nROTARY ENCODER 1 CHANGE")
        CONSOLE("  |-- position: ")
        CONSOLE_CRLF(encoder_1_position)
        CONSOLE("  |-- direction: ")
        CONSOLE_CRLF(encoder_1_direction)

        if(state != STATE_BRIGHTNESS)
        {
            state = STATE_BRIGHTNESS;
        }    
        else
        {
            updateBrightness(encoder_1_direction);
        }
    }
   
    if(previous_encoder_2_position != encoder_2_position)
    {
        *rotary_encoder_timer = millis();
        previous_encoder_2_position = encoder_2_position;
        encoder_2_direction = (int)(encoder_2.getDirection());

        CONSOLE_CRLF("\r\nROTARY ENCODER 2 CHANGE")
        CONSOLE("  |-- position: ")
        CONSOLE_CRLF(encoder_2_position)
        CONSOLE("  |-- direction: ")
        CONSOLE_CRLF(encoder_2_direction)

        if(state != STATE_COLOR)
        {
            state = STATE_COLOR;
        }  
    }

    if(encoder_1_switch == LOW && millis() - encoder_1_switch_debounce_timer > ENCODER_SWITCH_DEBOUNCE_TIMER_MS)
    {
        encoder_1_switch_debounce_timer = millis();
        *rotary_encoder_timer = millis();

        // unused - reserved         
    }

    if(encoder_2_switch == LOW && millis() - encoder_2_switch_debounce_timer > ENCODER_SWITCH_DEBOUNCE_TIMER_MS && state == STATE_COLOR)
    {
        encoder_2_switch_debounce_timer = millis();
        *rotary_encoder_timer = millis();

        current_CPT = (current_CPT == CPT_COLOR_TEMPERATURE) ? CPT_COLOR_HUE : CPT_COLOR_TEMPERATURE;  
        CONSOLE("\r\nCOLOR PICKER TYPE CHANGE: ")  
        CONSOLE_CRLF(CPT_String[(uint8_t)current_CPT])      
    }
}

void updatePreferences()
{
    if(current_CPT != (COLOR_PICKER_TYPE)preferences.getUChar("CPT"))
    {
        preferences.putUChar("CPT", (uint8_t)current_CPT);
    }

    if(currentColor.r != preferences.getUChar("color-R"))
    {
        preferences.putUChar("color-R", currentColor.r);    
    }

    if(currentColor.g != preferences.getUChar("color-G"))
    {
        preferences.putUChar("color-G", currentColor.g);    
    }

    if(currentColor.b != preferences.getUChar("color-B"))
    {
        preferences.putUChar("color-B", currentColor.b);    
    }

    if(currentBrightness != preferences.getUChar("brightness"))
    {
        preferences.putUChar("brightness", currentBrightness);
    }
}

void setup()
{
    delay(DELAY_BEFORE_STARTUP_MS);
    CONSOLE_SERIAL.begin(CONSOLE_BAUDRATE);
    CONSOLE_CRLF("~~~ SETUP ~~~")

    loadDefaultValues();
    loadPreferences();
    setupDisplay();
    setupRotaryEncoders();
    setupSwitches();
    setup_LED_strip();
    CONSOLE_CRLF()

    state = STATE_MAIN;

    CONSOLE_CRLF("~~~ LOOP ~~~")
}

void loop() 
{
    static uint32_t mainScreenTimer = 0;
    static uint32_t rotary_encoder_timer = 0;

    // handle inputs
    checkSwitches();
    checkRotaryEncoders(&rotary_encoder_timer);

    if((state == STATE_BRIGHTNESS || state == STATE_COLOR) && millis() - rotary_encoder_timer > ANY_SETTING_SCREEN_TIMER_MS)
    {
        state = STATE_MAIN;
    }

    // handle state change
    if(state != previousState)
    {
        previousState = state;

        CONSOLE("\r\nSTATE CHANGE: ");
        CONSOLE_CRLF(stateString[(uint8_t)state])

        if(state == STATE_COLOR)
        {
            CONSOLE("  |-- color picker type: ")
            CONSOLE_CRLF(CPT_String[(uint8_t)current_CPT]);
        }

        if(state == STATE_MAIN)
        {
            // in case there has been any changes to preferences
            updatePreferences();

            clearDisplay();
            updateMainScreen(true, 88, 88, 88, 4, 8888, 88.88, 3);   
        }
        else if(state == STATE_BRIGHTNESS)
        {
            clearDisplay();
            loadBrightness(currentBrightness);  
        }
        else if(state == STATE_COLOR)
        {
            clearDisplay();

            if(current_CPT == CPT_COLOR_TEMPERATURE)
            {
                // TODO: draw color temperature bar 
            }
            else if(current_CPT == CPT_COLOR_HUE)
            {
                // TODO: draw color hue bar   
            }
        }
    }

    // update screen once a second
    if(millis() - mainScreenTimer > MAIN_SCREEN_TIMER_MS && state == STATE_MAIN)
    {
        mainScreenTimer = millis();
        updateMainScreen(false, 88, 88, 88, 4, 8888, 88.88, 3);
    }
}