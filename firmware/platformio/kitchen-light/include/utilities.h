#ifndef UTILITIES_H
#define UTILITIES_H

#include <FastLED.h>
#include "conf.h"

enum STATE {STATE_NONE, STATE_MAIN, STATE_BRIGHTNESS, STATE_COLOR, STATE_FACTORY_RESET};
enum COLOR_PICKER_TYPE {CPT_NONE, CPT_COLOR_TEMPERATURE, CPT_COLOR_HUE};
enum WIFI_SIGNAL {WIFI_SIGNAL_NONE, WIFI_SIGNAL_DISCONNECTED, WIFI_SIGNAL_BAD, WIFI_SIGNAL_GOOD, WIFI_SIGNAL_EXCELLENT};

extern const char* NTP_server_domain;
extern const char* openWeatherServerURL_formatable;

extern const char* monthNames[];
extern const char* stateString[];
extern const char* CPT_String[];
extern const char* wifiSignalString[];

extern char defaultSoftAP_ssid[MAX_SOFTAP_SSID_LENGTH];
extern char defaultSoftAP_pwd[MAX_SOFTAP_PWD_LENGTH];

#endif