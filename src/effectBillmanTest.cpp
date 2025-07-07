#include <FastLED.h>

#include "effects.h"
#include "globals.h"

void fillSpan(CRGB* leds, int startDiode, int stopDiode, CRGB color){
    for(int i = 0; i < NUM_STRIPS; i++){
      for(uint16_t j = (i*NUM_LEDS_PER_STRIP) + startDiode; j <= (i*NUM_LEDS_PER_STRIP)+stopDiode; j++){
        leds[j] = color;
      }
    }
}

void billmanTest(CRGB* leds, int& input_variable ){
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    fillSpan(leds, START_TOP, START_RIM-1, CRGB::Red);
    fillSpan(leds, START_RIM, START_DANGLE-1, CRGB::Blue);
    fillSpan(leds, START_DANGLE, NUM_LEDS, CRGB::Yellow);
}
