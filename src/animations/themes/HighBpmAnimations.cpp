#include <Arduino.h>
#include <FastLED.h>
#include <fl/colorutils_misc.h>
#include <fl/colorutils.h>
#include "../AnimationBase.h"
#include "../../config/Config.h"

// ---------------------- High BPM Animations ----------------------
class HeartbeatAnimation : public Animation {
public:
    HeartbeatAnimation(CRGB* leds, uint16_t count) : Animation(leds, count, "Heartbeat") {}
    void update() override {
        uint8_t pulse = beatsin8(40, MIN_BRIGHTNESS, brightness);
        CRGB color = CRGB::Red;
        color.fadeToBlackBy(220 - pulse);
        fill_solid(leds, numLeds, color);
    }
};
static Registrar<HeartbeatAnimation> heartbeatRegistrator("Heartbeat");

class StrobePulseAnimation : public Animation {
public:
    StrobePulseAnimation(CRGB* leds, uint16_t count) : Animation(leds, count, "Strobe Pulse -NoShuffle") {}
    void update() override {
        uint8_t pulse = beatsin8(120, 50, 255);
        CRGB color = pulse > 200 ? CRGB(CHSV(random8(), 255, brightness)) : CRGB::Black;
        fill_solid(leds, numLeds, color);
    }
};
static Registrar<StrobePulseAnimation> strobePulseRegistrator("Strobe Pulse -NoShuffle");

class BeatScannerAnimation : public Animation {
private:
    uint16_t pos = 0;
    uint8_t gHue = 0;
public:
    BeatScannerAnimation(CRGB* leds, uint16_t count) : Animation(leds, count, "Beat Scanner") {}
    void update() override {
        EVERY_N_MILLISECONDS(20) {
            pos = beatsin16(60, 0, numLeds-1);
            fadeToBlackBy(leds, numLeds, 50);
            leds[pos] += CHSV(gHue, 255, brightness);
            leds[(numLeds-1)-pos] += CHSV(gHue+128, 255, brightness);
            gHue += 2;
        }
    }
};
static Registrar<BeatScannerAnimation> beatScannerRegistrator("Beat Scanner");

class ColorSlamAnimation : public Animation {
private:
    uint8_t lastBeat = 0;
public:
    ColorSlamAnimation(CRGB* leds, uint16_t count) : Animation(leds, count, "Color Slam") {}
    void update() override {
        uint8_t beat = beatsin8(60, 0, 100);
        if (beat < 10 && lastBeat >= 10)
            fill_solid(leds, numLeds, CHSV(random8(), 255, brightness));
        else
            fadeToBlackBy(leds, numLeds, 30);
        lastBeat = beat;
    }
};
static Registrar<ColorSlamAnimation> colorSlamRegistrator("Color Slam");

class BeatDropAnimation : public Animation {
private:
    uint8_t dropCounter = 0;
    uint8_t gHue = 0;
public:
    BeatDropAnimation(CRGB* leds, uint16_t count) : Animation(leds, count, "Beat Drop") {}
    void update() override {
        EVERY_N_MILLISECONDS(50) {
            if (dropCounter == 0)
                fill_solid(leds, numLeds, CHSV(gHue, 255, beatsin8(30, 50, 150)));
            else if (dropCounter < 10)
                fill_solid(leds, numLeds, CRGB::White);
            else {
                fadeToBlackBy(leds, numLeds, 30);
                for (int i = 0; i < 5; i++)
                    leds[random16(numLeds)] = CHSV(gHue + random16(64), 255, brightness);
            }
            if (++dropCounter >= 30) {
                dropCounter = 0;
                gHue += 64;
            }
        }
    }
};
static Registrar<BeatDropAnimation> beatDropRegistrator("Beat Drop");

// ---------------------- Memory Leak Review ----------------------
// - No dynamic allocations within update() methods => ✅
// - Animation registry uses `new` but matches AnimationManager's management model => ✅
// - No use of malloc/free, raw file IO, or unbounded memory growth => ✅
// → **Conclusion:** No obvious memory leaks present.
