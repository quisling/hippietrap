#include <FastLED.h>

#include "effects.h"
#include "globals.h"


void powerTest(CRGB* leds, bool& blinker ){
  if (blinker){
    fill_solid(leds, NUM_LEDS, CRGB::Red);
    blinker = false;
  }else{
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    blinker = true;
  }

}