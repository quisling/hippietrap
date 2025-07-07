#include <FastLED.h>

#include "effects.h"
#include "globals.h"


void rainbowFlow(CRGB* leds, int& iterating_variable ){
    fill_rainbow(leds, NUM_LEDS, iterating_variable);
    iterating_variable+=5;
    if (iterating_variable >= 255){
        iterating_variable = 0;
    }
}