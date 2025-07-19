#include "AnimationManager.h"
#include "AnimationBase.h"
#include "../system/SystemManager.h"
#include <Preferences.h>
#include <Arduino.h>
#include <FastLED.h>
#include <vector>
#include <algorithm>
#include "../config/Config.h"
#include <esp_heap_caps.h>
#include <esp_system.h>
#include <esp_task_wdt.h>
#include "../config/PinConfig.h"

// Global definition
std::vector<AnimationInfo> globalAnimationRegistry;

AnimationManager::AnimationManager(SystemManager& systemManager, CRGB* leds) : systemManager(systemManager), leds(leds), numLeds(DEFAULT_NUM_LEDS),
      brightness(DEFAULT_BRIGHTNESS), currentPatternIndex(0), currentAnimation(nullptr),
      isInitialized(false), currentShuffleIndex(0), lastShuffleTime(0), inShuffleTransition(false), shuffleTransitionStart(0), shuffleTransitionNewIndex(0), currentShuffleDuration(SHUFFLE_DURATION) {

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

    Preferences prefs;
    prefs.begin(Config::PREF_NAMESPACE, false);
    uint16_t restoredNumLeds = prefs.getUShort(Config::PREF_NUM_LEDS_KEY, DEFAULT_NUM_LEDS);
    uint8_t restoredBrightness = prefs.getUChar(Config::PREF_BRIGHTNESS_KEY, DEFAULT_BRIGHTNESS);
    numLeds = std::clamp(restoredNumLeds, (uint16_t)MIN_LEDS, (uint16_t)MAX_LEDS);
    brightness = std::clamp(restoredBrightness, (uint8_t)MIN_BRIGHTNESS, (uint8_t)MAX_BRIGHTNESS);

    Serial.print(F("Boot with numLeds: ")); Serial.println(numLeds);
    Serial.print(F("Boot with brightness: ")); Serial.println(brightness);

    // Single FastLED controller init
    Serial.println(F("=== LED Initialization ==="));
    memset(leds, 0, sizeof(CRGB) * MAX_LEDS);
    Serial.print(F("Initializing controller for max ")); Serial.print(MAX_LEDS); Serial.println(F(" LEDs."));
    FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, MAX_LEDS);
    FastLED.setBrightness(brightness);
    #if defined(MAX_MILLIAMPS)
        Serial.print(F("Setting power limit: ")); Serial.print(MAX_MILLIAMPS); Serial.println(F(" mA"));
        FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_MILLIAMPS);
    #endif
    Serial.println(F("=== LED initialization complete! ==="));

    registerAnimations();
    Serial.print(F("Animations registered: "));
    Serial.println(globalAnimationRegistry.size());

    if (globalAnimationRegistry.empty()) {
        Serial.println(F("ERROR: No animations registered! Check theme includes."));
        prefs.end();
        return;
    }

    // Load saved pattern with validation
    uint8_t savedPatternIndex = prefs.getUChar(Config::PREF_PATTERN_KEY, 0);
    if (!prefs.isKey(Config::PREF_PATTERN_KEY)) {
        Serial.println(F("[WARNING] No saved pattern found; using default index 0"));
    } else if (savedPatternIndex >= globalAnimationRegistry.size()) {
        Serial.println(F("[WARNING] Saved pattern index invalid; resetting to 0"));
        savedPatternIndex = 0;
        prefs.putUChar(Config::PREF_PATTERN_KEY, savedPatternIndex);
    } else {
        Serial.print(F("[DEBUG] Loaded saved pattern index: ")); Serial.println(savedPatternIndex);
    }
    prefs.end();

    setCurrentPattern(savedPatternIndex);

     if (currentAnimation) {
        currentAnimation->setBrightness(brightness);
        currentAnimation->update();
    }

    isInitialized = true;
    Serial.println(F("Animation system ready"));
}

void AnimationManager::update() {
    bool skipAnimationUpdate = false;

    if (!isInitialized) {
        EVERY_N_SECONDS(5) { Serial.println(F("Animation not initialized")); }
        skipAnimationUpdate = true;
    }
    if (!leds) {
        EVERY_N_SECONDS(5) { Serial.println(F("ERROR: Null LED array")); }
        skipAnimationUpdate = true;
    }
    if (globalAnimationRegistry.empty()) {
        EVERY_N_SECONDS(5) { Serial.println(F("ERROR: No animations registered")); }
        skipAnimationUpdate = true;
    }
    if (!currentAnimation) {
        EVERY_N_SECONDS(5) { Serial.println(F("ERROR: No active animation")); }
        skipAnimationUpdate = true;
    }

    // Shuffle mode logic
    if (inShuffleMode()) {
        if (!lastShuffleTime || millis() - lastShuffleTime > currentShuffleDuration) {
            pickNewShuffle();
        }
        if (inShuffleTransition) {
            uint32_t elapsed = millis() - shuffleTransitionStart;
            if (elapsed >= shuffleTransitionDuration) {
                inShuffleTransition = false;
            } else {
                float progress = (float)elapsed / shuffleTransitionDuration;
                for (uint16_t i = 0; i < numLeds; i++) {
                    leds[i] = blend(oldLedsBuffer[i], tempLeds[i], progress * 255);
                }
                EVERY_N_SECONDS(5) {
                    Serial.print(F("[DEBUG] shuffleTransition progress: "));
                    Serial.print(progress * 100);
                    Serial.println(F("%"));
                    Serial.print(F("[DEBUG] Sample LED[0] in shuffleTransition: R:"));
                    Serial.print(leds[0].r);
                    Serial.print(F(" G:"));
                    Serial.print(leds[0].g);
                    Serial.print(F(" B:"));
                    Serial.println(leds[0].b);
                }
                skipAnimationUpdate = true;
            }
        }
    }

    // Normal animation update (only if not skipping)
    if (!skipAnimationUpdate) {
        try {
            currentAnimation->update();
            EVERY_N_SECONDS(10) {
                Serial.print(F("[DEBUG] Post-update sample LED[0] for "));
                Serial.print(currentAnimation->getName());
                Serial.print(F(": R:"));
                Serial.print(leds[0].r);
                Serial.print(F(" G:"));
                Serial.print(leds[0].g);
                Serial.print(F(" B:"));
                Serial.println(leds[0].b);
            }
        } catch (...) {
            Serial.print(F("[CRITICAL] Crash in animation update() for "));
            Serial.println(currentAnimation->getName());
            cleanupCurrentAnimation();
            createAnimation(0);
        }
    }

    logFastLEDDiagnostics();

    #if defined(WATCHDOG_C3_WORKAROUND)
    esp_task_wdt_reset();
    #endif
}

void AnimationManager::logFastLEDDiagnostics() {
    EVERY_N_MILLISECONDS(1000) {
        if (leds == nullptr) {
            Serial.println(F("[CRITICAL] FastLED buffer is null! Check memory allocation."));
            return;
        }

        uintptr_t addr = reinterpret_cast<uintptr_t>(leds);
        bool validAddress = (addr >= 0x3FC80000 && addr <= 0x3FCE0000);
        if (!validAddress) {
            Serial.print(F("[CAUTION] FastLED buffer address suspicious: 0x"));
            Serial.println(addr, HEX);
        }

        bool bufferOverrun = false;
        for (uint16_t i = numLeds; i < MAX_LEDS; i++) {
            if (leds[i] != CRGB::Black) {
                bufferOverrun = true;
                Serial.print(F("[CRITICAL] Buffer overrun at index "));
                Serial.println(i);
                break;
            }
        } 

        size_t heapFree = ESP.getFreeHeap();
        if (heapFree < 10000) {
            Serial.println(F("[CRITICAL] Heap memory low! Risk of crashes."));
        }
    }
}

void AnimationManager::registerAnimations() {
    Serial.println(F("DEBUG: Starting registerAnimations"));
    globalAnimationRegistry.clear();


    // --- Theme includes ---
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

const char* AnimationManager::getCurrentAnimationName() {
    if (currentAnimation) {
        return currentAnimation->getName();
    }
    return "N/A";
}

uint8_t AnimationManager::getPatternCount() {
    return globalAnimationRegistry.size();
}

uint8_t AnimationManager::getCurrentPatternIndex() {
    return currentPatternIndex;
}

void AnimationManager::setCurrentPattern(uint8_t index) {
    inShuffleTransition = false;
    if (globalAnimationRegistry.empty()) {
        Serial.println(F("ERROR: No animations registered"));
        currentAnimation = nullptr;
        currentPatternIndex = 0;
        Preferences prefs;
        prefs.begin(Config::PREF_NAMESPACE, false);
        if (prefs.putUChar(Config::PREF_PATTERN_KEY, 0) == 0) {
            Serial.println(F("[ERROR] Failed to save pattern index 0"));
        }
        prefs.end();
        return;
    }

    currentPatternIndex = std::clamp(index, (uint8_t)0, (uint8_t)(globalAnimationRegistry.size() - 1));

    createAnimation(inShuffleMode() ? currentShuffleIndex : currentPatternIndex);

    Preferences prefs;
    prefs.begin(Config::PREF_NAMESPACE, false);
    if (prefs.putUChar(Config::PREF_PATTERN_KEY, currentPatternIndex) == 0) {
        Serial.println(F("[ERROR] Failed to save pattern index"));
    } else {
        Serial.print(F("[DEBUG] Saved pattern index: ")); Serial.println(currentPatternIndex);
    }
    prefs.end();

    Serial.print(F("Pattern set: ")); Serial.print(currentPatternIndex);
    Serial.print(F(" - ")); Serial.println(getCurrentAnimationName());
}

void AnimationManager::setNumLeds(uint16_t count) {
    Serial.print(F("Updating LED count from "));
    Serial.print(numLeds);
    Serial.print(F(" to "));
    Serial.println(count);

    numLeds = std::clamp(count, (uint16_t)MIN_LEDS, (uint16_t)MAX_LEDS);
    cleanupCurrentAnimation();
    fill_solid(leds, MAX_LEDS, CRGB::Black);
    FastLED.setBrightness(brightness);
    if (currentPatternIndex < globalAnimationRegistry.size()) {
        createAnimation(inShuffleMode() ? currentShuffleIndex : currentPatternIndex);
    }
    Preferences prefs;
    prefs.begin(Config::PREF_NAMESPACE, false);
    if (prefs.putUShort(Config::PREF_NUM_LEDS_KEY, numLeds) == 0) {
        Serial.println(F("[ERROR] Failed to save numLeds"));
    }
    prefs.end();
    Serial.print(F("LED count set: ")); Serial.println(numLeds);
}

void AnimationManager::setBrightness(uint8_t value) {
    brightness = std::clamp(
        value,
        static_cast<uint8_t>(MIN_BRIGHTNESS),
        static_cast<uint8_t>(MAX_BRIGHTNESS)
    );
    FastLED.setBrightness(brightness);
    if (currentAnimation) {
        currentAnimation->setBrightness(brightness);
    }
    Preferences prefs;
    prefs.begin(Config::PREF_NAMESPACE, false);
    if (prefs.putUChar(Config::PREF_BRIGHTNESS_KEY, brightness) == 0) {
        Serial.println(F("[ERROR] Failed to save brightness"));
    }
    prefs.end();
}

void AnimationManager::createAnimation(uint8_t index) {
    Serial.print(F("DEBUG: Creating animation ")); Serial.println(index);
    cleanupCurrentAnimation();
    if (index >= globalAnimationRegistry.size()) {
        Serial.print(F("ERROR: Index too large: ")); Serial.print(index); Serial.print(F(" / ")); Serial.println(globalAnimationRegistry.size());
        index = 0;
    }
    if (numLeds < 1 || numLeds > MAX_LEDS) {
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

void AnimationManager::startShuffleTransition(uint8_t newIndex) {
    if (inShuffleTransition) {
        Serial.println(F("ShuffleTransition already in progress"));
        return;
    }
    if (currentAnimation) {
        currentAnimation->update();
        memcpy(oldLedsBuffer, leds, sizeof(CRGB) * numLeds);
    }
    // Prepare tempLeds for transition
    createAnimation(newIndex);
    if (currentAnimation) {
        currentAnimation->update();
        memcpy(tempLeds, leds, sizeof(CRGB) * numLeds);
    }
    inShuffleTransition = true;
    shuffleTransitionStart = millis();
    shuffleTransitionNewIndex = newIndex;
    Serial.println(F("ShuffleTransition started"));
}

void AnimationManager::pickNewShuffle() {
    lastShuffleTime = millis();
    switch (currentPatternIndex) {
    case 0:
        currentShuffleDuration = random(5000, 30000); // random 5-30s
        break;
    case 1:
        currentShuffleDuration = 5000; // 5s
        break;
    case 2:
        currentShuffleDuration = 10000; // 10s
        break;
    case 3:
        currentShuffleDuration = 300000; // 5min
        break;
    default:
        currentShuffleDuration = SHUFFLE_DURATION;
}
    // Build a list of valid shuffle indices (exclude solid colors)
    std::vector<uint8_t> validIndices;
    for (uint8_t i = 0; i < globalAnimationRegistry.size(); ++i) {
        const char* name = globalAnimationRegistry[i].name;
        if (strstr(name, "-NoShuffle") == nullptr) {
            validIndices.push_back(i);
        }
    }
    if (validIndices.size() <= 1) {
        Serial.println(F("No valid animations to shuffle"));
        return;
    }
    // Pick a new shuffle index different from current
    uint8_t newIndex;
    do {
        newIndex = validIndices[random8(validIndices.size())];
    } while (newIndex == currentShuffleIndex);
    currentShuffleIndex = newIndex;
    startShuffleTransition(currentShuffleIndex);
    Serial.print(F("Shuffled to index: "));
    Serial.print(currentShuffleIndex);
    Serial.print(F(" - Name: "));
    Serial.println(globalAnimationRegistry[currentShuffleIndex].name);
}


