#include <FastLED.h>
#include <chrono>
#include <iostream>

#include "effects.h"
#include "globals.h"


void clockLed(CRGB* leds){
  using namespace std::chrono;

  fill_solid(leds, NUM_LEDS, CRGB::Black);

  auto now = system_clock::now();
  auto secs = duration_cast<seconds>(now.time_since_epoch());

  auto hours   = duration_cast<std::chrono::hours>(secs);
  auto minutes = duration_cast<std::chrono::minutes>(secs - hours);
  auto seconds = secs - hours - minutes;

  /*for (auto i = 0; i <= seconds.count(); i++){
    leds[i] = CRGB::Green;
  }*/
  int ledHour = ((11+ hours.count())%12)*NUM_LEDS_PER_STRIP;
  int ledMinutes = ((55 + minutes.count()/5)%12)*NUM_LEDS_PER_STRIP;
  int ledSeconds = ((55 + seconds.count()/5)%12)*NUM_LEDS_PER_STRIP;
  for (auto i = ledSeconds; i <= ledSeconds+8; i++){
    leds[i] = CRGB::White;
  }
  for (auto i = ledMinutes; i <= ledMinutes+6; i++){
    leds[i] = CRGB::Green;
  }
  for (auto i = ledHour; i <= ledHour+3; i++){
    leds[i] = CRGB::Purple;
  }
  Serial.println("Hello world");
}