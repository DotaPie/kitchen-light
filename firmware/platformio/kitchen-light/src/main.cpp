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
COLOR_PICKER_TYPE current_CPT, previous_CPT;
CRGB currentColor, previousColor;
uint8_t previousSwitch_1, previousSwitch_2;
uint16_t encoder_1_position, encoder_2_position;

const char* stateString[] = {"NONE", "MAIN", "BRIGHTNESS", "COLOR"};
const char* CPT_String[] = {"NONE", "COLOR_TEMPERATURE", "COLOR_HUE"};

CRGB LED_stripArray[LED_STRIP_LED_COUNT];
RotaryEncoder encoder_1 = RotaryEncoder(RE_1_IN1_PIN, RE_1_IN2_PIN, RotaryEncoder::LatchMode::TWO03);
RotaryEncoder encoder_2 = RotaryEncoder(RE_2_IN1_PIN, RE_2_IN2_PIN, RotaryEncoder::LatchMode::TWO03);

Preferences preferences;

// function definitions

void loadPreferences()
{
    bool firstTimeRun = false;

    CONSOLE("Loading preferences: ")
    preferences.begin("app", false);

    if(preferences.getUInt("firstRun", 0) == 0)
    {
        firstTimeRun = true;
        preferences.putUInt("firstRun", 100);

        preferences.putUChar("CPT", (uint8_t)CPT_COLOR_TEMPERATURE);

        preferences.putUChar("color-R", 255); // TODO .. neutral K 
        preferences.putUChar("color-G", 255); // TODO .. neutral K 
        preferences.putUChar("color-B", 255); // TODO .. neutral K 
    }

    current_CPT = (COLOR_PICKER_TYPE)preferences.getUChar("CPT", (uint8_t)CPT_NONE);  
    currentColor = CRGB(preferences.getUChar("color-R", 255), preferences.getUChar("color-G", 255), preferences.getUChar("color-B", 255)); // TODO .. neutral K  
      
    previous_CPT = CPT_NONE;  
    previousColor = CRGB(0, 0, 0);

    encoder_1_position = 0;
    encoder_2_position = 0;
    
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
    CONSOLE("LED strip: ")
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
    CONSOLE("Rotary encoder #1: ")
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

    CONSOLE_CRLF("OK") 
}

void loadDefaultValues()
{
    CONSOLE("Loading default values: ")
    state = STATE_MAIN;
    previousState = STATE_NONE;
    previousSwitch_1 = digitalRead(SWITCH_1_PIN);
    previousSwitch_2 = digitalRead(SWITCH_2_PIN);
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

        CONSOLE("SWITCH_1 STATE CHANGE: ");
        CONSOLE_CRLF(switch_1 == LOW ? "ON" : "OFF");
    }

    if(switch_2 != previousSwitch_2 && millis() - switch_2_debounce_timer > SWITCH_DEBOUNCE_TIMER_MS)
    {
        switch_2_debounce_timer = millis();
        previousSwitch_2 = switch_2;

        update_LED_strip();

        CONSOLE("SWITCH_2 STATE CHANGE: ");
        CONSOLE_CRLF(switch_2 == LOW ? "ON" : "OFF");
    }
}

void checkRotaryEncoders(uint32_t *rotary_encoder_1_timer, uint32_t *rotary_encoder_2_timer)
{
    encoder_1.tick();
    encoder_2.tick();

    uint16_t encoder_1_new_position = encoder_1.getPosition();
    int8_t encoder_1_direction = (int8_t)encoder_1.getDirection();

    uint16_t encoder_2_new_position = encoder_2.getPosition();
    int8_t encoder_2_direction = (int8_t)encoder_2.getDirection();

    uint8_t encoder_1_switch = digitalRead(RE_1_SW_PIN);
    uint8_t encoder_2_switch = digitalRead(RE_2_SW_PIN);

    static uint32_t encoder_1_switch_debounce_timer = 0;
    static uint32_t encoder_2_switch_debounce_timer = 0;

    if(encoder_1_position != encoder_1_new_position)
    {
        *rotary_encoder_1_timer = millis();
        encoder_1_position = encoder_1_new_position;

        CONSOLE_CRLF("ROTARY ENCODER 1 CHANGE")
        CONSOLE("  |-- position: ")
        CONSOLE_CRLF(encoder_1_new_position % ROTARY_ENCODER_STEPS)
        CONSOLE("  |-- direction: ")
        CONSOLE_CRLF(encoder_1_direction)

        if(state != STATE_BRIGHTNESS)
        {
            state = STATE_BRIGHTNESS;
        }    
    }
    else if(encoder_2_position != encoder_2_new_position)
    {
        *rotary_encoder_2_timer = millis();
        encoder_2_position = encoder_2_new_position;

        CONSOLE_CRLF("ROTARY ENCODER 2 CHANGE")
        CONSOLE("  |-- position: ")
        CONSOLE_CRLF(encoder_2_new_position % ROTARY_ENCODER_STEPS)
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

        // RESERVED         
    }

    if(encoder_2_switch == LOW && millis() - encoder_2_switch_debounce_timer > ENCODER_SWITCH_DEBOUNCE_TIMER_MS)
    {
        encoder_2_switch_debounce_timer = millis();
        current_CPT = (current_CPT == CPT_COLOR_TEMPERATURE) ? CPT_COLOR_HUE : CPT_COLOR_TEMPERATURE;   
        CONSOLE("COLOR PICKER TYPE CHANGE: ")  
        CONSOLE_CRLF(CPT_String[(uint8_t)current_CPT])      
    }
}

void setup()
{
    delay(DELAY_BEFORE_STARTUP_MS);
    CONSOLE_SERIAL.begin(CONSOLE_BAUDRATE);
    CONSOLE_CRLF()
    CONSOLE_CRLF("~~~ SETUP ~~~")
    CONSOLE_CRLF()

    loadDefaultValues();
    loadPreferences();
    setupDisplay();
    setupRotaryEncoders();
    setupSwitches();
    setup_LED_strip();
    CONSOLE_CRLF()

    state = STATE_MAIN;

    CONSOLE_CRLF()
    CONSOLE_CRLF("~~~ LOOP ~~~")
    CONSOLE_CRLF()
}

void loop() 
{
    static uint32_t mainScreenTimer = 0;
    static uint32_t rotary_encoder_1_timer = 0;
    static uint32_t rotary_encoder_2_timer = 0;

    if(state != previousState)
    {
        previousState = state;

        if(state == STATE_MAIN)
        {
            clearDisplay();
            updateMainScreen(true, 88, 88, 88, 4, 8888, 88.88, 3);   
        }
        else if(state == STATE_BRIGHTNESS)
        {
            clearDisplay();
            // TODO: draw brightness bar    
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

        CONSOLE("STATE CHANGE: ");
        CONSOLE_CRLF(stateString[(uint8_t)state])

        if(state == STATE_COLOR)
        {
            CONSOLE("  |-- color picker type: ")
            CONSOLE_CRLF(CPT_String[(uint8_t)current_CPT]);
        }
    }

    // update screen once a second
    if(millis() - mainScreenTimer > MAIN_SCREEN_TIMER_MS && state == STATE_MAIN)
    {
        mainScreenTimer = millis();
        updateMainScreen(false, 88, 88, 88, 4, 8888, 88.88, 3);
    }

    checkSwitches();
    checkRotaryEncoders(&rotary_encoder_1_timer, &rotary_encoder_2_timer);

    if(state == STATE_BRIGHTNESS && millis() - rotary_encoder_1_timer > ANY_SETTING_SCREEN_TIMER_MS)
    {
        state = STATE_MAIN;
    }
    else if(state == STATE_COLOR && millis() - rotary_encoder_2_timer > ANY_SETTING_SCREEN_TIMER_MS)
    {
        state = STATE_MAIN;
    }
}