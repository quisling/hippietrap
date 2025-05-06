#include <FastLED.h>

#include "effects.h"
#include "globals.h"

void wobbleRing(CRGB* leds,int& chaseLed){
    int firstWobble, nextWobble, variability = 4;
    
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    int r =random8()%variability;
    chaseLed = chaseLed + r - (0.5*variability);
    r =random8()%variability;
    nextWobble = chaseLed + r - (0.5*variability);
    leds[chaseLed] = CRGB::White;
    for (int i=1 ;i== NUM_STRIPS-1; i++)
    {
        leds[(i*NUM_LEDS_PER_STRIP)+nextWobble] = CRGB::White;
        r =random8()%variability;
        nextWobble = chaseLed + r - (0.5*variability);
    }
}