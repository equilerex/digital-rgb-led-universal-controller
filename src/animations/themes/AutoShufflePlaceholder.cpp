#include <Arduino.h>
#include <FastLED.h>
#include "../AnimationBase.h"
#include "../../config/Config.h"

class AutoShuffleAnimation : public Animation {
public:
    AutoShuffleAnimation(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "Shuffle") {}
    void update() override {}
};
static Registrar<AutoShuffleAnimation> AutoShuffleAnimationRegistrar("AutoShuffleAnimation");