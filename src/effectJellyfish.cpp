/*
#include <FastLED.h>
#include "effects.h"
#include "globals.h"

// ----- COLOR DEFINITIONS -----
CRGB dimPurple = CRGB(64, 0, 64);      // Dim purple (low brightness)
CRGB brightPurple = CRGB(180, 0, 255); // Bright purple (high brightness)

void Jellyfish() {
    // Animate a single bright purple LED moving along the strip

    // Loop through each LED position
    for (int i = 0; i < NUM_LEDS_TOP; i++) {
        // Step 1: Set all LEDs to dim purple
        fill_solid(LEDS_TOP, NUM_LEDS_TOP, dimPurple);

        // Step 2: Set the current LED to bright purple
        LEDS_TOP[i] = brightPurple;

        // Step 3: Show the updated colors
        FastLED.show();

        // Step 4: Wait a bit before moving to the next LED
        delay(50); // Adjust speed of animation here
    }
}
    */