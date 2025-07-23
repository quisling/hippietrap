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