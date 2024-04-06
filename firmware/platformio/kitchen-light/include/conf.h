#ifndef CONF_H
#define CONF_H

/* What is affected by development macro?
 *
 * 1) Soft AP SSID is always Kitchen light #1234 and Soft AP PWD is always 12345678, instead of being randomly generated on each factory reset.
 * 2) Default LED brightness is 16 instead 255 (full).
 */
//#define DEVELOPMENT

// general
#define DELAY_BEFORE_STARTUP_MS 10

// LED strip
#define LED_STRIP_MAX_LED_COUNT 9999
#define LED_STRIP_TYPE WS2812B
#define COLOR_ORDER GRB

// dont burn eyes out when developing
#ifdef DEVELOPMENT
#define DEFAULT_BRIGHTNESS 16
#else
#define DEFAULT_BRIGHTNESS 255
#endif
#define COLOR_BLENDING LINEARBLEND

// display
#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 320
#define DISPLAY_ROTATION_DEGREE 270 // rotation is increasing CW, supported values are [0, 90, 180, 270]
#define PICKER_OFFSET_X 16
#define PICKER_OFFSET_Y 80
#define PICKER_HEIGHT 80
#define PICKER_WIDTH 288 // number should be divisible by 6, or behaviour might vary
#define MAIN_SCREEN_TIMER_MS 1000 // do not change !!! used by doubledot animation, etc.
#define ANY_SETTING_SCREEN_TIMER_MS 2000 

// encoders
#define ROTARY_ENCODER_STEPS 30
#define ENCODER_SWITCH_DEBOUNCE_TIMER_MS 333
#define BRIGHTNESS_STEP 8 // 8 brightness change for 1 encoder step
#define COLOR_HUE_INDEX_STEP 4 // 4 color picker change for 1 encoder step
#define COLOR_TEMPERATURE_INDEX_STEP 4 // 8 color picker change for 1 encoder step

// preferences
#define DEFAULT_PREFERENCES_ID 107 // change this to other number if you want to force default configuration after next upload
#define MAX_PREFERENCE_LENGTH 128
#define FACTORY_RESET_TIMEOUT_MS 5000
#define INVALID_WIFI_SSID "****"
#define INVALID_WIFI_PWD "****"
#define INVALID_TIMEZONE "****"
#define INVALID_CITY "****"
#define INVALID_COUNTRY_CODE "**"
#define INVALID_LAT_LON "****"
#define INVALID_API_KEY "****"

// wifi
#define WIFI_SSID_MAX_LENGTH 128
#define WIFI_PWD_MAX_LENGTH 128
#define WIFI_CONNECT_TIMEOUT_MS 20000 // 20 s
#define WIFI_SERVER_PORT 80
#define MAX_SOFTAP_SSID_LENGTH 64
#define MAX_SOFTAP_PWD_LENGTH 32
#define SERVER_CLIENT_TIMEOUT_MS 10000 // 10 s
#define WIFI_CONNECTION_CHECK_TIMER_MS 1000

// datetime
#define YEAR_OFFSET 1900
#define TIME_ZONE_MAX_LENGTH 64 
#define SETUP_SYNC_DATE_TIME_TIMEOUT_MS 15000 // 15 s
#define LOOP_SYNC_DATE_TIME_TIMEOUT_MS 10
#define DATE_TIME_SYNC_TIMEOUT_MS 604800000 // 1 week

// weather
#define CITY_MAX_LENGTH 64
#define COUNTRY_CODE_MAX_LENGTH 2
#define LAT_LON_MAX_LENGTH 16
#define API_KEY_MAX_LENGTH 128
#define UPDATE_WEATHER_MS 600000 // 10 min
#define MAX_HTTP_PAYLOAD_SIZE 4096
#define MAX_SERVER_URL_SIZE 512
#define WEATHER_SYNC_TIMEOUT_MS 1800000 // 30 min

// soft AP
#define SOFT_AP_TIMEOUT_MS 3600000 // 1 hour

#endif