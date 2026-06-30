#pragma once
void chaserLed(CRGB* leds,int& iterating_variable, bool& blinker);
void wobbleRing(CRGB* leds,int& iterating_variable);
void setNumberLeds(CRGB* leds,bool& blinker);
void rainbowFlow(CRGB* leds,int& iterating_variable);
void menuIndicator(CRGB* leds,int menuChoice);
void clockLed(CRGB* leds);
void debugBillman(CRGB* leds, int& iterating_variable);
void jellyFish(CRGB* leds, int& iterating_variable);
void explosionEffect(CRGB* leds, int& iterating_variable);
bool ringsOutward(CRGB* leds);
void fillSpan(CRGB* leds, int startDiode, int stopDiode, CRGB color);
void audioSpectrum(CRGB* leds);
void sparklingRain(CRGB* leds);
