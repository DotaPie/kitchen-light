// core includes
#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_sntp.h"

// project includes
#include "console.h"
#include "pinout.h"
#include "conf.h"
#include "macros.h"
#include "display.h"
#include "utilities.h"
#include "html.h"

// libs
#include <RotaryEncoder.h>
#include <FastLED.h>
#include <ArduinoJson.h>

// core globals
ScreenState state = ScreenState::MAIN; 
ScreenState previousState = ScreenState::NONE;
bool validWifiSetup = false; // true if connected to wi-fi, internet connection does not matter here
Preferences preferences;

// TODO: all this and other stuff to structs
// TODO: move shit away from main.cpp, ideally keep only setup() and loop()

// preferences globals: will be loaded in setup -> loadPreferences();
ColorPickerType current_CPT, previous_CPT;
uint16_t currentColorHueIndex, previousColorHueIndex;
uint16_t currentColorTemperatureIndex, previousColorTemperatureIndex;
uint8_t currentBrightness, previousBrightness;
uint16_t rng_id;
uint32_t rng_pwd;  
char wifi_ssid[WIFI_SSID_MAX_LENGTH + 1] = "";
char wifi_pwd[WIFI_PWD_MAX_LENGTH + 1] = "";
char timeZone[TIME_ZONE_MAX_LENGTH + 1] = "";
char city[CITY_MAX_LENGTH + 1] = "";
char countryCode[COUNTRY_CODE_MAX_LENGTH + 1] = "";
char lat[LAT_LON_MAX_LENGTH + 1] = "";
char lon[LAT_LON_MAX_LENGTH + 1] = "";
char openWeatherAPI_key[API_KEY_MAX_LENGTH + 1] = "";
uint16_t numberOfLeds;
uint16_t previousNumberOfLeds;

// encoder globals
RotaryEncoder encoder_1 = RotaryEncoder(RE_1_IN1_PIN, RE_1_IN2_PIN, RotaryEncoder::LatchMode::TWO03);
RotaryEncoder encoder_2 = RotaryEncoder(RE_2_IN1_PIN, RE_2_IN2_PIN, RotaryEncoder::LatchMode::TWO03);
long previous_encoder_1_position = 0; 
long previous_encoder_2_position = 0;
uint32_t encoder_1_switch_debounce_timer = 0;
uint32_t encoder_2_switch_debounce_timer = 0;

// weather telemetry globals
float temperature_C = -273.15;
uint8_t humidity = 255;
float windSpeed = -1.0;
WifiSignal wifiSignal = WifiSignal::DISCONNECTED;
Weather weather = Weather::NONE;
bool validWeather = false;
uint32_t weatherSyncTimer = 0; // value does not matter

// time sync globals
bool validDateTime = false;
struct tm timeInfo;
volatile uint32_t dateTimeSyncTimer = 0;

// other globals
WiFiServer server(WIFI_SERVER_PORT); 
CRGB LED_stripArray[LED_STRIP_MAX_LED_COUNT];
 
void loadPreferences()
{
    bool firstTimeRun = false;

    CONSOLE("Loading preferences: ")
    preferences.begin("app", false);

    // write default config
    if(preferences.getUInt("firstRun", 0) != DEFAULT_PREFERENCES_ID)
    {
        randomSeed(analogRead(UNCONNECTED_ANALOG_PIN));
        firstTimeRun = true;
        preferences.putUInt("firstRun", DEFAULT_PREFERENCES_ID);
        preferences.putUChar("CPT", (uint8_t)ColorPickerType::COLOR_TEMPERATURE);
        preferences.putUInt("color-hue", 0);
        preferences.putUInt("color-t", 0);
        preferences.putUChar("brightness", DEFAULT_BRIGHTNESS);
        preferences.putBytes("wifi_ssid", INVALID_WIFI_SSID, WIFI_SSID_MAX_LENGTH + 1);
        preferences.putBytes("wifi_pwd", INVALID_WIFI_PWD, WIFI_PWD_MAX_LENGTH + 1);
        preferences.putBytes("time-zone", INVALID_TIMEZONE, TIME_ZONE_MAX_LENGTH + 1);
        preferences.putBytes("city", INVALID_CITY, CITY_MAX_LENGTH + 1);
        preferences.putBytes("country-c", INVALID_COUNTRY_CODE, COUNTRY_CODE_MAX_LENGTH + 1);
        preferences.putBytes("lat", INVALID_LAT_LON, LAT_LON_MAX_LENGTH + 1);
        preferences.putBytes("lon", INVALID_LAT_LON, LAT_LON_MAX_LENGTH + 1);
        preferences.putBytes("api-key", INVALID_API_KEY, API_KEY_MAX_LENGTH + 1);
        #ifdef DEVELOPMENT
            preferences.putUInt("rng-id", 1234);
            preferences.putUInt("rng-pwd", 12345678);    
        #else
            preferences.putUInt("rng-id", random(1000, 10000));
            preferences.putUInt("rng-pwd", random(10000000, 100000000));
        #endif
        preferences.putUInt("n-leds", 0);
    }

    current_CPT = (ColorPickerType)preferences.getUChar("CPT", (uint8_t)ColorPickerType::NONE);  
    previous_CPT = current_CPT;
    currentColorHueIndex = preferences.getUInt("color-hue", 0); 
    previousColorHueIndex = currentColorHueIndex;
    currentColorTemperatureIndex = preferences.getUInt("color-t", 0); 
    previousColorTemperatureIndex = currentColorTemperatureIndex;
    currentBrightness = preferences.getUChar("brightness", DEFAULT_BRIGHTNESS);
    previousBrightness = currentBrightness;    

    preferences.getBytes("wifi_ssid", wifi_ssid, WIFI_SSID_MAX_LENGTH + 1);
    preferences.getBytes("wifi_pwd", wifi_pwd, WIFI_PWD_MAX_LENGTH + 1);
    preferences.getBytes("time-zone", timeZone, TIME_ZONE_MAX_LENGTH + 1);
    preferences.getBytes("city", city, CITY_MAX_LENGTH + 1);
    preferences.getBytes("country-c", countryCode, COUNTRY_CODE_MAX_LENGTH + 1);
    preferences.getBytes("lat", lat, LAT_LON_MAX_LENGTH + 1);
    preferences.getBytes("lon", lon, LAT_LON_MAX_LENGTH + 1);
    preferences.getBytes("api-key", openWeatherAPI_key, API_KEY_MAX_LENGTH + 1);

    rng_id = preferences.getUInt("rng-id", 1234);
    rng_pwd = preferences.getUInt("rng-pwd", 12345678);

    numberOfLeds = preferences.getUInt("n-leds", 0);
    previousNumberOfLeds = numberOfLeds;

    sprintf(defaultSoftAP_ssid, "Kitchen light #%d", rng_id);
    sprintf(defaultSoftAP_pwd, "%d", rng_pwd);
    
    CONSOLE_CRLF("OK")

    CONSOLE("  |-- first time run: ") 
    CONSOLE_CRLF(firstTimeRun ? "Yes" : "No")

    CONSOLE("  |-- current color picker type: ") 
    CONSOLE_CRLF(CPT_String[(uint8_t)current_CPT])

    CONSOLE("  |-- current color hue index: ") 
    CONSOLE_CRLF(currentColorHueIndex)

    CONSOLE("  |-- current color temperature index: ") 
    CONSOLE_CRLF(currentColorTemperatureIndex)

    CONSOLE("  |-- wifi ssid: ");
    CONSOLE_CRLF(wifi_ssid)

    CONSOLE("  |-- wifi password: ");
    CONSOLE_CRLF(wifi_pwd)

    CONSOLE("  |-- time zone: ");
    CONSOLE_CRLF(timeZone)

    CONSOLE("  |-- city: ");
    CONSOLE_CRLF(city)

    CONSOLE("  |-- country code: ");
    CONSOLE_CRLF(countryCode)

    CONSOLE("  |-- lat: ");
    CONSOLE_CRLF(lat)

    CONSOLE("  |-- lon: ");
    CONSOLE_CRLF(lon)

    CONSOLE("  |-- openweather API key: ");
    CONSOLE_CRLF(openWeatherAPI_key)

    CONSOLE("  |-- rng id: ")
    CONSOLE_CRLF(rng_id)

    CONSOLE("  |-- rng pwd: ")
    CONSOLE_CRLF(rng_pwd)

    CONSOLE("  |-- number of LEDs: ")
    CONSOLE_CRLF(numberOfLeds)
}

void update_LED_strip()
{
    CRGB color = (current_CPT == ColorPickerType::COLOR_HUE) ? calculateColorHueFromPickerPosition(currentColorHueIndex) : calculateColorTemperatureFromPickerPosition(currentColorTemperatureIndex);
    
    for(uint16_t i = 0; i < numberOfLeds; i++)
    {
        LED_stripArray[i] = color;     
    }

    FastLED.show();
}

void updateNumberOfLeds(long direction, bool valueLocked)
{
    int32_t tempNumberOfLeds = numberOfLeds + (direction);

    if(tempNumberOfLeds < 0)
    {
        tempNumberOfLeds = 0;
    }
    else if(tempNumberOfLeds > LED_STRIP_MAX_LED_COUNT)
    {
        tempNumberOfLeds = LED_STRIP_MAX_LED_COUNT;
    }
    else 
    {
        numberOfLeds = (uint16_t)tempNumberOfLeds;     
    }

    CONSOLE_CRLF("NUMBER OF LEDS UPDATE")
    CONSOLE("  |-- previous value: ")
    CONSOLE_CRLF(previousNumberOfLeds)
    CONSOLE("  |-- new value: ")
    CONSOLE_CRLF(numberOfLeds)

    if(previousNumberOfLeds != numberOfLeds)
    {
        updateDisplayNumberOfLeds(numberOfLeds, valueLocked);
    }

    previousNumberOfLeds = numberOfLeds;
}

void setup_LED_strip()
{
    if(numberOfLeds == 0)
    {
        FastLED.addLeds<LED_STRIP_TYPE, LED_STRIP_PIN, COLOR_ORDER>(LED_stripArray, LED_STRIP_MAX_LED_COUNT).setCorrection(TypicalLEDStrip);

        for(uint16_t i = 0; i < LED_STRIP_MAX_LED_COUNT; i++)
        {
            LED_stripArray[i] = CRGB(0, 0, 0);    
        }

        FastLED.setBrightness(DEFAULT_BRIGHTNESS);
        FastLED.show();

        loadDisplayNumberOfLeds();
        updateDisplayNumberOfLeds(numberOfLeds, false);

        while(digitalRead(RE_1_SW_PIN) == HIGH && digitalRead(RE_2_SW_PIN) == HIGH)
        {
            int8_t encoder_1_direction, encoder_2_direction;

            encoder_1.tick();
            encoder_2.tick();

            long encoder_1_position = encoder_1.getPosition();
            long encoder_2_position = encoder_2.getPosition();

            if( previous_encoder_1_position != encoder_1_position)
            {
                previous_encoder_1_position = encoder_1_position;
                encoder_1_direction = (int)(encoder_1.getDirection());

                CONSOLE_CRLF("ROTARY ENCODER 1 CHANGE")
                CONSOLE("  |-- position: ")
                CONSOLE_CRLF(encoder_1_position)
                CONSOLE("  |-- direction: ")
                CONSOLE_CRLF(encoder_1_direction)

                updateNumberOfLeds(encoder_1_direction, false);

                // in case we decrease value, we first need to pass the numberOfLeds + 1, so we can set last LED from previous numberOfLeds to black
                FastLED.addLeds<LED_STRIP_TYPE, LED_STRIP_PIN, COLOR_ORDER>(LED_stripArray, encoder_1_direction == -1 ? numberOfLeds + 1 : numberOfLeds).setCorrection(TypicalLEDStrip);

                for(uint16_t i = 0; i < numberOfLeds; i++)
                {
                    LED_stripArray[i] = CRGB(0, 255, 0);    
                }

                // in case we decrease value, make sure we set the last LED from previous numberOfLeds to black
                if(encoder_1_direction == -1)
                {
                    LED_stripArray[numberOfLeds] = CRGB(0, 0, 0);    
                }

                FastLED.show();

                // in case we decrease value, make sure we pass the proper numberOfLeds
                if(encoder_1_direction == -1)
                {
                    FastLED.addLeds<LED_STRIP_TYPE, LED_STRIP_PIN, COLOR_ORDER>(LED_stripArray, numberOfLeds).setCorrection(TypicalLEDStrip);
                }
            }
        
            if(previous_encoder_2_position != encoder_2_position)
            {
                previous_encoder_2_position = encoder_2_position;
                encoder_2_direction = (int)(encoder_2.getDirection());

                CONSOLE_CRLF("ROTARY ENCODER 2 CHANGE")
                CONSOLE("  |-- position: ")
                CONSOLE_CRLF(encoder_2_position)
                CONSOLE("  |-- direction: ")
                CONSOLE_CRLF(encoder_2_direction)

                updateNumberOfLeds(encoder_2_direction, false);

                // in case we decrease value, we first need to pass the size numberOfLeds + 1, so we can set last LED from previous numberOfLeds to black
                FastLED.addLeds<LED_STRIP_TYPE, LED_STRIP_PIN, COLOR_ORDER>(LED_stripArray, encoder_2_direction == -1 ? numberOfLeds + 1 : numberOfLeds).setCorrection(TypicalLEDStrip);

                for(uint16_t i = 0; i < numberOfLeds; i++)
                {
                    LED_stripArray[i] = CRGB(0, 255, 0);    
                }

                // in case we decrease value, make sure we set the last LED from previous numberOfLeds to black
                if(encoder_2_direction == -1)
                {
                    LED_stripArray[numberOfLeds] = CRGB(0, 0, 0);    
                }

                FastLED.show();

                // in case we decrease value, make sure we pass the proper size
                if(encoder_2_direction == -1)
                {
                    FastLED.addLeds<LED_STRIP_TYPE, LED_STRIP_PIN, COLOR_ORDER>(LED_stripArray, numberOfLeds).setCorrection(TypicalLEDStrip);
                }
            }   
        }

        clearDisplay();
        updateDisplayNumberOfLeds(numberOfLeds, true);
        delay(2000);

        preferences.putUInt("n-leds", numberOfLeds); // make change persistent
    }

    CONSOLE("LED strip: ")
    FastLED.addLeds<LED_STRIP_TYPE, LED_STRIP_PIN, COLOR_ORDER>(LED_stripArray, numberOfLeds).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(currentBrightness);
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

void updateBrightness(int direction)
{
    int32_t tempBrightness = currentBrightness + (direction * BRIGHTNESS_STEP);

    if(tempBrightness < 0)
    {
        currentBrightness = 0;
    }
    else if(tempBrightness > 255)
    {
        currentBrightness = 255;
    }
    else 
    {
        currentBrightness = (uint8_t)tempBrightness;     
    }

    CONSOLE_CRLF("BRIGHTNESS UPDATE")
    CONSOLE("  |-- previous value: ")
    CONSOLE_CRLF(previousBrightness)
    CONSOLE("  |-- new value: ")
    CONSOLE_CRLF(currentBrightness)

    FastLED.setBrightness(currentBrightness);
    FastLED.show();

    if(previousBrightness != currentBrightness)
    {
        updateDisplayBrightness(currentBrightness);
    }

    previousBrightness = currentBrightness;
}

void updateColorHue(int direction)
{   
    int32_t tempColorHueIndex = currentColorHueIndex + (direction * COLOR_HUE_INDEX_STEP);

    if(tempColorHueIndex < 0)
    {
        currentColorHueIndex = PICKER_WIDTH - 1;
    }
    else if(tempColorHueIndex >= PICKER_WIDTH)
    {
        currentColorHueIndex = 0;
    }
    else 
    {
        currentColorHueIndex = (uint16_t)tempColorHueIndex;     
    }

    CRGB currentColor = calculateColorHueFromPickerPosition(currentColorHueIndex);
    CRGB previousColor = calculateColorHueFromPickerPosition(previousColorHueIndex);
    

    CONSOLE_CRLF("COLOR HUE UPDATE")
    CONSOLE("  |-- previous picker value: ")
    CONSOLE_CRLF(previousColorHueIndex)
    CONSOLE("  |-- previous color value: ")
    CONSOLE("[R: ")
    CONSOLE(previousColor.r)
    CONSOLE("| G: ")
    CONSOLE(previousColor.g)
    CONSOLE("| B: ")
    CONSOLE(previousColor.b)
    CONSOLE_CRLF("]")
    CONSOLE("  |-- new picker value: ")
    CONSOLE_CRLF(currentColorHueIndex)
    CONSOLE("  |-- new color value: ")
    CONSOLE("[R: ")
    CONSOLE(currentColor.r)
    CONSOLE("| G: ")
    CONSOLE(currentColor.g)
    CONSOLE("| B: ")
    CONSOLE(currentColor.b)
    CONSOLE_CRLF("]")

    update_LED_strip();

    if(previousColorHueIndex != currentColorHueIndex)
    {
        updateDisplayColorHue(currentColorHueIndex, previousColorHueIndex); 
    }
     
    previousColorHueIndex = currentColorHueIndex;
}

void updateColorTemperature(int direction)
{   
    int32_t tempColorTemperatureIndex = currentColorTemperatureIndex + (direction * COLOR_TEMPERATURE_INDEX_STEP);

    if(tempColorTemperatureIndex < 0)
    {
        currentColorTemperatureIndex = 0;
    }
    else if(tempColorTemperatureIndex >= PICKER_WIDTH)
    {
        currentColorTemperatureIndex = PICKER_WIDTH - 1;
    }
    else 
    {
        currentColorTemperatureIndex = (uint16_t)tempColorTemperatureIndex;     
    }

    CRGB currentColor = calculateColorTemperatureFromPickerPosition(currentColorTemperatureIndex);
    CRGB previousColor = calculateColorTemperatureFromPickerPosition(previousColorTemperatureIndex);
    
    CONSOLE_CRLF("COLOR TEMPERATURE UPDATE")
    CONSOLE("  |-- previous picker value: ")
    CONSOLE_CRLF(previousColorTemperatureIndex)
    CONSOLE("  |-- previous color value: ")
    CONSOLE("[R: ")
    CONSOLE(previousColor.r)
    CONSOLE("| G: ")
    CONSOLE(previousColor.g)
    CONSOLE("| B: ")
    CONSOLE(previousColor.b)
    CONSOLE_CRLF("]")
    CONSOLE("  |-- new picker value: ")
    CONSOLE_CRLF(currentColorTemperatureIndex)
    CONSOLE("  |-- new color value: ")
    CONSOLE("[R: ")
    CONSOLE(currentColor.r)
    CONSOLE("| G: ")
    CONSOLE(currentColor.g)
    CONSOLE("| B: ")
    CONSOLE(currentColor.b)
    CONSOLE_CRLF("]")

    update_LED_strip();

    if(previousColorTemperatureIndex != currentColorTemperatureIndex)
    {
        updateDisplayColorTemperature(currentColorTemperatureIndex, previousColorTemperatureIndex); 
    }
     
    previousColorTemperatureIndex = currentColorTemperatureIndex;
}

void checkRotaryEncoders(uint32_t *rotary_encoder_timer)
{
    int8_t encoder_1_direction, encoder_2_direction;

    encoder_1.tick();
    encoder_2.tick();

    long encoder_1_position = encoder_1.getPosition();
    long encoder_2_position = encoder_2.getPosition();

    uint8_t encoder_1_switch = digitalRead(RE_1_SW_PIN);
    uint8_t encoder_2_switch = digitalRead(RE_2_SW_PIN);

    if( previous_encoder_1_position != encoder_1_position)
    {
        *rotary_encoder_timer = millis();
        previous_encoder_1_position = encoder_1_position;
        encoder_1_direction = (int)(encoder_1.getDirection());

        CONSOLE_CRLF("ROTARY ENCODER 1 CHANGE")
        CONSOLE("  |-- position: ")
        CONSOLE_CRLF(encoder_1_position)
        CONSOLE("  |-- direction: ")
        CONSOLE_CRLF(encoder_1_direction)

        if(state != ScreenState::BRIGHTNESS)
        {
            state = ScreenState::BRIGHTNESS;
        }    
        else
        {
            updateBrightness(encoder_1_direction);
        }
    }
   
    if(previous_encoder_2_position != encoder_2_position)
    {
        *rotary_encoder_timer = millis();
        previous_encoder_2_position = encoder_2_position;
        encoder_2_direction = (int)(encoder_2.getDirection());

        CONSOLE_CRLF("ROTARY ENCODER 2 CHANGE")
        CONSOLE("  |-- position: ")
        CONSOLE_CRLF(encoder_2_position)
        CONSOLE("  |-- direction: ")
        CONSOLE_CRLF(encoder_2_direction)

        if(state != ScreenState::COLOR)
        {
            state = ScreenState::COLOR;
        } 
        else
        {
            if(current_CPT == ColorPickerType::COLOR_TEMPERATURE)
            {
                updateColorTemperature(encoder_2_direction);
            }
            else if(current_CPT == ColorPickerType::COLOR_HUE)
            {
                updateColorHue(encoder_2_direction);
            }
        } 
    }

    if(encoder_1_switch == LOW && encoder_2_switch == LOW)
    {
        clearDisplay();
        loadAndExecuteFactoryReset(&preferences); // this function is blocking, either ends up in reset or continue to main state

        encoder_1_switch_debounce_timer = millis();
        encoder_2_switch_debounce_timer = millis();

        state = ScreenState::MAIN;
    }
    else
    {
        if(encoder_1_switch == LOW && millis() - encoder_1_switch_debounce_timer > ENCODER_SWITCH_DEBOUNCE_TIMER_MS)
        {
            encoder_1_switch_debounce_timer = millis();
            *rotary_encoder_timer = millis();

            if(state == ScreenState::BRIGHTNESS)
            {
                // unused - reserved
            }
            else if(state == ScreenState::MAIN || state == ScreenState::COLOR)
            {
                state = ScreenState::BRIGHTNESS;
            }      
        }

        if(encoder_2_switch == LOW && millis() - encoder_2_switch_debounce_timer > ENCODER_SWITCH_DEBOUNCE_TIMER_MS)
        {
            encoder_2_switch_debounce_timer = millis();
            *rotary_encoder_timer = millis();

            if(state == ScreenState::COLOR)
            {
                current_CPT = (current_CPT == ColorPickerType::COLOR_TEMPERATURE) ? ColorPickerType::COLOR_HUE : ColorPickerType::COLOR_TEMPERATURE;  
                CONSOLE("COLOR PICKER TYPE CHANGE: ")  
                CONSOLE_CRLF(CPT_String[(uint8_t)current_CPT])  
            }  
            else if(state == ScreenState::MAIN || state == ScreenState::BRIGHTNESS)
            {
                state = ScreenState::COLOR;
            }   
        }
    }
}

void updateColorAndBrightnessPreferences()
{
    if(current_CPT != (ColorPickerType)preferences.getUChar("CPT"))
    {
        preferences.putUChar("CPT", (uint8_t)current_CPT);
    }

    if(currentColorHueIndex != preferences.getUInt("color-hue"))
    {
        preferences.putUInt("color-hue", currentColorHueIndex);
    }

    if(currentColorTemperatureIndex != preferences.getUInt("color-t"))
    {
        preferences.putUInt("color-t", currentColorTemperatureIndex);
    }

    if(currentBrightness != preferences.getUChar("brightness"))
    {
        preferences.putUChar("brightness", currentBrightness);
    }
}

void updateWifiSignal()
{
    int8_t rssi = WiFi.RSSI();
    wl_status_t wifiStatus = WiFi.status();

    CONSOLE("UPDATING WIFI SIGNAL: ")

    if(wifiStatus == WL_DISCONNECTED || !validWifiSetup)
    {
        wifiSignal = WifiSignal::DISCONNECTED;    
    }
    else if(rssi < -75)
    {
        wifiSignal = WifiSignal::BAD;
    } 
    else if(rssi >= -75 && rssi < -55) 
    {
        wifiSignal = WifiSignal::GOOD;    
    }
    else if(rssi >= -55 && rssi != 0)
    {
        wifiSignal = WifiSignal::EXCELLENT;
    } 

    CONSOLE_CRLF("OK")

    CONSOLE("  |-- RSSI: ")
    CONSOLE_CRLF(rssi)

    CONSOLE("  |-- Wi-Fi signal: ")
    CONSOLE_CRLF(wifiSignalString[(uint8_t)wifiSignal])
}

bool setupWifi()
{
    int8_t rssi;
    uint32_t timeout = millis();

    if(strcmp(wifi_ssid, INVALID_WIFI_SSID) == 0 && strcmp(wifi_pwd, INVALID_WIFI_PWD) == 0)
    {
        CONSOLE("WIFI STATUS: ")
        CONSOLE_CRLF("CREDENTIALS NOT SET")

        return false;    
    }

    WiFi.begin(wifi_ssid, wifi_pwd);

    while(WiFi.status() != WL_CONNECTED)
    {
        if(millis() > WIFI_CONNECT_TIMEOUT_MS)
        {
            CONSOLE("WIFI STATUS: ")
            CONSOLE_CRLF("ERROR")

            WiFi.disconnect();

            wifiSignal = WifiSignal::DISCONNECTED;
            return false;
        }

        CONSOLE("WIFI STATUS: ")
        CONSOLE_CRLF("CONNECTING")

        delay(1000);
    }

    rssi = WiFi.RSSI();

    CONSOLE("WIFI STATUS: ")
    CONSOLE_CRLF("OK")
    CONSOLE("  |-- AP name: ")
    CONSOLE_CRLF(WiFi.SSID())
    CONSOLE("  |-- IP: ")
    CONSOLE_CRLF(WiFi.localIP())

    validWifiSetup = true;
    updateWifiSignal();

    return true;
}

bool syncDateTime(bool waitForSync, uint32_t syncTimeoutMs)
{
    bool success;

    CONSOLE("SYNCING LOCAL TIME: ")

    configTime(0, 0, NTP_server_domain); // sync datetime, not setting gmt or daylight saving offset here

    if(!waitForSync)
    {
        CONSOLE_CRLF("SKIPPED WAIT FOR SYNC")
        return false;
    }

    success = getLocalTime(&timeInfo, syncTimeoutMs);
    CONSOLE_CRLF(success ? "OK" : "ERROR")

    return success;
}

void setTimezone()
{
    CONSOLE("SETTING TIME ZONE: ")

    setenv("TZ", timeZone, 1);
    tzset();

    CONSOLE_CRLF("OK")
    CONSOLE("  |-- timezone: ")
    CONSOLE_CRLF(timeZone)
}

void httpGETRequest(char* serverName, char *payload) {
    WiFiClient client;
    HTTPClient http;

    // Your Domain name with URL path or IP address with path
    http.begin(client, serverName);
    http.setTimeout(1000);

    // Send HTTP POST request
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) 
    {
        http.getString().toCharArray(payload, MAX_HTTP_PAYLOAD_SIZE + 1);
    }
    
    http.end();
}

/* Instead of analyzing description parameter of weather, we simply save icon parameter, which
 * exactly describes what kind of picture shall we use for given weather.
 * https://openweathermap.org/weather-conditions
 */
void updateWeather(const char* openweatherIconString)
{
    if(strcmp(openweatherIconString, "01d") == 0)
    {
        weather = Weather::CLEAR_SKY_DAY;
    }
    else if(strcmp(openweatherIconString, "01n") == 0)
    {
        weather = Weather::CLEAR_SKY_NIGHT;
    }
    else if(strcmp(openweatherIconString, "02d") == 0)
    {
        weather = Weather::FEW_CLOUDS_DAY;
    }
    else if(strcmp(openweatherIconString, "02n") == 0)
    {
        weather = Weather::FEW_CLOUDS_NIGHT;
    }
    else if(strcmp(openweatherIconString, "03d") == 0)
    {
        weather = Weather::SCATTERED_CLOUDS_DAY;
    }
    else if(strcmp(openweatherIconString, "03n") == 0)
    {
        weather = Weather::SCATTERED_CLOUDS_NIGHT;
    }
    else if(strcmp(openweatherIconString, "04d") == 0)
    {
        weather = Weather::BROKEN_CLOUDS_DAY;
    }
    else if(strcmp(openweatherIconString, "04n") == 0)
    {
        weather = Weather::BROKEN_CLOUDS_NIGHT;
    }
    else if(strcmp(openweatherIconString, "09d") == 0)
    {
        weather = Weather::SHOWER_RAIN_DAY;
    }
    else if(strcmp(openweatherIconString, "09n") == 0)
    {
        weather = Weather::SHOWER_RAIN_NIGHT;
    }
    else if(strcmp(openweatherIconString, "10d") == 0)
    {
        weather = Weather::RAIN_DAY;
    }
    else if(strcmp(openweatherIconString, "10n") == 0)
    {
        weather = Weather::RAIN_NIGHT;
    }
    else if(strcmp(openweatherIconString, "11d") == 0)
    {
        weather = Weather::THUNDERSTORM_DAY;
    }
    else if(strcmp(openweatherIconString, "11n") == 0)
    {
        weather = Weather::THUNDERSTORM_NIGHT;
    }
    else if(strcmp(openweatherIconString, "13d") == 0)
    {
        weather = Weather::SNOW_DAY;
    }
    else if(strcmp(openweatherIconString, "13n") == 0)
    {
        weather = Weather::SNOW_DAY;
    }
    else if(strcmp(openweatherIconString, "50d") == 0)
    {
        weather = Weather::MIST_DAY;
    }
    else if(strcmp(openweatherIconString, "50n") == 0)
    {
        weather = Weather::MIST_NIGHT;
    }
    else
    {
        weather = Weather::NONE;
    }
}

bool updateWeatherTelemetry()
{
    char payloadJSON[MAX_HTTP_PAYLOAD_SIZE + 1] = "";
    char serverURL[MAX_SERVER_URL_SIZE + 1] = "";  
    JsonDocument doc;
    
    if(strcmp(city, INVALID_CITY) != 0 && strcmp(countryCode, INVALID_COUNTRY_CODE))
    {
        sprintf(serverURL, openWeatherServerUrlformatableCityAndCountryCode, city, countryCode, openWeatherAPI_key);
    }
    else if(strcmp(lat, INVALID_LAT_LON) != 0 && strcmp(lon, INVALID_LAT_LON) != 0)
    {
        sprintf(serverURL, openWeatherServerUrlformatableLatLon, lat, lon, openWeatherAPI_key);   
    }

    httpGETRequest(serverURL, payloadJSON);

    CONSOLE("JSON RESPONSE: ");
    CONSOLE_CRLF(payloadJSON)

    CONSOLE("JSON DESERIALIZATON: ")
    DeserializationError error = deserializeJson(doc, payloadJSON);

    if(error)
    {
        CONSOLE_CRLF("ERROR");
        return false;
    }
    
    CONSOLE_CRLF("OK");

    temperature_C = doc["main"]["temp"].as<float>();
    humidity = doc["main"]["humidity"].as<uint8_t>();
    windSpeed = doc["wind"]["speed"].as<float>();
    const char *openweatherIconString = doc["weather"][0]["icon"].as<const char*>();

    updateWeather(openweatherIconString);

    CONSOLE_CRLF("WEATHER UPDATED");
    CONSOLE("  |-- temperature: ");
    CONSOLE_CRLF(temperature_C);
    CONSOLE("  |-- humidity: ");
    CONSOLE_CRLF(humidity);
    CONSOLE("  |-- wind speed: ");
    CONSOLE_CRLF(windSpeed);
    CONSOLE("  |-- openweather icon string: ");
    CONSOLE_CRLF(openweatherIconString);
    CONSOLE("  |-- weather string: ");
    CONSOLE_CRLF(weatherString[(uint8_t)weather]);

    return true;
}

void enableAP()
{
    WiFi.softAP(defaultSoftAP_ssid, defaultSoftAP_pwd);
    server.begin();

    CONSOLE_CRLF("SOFT AP INFO")
    CONSOLE("  |-- IP: ")
    CONSOLE_CRLF(WiFi.softAPIP())
    CONSOLE("  |-- SSID: ")
    CONSOLE_CRLF(defaultSoftAP_ssid)
    CONSOLE("  |-- password: ")
    CONSOLE_CRLF(defaultSoftAP_pwd)
}

void decodeUrlCodes(char *valBuffFiltered, char *valBuff)
{
    uint16_t index = 0;
    char buff[3] = "";
    char c;
    char *ptr;
    
    for(uint16_t i = 0; i < strlen(valBuff); i++)
    {
        c = valBuff[i];

        if(c == '%')
        {
            buff[0] = valBuff[i + 1];      
            buff[1] = valBuff[i + 2]; 

            valBuffFiltered[index++] = (char)strtol(buff, &ptr, 16); 
            i += 2; // skip the bytes we just stored
            continue;
        }
        else
        {
            valBuffFiltered[index++] = c; 
        }                       
    }           
}

void parseParameter(char *buff, const char *param, char *saveToParam, uint16_t paramMaxLength, char terminationChar)
{
    char valBuff[MAX_PREFERENCE_LENGTH + 1] = "";
    char c = '\0';
    uint16_t index = 0;
    char *ptr = strstr(buff, param);

    while(index < MAX_PREFERENCE_LENGTH)
    {
        char c = ptr[index + (strlen(param))]; // '=' already in param 

        if(c == terminationChar)
        {
            //char valBuffFiltered[MAX_PREFERENCE_LENGTH + 1] = "";
            //decodeUrlCodes(valBuffFiltered, valBuff); // for example '/' is encoded into %2F, put it back to '/'
            //strcpy(saveToParam, valBuffFiltered);
            strcpy(saveToParam, valBuff);
            break;
        }
       
        valBuff[index++] = c;

        if(index > paramMaxLength)  
        {
            CONSOLE_CRLF("PARSING PARAMETER: OVERFLOW")
            break;
        }     
    }
}

void parseNewParams(char *buff, WeatherLocationType weatherLocationType)
{
    // including '=' to param search, so we do not mistake for example "city=" for "city" in "cityAndCountryCode"

    parseParameter(buff, "ssid=", wifi_ssid, WIFI_SSID_MAX_LENGTH, '&');
    parseParameter(buff, "pwd=", wifi_pwd, WIFI_PWD_MAX_LENGTH, '&');

    if(weatherLocationType == WeatherLocationType::CITY_AND_COUNTRY_CODE)
    {
        parseParameter(buff, "city=", city, CITY_MAX_LENGTH, '&');
        parseParameter(buff, "country-code=", countryCode, COUNTRY_CODE_MAX_LENGTH, '&');
    }
    else if(weatherLocationType == WeatherLocationType::LAT_LON)
    {
        parseParameter(buff, "lat=", lat, LAT_LON_MAX_LENGTH, '&');
        parseParameter(buff, "lon=", lon, LAT_LON_MAX_LENGTH, '&');   
    }

    parseParameter(buff, "timezone=", timeZone, TIME_ZONE_MAX_LENGTH, '&'); 
    parseParameter(buff, "api-key=", openWeatherAPI_key, API_KEY_MAX_LENGTH, ' '); // last parameter in form terminates in space in HTTP header
}

void saveParsedParamsToPreferences(char *buff, WeatherLocationType weatherLocationType)
{
    preferences.putBytes("wifi_ssid", wifi_ssid, WIFI_SSID_MAX_LENGTH + 1);
    preferences.putBytes("wifi_pwd", wifi_pwd, WIFI_PWD_MAX_LENGTH + 1);
    
    if(weatherLocationType == WeatherLocationType::CITY_AND_COUNTRY_CODE)
    {
        preferences.putBytes("city", city, CITY_MAX_LENGTH + 1);
        preferences.putBytes("country-c", countryCode, COUNTRY_CODE_MAX_LENGTH + 1);
    }
    else if(weatherLocationType == WeatherLocationType::LAT_LON)
    {
        preferences.putBytes("lat", lat, LAT_LON_MAX_LENGTH + 1);
        preferences.putBytes("lon", lon, LAT_LON_MAX_LENGTH + 1);    
    }
    
    preferences.putBytes("time-zone", timeZone, TIME_ZONE_MAX_LENGTH + 1);
    preferences.putBytes("api-key", openWeatherAPI_key, API_KEY_MAX_LENGTH + 1);
}

void handleServerClients()
{
    WiFiClient client = server.available();   // Listen for incoming clients
    char buff[MAX_HTTP_PAYLOAD_SIZE] = "";
    uint16_t index = 0;
    uint32_t timeout = 0;
    WeatherLocationType weatherLocationType = WeatherLocationType::NONE;
    
    if(client)
    {
        CONSOLE_CRLF("SERVER: NEW CLIENT") 
        CONSOLE_CRLF("PACKET:") 
        CONSOLE_CRLF("- - - - - - - - -")

        timeout = millis();

        while(client.connected())
        {
            if(millis() - timeout > SERVER_CLIENT_TIMEOUT_MS)
            {
                CONSOLE_CRLF("- - - - - - - - -")
                CONSOLE_CRLF("SERVER: CLIENT TIMEOUT") 
                break;
            }

            while(client.available() > 0)
            {
                char c = client.read();
                CONSOLE(c)   
                buff[index++] = c; 
            }

            // look for end of header of any request
            if(strstr(buff, "\r\n\r\n") != NULL)
            {
                CONSOLE_CRLF("- - - - - - - - -")
                CONSOLE_CRLF("SERVER: HTTP REQUEST RECEIVED")

                // if request contains form data in URL, parse it, save it, send back ack html page and reboot
                if(  
                    strstr(buff, "ssid=") != NULL &&
                    strstr(buff, "pwd=") != NULL &&
                    strstr(buff, "location=") != NULL &&
                    strstr(buff, "timezone=") != NULL &&
                    strstr(buff, "api-key=") != NULL
                )
                {
                    CONSOLE_CRLF("SERVER: SETUP PACKET RECEIVED")
                    CONSOLE_CRLF("  |-- received all required parameters")

                    CONSOLE_CRLF("SERVER: SAVING NEW PARAMETERS TO PREFERENCES")

                    if(strstr(buff, "location=cityAndCountryCode") != NULL)
                    {
                        weatherLocationType = WeatherLocationType::CITY_AND_COUNTRY_CODE;
                    }
                    else if(strstr(buff, "location=latLon") != NULL)
                    {
                        weatherLocationType = WeatherLocationType::LAT_LON;
                    }

                    parseNewParams(buff, weatherLocationType);   
                    saveParsedParamsToPreferences(buff, weatherLocationType);

                    if(weatherLocationType == WeatherLocationType::CITY_AND_COUNTRY_CODE)
                    {
                        sprintf(buff, htmlWebPageCompleteFormatterCityAndCountryCode, wifi_ssid, wifi_pwd, city, countryCode, timeZone, openWeatherAPI_key);
                    }
                    else if(weatherLocationType == WeatherLocationType::LAT_LON)
                    {
                        sprintf(buff, htmlWebPageCompleteFormatterLatAndLon, wifi_ssid, wifi_pwd, lat, lon, timeZone, openWeatherAPI_key);
                    }
                    
                    client.print(buff);  
                    client.flush();
                    delay(1000); // sometimes flush was not enaugh
                    client.stop();
                    CONSOLE_CRLF("SERVER: CLIENT DISCONNECTED")   

                    CONSOLE_CRLF("ESP32: RESTART")  
                    ESP.restart();   
                }
                // for any request except favicon request, send HTML form (basically index.html)
                else if(strstr(buff, "favicon") == NULL)
                {
                    client.print(htmlWebPageForm); 
                }
                else
                {
                    // if favicon request, do nothing       
                }

                break;
            }
        }  
    }
}
/* To make sure date and time is always synced, reset it to some invalid value (basically any year <2015).
 * Alternatively we could enable user to use datetime even in offline mode, but to set it manually with knobs.
 * However for now I have decided to make datetime work only in online mode.
 */
void resetDatetime()
{
    struct timeval tv;
    tv.tv_sec = 0;
    settimeofday(&tv, NULL);
}

/* Default sync period is 1h. 
 * This can be changed by including "esp_sntp.h" and calling sntp_set_sync_interval(ms)
 * cbSyncTime() is callback to sync.
 * We use this callback to cover case when sync could possibly take couple of seconds, by displaying previous values.
 */
void callbackSyncTime(struct timeval *tv)
{
    dateTimeSyncTimer = millis();
}

void setup()
{
    delay(DELAY_BEFORE_STARTUP_MS);

    sntp_set_time_sync_notification_cb(callbackSyncTime);

    CONSOLE_SERIAL.begin(CONSOLE_BAUDRATE);
    CONSOLE_CRLF("~~~ SETUP ~~~")

    loadPreferences();
    resetDatetime();
    setupRotaryEncoders();
    setupDisplay();
    setup_LED_strip();
    showPleaseWaitOnDisplay();

    if(setupWifi())
    {
        validDateTime = syncDateTime(true, SETUP_SYNC_DATE_TIME_TIMEOUT_MS);
        setTimezone();
        validWeather = updateWeatherTelemetry();

        // in setup we can retry telemetry request
        if(!validWeather)
        {
            validWeather = updateWeatherTelemetry();   
        }

        if(validWeather)
        {
            weatherSyncTimer = millis();
        }
    }
    else
    {
        enableAP();
    }

    state = ScreenState::MAIN;
    
    CONSOLE_CRLF("~~~ LOOP ~~~")
}

void loop() 
{
    static uint32_t mainScreenTimer = 0; // force screen refresh immediately after enetering loop
    static uint32_t wifiConnectionCheckTimer = 0; // force wifi check immediately after enetering loop
    static uint32_t wifiConnectionCheckTimeoutSuccess = 0; // value does not matter
    static uint32_t weatherTimer = millis(); // start counting timer to weather sync after entering loop
    static uint32_t rotary_encoder_timer = 0; // value does not matter
    static uint32_t softApTimeout = millis(); // start counting timer to soft AP timeout after entering loop
    static bool offlineMode = false;

    // handle inputs
    checkRotaryEncoders(&rotary_encoder_timer);

    // auto state change to main after period of time
    if((state == ScreenState::BRIGHTNESS || state == ScreenState::COLOR) && millis() - rotary_encoder_timer > ANY_SETTING_SCREEN_TIMER_MS)
    {
        state = ScreenState::MAIN;
    }

    // handle state change
    if(state != previousState)
    {
        previousState = state;

        CONSOLE("SCREENSTATE CHANGE: ");
        CONSOLE_CRLF(stateString[(uint8_t)state])

        if(state == ScreenState::MAIN)
        {
            // in case there has been any changes to preferences
            updateColorAndBrightnessPreferences();

            mainScreenTimer = millis();
            validDateTime = getLocalTime(&timeInfo, LOOP_SYNC_DATE_TIME_TIMEOUT_MS);

            /* TODO: not sure if "sntp_set_time_sync_notification_cb()" is called only on success or even on fail, needs verification
             *
             * Idea is to override validDateTime from getLocalTime() return after long period of time.
             */
            clearDisplay();
            updateMainScreen(
                offlineMode, 
                validWifiSetup, 
                validWeather ? validWeather : ((millis() - weatherSyncTimer < WEATHER_SYNC_TIMEOUT_MS && validWifiSetup) ? true : validWeather), // keep displaying weather up to WEATHER_SYNC_TIMEOUT_MS even if not updated
                (millis() - dateTimeSyncTimer > DATE_TIME_SYNC_TIMEOUT_MS) ? false : validDateTime, 
                true, 
                timeInfo.tm_hour, 
                timeInfo.tm_min, 
                timeInfo.tm_mday, 
                timeInfo.tm_mon, 
                timeInfo.tm_year + YEAR_OFFSET, 
                temperature_C, 
                humidity, 
                windSpeed, 
                weather, 
                wifiSignal);  
        }
        else if(state == ScreenState::BRIGHTNESS)
        {
            clearDisplay();
            loadDisplayBrightness(currentBrightness);  
        }
        else if(state == ScreenState::COLOR)
        {
            CONSOLE("  |-- CPT: ")
            CONSOLE_CRLF(CPT_String[(uint8_t)current_CPT]);

            clearDisplay();

            if(current_CPT == ColorPickerType::COLOR_TEMPERATURE)
            {
                loadDisplayColorTemperature(currentColorTemperatureIndex, previousColorTemperatureIndex);
            }
            else if(current_CPT == ColorPickerType::COLOR_HUE)
            {
                loadDisplayColorHue(currentColorHueIndex, previousColorHueIndex);  
            }
        }
    }

    // handle CPT change (state remains ScreenState::COLOR)
    if(current_CPT != previous_CPT && state == ScreenState::COLOR) 
    {
        previous_CPT = current_CPT;

        CONSOLE("CPT CHANGE: ");
        CONSOLE_CRLF(CPT_String[(uint8_t)current_CPT])

        clearDisplay(); 

        if(current_CPT == ColorPickerType::COLOR_TEMPERATURE)
        {
            loadDisplayColorTemperature(currentColorTemperatureIndex, previousColorTemperatureIndex);
            updateColorTemperature(0); // force color change without direction from encoder
        }
        else if(current_CPT == ColorPickerType::COLOR_HUE)
        {
            loadDisplayColorHue(currentColorHueIndex, previousColorHueIndex);
            updateColorHue(0); // force color change without direction from encoder  
        }                
    }

    if(!offlineMode)
    {
        // allow clients to connect to soft AP and configure device
        if(!validWifiSetup)
        {
            handleServerClients();
    
            if(millis() - softApTimeout > SOFT_AP_TIMEOUT_MS)
            {
                offlineMode = true;

                WiFi.disconnect();
                WiFi.mode(WIFI_OFF);
            }
        }
        else if(validWifiSetup)
        {
            // check wifi connection
            if(millis() - wifiConnectionCheckTimer > WIFI_CONNECTION_CHECK_TIMER_MS)
            {
                wifiConnectionCheckTimer = millis();

                updateWifiSignal();
            }

            if(state == ScreenState::MAIN)
            {
                // once a second update local datetime and main screen (if anything needs update)
                if(millis() - mainScreenTimer > MAIN_SCREEN_TIMER_MS)
                {
                    mainScreenTimer = millis();
                    validDateTime = getLocalTime(&timeInfo, LOOP_SYNC_DATE_TIME_TIMEOUT_MS);

                    /* TODO: not sure if "sntp_set_time_sync_notification_cb()" is called only on success, needs verification
                     *
                     * Idea is to override validDateTime from getLocalTime() return after long period of time.
                     */
                    updateMainScreen(
                            offlineMode, 
                            validWifiSetup, 
                            validWeather ? validWeather : ((millis() - weatherSyncTimer < WEATHER_SYNC_TIMEOUT_MS && validWifiSetup) ? true : validWeather), // keep displaying weather up to WEATHER_SYNC_TIMEOUT_MS even if not updated
                            (millis() - dateTimeSyncTimer > DATE_TIME_SYNC_TIMEOUT_MS) ? false : validDateTime, 
                            false, 
                            timeInfo.tm_hour, 
                            timeInfo.tm_min, 
                            timeInfo.tm_mday, 
                            timeInfo.tm_mon, 
                            timeInfo.tm_year + YEAR_OFFSET, 
                            temperature_C, 
                            humidity, 
                            windSpeed, 
                            weather, 
                            wifiSignal); 
                }

                // update weather
                if(millis() - weatherTimer > UPDATE_WEATHER_MS)
                {
                    weatherTimer = millis();

                    if(WiFi.status() == WL_CONNECTED)
                    {
                        validWeather = updateWeatherTelemetry();

                        if(validWeather)
                        {
                            weatherSyncTimer = millis();
                        }
                    }
                    else
                    {
                        validWeather = false;
                    }
                }
            }
        }
    }
}