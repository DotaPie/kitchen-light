// core includes
#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <HTTPClient.h>

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
STATE state = STATE_MAIN; 
STATE previousState = STATE_NONE;
bool validWifiConnection = false;
Preferences preferences;

// preferences globals: will be loaded in setup -> loadPreferences();
COLOR_PICKER_TYPE current_CPT, previous_CPT;
uint16_t currentColorHueIndex, previousColorHueIndex;
uint16_t currentColorTemperatureIndex, previousColorTemperatureIndex;
uint8_t currentBrightness, previousBrightness;
uint16_t rng_id;
uint32_t rng_pwd;  

// user config globals: will be loaded after initial device setup
char wifi_ssid[WIFI_SSID_MAX_LENGTH + 1] = "";
char wifi_pwd[WIFI_PWD_MAX_LENGTH + 1] = "";
char timeZone[TIME_ZONE_MAX_LENGTH + 1] = ""; // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.json
char city[CITY_MAX_LENGTH + 1] = "";
char countryCode[COUNTRY_CODE_MAX_LENGTH + 1] = "";
char openWeatherAPI_key[API_KEY_MAX_LENGTH + 1] = "";

// switch and encoder globals
RotaryEncoder encoder_1 = RotaryEncoder(RE_1_IN1_PIN, RE_1_IN2_PIN, RotaryEncoder::LatchMode::TWO03);
RotaryEncoder encoder_2 = RotaryEncoder(RE_2_IN1_PIN, RE_2_IN2_PIN, RotaryEncoder::LatchMode::TWO03);
long previous_encoder_1_position = 0; 
long previous_encoder_2_position = 0;
uint32_t encoder_1_switch_debounce_timer = 0;
uint32_t encoder_2_switch_debounce_timer = 0;
uint8_t previousSwitch_1, previousSwitch_2; // will be loaded in setup -> loadDefaults();

// weather telemetry globals
float temperature_C = -273.15;
uint8_t humidity = 255;
float windSpeed = -1.0;
WIFI_SIGNAL wifiSignal = WIFI_SIGNAL_DISCONNECTED;
WEATHER weather = WEATHER_NONE;

// other globals
WiFiServer server(WIFI_SERVER_PORT); 
CRGB LED_stripArray[LED_STRIP_LED_COUNT];
 
void loadPreferences()
{
    bool firstTimeRun = false;

    CONSOLE("Loading preferences: ")
    preferences.begin("app", false);

    if(preferences.getUInt("firstRun", 0) != DEFAULT_ID)
    {
        randomSeed(analogRead(UNCONNECTED_ANALOG_PIN));
        firstTimeRun = true;
        preferences.putUInt("firstRun", DEFAULT_ID);
        preferences.putUChar("CPT", (uint8_t)CPT_COLOR_TEMPERATURE);
        preferences.putUInt("color-hue", 0);
        preferences.putUInt("color-t", 0);
        preferences.putUChar("brightness", DEFAULT_BRIGHTNESS);
        preferences.putBytes("wifi_ssid", "****", WIFI_SSID_MAX_LENGTH + 1);
        preferences.putBytes("wifi_pwd", "****", WIFI_PWD_MAX_LENGTH + 1);
        preferences.putBytes("time-zone", "GMT0", TIME_ZONE_MAX_LENGTH + 1);
        preferences.putBytes("city", "****", CITY_MAX_LENGTH + 1);
        preferences.putBytes("country-c", "**", COUNTRY_CODE_MAX_LENGTH + 1);
        preferences.putBytes("api-key", "****", API_KEY_MAX_LENGTH + 1);
        preferences.putUInt("rng-id", random(1000, 10000));
        preferences.putUInt("rng-pwd", random(10000000, 100000000));
    }

    current_CPT = (COLOR_PICKER_TYPE)preferences.getUChar("CPT", (uint8_t)CPT_NONE);  
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
    preferences.getBytes("api-key", openWeatherAPI_key, API_KEY_MAX_LENGTH + 1);

    rng_id = preferences.getUInt("rng-id", 1234);
    rng_pwd = preferences.getUInt("rng-pwd", 123456);

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

    CONSOLE("  |-- openweather API key: ");
    CONSOLE_CRLF(openWeatherAPI_key)

    CONSOLE("  |-- rng id: ")
    CONSOLE_CRLF(rng_id)

    CONSOLE("  |-- rng pwd: ")
    CONSOLE_CRLF(rng_pwd)
}

void update_LED_strip()
{
    for(uint16_t i = 0; i < LED_STRIP_LED_COUNT; i++)
    {
        LED_stripArray[i] = CRGB(0, 0, 0);    
    }

    // zone 1 + 3
    if(previousSwitch_1 == LOW)
    {
        for(uint16_t i = ZONE_1_LED_START_INDEX; i <= ZONE_1_LED_END_INDEX; i++)
        {
            LED_stripArray[i] = (current_CPT == CPT_COLOR_HUE) ? calculateColorHueFromPickerPosition(currentColorHueIndex) : calculateColorTemperatureFromPickerPosition(currentColorTemperatureIndex);     
        }

        for(uint16_t i = ZONE_3_LED_START_INDEX; i <= ZONE_3_LED_END_INDEX; i++)
        {
            LED_stripArray[i] = (current_CPT == CPT_COLOR_HUE) ? calculateColorHueFromPickerPosition(currentColorHueIndex) : calculateColorTemperatureFromPickerPosition(currentColorTemperatureIndex);    
        }
    }

    // zone 2
    if(previousSwitch_2 == LOW)
    {
        for(uint16_t i = ZONE_2_LED_START_INDEX; i <= ZONE_2_LED_END_INDEX; i++)
        {
            LED_stripArray[i] = (current_CPT == CPT_COLOR_HUE) ? calculateColorHueFromPickerPosition(currentColorHueIndex) : calculateColorTemperatureFromPickerPosition(currentColorTemperatureIndex);     
        }
    }

    FastLED.show();
}

void LED_strip_load_animation()
{
    for(uint16_t i = 0; i < LED_STRIP_LED_COUNT; i++)
    {
        for(uint16_t j = 0; j < i; j++)
        {
            LED_stripArray[j] = ColorFromPalette(RainbowColors_p, (uint8_t)(j/2), DEFAULT_BRIGHTNESS, COLOR_BLENDING);   
        }

        for(uint16_t j = i; j < LED_STRIP_LED_COUNT; j++)
        {
            LED_stripArray[j] = CRGB(0, 0, 0);   
        }  

        FastLED.show();
        delay(6);
    }
}   

void setup_LED_strip()
{
    CONSOLE("LED strip: ")
    FastLED.addLeds<LED_STRIP_TYPE, LED_STRIP_PIN, COLOR_ORDER>(LED_stripArray, LED_STRIP_LED_COUNT).setCorrection(TypicalLEDStrip);

    #ifndef DEVELOPMENT   
    FastLED.setBrightness(DEFAULT_BRIGHTNESS);
    LED_strip_load_animation();
    #endif

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

void setupSwitches()
{
    CONSOLE("Switches: ")   
    pinMode(SWITCH_1_PIN, INPUT_PULLUP);
    pinMode(SWITCH_2_PIN, INPUT_PULLUP);
    CONSOLE_CRLF("OK") 

    CONSOLE("  |-- Switch state #1: ") 
    CONSOLE_CRLF(digitalRead(SWITCH_1_PIN) == LOW ? "ON" : "OFF")

    CONSOLE("  |-- Switch state #2: ") 
    CONSOLE_CRLF(digitalRead(SWITCH_2_PIN) == LOW ? "ON" : "OFF")
}

void loadDefaultValues()
{
    CONSOLE("Loading default values: ")

    previousSwitch_1 = digitalRead(SWITCH_1_PIN);
    previousSwitch_2 = digitalRead(SWITCH_2_PIN);

    CONSOLE_CRLF("OK")
}

void checkSwitches()
{
    bool switch_1 = digitalRead(SWITCH_1_PIN);
    bool switch_2 = digitalRead(SWITCH_2_PIN);
    static uint32_t switch_1_debounce_timer = 0;
    static uint32_t switch_2_debounce_timer = 0;

    if(switch_1 != previousSwitch_1 && millis() - switch_1_debounce_timer > SWITCH_DEBOUNCE_TIMER_MS)
    {
        switch_1_debounce_timer = millis();
        previousSwitch_1 = switch_1;

        update_LED_strip();

        CONSOLE("SWITCH_1 STATE CHANGE: ");
        CONSOLE_CRLF(switch_1 == LOW ? "ON" : "OFF");
    }

    if(switch_2 != previousSwitch_2 && millis() - switch_2_debounce_timer > SWITCH_DEBOUNCE_TIMER_MS)
    {
        switch_2_debounce_timer = millis();
        previousSwitch_2 = switch_2;

        update_LED_strip();

        CONSOLE("SWITCH_2 STATE CHANGE: ");
        CONSOLE_CRLF(switch_2 == LOW ? "ON" : "OFF");
    }
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

        if(state != STATE_BRIGHTNESS)
        {
            state = STATE_BRIGHTNESS;
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

        if(state != STATE_COLOR)
        {
            state = STATE_COLOR;
        } 
        else
        {
            if(current_CPT == CPT_COLOR_TEMPERATURE)
            {
                updateColorTemperature(encoder_2_direction);
            }
            else if(current_CPT == CPT_COLOR_HUE)
            {
                updateColorHue(encoder_2_direction);
            }
        } 
    }

    if(encoder_1_switch == LOW && encoder_2_switch == LOW)
    {
        state = STATE_FACTORY_RESET;
    }
    else
    {
        if(encoder_1_switch == LOW && millis() - encoder_1_switch_debounce_timer > ENCODER_SWITCH_DEBOUNCE_TIMER_MS)
        {
            encoder_1_switch_debounce_timer = millis();
            *rotary_encoder_timer = millis();

            if(state == STATE_BRIGHTNESS)
            {
                // unused - reserved
            }
            else if(state == STATE_MAIN || state == STATE_COLOR)
            {
                state = STATE_BRIGHTNESS;
            }      
        }

        if(encoder_2_switch == LOW && millis() - encoder_2_switch_debounce_timer > ENCODER_SWITCH_DEBOUNCE_TIMER_MS)
        {
            encoder_2_switch_debounce_timer = millis();
            *rotary_encoder_timer = millis();

            if(state == STATE_COLOR)
            {
                current_CPT = (current_CPT == CPT_COLOR_TEMPERATURE) ? CPT_COLOR_HUE : CPT_COLOR_TEMPERATURE;  
                CONSOLE("COLOR PICKER TYPE CHANGE: ")  
                CONSOLE_CRLF(CPT_String[(uint8_t)current_CPT])  
            }  
            else if(state == STATE_MAIN || state == STATE_BRIGHTNESS)
            {
                state = STATE_COLOR;
            }   
        }
    }
}

void updatePreferences()
{
    if(current_CPT != (COLOR_PICKER_TYPE)preferences.getUChar("CPT"))
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

void updateWifiSignal(int8_t rssi)
{
    CONSOLE("UPDATING WIFI SIGNAL: ")

    if(!validWifiConnection)
    {
        wifiSignal = WIFI_SIGNAL_DISCONNECTED;    
    }
    else if(rssi < -75)
    {
        wifiSignal = WIFI_SIGNAL_BAD;
    } 
    else if(rssi >= -75 && rssi < -55) 
    {
        wifiSignal = WIFI_SIGNAL_GOOD;    
    }
    else if(rssi >= -55)
    {
        wifiSignal = WIFI_SIGNAL_EXCELLENT;
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

    WiFi.begin(wifi_ssid, wifi_pwd);

    while(WiFi.status() != WL_CONNECTED)
    {
        if(millis() > WIFI_CONNECT_TIMEOUT_MS)
        {
            CONSOLE("Wi-Fi status: ")
            CONSOLE_CRLF("ERROR")

            WiFi.disconnect();

            wifiSignal = WIFI_SIGNAL_DISCONNECTED;
            return false;
        }
        
        CONSOLE("Wi-Fi status: ")
        CONSOLE_CRLF("CONNECTING")

        delay(1000);
    }

    rssi = WiFi.RSSI();

    CONSOLE("Wi-Fi status: ")
    CONSOLE_CRLF("OK")
    CONSOLE("  |-- AP name: ")
    CONSOLE_CRLF(WiFi.SSID())
    CONSOLE("  |-- IP: ")
    CONSOLE_CRLF(WiFi.localIP())

    validWifiConnection = true;
    updateWifiSignal(rssi);

    return true;
}

// TODO: fix formatting issue from URL of special symbols such as '/'
// TODO: verify AP disconnection, weather response not comming, etc...
void syncDateTime()
{
    CONSOLE("SYNCING LOCAL TIME: ")

    configTime(0, 0, NTP_server_domain); // timezone offset

    struct tm timeInfo;
    while(!getLocalTime(&timeInfo)); // sometimes this might take while

    setenv("TZ", timeZone, 1);
    tzset();

    CONSOLE_CRLF("OK")
    CONSOLE("  |-- timezone: ")
    CONSOLE_CRLF(timeZone)
}

void updateLocalTime(tm *timeInfo)
{
    if (!validWifiConnection)
    {
        return;
    }

    CONSOLE("UPDATING LOCAL TIME: ")

    if(!getLocalTime(timeInfo))
    {
        CONSOLE_CRLF("ERROR")
        return;
    }

    CONSOLE_CRLF("OK")
}

void httpGETRequest(char* serverName, char *payload) {
    WiFiClient client;
    HTTPClient http;

    // Your Domain name with URL path or IP address with path
    http.begin(client, serverName);

    // Send HTTP POST request
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) 
    {
        http.getString().toCharArray(payload, MAX_HTTP_PAYLOAD_SIZE + 1);
    }
    
    http.end();
}

/* https://openweathermap.org/weather-conditions
 */
void updateWeather(const char* openweatherIconString)
{
    if(strcmp(openweatherIconString, "01d") == 0)
    {
        weather = WEATHER_CLEAR_SKY_DAY;
    }
    else if(strcmp(openweatherIconString, "01n") == 0)
    {
        weather = WEATHER_CLEAR_SKY_NIGHT;
    }
    else if(strcmp(openweatherIconString, "02d") == 0)
    {
        weather = WEATHER_FEW_CLOUDS_DAY;
    }
    else if(strcmp(openweatherIconString, "02n") == 0)
    {
        weather = WEATHER_FEW_CLOUDS_NIGHT;
    }
    else if(strcmp(openweatherIconString, "03d") == 0)
    {
        weather = WEATHER_SCATTERED_CLOUDS_DAY;
    }
    else if(strcmp(openweatherIconString, "03n") == 0)
    {
        weather = WEATHER_SCATTERED_CLOUDS_NIGHT;
    }
    else if(strcmp(openweatherIconString, "04d") == 0)
    {
        weather = WEATHER_BROKEN_CLOUDS_DAY;
    }
    else if(strcmp(openweatherIconString, "04n") == 0)
    {
        weather = WEATHER_BROKEN_CLOUDS_NIGHT;
    }
    else if(strcmp(openweatherIconString, "09d") == 0)
    {
        weather = WEATHER_SHOWER_RAIN_DAY;
    }
    else if(strcmp(openweatherIconString, "09n") == 0)
    {
        weather = WEATHER_SHOWER_RAIN_NIGHT;
    }
    else if(strcmp(openweatherIconString, "10d") == 0)
    {
        weather = WEATHER_RAIN_DAY;
    }
    else if(strcmp(openweatherIconString, "10n") == 0)
    {
        weather = WEATHER_RAIN_NIGHT;
    }
    else if(strcmp(openweatherIconString, "11d") == 0)
    {
        weather = WEATHER_THUNDERSTORM_DAY;
    }
    else if(strcmp(openweatherIconString, "11n") == 0)
    {
        weather = WEATHER_THUNDERSTORM_NIGHT;
    }
    else if(strcmp(openweatherIconString, "13d") == 0)
    {
        weather = WEATHER_SNOW_DAY;
    }
    else if(strcmp(openweatherIconString, "13n") == 0)
    {
        weather = WEATHER_SNOW_DAY;
    }
    else if(strcmp(openweatherIconString, "50d") == 0)
    {
        weather = WEATHER_MIST_DAY;
    }
    else if(strcmp(openweatherIconString, "50n") == 0)
    {
        weather = WEATHER_MIST_NIGHT;
    }
    else
    {
        weather = WEATHER_NONE;
    }
}

void updateWeatherTelemetry()
{
    char payloadJSON[MAX_HTTP_PAYLOAD_SIZE + 1] = "";
    char serverURL[MAX_SERVER_URL_SIZE + 1] = "";  
    JsonDocument doc;

    sprintf(serverURL, openWeatherServerURL_formatable, city, countryCode, openWeatherAPI_key);

    for(uint8_t i = 1; i <= NUMBER_OF_RETRIES_FOR_WEATHER; i++)
    {
        httpGETRequest(serverURL, payloadJSON);

        CONSOLE("JSON RESPONSE: ");
        CONSOLE_CRLF(payloadJSON)

        CONSOLE("JSON DERESIALIZATON: ")
        DeserializationError error = deserializeJson(doc, payloadJSON);

        if(error)
        {
            CONSOLE("ERROR");

            if(i == NUMBER_OF_RETRIES_FOR_WEATHER)
            {
                CONSOLE_CRLF();
                return;
            }
            else
            {
                CONSOLE_CRLF(" (retrying)");
            }
        }
        else
        {
            break;
        }
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

void decodeUrlCodes(char *valBuff, char *valBuffFiltered)
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

void saveNewParamsToPreferences(char *buff)
{
    char valBuff[MAX_PREFERENCE_LENGTH + 1] = "";
    char c = '\0';
    uint16_t index = 0;
    char *ptr;
    
    // ssid
    ptr = strstr(buff, "ssid");
    memset(valBuff, '\0', MAX_PREFERENCE_LENGTH + 1);
    index = 0;
    while(index < MAX_PREFERENCE_LENGTH)
    {
        char c = ptr[index + 5]; // skip "ssid="

        if(c == '&')
        {
            strcpy(wifi_ssid, valBuff);
            break;
        }
       
        valBuff[index] = c;
        index += 1; 

        if(index >= WIFI_SSID_MAX_LENGTH)  
        {
            CONSOLE("PARSING PARAMETER: OVERFLOW")
            break;
        }     
    }

    // pwd
    ptr = strstr(buff, "pwd");
    memset(valBuff, '\0', MAX_PREFERENCE_LENGTH + 1);
    index = 0;
    while(index < MAX_PREFERENCE_LENGTH)
    {
        char c = ptr[index + 4]; // skip "pwd="

        if(c == '&')
        {
            strcpy(wifi_pwd, valBuff);
            break;
        }
       
        valBuff[index] = c;
        index += 1; 

        if(index >= WIFI_PWD_MAX_LENGTH)  
        {
            CONSOLE("PARSING PARAMETER: OVERFLOW")
            break;
        }     
    }

    // city
    ptr = strstr(buff, "city");
    memset(valBuff, '\0', MAX_PREFERENCE_LENGTH + 1);
    index = 0;
    while(index < MAX_PREFERENCE_LENGTH)
    {
        char c = ptr[index + 5]; // skip "city="

        if(c == '&')
        {
            strcpy(city, valBuff);
            break;
        }
       
        valBuff[index] = c;
        index += 1; 

        if(index >= CITY_MAX_LENGTH)  
        {
            CONSOLE("PARSING PARAMETER: OVERFLOW")
            break;
        }     
    }

    // country-code
    ptr = strstr(buff, "country-code");
    memset(valBuff, '\0', MAX_PREFERENCE_LENGTH + 1);
    index = 0;
    while(index < MAX_PREFERENCE_LENGTH)
    {
        char c = ptr[index + 13]; // skip "country-code="

        if(c == '&')
        {
            strcpy(countryCode, valBuff);
            break;
        }
       
        valBuff[index] = c;
        index += 1; 

        if(index >= COUNTRY_CODE_MAX_LENGTH)  
        {
            CONSOLE("PARSING PARAMETER: OVERFLOW")
            break;
        }     
    }

    // time zone
    ptr = strstr(buff, "timezones");
    memset(valBuff, '\0', MAX_PREFERENCE_LENGTH + 1);
    index = 0;
    while(index < MAX_PREFERENCE_LENGTH)
    {
        char c = ptr[index + 10]; // skip "timezones="

        if(c == '&')
        {
            char valBuffFiltered[MAX_PREFERENCE_LENGTH + 1] = "";
            decodeUrlCodes(valBuff, valBuffFiltered); // for example '/' is encoded into %2F, put it back to '/'
            strcpy(timeZone, valBuffFiltered);
            break;
        }
       
        valBuff[index] = c;
        index += 1;   

        if(index >= TIME_ZONE_MAX_LENGTH)  
        {
            CONSOLE("PARSING PARAMETER: OVERFLOW")
            break;
        }    
    }

    // daylight saving enabled
    ptr = strstr(buff, "api-key");
    memset(valBuff, '\0', MAX_PREFERENCE_LENGTH + 1);
    index = 0;
    while(index < MAX_PREFERENCE_LENGTH)
    {
        char c = ptr[index + 8]; // skip "api-key="

        // last parameter ends in ' ' instead of &
        if(c == ' ')
        {
            strcpy(openWeatherAPI_key, valBuff);
            break;
        }
       
        valBuff[index] = c;
        index += 1;   

        if(index >= API_KEY_MAX_LENGTH)  
        {
            CONSOLE("PARSING PARAMETER: OVERFLOW")
            break;
        }   
    }

    preferences.putBytes("wifi_ssid", wifi_ssid, WIFI_SSID_MAX_LENGTH + 1);
    preferences.putBytes("wifi_pwd", wifi_pwd, WIFI_PWD_MAX_LENGTH + 1);
    preferences.putBytes("time-zone", timeZone, TIME_ZONE_MAX_LENGTH + 1);
    preferences.putBytes("city", city, CITY_MAX_LENGTH + 1);
    preferences.putBytes("country-c", countryCode, COUNTRY_CODE_MAX_LENGTH + 1);
    preferences.putBytes("api-key", openWeatherAPI_key, API_KEY_MAX_LENGTH + 1);
}

void handleServerClients()
{
    WiFiClient client = server.available();   // Listen for incoming clients
    char buff[MAX_HTTP_PAYLOAD_SIZE] = "";
    uint16_t index = 0;
    uint32_t timeout = 0;
    static bool formPacketComplete = false;
    static bool setupPacketComplete = false;

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

            if(client.available() > 0)
            {
                char c =  client.read();
                CONSOLE(c)   
                buff[index++] = c; 
            }

            // look for end of header of any request
            if(strstr(buff, "\r\n\r\n") != NULL && !formPacketComplete)
            {
                CONSOLE_CRLF("- - - - - - - - -")
                CONSOLE_CRLF("SERVER: FORM PACKET RECEIVED")
                formPacketComplete = true;
                break;
            }

            if( 
                strstr(buff, "\r\n\r\n") != NULL && 
                formPacketComplete &&
                !setupPacketComplete &&
                strstr(buff, "ssid") != NULL &&
                strstr(buff, "pwd") != NULL &&
                strstr(buff, "city") != NULL &&
                strstr(buff, "country-code") != NULL &&
                strstr(buff, "timezones") != NULL &&
                strstr(buff, "api-key") != NULL
            )
            {
                CONSOLE_CRLF("- - - - - - - - -")
                CONSOLE_CRLF("SERVER: SETUP PACKET RECEIVED")
                CONSOLE_CRLF("  |-- received all required parameters")

                CONSOLE_CRLF("SERVER: SAVING NEW PARAMETERS TO PREFERENCES")
                saveNewParamsToPreferences(buff);   

                setupPacketComplete = true;
                break;
            }
        }  

        if(formPacketComplete && !setupPacketComplete)
        {
            client.print(htmlWebPageForm);    
        }
        else if(formPacketComplete && setupPacketComplete)
        {
            sprintf(buff, htmlWebPageCompleteFormatter, wifi_ssid, wifi_pwd, city, countryCode, timeZone, openWeatherAPI_key);
            client.print(buff);  
            client.flush();
            delay(1000);
            client.stop();
            CONSOLE_CRLF("SERVER: CLIENT DISCONNECTED")   

            CONSOLE_CRLF("ESP32: RESTART")  
            ESP.restart();       
        }
        else
        {
            delay(1000);
            client.stop();
            CONSOLE_CRLF("SERVER: CLIENT DISCONNECTED")
        }
    }
}

void setup()
{
    delay(DELAY_BEFORE_STARTUP_MS);
    CONSOLE_SERIAL.begin(CONSOLE_BAUDRATE);
    CONSOLE_CRLF("~~~ SETUP ~~~")

    loadDefaultValues();
    loadPreferences();
    setupDisplay();
    setupRotaryEncoders();
    setupSwitches();
    setup_LED_strip();
    CONSOLE_CRLF()

    // we could do this before loading animation of LED strip, but on single core CPUs this could cause issues
    if(setupWifi())
    {
        syncDateTime();
        updateWeatherTelemetry();
    }
    else
    {
        enableAP();
    }
    
    state = STATE_MAIN;

    CONSOLE_CRLF("~~~ LOOP ~~~")
}

void loop() 
{
    static uint32_t mainScreenTimer = 0;
    static uint32_t weatherTimer = millis();
    static uint32_t rotary_encoder_timer = 0;
    struct tm timeInfo;
    static uint16_t dayOfTimeSync = 0;
    static bool readyToTimeSync = true;

    // handle inputs
    checkSwitches();
    checkRotaryEncoders(&rotary_encoder_timer);

    // auto state change to main
    if((state == STATE_BRIGHTNESS || state == STATE_COLOR) && millis() - rotary_encoder_timer > ANY_SETTING_SCREEN_TIMER_MS)
    {
        state = STATE_MAIN;
    }

    // display state change
    if(state != previousState)
    {
        previousState = state;

        CONSOLE("STATE CHANGE: ");
        CONSOLE_CRLF(stateString[(uint8_t)state])

        if(state == STATE_COLOR)
        {
            CONSOLE("  |-- color picker type: ")
            CONSOLE_CRLF(CPT_String[(uint8_t)current_CPT]);
        }

        if(state == STATE_MAIN)
        {
            // in case there has been any changes to preferences
            updatePreferences();
            updateLocalTime(&timeInfo);
            updateWifiSignal(WiFi.RSSI());

            clearDisplay();
            
            mainScreenTimer = millis();
            updateMainScreen(validWifiConnection, true, timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_mday, timeInfo.tm_mon, timeInfo.tm_year + YEAR_OFFSET, temperature_C, humidity, windSpeed, weather, wifiSignal);   
        }
        else if(state == STATE_BRIGHTNESS)
        {
            clearDisplay();
            loadDisplayBrightness(currentBrightness);  
        }
        else if(state == STATE_COLOR) // handle transition from "state = STATE_MAIN" to "state == STATE_COLOR" 
        {
            clearDisplay();

            if(current_CPT == CPT_COLOR_TEMPERATURE)
            {
                loadDisplayColorTemperature(currentColorTemperatureIndex, previousColorTemperatureIndex);
            }
            else if(current_CPT == CPT_COLOR_HUE)
            {
                loadDisplayColorHue(currentColorHueIndex, previousColorHueIndex);  
            }
        }
        else if(state == STATE_FACTORY_RESET)
        {
            clearDisplay();
            loadAndExecuteFactoryReset(&preferences);// this function is blocking, either ends up in reset or continue
            encoder_1_switch_debounce_timer = millis();
            encoder_2_switch_debounce_timer = encoder_1_switch_debounce_timer;
            state = STATE_MAIN;
        }
    }
    else if(current_CPT != previous_CPT && state == STATE_COLOR) // handle CPT change during state == STATE_COLOR
    {
        previous_CPT = current_CPT;

        CONSOLE("CPT CHANGE: ");
        CONSOLE_CRLF(CPT_String[(uint8_t)current_CPT])

        clearDisplay(); 

        if(current_CPT == CPT_COLOR_TEMPERATURE)
        {
            loadDisplayColorTemperature(currentColorTemperatureIndex, previousColorTemperatureIndex);
            updateColorTemperature(0); // force color change without direction from encoder
        }
        else if(current_CPT == CPT_COLOR_HUE)
        {
            loadDisplayColorHue(currentColorHueIndex, previousColorHueIndex);
            updateColorHue(0); // force color change without direction from encoder  
        }                
    }

    // update screen
    if(millis() - mainScreenTimer > MAIN_SCREEN_TIMER_MS && state == STATE_MAIN)
    {
        mainScreenTimer = millis();
        updateLocalTime(&timeInfo);
        updateWifiSignal(WiFi.RSSI());
        
        updateMainScreen(validWifiConnection, false, timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_mday, timeInfo.tm_mon, timeInfo.tm_year + YEAR_OFFSET, temperature_C, humidity, windSpeed,  weather, wifiSignal); 
    }

    // update weather
    if(millis() - weatherTimer > UPDATE_WEATHER_MS && state == STATE_MAIN && validWifiConnection)
    {
        weatherTimer = millis();
        updateWeatherTelemetry();
    }

    if(!validWifiConnection)
    {
        handleServerClients();
    }

    if(readyToTimeSync && timeInfo.tm_hour == WHEN_TO_TIME_SYNC_HOUR)
    {
        readyToTimeSync = false;
        dayOfTimeSync = timeInfo.tm_mday;
        syncDateTime();
    }

    if(dayOfTimeSync != timeInfo.tm_mday)
    {
        readyToTimeSync = true;    
    }
}