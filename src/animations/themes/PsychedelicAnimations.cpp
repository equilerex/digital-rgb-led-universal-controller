#include <Arduino.h>
#include <FastLED.h>
#include <fl/colorutils.h>
#include <fl/colorutils_misc.h>
#include "../AnimationBase.h"
#include "../../config/Config.h"

class TwoSinPsyAnimation : public Animation {
private:
    uint8_t wavebright = 128, thathue = 80;
    uint8_t thisrot = 1, thatrot = 0, allsat = 255;
    uint8_t thisspeed = 1, thatspeed = 1;
    uint8_t allfreq = 32, thisphase = 0, thatphase = 0;
    uint8_t thiscutoff = 96, thatcutoff = 96;
public:
    TwoSinPsyAnimation(CRGB* leds, uint16_t count) : Animation(leds, count, "Two Sin Psy") {}
    void update() override {
        thisphase += thisspeed;
        thatphase += thatspeed;
        uint8_t thishue = thisrot, thathue2 = thishue + 128 + thatrot;
        for (int k = 0; k < numLeds; k++) {
            int thisbright = qsuba(cubicwave8((k*allfreq)+thisphase), thiscutoff);
            int thatbright = qsuba(cubicwave8((k*allfreq)+128+thatphase), thatcutoff);
            leds[k] = CHSV(thishue, allsat, thisbright);
            leds[k] += CHSV(thathue2, allsat, thatbright);
        }
    }
};
static Registrar<TwoSinPsyAnimation> twoSinPsyRegistrator("Two Sin nPsy");

class ThreeSinTwoAnimation : public Animation {
private:
    uint8_t wave1 = 0, wave2 = 0, wave3 = 0;
    uint8_t inc1 = 2, inc2 = 1; int8_t inc3 = -3;
    uint8_t lvl1 = 80, lvl2 = 80, lvl3 = 80;
    uint8_t mul1 = 5, mul2 = 8, mul3 = 7;
public:
    ThreeSinTwoAnimation(CRGB* leds, uint16_t count) : Animation(leds, count, "Three Sin Two") {}
    void update() override {
        wave1 += inc1;
        wave2 += inc2;
        wave3 += inc3;
        for (int k=0; k<numLeds; k++) {
            leds[k].r = qsub8(sin8(mul1*k + wave1), lvl1);
            leds[k].g = qsub8(sin8(mul2*k + wave2), lvl2);
            leds[k].b = qsub8(sin8(mul3*k + wave3), lvl3);
        }
    }
};
static Registrar<ThreeSinTwoAnimation> threeSinTwoRegistrator("Three Sin Two");

class PopFadeAnimation : public Animation {
private:
    uint8_t ranamount = 50, numcolours = 1, maxbar = 1, fadeval = 224;
    bool boolcolours = false;
    uint32_t colours[1] = { 0x13b0f2 };
public:
    PopFadeAnimation(CRGB* leds, uint16_t count) : Animation(leds, count, "Pop Fade") {}
    void update() override {
        unsigned long thiscolour = colours[0];
        int idex = random16(0, ranamount);
        if (idex < numLeds) {
            if (boolcolours) thiscolour = random(0xFFFFFF);
            int barlen = random16(1, maxbar);
            for (int i = 0; i < barlen && (idex + i < numLeds); i++) {
                leds[idex + i] = thiscolour;
            }
        }
        nscale8(leds, numLeds, fadeval);
    }
};
static Registrar<PopFadeAnimation> popFadeRegistrator("Pop Fade");

class PlasmaEffectAnimation : public Animation {
private:
    unsigned long plasmaTime = 0;
public:
    PlasmaEffectAnimation(CRGB* leds, uint16_t count) : Animation(leds, count, "Plasma Effect") {}
    void update() override {
        plasmaTime += 20;
        for (int i = 0; i < numLeds; i++) {
            int x = i * 10;
            int y = plasmaTime / 10;
            int colorIndex = sin8(x + y) + cos8(x - y);
            leds[i] = ColorFromPalette(RainbowColors_p, colorIndex, brightness);
        }
    }
};
static Registrar<PlasmaEffectAnimation> plasmaEffectRegistrator("Plasma Effect");

class LavaLampAnimation : public Animation {
private:
    uint32_t x = 0;
    unsigned long lastBrightnessChange = 0;
public:
    LavaLampAnimation(CRGB* leds, uint16_t count) : Animation(leds, count, "Lava Lamp") {}
    void update() override {
        x += 10;
        for (int i = 0; i < numLeds; i++) {
            uint8_t noise = inoise8(i * 50, x);
            uint8_t hue = map(noise, 0, 255, 10, 30);
            leds[i] = CHSV(hue, 255, noise);
        }
        unsigned long currentMillis = millis();
        if (currentMillis - lastBrightnessChange > 30000) {
            FastLED.setBrightness(random8(MIN_BRIGHTNESS + 20, brightness - 20));
            lastBrightnessChange = currentMillis;
        }
    }
};
static Registrar<LavaLampAnimation> lavaLampRegistrator("Lava Lamp");
