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
    
    leds[chaseLed] = CRGB::Black;
    leds[chaseLed+1] = CRGB::Blue;
    leds[chaseLed+2] = CRGB::White;
    chaseLed++;
    if (chaseLed > NUM_LEDS)
    {
      chaseLed = 0;
    }
}