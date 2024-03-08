#ifndef CONF_H
#define CONF_H

// general
#define DELAY_BEFORE_STARTUP_MS 10

// LED strip
#define DEVELOPMENT // LED strip only with 3 LEDs

#ifdef DEVELOPMENT
#define LED_STRIP_LED_COUNT 3
#define ZONE_1_LED_START_INDEX 0
#define ZONE_1_LED_END_INDEX 0
#define ZONE_2_LED_START_INDEX 1
#define ZONE_2_LED_END_INDEX 1 
#define ZONE_3_LED_START_INDEX 2 
#define ZONE_3_LED_END_INDEX 2
#elif
#define LED_STRIP_LED_COUNT 213
#define ZONE_1_LED_START_INDEX 0
#define ZONE_1_LED_END_INDEX ?
#define ZONE_2_LED_START_INDEX ?
#define ZONE_2_LED_END_INDEX ?
#define ZONE_3_LED_START_INDEX ? 
#define ZONE_3_LED_END_INDEX 212
#endif

#define LED_STRIP_TYPE WS2812B
#define COLOR_ORDER GRB
#define DEFAULT_BRIGHTNESS 255
#define COLOR_BLENDING LINEARBLEND


// display
#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 320
#define DISPLAY_ROTATION_DEGREE 270 // rotation is increasing CW, supported values are [0, 90, 180, 270]
#define PICKER_OFFSET_X 16
#define PICKER_OFFSET_Y 80
#define PICKER_HEIGHT 80
#define PICKER_WIDTH 288 // number should be divisible by 6, or behaviour might vary
#define MAIN_SCREEN_TIMER_MS 1000 // do not change, doubledot animation is tied to this time
#define ANY_SETTING_SCREEN_TIMER_MS 2000

// switches
#define SWITCH_DEBOUNCE_TIMER_MS 5

// encoders
#define ROTARY_ENCODER_STEPS 30
#define ENCODER_SWITCH_DEBOUNCE_TIMER_MS 333
#define BRIGHTNESS_STEP 8 // 8 brightness change for 1 encoder step
#define COLOR_HUE_INDEX_STEP 4 // 4 color picker change for 1 encoder step
#define COLOR_TEMPERATURE_INDEX_STEP 4 // 8 color picker change for 1 encoder step

// preferences
#define DEFAULT_ID 103 // change this to other number if you want to force default configuration after next upload

#endif