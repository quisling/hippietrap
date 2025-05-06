#include <FastLED.h>
#include "effects.h"
#include "globals.h"

#define BRIGHTNESS  10
#define FRAMES_PER_SECOND 30

CRGB leds[NUM_STRIPS * NUM_LEDS_PER_STRIP];

int chaseLed=0;
bool blinker = true;

void setup() {
  delay(3000); // sanity delay
    // tell FastLED there's 60 NEOPIXEL leds on pin 10, starting at index 0 in the led array
    FastLED.addLeds<NEOPIXEL, DATA1>(leds, 0, NUM_LEDS_PER_STRIP);
    // tell FastLED there's 60 NEOPIXEL leds on pin 11, starting at index 60 in the led array
    FastLED.addLeds<NEOPIXEL, DATA2>(leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<NEOPIXEL, DATA3>(leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<NEOPIXEL, DATA4>(leds, 3 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<NEOPIXEL, DATA5>(leds, 4 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<NEOPIXEL, DATA6>(leds, 5 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<NEOPIXEL, DATA7>(leds, 6 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<NEOPIXEL, DATA8>(leds, 7 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<NEOPIXEL, DATA9>(leds, 8 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<NEOPIXEL, DATA10>(leds, 9 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<NEOPIXEL, DATA11>(leds, 10 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<NEOPIXEL, DATA12>(leds, 11 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<NEOPIXEL, DATA13>(leds, 12 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<NEOPIXEL, DATA14>(leds, 13 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<NEOPIXEL, DATA15>(leds, 14 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<NEOPIXEL, DATA16>(leds, 15 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
    
    FastLED.setBrightness( BRIGHTNESS );

}

void loop()
{
  // Add entropy to random number generator
  random16_add_entropy( random());

  chaserLed(leds, chaseLed, blinker);
  //wobbleRing(leds, chaseLed);
  FastLED.show(); // display this frame
  FastLED.delay(1000 / FRAMES_PER_SECOND);

}
  
  
  