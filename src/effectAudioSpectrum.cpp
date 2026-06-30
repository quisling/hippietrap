#include <FastLED.h>
#include "driver/i2s.h"
#include "dsps_fft2r.h"
#include "dsps_wind.h"
#include "effects.h"
#include "globals.h"

// ── Globals owned by main.cpp ────────────────────────────────────────────────
extern int32_t  fft_raw[];          // raw I2S samples (FFT_SIZE × int32_t)
extern float    complex_buffer[];   // interleaved re/im FFT buffer (FFT_SIZE × 2)
extern float    bandMagnitudes[];   // per-band magnitude output (16 entries)
extern const int bands[];           // band boundary array (17 entries)
extern const int numBands;          // = 16

// ── Auto-gain constants ───────────────────────────────────────────────────────
// At 16 kHz sample rate with 512-point FFT: ~31.25 frames/second
static const int   GAIN_WINDOW_FRAMES = 625;  // 20 s × 31.25 frames/s
static const float TARGET_FILL        = 0.50f; // target average fill level per band
static const float GAIN_STEP_UP       = 0.02f; // raise gain when band avg is below target
static const float GAIN_STEP_DOWN     = 0.05f; // lower gain faster when band avg is above target
static const float GAIN_MIN           = 0.1f;
static const float GAIN_MAX           = 50.0f;
static const int   STARTUP_FRAMES     = 310;   // ~10 s ramp-in period
static const uint32_t FADE_TIME_MS    = 2000;  // time for display to fall full scale in ms

// ── Static state (persists across frames) ────────────────────────────────────
static float    bandGain[16];                        // per-band gain, initialised to 2.24 (geometric midpoint of GAIN_MIN/GAIN_MAX)
static float    displayLevel[16];                    // current displayed fill per arm, decays pixel-by-pixel on signal drop
static uint32_t lastFrameMs = 0;                     // millis() at last frame, for decay timing
static uint8_t  fillHistory[16][GAIN_WINDOW_FRAMES]; // fill level 0–255 per band per frame
static uint32_t fillSum[16];                         // running sum per band (max 255×1875 = 478125)
static int      fillHistoryIdx = 0;                  // shared write index across all bands
static int      frameCount     = 0;
static bool     gainInitDone   = false;

// ─────────────────────────────────────────────────────────────────────────────
// runFFT(): read mic → apply Hann window → FFT → fill bandMagnitudes[]
// ─────────────────────────────────────────────────────────────────────────────
static void runFFT() {
  size_t bytes_read = 0;
  i2s_read(I2S_NUM_0, fft_raw, FFT_SIZE * sizeof(int32_t), &bytes_read, portMAX_DELAY);
  int samples_read = (int)(bytes_read / sizeof(int32_t));

  // ── Stage 1: raw I2S samples ─────────────────────────────────────────────
  int32_t raw_min = INT32_MAX, raw_max = INT32_MIN;
  for (int i = 0; i < samples_read; i++) {
    if (fft_raw[i] < raw_min) raw_min = fft_raw[i];
    if (fft_raw[i] > raw_max) raw_max = fft_raw[i];
  }
  Serial.printf("[AS-1] bytes_read=%d samples=%d  raw[0]=%ld raw[1]=%ld raw[255]=%ld  min=%ld max=%ld\n",
                (int)bytes_read, samples_read,
                (long)fft_raw[0], (long)fft_raw[1],
                samples_read > 255 ? (long)fft_raw[255] : 0L,
                (long)raw_min, (long)raw_max);

  // ── Stage 2: normalize + window → complex buffer ─────────────────────────
  float hann_window[FFT_SIZE];
  dsps_wind_hann_f32(hann_window, FFT_SIZE);
  float normed_max = 0.0f;
  for (int i = 0; i < FFT_SIZE; i++) {
    int32_t raw   = fft_raw[i] >> 8;               // drop lowest 8 bits
    float normed  = (float)raw / 8388608.0f;        // normalize to [-1, 1]
    float windowed = normed * hann_window[i];
    complex_buffer[i * 2]     = windowed;
    complex_buffer[i * 2 + 1] = 0.0f;
    float absv = windowed < 0 ? -windowed : windowed;
    if (absv > normed_max) normed_max = absv;
  }
  Serial.printf("[AS-2] normed[0]=%.6f normed[1]=%.6f normed[255]=%.6f  windowed_max=%.6f\n",
                (float)(fft_raw[0] >> 8) / 8388608.0f * hann_window[0],
                (float)(fft_raw[1] >> 8) / 8388608.0f * hann_window[1],
                (float)(fft_raw[255] >> 8) / 8388608.0f * hann_window[255],
                normed_max);

  // ── Stage 3: FFT ─────────────────────────────────────────────────────────
  dsps_fft2r_fc32(complex_buffer, FFT_SIZE);
  dsps_bit_rev_fc32(complex_buffer, FFT_SIZE);

  float fft_mag_max = 0.0f;
  for (int i = 0; i < FFT_SIZE / 2; i++) {
    float re = complex_buffer[i * 2], im = complex_buffer[i * 2 + 1];
    float mag = sqrtf(re * re + im * im);
    if (mag > fft_mag_max) fft_mag_max = mag;
  }
  Serial.printf("[AS-3] FFT done. bin[0]=%.6f bin[1]=%.6f bin[2]=%.6f  peak_mag=%.6f\n",
                sqrtf(complex_buffer[0]*complex_buffer[0] + complex_buffer[1]*complex_buffer[1]),
                sqrtf(complex_buffer[2]*complex_buffer[2] + complex_buffer[3]*complex_buffer[3]),
                sqrtf(complex_buffer[4]*complex_buffer[4] + complex_buffer[5]*complex_buffer[5]),
                fft_mag_max);

  // ── Stage 4: band magnitudes ──────────────────────────────────────────────
  const float binWidth = (float)SAMPLE_RATE / FFT_SIZE;
  for (int band = 0; band < numBands; band++) {
    int startBin = (int)(bands[band]     / binWidth);
    int endBin   = (int)(bands[band + 1] / binWidth);
    if (startBin >= FFT_SIZE / 2) startBin = FFT_SIZE / 2 - 1;
    if (endBin   >  FFT_SIZE / 2) endBin   = FFT_SIZE / 2;

    bandMagnitudes[band] = 0.0f;
    for (int i = startBin; i < endBin; i++) {
      float re = complex_buffer[i * 2];
      float im = complex_buffer[i * 2 + 1];
      bandMagnitudes[band] += sqrtf(re * re + im * im);
    }
    if (endBin > startBin)
      bandMagnitudes[band] /= (endBin - startBin);
  }
  Serial.printf("[AS-4] bands(bins): ");
  for (int b = 0; b < numBands; b++) {
    int sb = (int)(bands[b]   / binWidth);
    int eb = (int)(bands[b+1] / binWidth);
    Serial.printf("b%d[%d-%d]=%.5f ", b, sb, eb, bandMagnitudes[b]);
  }
  Serial.println();
}

// ─────────────────────────────────────────────────────────────────────────────
// audioSpectrum(): main effect entry point called every loop() iteration
// ─────────────────────────────────────────────────────────────────────────────
void audioSpectrum(CRGB* leds) {
  runFFT();

  // ── One-time initialisation ──────────────────────────────────────────────
  if (!gainInitDone) {
    for (int b = 0; b < 16; b++) {
      bandGain[b]     = 2.24f;
      fillSum[b]      = 0;
      displayLevel[b] = 0.0f;
    }
    lastFrameMs = millis();
    memset(fillHistory, 0, sizeof(fillHistory));
    gainInitDone = true;
  }

  // ── Compute per-band fill levels and update AGC ──────────────────────────
  float fillLevel[NUM_STRIPS];

  for (int arm = 0; arm < NUM_STRIPS; arm++) {
    // Map arm → band magnitude
    float mag;
#if LARGE_PARASOL
    mag = bandMagnitudes[arm];           // 16 arms, 16 bands — 1:1
#else
    mag = (bandMagnitudes[arm * 2] + bandMagnitudes[arm * 2 + 1]) * 0.5f; // 8 arms, pair-averaged
#endif

    // Apply per-band gain, clamp to [0, 1]
    float fl = mag * bandGain[arm];
    if (fl > 1.0f) fl = 1.0f;
    if (fl < 0.0f) fl = 0.0f;

    // Apply startup ramp: blend toward 0.5 for the first STARTUP_FRAMES
    if (frameCount < STARTUP_FRAMES) {
      float ramp = (float)frameCount / (float)STARTUP_FRAMES; // 0.0 → 1.0
      fl = fl * ramp;
    }

    fillLevel[arm] = fl;

    // ── Rolling average: store fill as uint8_t 0–255 ──────────────────────
    uint8_t newEntry = (uint8_t)(fl * 255.0f + 0.5f);
    fillSum[arm] -= fillHistory[arm][fillHistoryIdx];
    fillHistory[arm][fillHistoryIdx] = newEntry;
    fillSum[arm] += newEntry;

    // avgFill in [0, 1]
    float avgFill = (float)fillSum[arm] / ((float)GAIN_WINDOW_FRAMES * 255.0f);

    // Nudge gain toward keeping avgFill at TARGET_FILL
    if (avgFill < TARGET_FILL) {
      bandGain[arm] += GAIN_STEP_UP;
    } else {
      bandGain[arm] -= GAIN_STEP_DOWN;
    }
    if (bandGain[arm] < GAIN_MIN) bandGain[arm] = GAIN_MIN;
    if (bandGain[arm] > GAIN_MAX) bandGain[arm] = GAIN_MAX;
  }

  fillHistoryIdx = (fillHistoryIdx + 1) % GAIN_WINDOW_FRAMES;
  frameCount++;

  // ── Debug output ─────────────────────────────────────────────────────────
  Serial.printf("[AS-5] gains:");
  for (int arm = 0; arm < NUM_STRIPS; arm++) {
    Serial.printf(" %.2f", bandGain[arm]);
  }
  Serial.printf(" | fill:");
  for (int arm = 0; arm < NUM_STRIPS; arm++) {
    Serial.printf(" %3d%%", (int)(fillLevel[arm] * 100.0f));
  }
  Serial.println();

  // ── Decay displayed levels toward live signal ────────────────────────────
  uint32_t now       = millis();
  uint32_t elapsedMs = now - lastFrameMs;
  lastFrameMs        = now;
  // decayPerMs covers the full 0.0–1.0 range in FADE_TIME_MS
  float decayPerMs   = 1.0f / (float)FADE_TIME_MS;
  float decayStep    = decayPerMs * (float)elapsedMs;

  for (int arm = 0; arm < NUM_STRIPS; arm++) {
    if (fillLevel[arm] >= displayLevel[arm]) {
      // Signal rose — snap up instantly
      displayLevel[arm] = fillLevel[arm];
    } else {
      // Signal dropped — decay one step, but never below the live signal
      displayLevel[arm] -= decayStep;
      if (displayLevel[arm] < fillLevel[arm]) displayLevel[arm] = fillLevel[arm];
    }
  }

  // ── Render LEDs ──────────────────────────────────────────────────────────
  for (int arm = 0; arm < NUM_STRIPS; arm++) {
    int litCount = (int)(displayLevel[arm] * NUM_LEDS_PER_STRIP + 0.5f);
    int baseIdx  = arm * NUM_LEDS_PER_STRIP;

    for (int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
      // j=0 is tip, j=NUM_LEDS_PER_STRIP-1 is hub
      // bar fills inward from hub: LEDs >= (NUM_LEDS_PER_STRIP - litCount) are lit
      if (j >= (NUM_LEDS_PER_STRIP - litCount)) {
        // Color: blue (hue 160) at tip, red (hue 0) at hub
        uint8_t hue = (uint8_t)((160 * (NUM_LEDS_PER_STRIP - 1 - j)) / (NUM_LEDS_PER_STRIP - 1));
        leds[baseIdx + j] = CHSV(hue, 255, 255);
      } else {
        leds[baseIdx + j] = CRGB::Black;
      }
    }
  }

  FastLED.show();
}
