#include <Arduino.h>
#include <FastLED.h>
#include "../AnimationBase.h"
#include "../../config/Config.h"
#include "fl/colorutils.h"
#include "fl/colorutils_misc.h"

class RedPurpleBlueAnimation : public Animation {
private:
    uint8_t hue;
public:
    RedPurpleBlueAnimation(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "Red Purple Blue"), hue(255) {}
    void update() override {
        hue = colorModifier;
        fill_solid(leds, numLeds, CRGB(hue, 0, 255-hue));
    }
};
static Registrar<RedPurpleBlueAnimation> redPurpleBlueRegistrar("Red Purple Blue");

class GreenYellowRedAnimation : public Animation {
private:
    uint8_t hue;
public:
    GreenYellowRedAnimation(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "Green Yellow Red"), hue(255) {}
    void update() override {
        hue = colorModifier;
        fill_solid(leds, numLeds, CRGB(255-hue, hue, 0));
    }
};
static Registrar<GreenYellowRedAnimation> greenYellowRedRegistrar("Green Yellow Red");

class GreenBlueAnimation : public Animation {
private:
    uint8_t hue;
public:
    GreenBlueAnimation(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "Green Blue"), hue(255) {}
    void update() override {
        hue = colorModifier;
        fill_solid(leds, numLeds, CRGB(0, 255-hue, hue));
    }
};
static Registrar<GreenBlueAnimation> greenBlueRegistrar("Green Blue");

class OrangeAnimation : public Animation {
public:
    OrangeAnimation(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "Orange") {}
    void update() override {
        fill_solid(leds, numLeds, CRGB(255, 100, 0));
    }
};
static Registrar<OrangeAnimation> orangeRegistrar("Orange");

class PurpleAnimation : public Animation {
public:
    PurpleAnimation(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "Purple") {}
    void update() override {
        fill_solid(leds, numLeds, CRGB(255, 0, 255));
    }
};
static Registrar<PurpleAnimation> purpleRegistrar("Purple");