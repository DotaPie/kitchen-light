#ifndef UTILITIES_H
#define UTILITIES_H

#include <FastLED.h>

#define KELVIN2RGB_LOOKUP_TABLE_MIN_VALUE 1000
#define KELVIN2RGB_LOOKUP_TABLE_MAX_VALUE 12000
#define KELVIN2RGB_LOOKUP_TABLE_STEP 100

enum STATE {STATE_NONE, STATE_MAIN, STATE_BRIGHTNESS, STATE_COLOR};
enum COLOR_PICKER_TYPE {CPT_NONE, CPT_COLOR_TEMPERATURE, CPT_COLOR_HUE};

extern const char* monthNames[];
extern const CRGB kelvin2RGB_lookupTable[];
extern const char* stateString[];
extern const char* CPT_String[];

#endif