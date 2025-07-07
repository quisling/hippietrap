#include <FastLED.h>

#include "effects.h"
#include "globals.h"


void setNumberLeds(CRGB* leds, bool& blinker ){
  fill_solid(leds, NUM_LEDS, CRGB::Black);
    for(int i = 0; i < NUM_STRIPS; i++){
      for(uint16_t j = i*NUM_LEDS_PER_STRIP; j <= (i*NUM_LEDS_PER_STRIP)+49;j++){
        leds[j] = CRGB::Green;
        //if (DEBUG_MODE) Serial.print("Setting Diode: ");
        //if (DEBUG_MODE) Serial.println(j);  
      }
      //if (DEBUG_MODE) Serial.print("Setting strip: ");
      //if (DEBUG_MODE) Serial.println(i);
    }
}