#include <Arduino.h>
#include <FastLED.h>
#include "../AnimationBase.h"
#include "../../config/Config.h"
#include <fl/colorutils.h>
#include <fl/colorutils_misc.h>

class HyperSpinAnimation : public Animation {
private:
    uint16_t angle = 0;
public:
    HyperSpinAnimation(CRGB* leds, uint16_t count) : Animation(leds, count, "Hyper Spin") {}
    void update() override {
        angle += beatsin16(60, 10, 30);
        for (int i = 0; i < numLeds; i++) {
            uint8_t colorIndex = (i * 10) - angle;
            leds[i] = ColorFromPalette(RainbowStripeColors_p, colorIndex, beatsin8(120, 200, brightness), LINEARBLEND);
        }
    }
};
static Registrar<HyperSpinAnimation> hyperSpinRegistrator("Hyper Spin");

class BeatTrailsAnimation : public Animation {
private:
    uint8_t gHue = 0;
public:
    BeatTrailsAnimation(CRGB* leds, uint16_t count) : Animation(leds, count, "Beat Trails") {}
    void update() override {
       // fadeToBlackBy(leds, numLeds, 40);
        int pos = beatsin16(30, 0, numLeds-1);
        leds[pos] += CHSV(gHue, 255, brightness);
        EVERY_N_MILLISECONDS(10) { gHue++; }
    }
};
static Registrar<BeatTrailsAnimation> beatTrailsRegistrator("Beat Trails");
