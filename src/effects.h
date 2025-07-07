#pragma once
void chaserLed(CRGB* leds,int& input_variable, bool& blinker);
void wobbleRing(CRGB* leds,int& input_variable);
void powerTest (CRGB* leds,bool& blinker);
void rainbowFlow (CRGB* leds,int& input_variable);
void menuIndicator(CRGB* leds,int menuChoice);
void clockLed(CRGB* leds);
void billmanTest(CRGB* leds, int );
//CRGB - Typen, att det är en karta. Leds - informationene över vilka led som är vilka i matrisn, och vilken färg de har
