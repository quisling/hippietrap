#include <FastLED.h>

#include "effects.h"
#include "globals.h"


void menuIndicator(CRGB* leds, int menuChoice ){
    fill_solid(leds,NUM_LEDS,CRGB::Black);
    fill_rainbow(leds, menuChoice, CRGB::Red);
}