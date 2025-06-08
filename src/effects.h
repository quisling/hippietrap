#pragma once
void chaserLed(CRGB* leds,int& chaseLed, bool& blinker);
void wobbleRing(CRGB* leds,int& chaseLed);
void powerTest (CRGB* leds,bool& blinker);
void rainbowFlow (CRGB* leds,int& chaseLed);
void menuIndicator(CRGB* leds,int menuChoice);
void clockLed(CRGB* leds);