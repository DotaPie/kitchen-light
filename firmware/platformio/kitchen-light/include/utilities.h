#ifndef UTILITIES_H
#define UTILITIES_H

#include <FastLED.h>
#include "conf.h"

enum class ScreenState {NONE, MAIN, BRIGHTNESS, COLOR};
enum class ColorPickerType {NONE, COLOR_TEMPERATURE, COLOR_HUE};
enum class WifiSignal {NONE, DISCONNECTED, BAD, GOOD, EXCELLENT};
enum class Weather {    NONE, 
                        CLEAR_SKY_DAY, CLEAR_SKY_NIGHT,
                        FEW_CLOUDS_DAY, FEW_CLOUDS_NIGHT,
                        SCATTERED_CLOUDS_DAY, SCATTERED_CLOUDS_NIGHT,
                        BROKEN_CLOUDS_DAY, BROKEN_CLOUDS_NIGHT,
                        SHOWER_RAIN_DAY, SHOWER_RAIN_NIGHT,
                        RAIN_DAY, RAIN_NIGHT,
                        THUNDERSTORM_DAY, THUNDERSTORM_NIGHT,
                        SNOW_DAY, SNOW_NIGHT,
                        MIST_DAY, MIST_NIGHT
                    };
enum class WeatherLocationType {NONE, CITY_AND_COUNTRY_CODE, LAT_LON};

extern const char* NTP_server_domain;
extern const IPAddress pingIp;
extern const char* openWeatherServerUrlformatableCityAndCountryCode;
extern const char* openWeatherServerUrlformatableLatLon;

extern const char* monthNames[];
extern const char* stateString[];
extern const char* CPT_String[];
extern const char* wifiSignalString[];
extern const char* weatherString[];

extern char defaultSoftAP_ssid[MAX_SOFTAP_SSID_LENGTH];
extern char defaultSoftAP_pwd[MAX_SOFTAP_PWD_LENGTH];

#endif