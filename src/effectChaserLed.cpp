#include <FastLED.h>

#include "effects.h"
#include "globals.h"


void chaserLed(CRGB* leds, int& input_variable,bool& blinker ){
    /*if (blinker){
      fill_solid(leds, NUM_LEDS, CRGB::Red); // Turn all off
      blinker = false;
    }else{
      fill_solid(leds, NUM_LEDS, CRGB::Green); // Turn all off
      blinker = true;
    }*/
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    leds[input_variable] = CRGB::Purple;
    leds[((input_variable+1)%NUM_LEDS)] = CRGB::Blue;
    leds[((input_variable+2)%NUM_LEDS)] = CRGB::White;
    leds[((input_variable+3)%NUM_LEDS)] = CRGB::Orange;
    leds[((input_variable+4)%NUM_LEDS)] = CRGB::Red;
    input_variable++;
    if (input_variable > NUM_LEDS)
    {
      input_variable = 0;
    }
}