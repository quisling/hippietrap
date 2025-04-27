#include <FastLED.h>

#define NUM_STRIPS 16
#define NUM_LEDS_PER_STRIP 7
#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS

#define DATA1   14
#define DATA2   13
#define DATA3   12
#define DATA4   11
#define DATA5   10
#define DATA6   9
#define DATA7   3
#define DATA8   8
#define DATA9   18
#define DATA10  17
#define DATA11  16
#define DATA12  15
#define DATA13  7
#define DATA14  6
#define DATA15  5
#define DATA16  4

#define CHIPSET     WS2815

#define BRIGHTNESS  10
#define FRAMES_PER_SECOND 5

CRGB leds[NUM_STRIPS * NUM_LEDS_PER_STRIP];

int chaseLed=0;
bool blinker = true;

void ChaserLed(){
  
    if (blinker){
      fill_solid(leds, NUM_LEDS, CRGB::Red); // Turn all off
      blinker = false;
    }else{
      fill_solid(leds, NUM_LEDS, CRGB::Green); // Turn all off
      blinker = true;
    }
    
    leds[chaseLed] = CRGB::Black;
    leds[chaseLed+1] = CRGB::Blue;
    leds[chaseLed+2] = CRGB::White;
    chaseLed++;
    if (chaseLed > NUM_LEDS)
    {
      chaseLed = 0;
    }
      
}
void setup() {
  delay(3000); // sanity delay
    // tell FastLED there's 60 NEOPIXEL leds on pin 10, starting at index 0 in the led array
    FastLED.addLeds<NEOPIXEL, DATA1>(leds, 0, NUM_LEDS_PER_STRIP);
    // tell FastLED there's 60 NEOPIXEL leds on pin 11, starting at index 60 in the led array
    FastLED.addLeds<NEOPIXEL, DATA2>(leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<NEOPIXEL, DATA3>(leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);FastLED.setBrightness( BRIGHTNESS );
    FastLED.addLeds<NEOPIXEL, DATA4>(leds, 3 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);FastLED.setBrightness( BRIGHTNESS );
    FastLED.addLeds<NEOPIXEL, DATA5>(leds, 4 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);FastLED.setBrightness( BRIGHTNESS );
    FastLED.addLeds<NEOPIXEL, DATA6>(leds, 5 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);FastLED.setBrightness( BRIGHTNESS );
    FastLED.addLeds<NEOPIXEL, DATA7>(leds, 6 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);FastLED.setBrightness( BRIGHTNESS );
    FastLED.addLeds<NEOPIXEL, DATA8>(leds, 7 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);FastLED.setBrightness( BRIGHTNESS );
    FastLED.addLeds<NEOPIXEL, DATA9>(leds, 8 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);FastLED.setBrightness( BRIGHTNESS );
    FastLED.addLeds<NEOPIXEL, DATA10>(leds, 9 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);FastLED.setBrightness( BRIGHTNESS );
    FastLED.addLeds<NEOPIXEL, DATA11>(leds, 10 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);FastLED.setBrightness( BRIGHTNESS );
    FastLED.addLeds<NEOPIXEL, DATA12>(leds, 11 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);FastLED.setBrightness( BRIGHTNESS );
    FastLED.addLeds<NEOPIXEL, DATA13>(leds, 12 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);FastLED.setBrightness( BRIGHTNESS );
    FastLED.addLeds<NEOPIXEL, DATA14>(leds, 13 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);FastLED.setBrightness( BRIGHTNESS );
    FastLED.addLeds<NEOPIXEL, DATA15>(leds, 14 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);FastLED.setBrightness( BRIGHTNESS );
    FastLED.addLeds<NEOPIXEL, DATA16>(leds, 15 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);FastLED.setBrightness( BRIGHTNESS );

}

void loop()
{
  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy( random());

  ChaserLed();
  FastLED.show(); // display this frame#include <Arduino.h>
  FastLED.delay(1000 / FRAMES_PER_SECOND);

}
  
  
  