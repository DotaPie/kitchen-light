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
#define RGB_PICKER_OFFSET_X 16
#define RGB_PICKER_OFFSET_Y 80
#define RGB_PICKER_HEIGHT 80

/* Note: Due to the nature of added complexity, RGB picker width is fixed to 288px (with display being 320px).
 */


#endif