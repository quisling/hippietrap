#pragma once
#define LARGE_PARASOL false

#define DATA1   14
#define DATA2   13
#define DATA3   12
#define DATA4   11
#define DATA5   10
#define DATA6   9
#define DATA7   3
#define DATA8   8


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

#define BUTTON_PIN 48

#define CHIPSET     WS2815

#if LARGE_PARASOL 
#define NUM_STRIPS 16
#define NUM_LEDS_PER_STRIP 73
#else
#define NUM_STRIPS 8
#define NUM_LEDS_PER_STRIP 108
#endif

#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS