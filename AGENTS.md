# AGENTS.md — hippietrap

## Hardware target

- **Board**: Freenove ESP32-S3 WROOM (`freenove_esp32_s3_wroom` in `platformio.ini`)
- **LED chipset**: WS2815 (12 V, redundant data line — not WS2812)
- **Microphone**: INMP441 via I2S (32-bit frames, data in upper 24 bits — shift right 8 before use). BCLK on GPIO 38 is confirmed working. **All-zero I2S reads with correct byte count** (2048 bytes / 512 samples) indicates a wiring fault (e.g. shorted SD line), not a GPIO or driver issue — check physical connections before changing pin assignments.
- **IMU**: DFRobot BMI160 on I2C addr `0x69` — wired but **`BMI_setup()` is commented out** in `setup()`; do not treat gyro data as available

## Hardware variants — compile-time selection

The same repo targets two physical parasols. The active variant is set in **`src/localSettings.h`** (git-ignored):

```cpp
#define LARGE_PARASOL true   // Jacob & Christian: 16 strips × 73 LEDs = 1 168 total
// or
#define LARGE_PARASOL false  // Billman & Elin:    8 strips × 105 LEDs = 840 total
```

`localSettings.h` also exposes `HARDCODED_MENU` (int). When non-zero it overrides `menuChoice` every loop — useful for locking to a single effect during development.

**Never hardcode `NUM_STRIPS`, `NUM_LEDS_PER_STRIP`, or `NUM_LEDS`** — always use those macros; they resolve differently per variant.

On SMALL_PARASOL, GPIO 4 and 5 are **button pins** (`BUTTON_PIN_2_MID`, `BUTTON_PIN_3_TOP`). On LARGE_PARASOL those same GPIOs are **LED data pins** (`DATA16`, `DATA15`). Adding code that assumes one role will break the other variant.

## Key architectural facts

- **LED array is flat**: `CRGB leds[NUM_LEDS]` in `main.cpp`. Strip `n` occupies indices `[n * NUM_LEDS_PER_STRIP … (n+1) * NUM_LEDS_PER_STRIP - 1]`.
- **`fillSpan(leds, start, stop, color)`** in `effects.cpp` applies the same diode offset across *every* strip simultaneously — it is not single-strip.
- **SMALL_PARASOL LED zones** (indices within a strip): TOP `0–44`, RIM `45–76`, DANGLE `77–104`. `START_RIM`/`START_DANGLE` macros are defined but only meaningful for SMALL_PARASOL.
- **Effect dispatch** uses `menuChoice` (int). Active effects are `1 = rainbowFlow`, `2 = audioSpectrum`. Long press (≥ 2 s) sets `explosionPending` and interrupts the current effect for one full explosion sequence.
- **`iterating_variable`** is a shared animation counter reset to 0 on each button press. Effects that use it advance it themselves; `explosionEffect` uses it as its phase counter (phases 0–2).

## Audio / FFT pipeline

- `runFFT()` in `effectAudioSpectrum.cpp` is the **live implementation**. `fastFourierTransformAudio()` in `main.cpp` is a superseded duplicate — do not call or extend the one in `main.cpp`.
- FFT is 512-point, 16 kHz sample rate → bin width ≈ 31.25 Hz. Buffers (`fft_raw`, `complex_buffer`, `bandMagnitudes`, `bands`, `numBands`) are owned by `main.cpp` and `extern`-declared in `effectAudioSpectrum.cpp`.
- `audioSpectrum()` contains a 60-second sliding-window auto-gain controller (static state inside `effectAudioSpectrum.cpp`). It does **not** reset between button presses.
- On LARGE_PARASOL: 16 bands → 16 arms, 1:1. On SMALL_PARASOL: 16 bands averaged in adjacent pairs → 8 arms.
- Every stage of `runFFT` and `audioSpectrum` emits `Serial.printf` debug lines (`[AS-1]`…`[AS-5]`) unconditionally — serial output is very verbose at runtime.

## Adding a new effect

1. Create `src/effectMyEffect.cpp`; include `<FastLED.h>`, `"effects.h"`, `"globals.h"`.
2. Declare the function in `src/effects.h`.
3. Add a `case N:` in the `switch(menuChoice)` in `main.cpp`; increment `MENU_MAX` in `globals.h`.
4. Use `fillSpan()` for operations across all strips, or iterate `arm * NUM_LEDS_PER_STRIP + j` directly.
5. Call `FastLED.show()` at the end of the effect — no central show call exists in `loop()`.

## Build & flash commands

```sh
# Build
pio run

# Flash
pio run --target upload

# Serial monitor (115200 baud)
pio device monitor

# Build + flash + monitor in one step
pio run --target upload && pio device monitor
```

No CI, no pre-commit hooks, no test suite.

## Unused dependencies

`TinyGSM` and `ArduinoJson` are declared in `platformio.ini` but have **zero usage** in any source file. Do not rely on them being functional; do not remove them without checking if a feature is planned.

## `DEBUG_MODE`

Defined as `true` in `globals.h`. Some guards use `#ifdef DEBUG_MODE` (truthy on mere definition), others check `if (DEBUG_MODE)` at runtime. Both patterns are present — be consistent with whichever is already used in the file being edited.
