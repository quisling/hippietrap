#pragma once

#include "localSettings.h"

#define MENU_MAX 5

// Pins for LEDS for SMALL_PARASOL with 8 strips, and base for LARGE_PARASOL
#define DATA1   14
#define DATA2   13
#define DATA3   12
#define DATA4   11
#define DATA5   10
#define DATA6   9
#define DATA7   3
#define DATA8   8

// Pins for LEDS for LARGE_PARASOL with additional 8 strips (16 total) in adition to LEDs for SMALL_PARASOL
#define DATA9   18
#define DATA10  17
#define DATA11  16
#define DATA12  15
#define DATA13  7
#define DATA14  6
#define DATA15  5
#define DATA16  4

#define MIC_I2C_WS 2
#define MIC_I2C_SCK 38
#define MIC_I2C_SD 1
#define SAMPLE_RATE 16000     // 16 kHz
#define FFT_SIZE 512         // Must be power of 2
#define MIC_SAMPLE_BITS 32    // INMP441 uses 32-bit frames (24-bit data)

#define BUTTON_PIN_1 48

#define CHIPSET     WS2815

#define START_TOP 0
#define START_RIM 44
#define START_DANGLE 77     //only applicable for SMALL_PARASOL


#if LARGE_PARASOL // Jacob & Christian modell

#define NUM_STRIPS 16
#define NUM_LEDS_PER_STRIP 73

    #else //SMALL_PARASOL - Billman & Elin modell
    // Change "#define LARGE_PARASOL true" in localSettings.h to false to enablo/swap

    #define BUTTON_PIN_2_MID 4
    #define BUTTON_PIN_3_TOP 5

    #define NUM_STRIPS 8
    #define NUM_LEDS_PER_STRIP 105

    #endif

#define NUM_LEDS_TOP (START_RIM -1)
#define NUM_LEDS_RIM (NUM_LEDS_TOP - NUM_LEDS_TOP -1)
#define NUM_LEDS_DANGLE (NUM_LEDS_TOP + NUM_LEDS_RIM)

#define LAST_LED_PER_STRIP (NUM_LEDS_PER_STRIP -1)

#define NUM_LEDS (NUM_LEDS_PER_STRIP * NUM_STRIPS)