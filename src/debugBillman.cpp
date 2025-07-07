#include <FastLED.h>

#include "effects.h"
#include "globals.h"

int biggestNumber;

void fillSpan(CRGB* leds, int startDiode, int stopDiode, CRGB color){
    
    for(int i = 0; i < NUM_STRIPS; i++){
      for(uint16_t j = (i*NUM_LEDS_PER_STRIP) + startDiode-1; j <= (i*NUM_LEDS_PER_STRIP)+stopDiode-1; j++){
        leds[j] = color;
        if (j > biggestNumber) biggestNumber = j;
      }
    if (DEBUG_MODE) Serial.println(i);
    }
}

void billmanTest(CRGB* leds, int& input_variable ){
    
    if (DEBUG_MODE) Serial.println("BillmanTest Start");
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    /*
    if (DEBUG_MODE) Serial.println("BillmanTest Black");
    fillSpan(leds, START_TOP, START_RIM, CRGB::Red);
    if (DEBUG_MODE) Serial.println("BillmanTest Red");
    fillSpan(leds, START_RIM, START_DANGLE, CRGB::Blue);
    if (DEBUG_MODE) Serial.println("BillmanTest Blue");
    fillSpan(leds, START_DANGLE, NUM_LEDS_PER_STRIP, CRGB::Yellow);
    if (DEBUG_MODE) Serial.println("BillmanTest Stop");
    */
  
    fillSpan(leds, 76, 76, CRGB::Yellow); // this LED should be before DAGNLE
    fillSpan(leds, 76, 76, CRGB::Yellow); // this LED should be before DAGNLE

   fillSpan(leds, 1, 5, CRGB::Green); //light up evry 10 led
   fillSpan(leds, 10, 10, CRGB::Green);
   fillSpan(leds, 20, 20, CRGB::Cyan);
   fillSpan(leds, 30, 30, CRGB::Green);
   fillSpan(leds, 40, 40, CRGB::Cyan);
   fillSpan(leds, 50, 50, CRGB::Green);
   fillSpan(leds, 60, 60, CRGB::Cyan);
   fillSpan(leds, 70, 70, CRGB::Green);
   fillSpan(leds, 80, 80, CRGB::Cyan);
   
   fillSpan(leds, 90, 90, CRGB::Green);
   fillSpan(leds, 100, 100, CRGB::Cyan);
   
   fillSpan(leds, 105, 105, CRGB::Blue);
   /*for(int i = 0; i < 5;i=i+5){
    if (((i / 5) % 2) == 0){
        fillSpan(leds, i, i+5, CRGB::Red);
    }else{
        fillSpan(leds, i, i+5, CRGB::Blue);
        }
    }*/
if (DEBUG_MODE) Serial.print("BillmanTest BiggestNumber");
if (DEBUG_MODE) Serial.println(NUM_STRIPS*NUM_LEDS_PER_STRIP);
if (DEBUG_MODE) Serial.println(biggestNumber);
}
