#include <FastLED.h>
#include "effects.h"
#include "globals.h"

// ----- COLOR DEFINITIONS -----
CRGB dimPurple = CRGB(64, 0, 64);      // Dim purple (low brightness)
CRGB brightPurple = CRGB(180, 0, 255); // Bright purple (high brightness)

void fillSpan2(CRGB* leds, int startDiode, int stopDiode, CRGB color){
    
    for(int i = 0; i < NUM_STRIPS; i++){
      for(uint16_t j = (i*NUM_LEDS_PER_STRIP) + startDiode-1; j <= (i*NUM_LEDS_PER_STRIP)+stopDiode-1; j++){
        leds[j] = color;
      }
    if (DEBUG_MODE) Serial.println(i);
    }
}

void jellyFish(CRGB* leds, int& iterating_variable){
    // Animate a single bright purple LED moving along the strip
    // Loop through each LED position
    fillSpan2(leds,START_TOP+1, START_RIM, dimPurple);
    fillSpan2(leds, iterating_variable, iterating_variable, brightPurple);
    if (DEBUG_MODE) Serial.println("In Jellyfish, iterating variable: ");
    if (DEBUG_MODE) Serial.println(iterating_variable);
}