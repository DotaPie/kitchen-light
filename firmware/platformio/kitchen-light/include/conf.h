#ifndef CONF_H
#define CONF_H

// general
#define DELAY_BEFORE_STARTUP_MS 10

// LED strip
#define LED_STRIP_LED_COUNT 3 // 213
#define LED_STRIP_TYPE WS2812B
#define COLOR_ORDER GRB
#define DEFAULT_BRIGHTNESS 255
#define COLOR_BLENDING LINEARBLEND
#define ZONE_1_LED_COUNT 2
#define ZONE_2_LED_COUNT 1

// display
#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 320
#define DISPLAY_ROTATION_DEGREE 270 // rotation is increasing CW, supported values are [0, 90, 180, 270]
#define PICKER_OFFSET_X 16
#define PICKER_OFFSET_Y 80
#define PICKER_HEIGHT 80
#define MAIN_SCREEN_TIMER_MS 1000
#define ANY_SETTING_SCREEN_TIMER_MS 3000
#define PICKER_WIDTH 288 // due to the nature of added complexity, pickers width is fixed to 288px (with display being 320px), changing value will break color hue and color temperature pickers

// switches
#define SWITCH_DEBOUNCE_TIMER_MS 5

// encoders
#define ROTARY_ENCODER_STEPS 30
#define ENCODER_SWITCH_DEBOUNCE_TIMER_MS 333
#define BRIGHTNESS_STEP 8 // 8 brightness change for 1 encoder step

// preferences
#define DEFAULT_ID 101 // change this to other number if you want to force default configuration

#endif