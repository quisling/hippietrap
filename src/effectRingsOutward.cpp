#include <FastLED.h>

#include "effects.h"
#include "globals.h"

bool ringsOutward(CRGB* leds)
{   
    //while ( true){

    
    int width = 10;
    for(int i = 0; i < NUM_LEDS_PER_STRIP; i++)
    {
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        srand(time(NULL));
        int end = i + width;
        if (end >= NUM_LEDS_PER_STRIP) end = NUM_LEDS_PER_STRIP-1;
        fillSpan(leds, i, end,CRGB::Green);
        FastLED.setBrightness(50);
        FastLED.show();
    }
//}
    return true;
}
