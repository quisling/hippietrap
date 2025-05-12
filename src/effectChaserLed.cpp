#include <FastLED.h>

#include "effects.h"
#include "globals.h"


void chaserLed(CRGB* leds, int& chaseLed,bool& blinker ){
    /*if (blinker){
      fill_solid(leds, NUM_LEDS, CRGB::Red); // Turn all off
      blinker = false;
    }else{
      fill_solid(leds, NUM_LEDS, CRGB::Green); // Turn all off
      blinker = true;
    }*/
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    leds[chaseLed] = CRGB::Purple;
    leds[((chaseLed+1)%NUM_LEDS)] = CRGB::Blue;
    leds[((chaseLed+2)%NUM_LEDS)] = CRGB::White;
    leds[((chaseLed+3)%NUM_LEDS)] = CRGB::Orange;
    leds[((chaseLed+4)%NUM_LEDS)] = CRGB::Red;
    chaseLed++;
    if (chaseLed > NUM_LEDS)
    {
      chaseLed = 0;
    }
}