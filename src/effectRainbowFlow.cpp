#include <FastLED.h>

#include "effects.h"
#include "globals.h"


void rainbowFlow(CRGB* leds, int& input_variable ){
    fill_rainbow(leds, NUM_LEDS, input_variable);
    input_variable+=5;
    if (input_variable >= 255){
        input_variable = 0;
    }
}