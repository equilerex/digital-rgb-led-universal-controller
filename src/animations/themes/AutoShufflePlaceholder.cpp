#include <Arduino.h>
#include <FastLED.h>
#include "../AnimationBase.h"
#include "../../config/Config.h"

class AutoShuffleAnimation : public Animation {
public:
    AutoShuffleAnimation(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "R Shuffle") {}
    void update() override {}
};
static Registrar<AutoShuffleAnimation> AutoShuffleAnimationRegistrar("R Shuffle");


class AutoShuffleAnimation1 : public Animation {
public:
    AutoShuffleAnimation1(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "10s Shuffle") {}
    void update() override {}
};
static Registrar<AutoShuffleAnimation1> AutoShuffleAnimation1Registrar("10s shuffle");



class AutoShuffleAnimation2 : public Animation {
public:
    AutoShuffleAnimation2(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "10s Shuffle") {}
    void update() override {}
};
static Registrar<AutoShuffleAnimation2> AutoShuffleAnimation2Registrar("10s shuffle");


class AutoShuffleAnimation3 : public Animation {
public:
    AutoShuffleAnimation3(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "Shuffle") {}
    void update() override {}
};
static Registrar<AutoShuffleAnimation3> AutoShuffleAnimation3Registrar("5m shuffle");
