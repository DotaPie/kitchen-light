#ifndef CONF_H
#define CONF_H

// general
#define DELAY_BEFORE_STARTUP_MS 10

// LED strip
#define DEVELOPMENT // LED strip only with 3 LEDs

#ifdef DEVELOPMENT
#define LED_STRIP_LED_COUNT 3
#elif
#define LED_STRIP_LED_COUNT 213
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
#define MAIN_SCREEN_TIMER_MS 1000 // do not change !!! used by doubledot animation, wi-fi check, etc.
#define ANY_SETTING_SCREEN_TIMER_MS 2000

// encoders
#define ROTARY_ENCODER_STEPS 30
#define ENCODER_SWITCH_DEBOUNCE_TIMER_MS 333
#define BRIGHTNESS_STEP 8 // 8 brightness change for 1 encoder step
#define COLOR_HUE_INDEX_STEP 4 // 4 color picker change for 1 encoder step
#define COLOR_TEMPERATURE_INDEX_STEP 4 // 8 color picker change for 1 encoder step

// preferences
#define DEFAULT_ID 100 // change this to other number if you want to force default configuration after next upload
#define MAX_PREFERENCE_LENGTH 60
#define FACTORY_RESET_TIMEOUT_MS 5000
#define INVALID_WIFI_SSID "****"
#define INVALID_WIFI_PWD "****"
#define INVALID_TIMEZONE "****"
#define INVALID_CITY "****"
#define INVALID_COUNTRY_CODE "**"
#define INVALID_API_KEY "****"

// wifi
#define WIFI_SSID_MAX_LENGTH 128
#define WIFI_PWD_MAX_LENGTH 128
#define WIFI_CONNECT_TIMEOUT_MS 12000
#define WIFI_SERVER_PORT 80
#define MAX_SOFTAP_SSID_LENGTH 64
#define MAX_SOFTAP_PWD_LENGTH 32
#define SERVER_CLIENT_TIMEOUT_MS 10000

// time
#define DEFAULT_GMT_OFFSET_SECONDS 3600
#define DEFAULT_DAYLIGHT_OFFSET_HOURS 0
#define SECONDS_IN_HOUR 3600
#define YEAR_OFFSET 1900
#define TIME_ZONE_MAX_LENGTH 64 
#define WHEN_TO_TIME_SYNC_HOUR 4 // sync time once a day on start of this hour
#define SETUP_SYNC_DATE_TIME_TIMEOUT_MS 10000
#define LOOP_SYNC_DATE_TIME_TIMEOUT_MS 10

// weather
#define CITY_MAX_LENGTH 64
#define COUNTRY_CODE_MAX_LENGTH 3
#define API_KEY_MAX_LENGTH 128
#define UPDATE_WEATHER_MS 600000
#define MAX_HTTP_PAYLOAD_SIZE 4096
#define MAX_SERVER_URL_SIZE 512
#define OPENWEATHER_TEMPERATURE_OFFSET 273.15

// soft AP
#define SOFT_AP_TIMEOUT_MS 3600000

#endif