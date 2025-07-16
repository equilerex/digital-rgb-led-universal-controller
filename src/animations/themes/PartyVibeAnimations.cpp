#include <Arduino.h>
#include <FastLED.h>
#include "../AnimationBase.h"
#include "../../config/Config.h"
#include <fl/colorutils.h>
#include <fl/colorutils_misc.h>

// ---------------------- Party Vibe Animations ----------------------
class RainbowMarchAnimation : public Animation {
private:
    uint8_t thishue = 0;
    uint8_t deltahue = 2;
public:
    RainbowMarchAnimation(CRGB* leds, uint16_t count) : Animation(leds, count, "Rainbow March") {}
    void update() override {
        EVERY_N_MILLISECONDS(10) {
            fill_rainbow(leds, numLeds, thishue, deltahue);
            thishue += 5;
        }
    }
};
static Registrar<RainbowMarchAnimation> rainbowMarchRegistrator("Rainbow March");

class ConfettiAnimation : public Animation {
private:
    uint8_t gHue = 0;
public:
    ConfettiAnimation(CRGB* leds, uint16_t count) : Animation(leds, count, "Confetti") {}
    void update() override {
        fadeToBlackBy(leds, numLeds, 10);
        int pos = random16(numLeds);
        leds[pos] += CHSV(gHue + random8(64), 200, brightness);
        EVERY_N_MILLISECONDS(20) { gHue++; }
    }
};
static Registrar<ConfettiAnimation> confettiRegistrator("Confetti");

class BpmAnimation : public Animation {
private:
    uint8_t gHue = 0;
public:
    BpmAnimation(CRGB* leds, uint16_t count) : Animation(leds, count, "BPM") {}
    void update() override {
        uint8_t beat = beatsin8(62, 64, brightness);
        for (int i = 0; i < numLeds; i++) {
            leds[i] = ColorFromPalette(PartyColors_p, gHue + (i * 2), beat - gHue + (i * 10));
        }
        EVERY_N_MILLISECONDS(20) { gHue++; }
    }
};
static Registrar<BpmAnimation> bpmRegistrator("BPM");

class TwinkleStarsAnimation : public Animation {
public:
    TwinkleStarsAnimation(CRGB* leds, uint16_t count) : Animation(leds, count, "Twinkle Stars") {}
    void update() override {
        fadeToBlackBy(leds, numLeds, 10);
        int pos = random16(numLeds);
        leds[pos] += CHSV(random8(64, 192), 200, brightness);
    }
};
static Registrar<TwinkleStarsAnimation> twinkleStarsRegistrator("Twinkle Stars");

class ColorWavesAnimation : public Animation {
private:
    uint16_t sPseudotime = 0, sLastMillis = 0, sHue16 = 0;
public:
    ColorWavesAnimation(CRGB* leds, uint16_t count) : Animation(leds, count, "Color Waves") {}
    void update() override {
        uint8_t brightdepth = scale8(beatsin88(341, 96, 224), brightness);
        uint16_t brightnessthetainc16 = beatsin88(203, 25 * 256, 40 * 256);
        uint8_t msmultiplier = beatsin88(147, 23, 60);
        uint16_t hue16 = sHue16;
        uint16_t hueinc16 = beatsin88(113, 1, 3000);
        uint16_t ms = millis();
        uint16_t deltams = ms - sLastMillis;
        sLastMillis = ms;
        sPseudotime += deltams * msmultiplier;
        sHue16 += deltams * beatsin88(400, 5, 9);
        uint16_t brightnesstheta16 = sPseudotime;
        for (uint16_t i = 0; i < numLeds; i++) {
            hue16 += hueinc16;
            uint8_t hue8 = hue16 / 256;
            if ((hue16 >> 7) & 0x100) hue8 = 255 - hue8;
            uint8_t bright = scale8(qadd8(brightdepth, sin8(brightnesstheta16)), brightness);
            brightnesstheta16 += brightnessthetainc16;
            leds[i] = CHSV(hue8, 255, bright);
        }
    }
};
static Registrar<ColorWavesAnimation> colorWavesRegistrator("Color Waves");


class TwoSinAnimation : public Animation {
private:
    uint8_t wavebright = 128, thathue = 80;
    uint8_t thisrot = 1, thatrot = 0, allsat = 255;
    uint8_t thisspeed = 1, thatspeed = 1;
    uint8_t allfreq = 32, thisphase = 0, thatphase = 0;
    uint8_t thiscutoff = 96, thatcutoff = 96;
public:
    TwoSinAnimation(CRGB* leds, uint16_t count) : Animation(leds, count, "Two Sin") {}
    void update() override {
        thisphase += thisspeed;
        thatphase += thatspeed;
        uint8_t thishue = thisrot, thathue2 = thishue + 128 + thatrot;
        for (int k = 0; k < numLeds; k++) {
            int thisbright = qsuba(cubicwave8((k * allfreq) + thisphase), thiscutoff);
            int thatbright = qsuba(cubicwave8((k * allfreq) + 128 + thatphase), thatcutoff);
            leds[k] = CHSV(thishue, allsat, thisbright);
            leds[k] += CHSV(thathue2, allsat, thatbright);
        }
    }
};
static Registrar<TwoSinAnimation> twoSinRegistrator("Two Sin");


class ThreeSinAnimation : public Animation {
private:
    uint8_t wave1 = 0, wave2 = 0, wave3 = 0;
    uint8_t inc1 = 2, inc2 = 1; int8_t inc3 = -3;
    uint8_t lvl1 = 80, lvl2 = 80, lvl3 = 80;
    uint8_t mul1 = 5, mul2 = 8, mul3 = 7;
public:
    ThreeSinAnimation(CRGB* leds, uint16_t count) : Animation(leds, count, "Three Sin") {}
    void update() override {
        wave1 += inc1; wave2 += inc2; wave3 += inc3;
        for (int k = 0; k < numLeds; k++) {
            leds[k].r = qsub8(sin8(mul1 * k + wave1), lvl1);
            leds[k].g = qsub8(sin8(mul2 * k + wave2), lvl2);
            leds[k].b = qsub8(sin8(mul3 * k + wave3), lvl3);
        }
    }
};
static Registrar<ThreeSinAnimation> threeSinRegistrator("Three Sin");


class PlasmaEffectTwoAnimation : public Animation {
private:
    unsigned long plasmaTime = 0;
public:
    PlasmaEffectTwoAnimation(CRGB* leds, uint16_t count) : Animation(leds, count, "Plasma Effect 2") {}
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
static Registrar<PlasmaEffectTwoAnimation> plasmaEffectTwoAnimationRegistrator("Plasma Effect 2");

class LavaLampAnimationTwo : public Animation {
private:
    uint32_t x = 0;
    unsigned long lastBrightnessChange = 0;
public:
    LavaLampAnimationTwo(CRGB* leds, uint16_t count) : Animation(leds, count, "Lava Lamp 2") {}
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
static Registrar<LavaLampAnimationTwo> LavaLampAnimationTwoRegistrator("Lava Lamp 2");
