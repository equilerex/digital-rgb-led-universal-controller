#include "AnimationManager.h"
#include "AnimationBase.h"
#include "../system/SystemManager.h"
#include <Preferences.h>
#include <Arduino.h>
#include <FastLED.h>
#include <vector>
#include <algorithm>
#include "../config/Config.h"

#include <esp_task_wdt.h>
#include <soc/rtc_wdt.h>

// Global definition
std::vector<AnimationInfo> globalAnimationRegistry;

AnimationManager::AnimationManager(SystemManager& systemManager, CRGB* leds) : systemManager(systemManager), leds(leds), numLeds(DEFAULT_NUM_LEDS),
      brightness(DEFAULT_BRIGHTNESS), currentPatternIndex(0), currentAnimation(nullptr),
      isInitialized(false), currentShuffleIndex(0), lastShuffleTime(0) {
    
    // Initialize LED arrays to zero
    memset(oldLedsBuffer, 0, sizeof(oldLedsBuffer));
    memset(tempLeds, 0, sizeof(tempLeds));
    
    Serial.print(F("AnimationManager constructor - LED array at: 0x"));
    Serial.println(reinterpret_cast<uintptr_t>(leds), HEX);
}

AnimationManager::~AnimationManager() {
    cleanupCurrentAnimation();
}

void AnimationManager::begin() {
    Serial.println(F("AnimationManager Starting..."));

    numLeds = std::clamp(systemManager.getSavedNumber(Config::PREF_NUM_LEDS_KEY, DEFAULT_NUM_LEDS),
                         static_cast<uint16_t>(MIN_LEDS), static_cast<uint16_t>(MAX_LEDS));
    brightness = std::clamp(
        static_cast<uint8_t>(systemManager.getSavedNumber(Config::PREF_BRIGHTNESS_KEY, DEFAULT_BRIGHTNESS)),
        static_cast<uint8_t>(MIN_BRIGHTNESS),
        static_cast<uint8_t>(MAX_BRIGHTNESS)
    );  // Fixed: all clamp args are uint8_t

    Serial.print(F("Boot with numLeds: ")); Serial.println(numLeds);
    Serial.print(F("Boot with brightness: ")); Serial.println(brightness);

    registerAnimations();
    Serial.print(F("Animations registered: "));
    Serial.println(globalAnimationRegistry.size());

    if (globalAnimationRegistry.empty()) {
        Serial.println(F("ERROR: No animations registered! Check theme includes."));
        return;
    }

    // Load saved pattern
    uint8_t savedPatternIndex = systemManager.getSavedNumber(Config::PREF_PATTERN_KEY, 0);
    setCurrentPattern(savedPatternIndex);

    isInitialized = true;
    Serial.println(F("Animation system ready"));
}

void AnimationManager::update() {
    if (!isInitialized) {
        EVERY_N_SECONDS(5) { Serial.println(F("Animation not initialized")); }
        return;
    }
    if (!leds) {
        EVERY_N_SECONDS(5) { Serial.println(F("ERROR: Null LED array")); }
        return;
    }
    if (globalAnimationRegistry.empty()) {
        EVERY_N_SECONDS(5) { Serial.println(F("ERROR: No animations registered")); }
        return;
    }
    if (!currentAnimation) {
        EVERY_N_SECONDS(5) { Serial.println(F("ERROR: No active animation")); }
        return;
    }

    if (currentPatternIndex == 0 && millis() - lastShuffleTime > 5000) {
        pickNewShuffle();
    }

    if (inTransition) {
        uint32_t elapsed = millis() - transitionStart;
        if (elapsed >= transitionDuration) {
            inTransition = false;
            currentPatternIndex = transitionNewIndex;
        } else {
            float progress = (float)elapsed / transitionDuration;
            for (uint16_t i = 0; i < numLeds; i++) {
                leds[i] = blend(oldLedsBuffer[i], tempLeds[i], progress * 255);
            }
        }
    } else {
        currentAnimation->update();
    }

    #if defined(WATCHDOG_C3_WORKAROUND)
    esp_task_wdt_reset();
    #endif
}

void AnimationManager::initLEDs() {
    Serial.println(F("=== LED Initialization ==="));
    memset(leds, 0, sizeof(CRGB) * MAX_LEDS);
    Serial.print(F("Using buffer for ")); Serial.print(numLeds); Serial.println(F(" LEDs."));
    FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, numLeds);
    FastLED.setBrightness(brightness);
    #if defined(MAX_MILLIAMPS)
    Serial.print(F("Setting power limit: ")); Serial.print(MAX_MILLIAMPS); Serial.println(F(" mA"));
    FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_MILLIAMPS);
    #endif
    Serial.println(F("=== LED initialization complete! ==="));
}

void AnimationManager::registerAnimations() {
    Serial.println(F("DEBUG: Starting registerAnimations"));
    globalAnimationRegistry.clear();
    #include "themes/AutoShufflePlaceholder.cpp"
    #include "themes/SlowAndSoothingAnimations.cpp"
    #include "themes/SolidColorAnimations.cpp"
    #include "themes/HighBpmAnimations.cpp"
    #include "themes/PartyVibeAnimations.cpp"
    #include "themes/PsychedelicAnimations.cpp"
    #include "themes/IntenseAnimations.cpp"
    #include "themes/CrazyAnimations.cpp"
    Serial.print(F("Animations registered: "));
    Serial.println(globalAnimationRegistry.size());
}

void AnimationManager::nextPattern() {
    uint8_t newIndex = (currentPatternIndex + 1) % globalAnimationRegistry.size();
    Serial.print(F("nextPattern: from ")); Serial.print(currentPatternIndex);
    Serial.print(F(" to ")); Serial.println(newIndex);
    setCurrentPattern(newIndex);
}

const char* AnimationManager::getCurrentPatternName() {
    uint8_t index = currentPatternIndex == 0 ? currentShuffleIndex : currentPatternIndex;
    if (!globalAnimationRegistry.empty() && index < globalAnimationRegistry.size()) {
        return globalAnimationRegistry[index].name ? globalAnimationRegistry[index].name : "Missing";
    }
    return "Unknown";
}

uint8_t AnimationManager::getPatternCount() {
    return globalAnimationRegistry.size();
}

uint8_t AnimationManager::getCurrentPatternIndex() {
    return currentPatternIndex;
}


void AnimationManager::setCurrentPattern(uint8_t index) {
    if (globalAnimationRegistry.empty()) {
        Serial.println(F("ERROR: No animations registered"));
        currentAnimation = nullptr;
        currentPatternIndex = 0;
        return;
    }

    // Fix: ensure all clamp arguments are uint8_t
    index = std::clamp(index, static_cast<uint8_t>(0), static_cast<uint8_t>(globalAnimationRegistry.size() - 1));
    currentPatternIndex = index;


    createAnimation(index);


    systemManager.pushSavedNumber(Config::PREF_PATTERN_KEY, currentPatternIndex);
    Serial.print(F("Pattern set: ")); Serial.print(currentPatternIndex);
    Serial.print(F(" - ")); Serial.println(getCurrentPatternName());
}

void AnimationManager::setNumLeds(uint16_t count) {
    Serial.print(F("Updating LED count from "));
    Serial.print(numLeds);
    Serial.print(F(" to "));
    Serial.println(count);

    numLeds = std::clamp(count, (uint16_t)MIN_LEDS, (uint16_t)MAX_LEDS);
    cleanupCurrentAnimation();
    fill_solid(leds, MAX_LEDS, CRGB::Black);
    FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, numLeds);
    FastLED.setBrightness(brightness);
    if (currentPatternIndex < globalAnimationRegistry.size()) {
        createAnimation(currentPatternIndex);
    }
    systemManager.pushSavedNumber(Config::PREF_NUM_LEDS_KEY, numLeds);
    Serial.print(F("LED count set: ")); Serial.println(numLeds);
}

void AnimationManager::setBrightness(uint8_t value) {
    brightness = std::clamp(
        value,
        static_cast<uint8_t>(MIN_BRIGHTNESS),
        static_cast<uint8_t>(MAX_BRIGHTNESS)
    );  // Fixed: all clamp args are uint8_t
    FastLED.setBrightness(brightness);
    if (currentAnimation) {
        currentAnimation->setBrightness(brightness);
    }
    systemManager.pushSavedNumber(Config::PREF_BRIGHTNESS_KEY, brightness);
    Serial.print(F("Brightness set: ")); Serial.println(brightness);
}

void AnimationManager::createAnimation(uint8_t index) {
    Serial.println(F("DEBUG: Creating animation"));
    cleanupCurrentAnimation();
    if (index >= globalAnimationRegistry.size()) {
        Serial.print(F("ERROR: Invalid index: ")); Serial.println(index);
        index = 0;
    }
    if (numLeds < 1 || numLeds > MAX_LEDS) {
        Serial.print(F("ERROR: Invalid LED count: ")); Serial.println(numLeds);
        numLeds = DEFAULT_NUM_LEDS;
    }
    fill_solid(leds, MAX_LEDS, CRGB::Black);

    Serial.print(F("Creating: ")); Serial.println(globalAnimationRegistry[index].name);
        currentAnimation = globalAnimationRegistry[index].createFn(leds, numLeds);
        if (currentAnimation) {
        currentAnimation->setBrightness(brightness);
        Serial.print(F("Animation created: ")); Serial.println(globalAnimationRegistry[index].name);
        } else {
        Serial.println(F("ERROR: Animation creation failed"));
        }
}

void AnimationManager::cleanupCurrentAnimation() {
    if (currentAnimation) {
        delete currentAnimation;
        currentAnimation = nullptr;
        Serial.println(F("Cleaned up current animation"));
    }
}

void AnimationManager::startTransition(uint8_t newIndex) {
    if (inTransition) {
        Serial.println(F("Transition already in progress"));
        return;
    }
    if (currentAnimation) {
        currentAnimation->update();
        memcpy(oldLedsBuffer, leds, sizeof(CRGB) * numLeds);
    }
    memcpy(tempLeds, leds, sizeof(CRGB) * numLeds);
    createAnimation(newIndex);
    if (currentAnimation) {
        currentAnimation->update();
        memcpy(tempLeds, leds, sizeof(CRGB) * numLeds);
    }
    inTransition = true;
    transitionStart = millis();
    Serial.println(F("Transition started"));
}

void AnimationManager::pickNewShuffle() {
    if (globalAnimationRegistry.size() <= 1) {
        Serial.println(F("No animations to shuffle"));
        return;
    }
    currentShuffleIndex = random8(1, globalAnimationRegistry.size());
    startTransition(currentShuffleIndex);
    lastShuffleTime = millis();
    Serial.print(F("Shuffled to temporary index: "));
    Serial.println(currentShuffleIndex);
}