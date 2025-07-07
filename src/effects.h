#pragma once
void chaserLed(CRGB* leds,int& iterating_variable, bool& blinker);
void wobbleRing(CRGB* leds,int& iterating_variable);
void setNumberLeds(CRGB* leds,bool& blinker);
void rainbowFlow(CRGB* leds,int& iterating_variable);
void menuIndicator(CRGB* leds,int menuChoice);
void clockLed(CRGB* leds);
void debugBillman(CRGB* leds, int& iterating_variable);
void jellyFish(CRGB* leds, int& iterating_variable);

//CRGB - Typen, att det är en karta. Leds - informationene över vilka led som är vilka i matrisn, och vilken färg de har
