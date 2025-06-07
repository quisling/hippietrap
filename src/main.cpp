#include <FastLED.h>
#include <DFRobot_BMI160.h> // BMI160 library
#include <Wire.h>      // I2C library

#include "effects.h"
#include "globals.h"


#define BRIGHTNESS  10
#define FRAMES_PER_SECOND 10
#define I2C_SDA 4
#define I2C_SCL 5
#define I2C_ADDR 0x68

CRGB leds[NUM_LEDS];

//DFRobot_BMI160 bmi160;

int chaseLed=0;
bool blinker = true;

void setup() {
  delay(3000); // sanity delay
  Serial.begin(115200); // Start serial at 115200 baud
  Serial.println("Hello from ESP32-S3!");

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
    if (LARGE_PARASOL){
      FastLED.addLeds<NEOPIXEL, DATA9>(leds, 8 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      FastLED.addLeds<NEOPIXEL, DATA10>(leds, 9 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      FastLED.addLeds<NEOPIXEL, DATA11>(leds, 10 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      FastLED.addLeds<NEOPIXEL, DATA12>(leds, 11 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      FastLED.addLeds<NEOPIXEL, DATA13>(leds, 12 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      FastLED.addLeds<NEOPIXEL, DATA14>(leds, 13 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      FastLED.addLeds<NEOPIXEL, DATA15>(leds, 14 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      FastLED.addLeds<NEOPIXEL, DATA16>(leds, 15 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
    }
    FastLED.setBrightness( BRIGHTNESS );
    //Wire.begin(I2C_SDA, I2C_SCL); // SDA = GPIO47, SCL = GPIO21
    //if (bmi160.I2cInit(I2C_ADDR) != BMI160_OK) {
    //  Serial.println("BMI160 init failed!");
    //  while (1);
    //}
}

void loop()
{
  // Add entropy to random number generator
  random16_add_entropy( random());

  //chaserLed(leds, chaseLed, blinker);
  wobbleRing(leds, chaseLed);
  //clockLed(leds);
  FastLED.show(); // display this frame
  FastLED.delay(1000 / FRAMES_PER_SECOND);
  Serial.println("Hello hello");
}
  
  
  