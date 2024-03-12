#ifndef UTILITIES_H
#define UTILITIES_H

#include <FastLED.h>
#include "conf.h"

enum STATE {STATE_NONE, STATE_MAIN, STATE_BRIGHTNESS, STATE_COLOR, STATE_FACTORY_RESET};
enum COLOR_PICKER_TYPE {CPT_NONE, CPT_COLOR_TEMPERATURE, CPT_COLOR_HUE};
enum WIFI_SIGNAL {WIFI_SIGNAL_NONE, WIFI_SIGNAL_DISCONNECTED, WIFI_SIGNAL_BAD, WIFI_SIGNAL_GOOD, WIFI_SIGNAL_EXCELLENT};
enum WEATHER {  WEATHER_NONE, 
                WEATHER_CLEAR_SKY_DAY, WEATHER_CLEAR_SKY_NIGHT,
                WEATHER_FEW_CLOUDS_DAY, WEATHER_FEW_CLOUDS_NIGHT,
                WEATHER_SCATTERED_CLOUDS_DAY, WEATHER_SCATTERED_CLOUDS_NIGHT,
                WEATHER_BROKEN_CLOUDS_DAY, WEATHER_BROKEN_CLOUDS_NIGHT,
                WEATHER_SHOWER_RAIN_DAY, WEATHER_SHOWER_RAIN_NIGHT,
                WEATHER_RAIN_DAY, WEATHER_RAIN_NIGHT,
                WEATHER_THUNDERSTORM_DAY, WEATHER_THUNDERSTORM_NIGHT,
                WEATHER_SNOW_DAY, WEATHER_SNOW_NIGHT,
                WEATHER_MIST_DAY, WEATHER_MIST_NIGHT
            };

extern const char* NTP_server_domain;
extern const char* openWeatherServerURL_formatable;

extern const char* monthNames[];
extern const char* stateString[];
extern const char* CPT_String[];
extern const char* wifiSignalString[];
extern const char* weatherString[];

extern char defaultSoftAP_ssid[MAX_SOFTAP_SSID_LENGTH];
extern char defaultSoftAP_pwd[MAX_SOFTAP_PWD_LENGTH];

#endif