#ifndef ANIMATION_MANAGER_H
#define ANIMATION_MANAGER_H

#include <vector>
#include <FastLED.h>
#include "AnimationBase.h"
#include "../config/Config.h"

// Forward declaration
class SystemManager;

class AnimationManager {
public:
    AnimationManager(SystemManager& systemManager, CRGB* leds);
    ~AnimationManager();

    void begin();
    void update();
    CRGB* getLEDs() { return leds; }
    uint16_t getNumLeds() const { return numLeds; }
    uint8_t getBrightness() const { return brightness; }
    void nextPattern();
    const char* getCurrentAnimationName();
    uint8_t getPatternCount();
    uint8_t getCurrentPatternIndex();
    bool isReady() { return isInitialized; }
    void setCurrentPattern(uint8_t index);
    void setNumLeds(uint16_t count);
    void setBrightness(uint8_t value);

    // Shuffle mode check
    bool inShuffleMode() const { return currentPatternIndex == 0; }

private:
    SystemManager& systemManager;
    CRGB* leds;
    uint16_t numLeds;
    uint8_t brightness;
    uint8_t currentPatternIndex;
    Animation* currentAnimation;
    bool isInitialized;
    CRGB oldLedsBuffer[MAX_LEDS];
    CRGB tempLeds[MAX_LEDS];

    // Shuffle/transition fields
    bool inShuffleTransition;
    uint32_t shuffleTransitionStart;
    const uint32_t shuffleTransitionDuration = SHUFFLE_TRANSITION_DURATION;
    uint8_t shuffleTransitionNewIndex;
    uint8_t currentShuffleIndex;
    unsigned long lastShuffleTime;

    void logFastLEDDiagnostics();
    void registerAnimations();
    void createAnimation(uint8_t index);
    void cleanupCurrentAnimation();
    void startShuffleTransition(uint8_t newIndex);
    void pickNewShuffle();
};

#endif // ANIMATION_MANAGER_H