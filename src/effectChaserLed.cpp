#include <FastLED.h>

#include "effects.h"
#include "globals.h"


void chaserLed(CRGB* leds, int& iterating_variable,bool& blinker ){
    /*if (blinker){
      fill_solid(leds, NUM_LEDS, CRGB::Red); // Turn all off
      blinker = false;
    }else{
      fill_solid(leds, NUM_LEDS, CRGB::Green); // Turn all off
      blinker = true;
    }*/
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    leds[iterating_variable] = CRGB::Purple;
    leds[((iterating_variable+1)%NUM_LEDS)] = CRGB::Blue;
    leds[((iterating_variable+2)%NUM_LEDS)] = CRGB::White;
    leds[((iterating_variable+3)%NUM_LEDS)] = CRGB::Orange;
    leds[((iterating_variable+4)%NUM_LEDS)] = CRGB::Red;
    iterating_variable++;
    if (iterating_variable >= NUM_LEDS)
    {
      iterating_variable = 0;
    }
}