#include <FastLED.h>

#include "effects.h"
#include "globals.h"




void paint(int brightness)
{
    FastLED.setBrightness( brightness);
    FastLED.show();
}


bool isLit(CRGB* leds)
{
    for(int i = 0; i < NUM_LEDS; i++)
    {
        if(leds[i] != CRGB::Black) return true;
    }
    return false;
}

void explosionEffect(CRGB* leds, int& iterating_variable )
{    
    int intrand = rand()%1024;
    srand(time(NULL)+intrand);
    int outerLed = NUM_LEDS_PER_STRIP -1;
    int ledsToKill = 400;
    switch (iterating_variable)
    {
        case 0: // initial fuse
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        paint(0);

        for (int i = 0; i < outerLed; i++)
        {
            fillSpan(leds, outerLed-i, outerLed-i,CRGB::White);
            paint(100);
            
            
            
            if(i != 0){
                fillSpan(leds, outerLed-(i-1), outerLed-(i-1),CRGB::OrangeRed);
            }
            if(i > 1){
                fillSpan(leds, outerLed-(i-2), outerLed-(i-2),CRGB::Red);
            }
            if(i > 2){
                fillSpan(leds, outerLed-(i-3), outerLed-(i-3),CRGB::Black);
            }
            paint(100);
           
            
        }
        fill_solid(leds, outerLed, CRGB::Black);
        paint(0);
        iterating_variable++;
        return;
        break;

        case 1: // explosion
            
            fillSpan(leds, 0, 5, CRGB::White);
            paint(100);
            fillSpan(leds, 6, 20, CRGB::White);
            paint(80);
            fillSpan(leds, 21, 40, CRGB::White);
            paint(60);
            fillSpan(leds, 41, 55, CRGB::White);
            paint(50);
            fillSpan(leds, 56, 68, CRGB::White);
            paint(40);
            fillSpan(leds, 69, 73, CRGB::White);
            delay(100);
        
            iterating_variable++;
            return;
        break;

        case 2: // sparkle
    

        do
        {
        //for(int j = 0; j < 32; j++){
            int cled = 0;
            
            for (int i = 1; i < ledsToKill; i++){
                
                cled = rand()%NUM_LEDS;
                leds[cled] = CRGB::Black;
            }

            paint(50);
            delay(75);

            for(int i = 0; i < NUM_LEDS; i++)
            {
                if(leds[i] == CRGB::White)
                {
                    if(i+1 < NUM_LEDS) leds[i+1] = CRGB::Orange;
                    if(i-1 >= 0) leds[i-1] = CRGB::Orange;
                    leds[i] = CRGB::Black;
                }else if(leds[i] == CRGB::Orange)
                {
                    if(i+1 < NUM_LEDS)leds[i+1] = CRGB::OrangeRed;
                    if(i-1 >= 0) leds[i-1] = CRGB::OrangeRed;
                    leds[i] = CRGB::Black;
                }else if(leds[i] == CRGB::OrangeRed)
                {
                    if(i+1 < NUM_LEDS)leds[i+1] = CRGB::Red;
                    if(i-1 >= 0) leds[i-1] = CRGB::Red;
                    leds[i] = CRGB::DarkRed;
                }else if(leds[i] == CRGB::Red)
                {
                    if(i+1 < NUM_LEDS)leds[i+1] = CRGB::DarkRed;
                    if(i-1 >= 0) leds[i-1] = CRGB::DarkRed;
                    //leds[i] = CRGB::Black;
                }
                
            }
                
            paint(50);
            ledsToKill = ledsToKill-4;
            if(ledsToKill < 50) ledsToKill = 50;
        }while(isLit(leds));
        fill_solid(leds,NUM_LEDS,CRGB::Black);

        iterating_variable++;
        return;
        break;

        default:
        
        
        delay(1000);
        return;
        break;

    }






}

