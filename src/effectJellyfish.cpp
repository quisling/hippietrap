#include <FastLED.h>
#include "effects.h"
#include "globals.h"

// ----- COLOR DEFINITIONS -----
CRGB dimPurple = CRGB(64, 0, 64);      // Dim purple (low brightness)
CRGB brightPurple = CRGB(180, 0, 255); // Bright purple (high brightness)
CRGB dimBlue = CRGB(0, 0, 64);  // Dim blue (low brightness)


void jellyFish(CRGB* leds, int& iterating_variable){
    // Reset
    fillSpan(leds,START_TOP, LAST_LED_PER_STRIP, CRGB :: Black);
    
    fillSpan(leds,START_TOP, START_RIM, dimPurple);
    fillSpan(leds,START_RIM, START_DANGLE, dimBlue);

    // Loop through each LED position
    fillSpan(leds, iterating_variable, iterating_variable, brightPurple);

    if (iterating_variable == START_RIM) {
        fillSpan (leds, START_RIM+1, START_DANGLE, brightPurple);
        iterating_variable = START_DANGLE;
        }

    iterating_variable++; //adds 1 every loop. Use for effects that move the active "pixel" etc.
    if(iterating_variable > LAST_LED_PER_STRIP) iterating_variable = 0;


    #ifdef DEBUG_MODE
      Serial.println("In Jellyfish, iterating variable: ");
      Serial.println(iterating_variable);
    #endif

}