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
static const int   GAIN_WINDOW_FRAMES = 1875; // 60 s × 31.25 frames/s
static const float SAT_THRESHOLD      = 0.95f; // fill ≥ 95 % = saturated
static const float TARGET_SAT_LOW     = 0.30f; // target range low end
static const float TARGET_SAT_HIGH    = 0.60f; // target range high end
static const float GAIN_STEP_UP       = 0.02f; // raise gain when too quiet
static const float GAIN_STEP_DOWN     = 0.05f; // lower gain faster when clipping
static const float GAIN_MIN           = 0.1f;
static const float GAIN_MAX           = 50.0f;
static const int   STARTUP_FRAMES     = 310;   // ~10 s ramp-in period

// ── Static state (persists across frames) ────────────────────────────────────
static float   gain           = 1.0f;
static int     frameCount     = 0;
static uint8_t satHistory[GAIN_WINDOW_FRAMES]; // 1 = frame had a saturated arm
static int     satHistoryIdx  = 0;
static int     satCount       = 0; // running sum of satHistory[]

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

  // ── Compute per-arm fill levels ──────────────────────────────────────────
  float fillLevel[NUM_STRIPS];

  for (int arm = 0; arm < NUM_STRIPS; arm++) {
    float mag;

#if LARGE_PARASOL
    // 16 arms, 16 bands — direct 1:1 mapping
    mag = bandMagnitudes[arm];
#else
    // 8 arms, 16 bands — average adjacent pairs
    mag = (bandMagnitudes[arm * 2] + bandMagnitudes[arm * 2 + 1]) * 0.5f;
#endif

    fillLevel[arm] = mag * gain;
    if (fillLevel[arm] > 1.0f) fillLevel[arm] = 1.0f;
    if (fillLevel[arm] < 0.0f) fillLevel[arm] = 0.0f;
  }

  // ── Auto-gain: update saturation history ────────────────────────────────
  uint8_t anySaturated = 0;
  for (int arm = 0; arm < NUM_STRIPS; arm++) {
    if (fillLevel[arm] >= SAT_THRESHOLD) { anySaturated = 1; break; }
  }

  // Subtract oldest entry, insert new one
  satCount -= satHistory[satHistoryIdx];
  satHistory[satHistoryIdx] = anySaturated;
  satCount += anySaturated;
  satHistoryIdx = (satHistoryIdx + 1) % GAIN_WINDOW_FRAMES;

  // Compute how many frames in the window had saturation
  float satFraction = (float)satCount / (float)GAIN_WINDOW_FRAMES;

  // During startup, blend computed gain with DEFAULT_GAIN
  float targetGain = gain;
  if (satFraction < TARGET_SAT_LOW)       targetGain = gain + GAIN_STEP_UP;
  else if (satFraction > TARGET_SAT_HIGH) targetGain = gain - GAIN_STEP_DOWN;

  if (frameCount < STARTUP_FRAMES) {
    float ramp = (float)frameCount / (float)STARTUP_FRAMES; // 0.0 → 1.0
    targetGain = 1.0f + ramp * (targetGain - 1.0f);
  }

  gain = targetGain;
  if (gain < GAIN_MIN) gain = GAIN_MIN;
  if (gain > GAIN_MAX) gain = GAIN_MAX;

  frameCount++;

  // ── Debug output ─────────────────────────────────────────────────────────
  Serial.printf("[AS-5] gain=%.2f sat=%.2f | fill:", gain, satFraction);
  for (int arm = 0; arm < NUM_STRIPS; arm++) {
    Serial.printf(" %3d%%", (int)(fillLevel[arm] * 100.0f));
  }
  Serial.println();

  // ── Render LEDs ──────────────────────────────────────────────────────────
  for (int arm = 0; arm < NUM_STRIPS; arm++) {
    int litCount = (int)(fillLevel[arm] * NUM_LEDS_PER_STRIP + 0.5f);
    int baseIdx  = arm * NUM_LEDS_PER_STRIP;

    for (int j = 0; j < NUM_LEDS_PER_STRIP; j++) {
      // j=0 is tip, j=NUM_LEDS_PER_STRIP-1 is hub
      // bar fills inward from hub: LEDs >= (NUM_LEDS_PER_STRIP - litCount) are lit
      if (j >= (NUM_LEDS_PER_STRIP - litCount)) {
        // Color: blue (hue 160) at tip (j=0), red (hue 0) at hub (j=max)
        uint8_t hue = (uint8_t)((160 * (NUM_LEDS_PER_STRIP - 1 - j)) / (NUM_LEDS_PER_STRIP - 1));
        leds[baseIdx + j] = CHSV(hue, 255, 255);
      } else {
        leds[baseIdx + j] = CRGB::Black;
      }
    }
  }

  FastLED.show();
}
