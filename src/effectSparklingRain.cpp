#include <FastLED.h>
#include "effects.h"
#include "globals.h"

// ── Configurable parameters ───────────────────────────────────────────────────
static const uint32_t SPAWN_INTERVAL_MS = 200;  // ms between spawn attempts; lower = more drops
static const float    DROP_SPEED        = 0.05f; // pixels per ms (~9 px/s at 0.3)
static const uint8_t  TAIL_LENGTH       = 15;   // solid fading tail length in pixels (10–20)
static const uint8_t  BREAKUP_LENGTH    = 25;   // sparse break-up zone length in pixels (20–30)
static const uint8_t  MAX_DROPS        = 20;    // drop pool size

// ── Drop state ────────────────────────────────────────────────────────────────
struct Drop {
    uint8_t  arm;    // which strip (0 … NUM_STRIPS-1)
    float    pos;    // head pixel position (float for sub-pixel precision)
    uint16_t seed;   // per-drop seed for stable break-up pattern
    bool     active;
};

static Drop     drops[MAX_DROPS];
static uint32_t lastFrameMs = 0;
static uint32_t lastSpawnMs = 0;
static bool     initDone    = false;

// ── Helpers ───────────────────────────────────────────────────────────────────

// Set a pixel on an arm, blending additive-style (take brighter channel per component)
// so overlapping drops don't fight each other.
static inline void setPixel(CRGB* leds, uint8_t arm, int pos, CRGB color) {
    if (pos < 0 || pos >= NUM_LEDS_PER_STRIP) return;
    int idx = arm * NUM_LEDS_PER_STRIP + pos;
    // Additive blend: keep the brightest value per channel
    leds[idx].r = qadd8(leds[idx].r, color.r);
    leds[idx].g = qadd8(leds[idx].g, color.g);
    leds[idx].b = qadd8(leds[idx].b, color.b);
}

// ── Main effect entry point ───────────────────────────────────────────────────
void sparklingRain(CRGB* leds) {

    // One-time init
    if (!initDone) {
        for (uint8_t i = 0; i < MAX_DROPS; i++) drops[i].active = false;
        lastFrameMs = millis();
        lastSpawnMs = millis();
        initDone    = true;
    }

    uint32_t now     = millis();
    uint32_t elapsed = now - lastFrameMs;
    lastFrameMs      = now;

    // ── Spawn ─────────────────────────────────────────────────────────────────
    if (now - lastSpawnMs >= SPAWN_INTERVAL_MS) {
        lastSpawnMs = now;
        // Find a free slot
        for (uint8_t i = 0; i < MAX_DROPS; i++) {
            if (!drops[i].active) {
                drops[i].arm    = random8(NUM_STRIPS);
                drops[i].pos    = 0.0f;
                drops[i].seed   = random16();
                drops[i].active = true;
                break;
            }
        }
    }

    // ── Advance ───────────────────────────────────────────────────────────────
    float advance = DROP_SPEED * (float)elapsed;
    for (uint8_t i = 0; i < MAX_DROPS; i++) {
        if (!drops[i].active) continue;
        drops[i].pos += advance;
        // Deactivate once the entire drop (including break-up tail) has left the strip
        if (drops[i].pos > (float)(NUM_LEDS_PER_STRIP + BREAKUP_LENGTH + 2)) {
            drops[i].active = false;
        }
    }

    // ── Clear ─────────────────────────────────────────────────────────────────
    fill_solid(leds, NUM_LEDS, CRGB::Black);

    // ── Render each active drop ───────────────────────────────────────────────
    for (uint8_t i = 0; i < MAX_DROPS; i++) {
        if (!drops[i].active) continue;

        uint8_t  arm  = drops[i].arm;
        int      head = (int)drops[i].pos;
        uint16_t seed = drops[i].seed;

        // Precursor: one pixel ahead of the head (in direction of travel), dim cool-white
        setPixel(leds, arm, head + 1, CHSV(200, 80, 50));

        // Head: bright near-white with a blue tint
        setPixel(leds, arm, head, CHSV(200, 60, 255));

        // Solid tail: behind the head, linearly fading brightness 200 → 25
        for (uint8_t t = 1; t <= TAIL_LENGTH; t++) {
            uint8_t bri = (uint8_t)(200 - (175 * t / TAIL_LENGTH));
            setPixel(leds, arm, head - t, CHSV(200, 80, bri));
        }

        // Break-up zone: sparse pixels behind the tail, low brightness
        for (uint8_t b = 1; b <= BREAKUP_LENGTH; b++) {
            // Mix seed and offset for better distribution across all seeds
            uint16_t hash = seed ^ (uint16_t)(b * 0xA3u);
            if (hash % 3 != 0) {
                // Brightness varies per pixel: 13–50
                uint8_t bri = 13 + (uint8_t)(((seed ^ (uint16_t)(b * 0x5Fu)) % 38));
                setPixel(leds, arm, head - TAIL_LENGTH - b, CHSV(210, 120, bri));
            }
        }
    }

    FastLED.show();
}
