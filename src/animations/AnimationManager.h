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
    const char* getCurrentPatternName();
    uint8_t getPatternCount();
    uint8_t getCurrentPatternIndex();
    bool isReady() { return isInitialized; }
    void setCurrentPattern(uint8_t index);
    void setNumLeds(uint16_t count);
    void setBrightness(uint8_t value);

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
    bool inTransition = false;
    uint32_t transitionStart = 0;
    const uint32_t transitionDuration = SHUFFLE_TRANSITION_DURATION; // Use dedicated duration
    uint8_t transitionNewIndex = 0;
    void logFastLEDDiagnostics();

    // Shuffle mode fields
    uint8_t currentShuffleIndex;
    unsigned long lastShuffleTime;
    bool inShuffleMode = false; // Track shuffle state

    void registerAnimations();
    void createAnimation(uint8_t index);
    void cleanupCurrentAnimation();
    void startTransition(uint8_t newIndex);
    void pickNewShuffle();
};

#endif // ANIMATION_MANAGER_H