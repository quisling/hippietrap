#include <FastLED.h>

#include "effects.h"
#include "globals.h"




void paint(int brightness)
{
    FastLED.setBrightness( brightness);
    FastLED.show();
}

void explosionEffect(CRGB* leds, int& iterating_variable )
{    
    fillSpan(leds, 0, 5, CRGB::White);
    paint(100);
    fillSpan(leds, 6, 30, CRGB::White);
    paint(70);
    fillSpan(leds, 31, 73, CRGB::White);
    paint(50);
    delay(100);

    fill_solid(leds, NUM_LEDS, CRGB::Black);
    paint(0);

    delay(2000);





}

