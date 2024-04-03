#include "utilities.h"
#include "conf.h"

const char* monthNames[] = {"Jan",
                            "Feb",
                            "Mar",
                            "April",
                            "May",
                            "June",
                            "July",
                            "Aug",
                            "Sep",
                            "Oct",
                            "Nov",
                            "Dec"};

const char* stateString[] = {"NONE", "MAIN", "BRIGHTNESS", "COLOR", "FACTORY_RESET"};
const char* CPT_String[] = {"NONE", "COLOR_TEMPERATURE", "COLOR_HUE"};
const char* wifiSignalString[] = {"WIFI_SIGNAL_NONE", "WIFI_SIGNAL_DISCONNECTED", "WIFI_SIGNAL_BAD", "WIFI_SIGNAL_GOOD", "WIFI_SIGNAL_EXCELLENT"};
const char* weatherString[] = {"NONE", 
                                "CLEAR_SKY_DAY", "CLEAR_SKY_NIGHT",
                                "FEW_CLOUDS_DAY", "FEW_CLOUDS_NIGHT",
                                "SCATTERED_CLOUDS_DAY", "SCATTERED_CLOUDS_NIGHT",
                                "BROKEN_CLOUDS_DAY", "BROKEN_CLOUDS_NIGHT",
                                "SHOWER_RAIN_DAY", "SHOWER_RAIN_NIGHT",
                                "RAIN_DAY", "RAIN_NIGHT",
                                "THUNDERSTORM_DAY", "THUNDERSTORM_NIGHT",
                                "SNOW_DAY", "SNOW_NIGHT",
                                "MIST_DAY", "MIST_NIGHT"    
                            };

const char* NTP_server_domain = "pool.ntp.org";
const char* openWeatherServerURL_formatable = "http://api.openweathermap.org/data/2.5/weather?q=%s,%s&APPID=%s&units=metric";

char defaultSoftAP_ssid[MAX_SOFTAP_SSID_LENGTH] = "";
char defaultSoftAP_pwd[MAX_SOFTAP_PWD_LENGTH] = "";