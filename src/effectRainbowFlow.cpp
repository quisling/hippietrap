#include <FastLED.h>

#include "effects.h"
#include "globals.h"


void rainbowFlow(CRGB* leds, int& chaseLed ){
    fill_rainbow(leds, NUM_LEDS, chaseLed);
    chaseLed+=5;
    if (chaseLed >= 255){
        chaseLed = 0;
    }
}