/**
 * Animation Manager Implementation
 */

#include "AnimationManager.h"
#include "../system/SystemManager.h"
#include <Preferences.h>
#include "SlowAndSoothingAnimations.h"
#include "SolidColorAnimations.h"
#include "HighBpmAnimations.h"
#include "PartyVibeAnimations.h"
#include "PsychedelicAnimations.h"
#include "IntenseAnimations.h"
#include "CrazyAnimations.h"
#include "TrialRunAnimations.h"
#include "../config/Config.h"

AnimationManager::AnimationManager(SystemManager& systemManager) :
    systemManager(systemManager),
    numLeds(DEFAULT_NUM_LEDS),
    brightness(DEFAULT_BRIGHTNESS),
    currentPatternIndex(0),
    currentAnimation(nullptr) {
    // Initialize the LED array to all black
    memset(leds, 0, sizeof(CRGB) * MAX_LEDS);
}

void AnimationManager::begin() {
    Serial.println(F("AnimationManager Starting..."));

    // Set conservative defaults first

    // Get saved number of LEDs and brightness from systemManager's preferences
    Serial.println(F("Reading preferences from SystemManager..."));
    numLeds = systemManager.getSavedNumber(PREF_NUM_LEDS_KEY, 50); // Default to lower count
    brightness = systemManager.getSavedNumber(PREF_BRIGHTNESS_KEY, 64); // Fixed: Using getSavedNumber for brightness
    currentPatternIndex = systemManager.getSavedNumber(PREF_PATTERN_KEY, 0);


    // Ensure settings are within valid ranges
    if (numLeds < 16) {
        numLeds = 16;
        Serial.println(F("Adjusted LED count to minimum: 16"));
    }
    if (numLeds > MAX_LEDS) {
        numLeds = MAX_LEDS;
        Serial.println(F("Adjusted LED count to maximum: "));
        Serial.println(MAX_LEDS);
    }
    if (brightness < 16) { // Start with lower minimum
        brightness = 16;
        Serial.println(F("Adjusted brightness to minimum: 16"));
    }
    if (brightness > 128) { // Cap at 50% initially
        brightness = 128;
        Serial.println(F("Adjusted brightness to safer maximum: 128"));
    }

    Serial.print(F("Using "));
    Serial.print(numLeds);
    Serial.println(F(" LEDs"));

    Serial.print(F("Using brightness: "));
    Serial.println(brightness);

    delay(1);

    // Initialize LEDs with the max LED count
    Serial.println(F("Calling initLEDs..."));
    initLEDs();
    Serial.println(F("initLEDs complete"));


    // Register all animations (just adds them to the registry, doesn't create instances)
    Serial.println(F("Registering animations..."));
    registerAnimations();


    // Initialize animations with current brightness
    setBrightness(brightness);

    // Ensure we have animations registered
    if (animationRegistry.empty()) {
        Serial.println(F("ERROR: No animations registered!"));
        return;
    }

    // Load saved pattern and apply it
    Serial.print(F("Restoring saved pattern index: "));
    Serial.println(currentPatternIndex);

    // Use setCurrentPattern which will validate the index and create the animation
    setCurrentPattern(currentPatternIndex);

    // Final check to ensure we have a valid animation
    if (!currentAnimation) {
        Serial.println(F("WARNING: Failed to create initial animation!"));
    }

    Serial.println(F("AnimationManager ready..."));
}

AnimationManager::~AnimationManager() {
    // Clean up current animation
    cleanupCurrentAnimation();
    
    // The LED array is static, so no need to delete it
    // FastLED will continue to reference our static array
}

void AnimationManager::update() {
    // Check if we have a current animation
    if (!currentAnimation) {
        // If we don't have an animation, try to create one
        if (animationRegistry.size() > 0) {
            Serial.println(F("No active animation, creating one..."));
            createAnimation(currentPatternIndex);
        } else {
            // If there are no animations registered, just exit
            return;
        }
    }

    // Update the current animation at a controlled framerate
    // Different patterns have different optimal update rates
    EVERY_N_MILLISECONDS(50) {
        if (currentAnimation) {
            // Update with error handling to prevent crashes
            try {
                currentAnimation->update();
            } catch (...) {
                // This would only work with try/catch support enabled,
                // but the idea is to gracefully handle any animation errors
                Serial.println(F("ERROR: Exception in animation update"));
            }
            // Let main loop handle showing LEDs - don't call FastLED.show() here

            // Uncomment this line for debugging where it might be hanging
            // Serial.println(F("Animation update complete"));

        }
    }

    // Slowly cycle the "base color" through the rainbow
    // This can be used by animations that need a global hue value
    EVERY_N_MILLISECONDS(20) {
        // Note: Individual animations should manage their own hue if needed
        // This is here for compatibility with FastLED examples
    }
}

void AnimationManager::nextPattern() {
    // Move to the next pattern
    uint8_t newIndex = (currentPatternIndex + 1) % animationRegistry.size();
    Serial.print(F("nextPattern called, changing from "));
    Serial.print(currentPatternIndex);
    Serial.print(F(" to "));
    Serial.println(newIndex);
    setCurrentPattern(newIndex);
}

const char* AnimationManager::getCurrentPatternName() {
    // Get the name of the current pattern
    if (!animationRegistry.empty() && currentPatternIndex < animationRegistry.size()) {
        return animationRegistry[currentPatternIndex].name;
    }
    return "Unknown";
}

uint8_t AnimationManager::getPatternCount() {
    return animationRegistry.size();
}

uint8_t AnimationManager::getCurrentPatternIndex() {
    return currentPatternIndex;
}

void AnimationManager::setCurrentPattern(uint8_t index) {
    // Ensure animations are registered
    if (animationRegistry.empty()) {
        Serial.println(F("WARNING: Cannot set pattern - no animations registered"));
        return;
    }

    // Validate the index is within bounds
    if (index >= animationRegistry.size()) {
        Serial.print(F("Invalid pattern index: "));
        Serial.print(index);
        Serial.print(F(" (max: "));
        Serial.print(animationRegistry.size() - 1);
        Serial.println(F(")"));

        // Fallback to a valid index if current one is invalid
        if (currentPatternIndex < animationRegistry.size()) {
            // Keep current if it's valid
            index = currentPatternIndex;
        } else {
            // Otherwise default to first pattern
            index = 0;
        }
    }

    // Update the current pattern index
    currentPatternIndex = index;

    // Create the animation
    createAnimation(index);

    // Save current pattern to preferences
    systemManager.pushSavedNumber(PREF_PATTERN_KEY, currentPatternIndex);

    Serial.print(F("Pattern set and saved: "));
    Serial.print(currentPatternIndex);
    Serial.print(F(" - "));
    Serial.println(getCurrentPatternName());
}

void AnimationManager::setBrightness(uint8_t value) {
    // Store the new brightness value
    brightness = value;

    // Check for potentially invalid values
    if (brightness == 0 || brightness > 255) {
        Serial.print(F("WARNING: Invalid brightness value: "));
        Serial.print(brightness);
        brightness = MIN_BRIGHTNESS; // Safe default
    }

    // Check for potentially invalid values
    if (brightness == 0 || brightness > 255) {
        Serial.print(F("WARNING: Invalid brightness value: "));
        Serial.print(brightness);
        brightness = MIN_BRIGHTNESS; // Safe default
    }

    // Check for potentially invalid values
    if (brightness == 0 || brightness > 255) {
        Serial.print(F("WARNING: Invalid brightness value: "));
        Serial.print(brightness);
        brightness = MIN_BRIGHTNESS; // Safe default
    }

    // Ensure it's within valid range
    if (brightness < MIN_BRIGHTNESS) brightness = MIN_BRIGHTNESS;
    if (brightness > MAX_BRIGHTNESS) brightness = MAX_BRIGHTNESS;

    // Apply to FastLED
    FastLED.setBrightness(brightness);

    // Apply to current animation if it exists
    if (currentAnimation) {
        currentAnimation->setBrightness(brightness);
    }

    // Save to preferences
    systemManager.pushSavedNumber(PREF_BRIGHTNESS_KEY, brightness);

    Serial.print(F("Brightness set and saved: "));
    Serial.println(brightness);
}

void AnimationManager::updateLedCount(uint16_t count) {
    // Update the LED count - but don't reinitialize FastLED
    if (count == numLeds) {
        Serial.println(F("LED count unchanged, no update needed"));
        return; // No change needed
    }
    
    Serial.print(F("Updating LED count from "));
    Serial.print(numLeds);
    Serial.print(F(" to "));
    Serial.println(count);
    
    // Ensure count is within valid range before storing
    uint16_t safeCount = count;
    if (safeCount < 50) {
        safeCount = 50;
        Serial.println(F("Adjusted LED count to minimum: 50"));
    }
    if (safeCount > MAX_LEDS) {
        safeCount = MAX_LEDS;
        Serial.println(F("Adjusted LED count to maximum: "));
        Serial.println(MAX_LEDS);
    }
    
    // Store new LED count
    numLeds = safeCount;
    
    // Free existing animation
    cleanupCurrentAnimation();

    // Clear any LEDs that are now outside the range
    for (uint16_t i = numLeds; i < MAX_LEDS; i++) {
        leds[i] = CRGB::Black;
    }

    // Recreate the current animation with the new LED count
    if (currentPatternIndex < animationRegistry.size()) {
        Serial.println(F("Recreating animation with new LED count"));
        createAnimation(currentPatternIndex);
    }
    
    // Save to preferences
    Serial.println(F("Saving new LED count to preferences"));
    systemManager.pushSavedNumber(PREF_NUM_LEDS_KEY, numLeds);

    Serial.print(F("LED count set and saved: "));
    Serial.println(numLeds);
}

// Factory functions for creating animations
// Using a macro to reduce repetition and make the pattern obvious
#define ANIMATION_FACTORY(AnimationType) \
    static Animation* create##AnimationType(CRGB* leds, uint16_t numLeds) { \
        return new AnimationType(leds, numLeds); \
    }

// Slow and Soothing animations
ANIMATION_FACTORY(BreathingAnimation)
ANIMATION_FACTORY(AuroraAnimation)
ANIMATION_FACTORY(ColorMeditationAnimation)
ANIMATION_FACTORY(OceanWavesAnimation)
ANIMATION_FACTORY(MoonlightAnimation)
ANIMATION_FACTORY(ForestCanopyAnimation) 

std::vector<AnimationInfo> AnimationManager::getAnimationsByCategory(AnimationCategory category) {
    std::vector<AnimationInfo> result;

    for (const AnimationInfo& info : animationRegistry) {
        if (info.category == category) {
            result.push_back(info);
        }
    }

    return result;
}

const char* AnimationManager::getCurrentCategoryName() {
    if (currentPatternIndex < animationRegistry.size()) {
        AnimationCategory category = animationRegistry[currentPatternIndex].category;
        return getCategoryName(category);
    }
    return "Unknown";
}

const char* AnimationManager::getCategoryName(AnimationCategory category) {
    if (category < NUM_CATEGORIES) {
        return categoryNames[category];
    }
    return "Unknown";
}

void AnimationManager::registerAnimations() {
    // Define a static array of all animations in a single, concise table
    // IMPORTANT: This only registers metadata about animations (name, category, creation function)
    // It does NOT create any animation instances - they are created on demand when selected
    static const AnimationInfo ANIMATIONS[] = {
        // CATEGORY             NAME                    CREATOR FUNCTION
        // --------             ----                    ---------------
        // Slow and Soothing
        { "Breathing",          SLOW_AND_SOOTHING,     createBreathingAnimation },
        { "Aurora",             SLOW_AND_SOOTHING,     createAuroraAnimation },
        { "Color Meditation",   SLOW_AND_SOOTHING,     createColorMeditationAnimation },
        { "Ocean Waves",        SLOW_AND_SOOTHING,     createOceanWavesAnimation },
        { "Moonlight",          SLOW_AND_SOOTHING,     createMoonlightAnimation },
        { "Forest Canopy",      SLOW_AND_SOOTHING,     createForestCanopyAnimation },
 
    };

    // Register all animations from the table
    const size_t COUNT = sizeof(ANIMATIONS) / sizeof(ANIMATIONS[0]);
    for (size_t i = 0; i < COUNT; i++) {
        animationRegistry.push_back(ANIMATIONS[i]);
    }
}

void AnimationManager::initLEDs() {
    Serial.println(F("=== Simplified LED Initialization ==="));

    // 1. Clear the LED array (already allocated statically)
    memset(leds, 0, sizeof(CRGB) * MAX_LEDS);
    Serial.print(F("Using static buffer for "));
    Serial.print(MAX_LEDS);
    Serial.println(F(" LEDs."));
   
    // 2. Configure FastLED controller once
    // This is the most critical part. We add the LEDs to the controller.
    // The template parameters are compile-time, but the 'leds' buffer and 'MAX_LEDS' are used here.
    FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, MAX_LEDS);
    Serial.println(F("FastLED controller configured.")); 
    
    // 3. Set power management
    #if defined(MAX_MILLIAMPS)
        Serial.print(F("Setting power limit: "));
        Serial.print(MAX_MILLIAMPS);
        Serial.println(F(" mA")); 
        FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_MILLIAMPS);
    #endif

    // 4. Explicitly clear the LEDs and show to ensure proper initialization
    FastLED.clear();
    FastLED.show();
    delay(10); // Small delay to ensure the clear takes effect
    
    Serial.println(F("=== LED initialization complete! ==="));
}


void AnimationManager::cleanupCurrentAnimation() {
    if (currentAnimation) {
        delete currentAnimation;
        currentAnimation = nullptr;
    }
}

void AnimationManager::createAnimation(uint8_t index) {
    // First, clean up any existing animation
    cleanupCurrentAnimation();

    // Validate the index is within range
    if (index >= animationRegistry.size()) {
        Serial.print(F("Invalid animation index: "));
        Serial.println(index);
        index = 0; // Fallback to the first animation if index is invalid
    }

    // Try to create the animation
    currentAnimation = animationRegistry[index].createFn(leds, numLeds);

    // Verify animation was created successfully
    if (!currentAnimation) {
        Serial.print(F("Failed to create animation: "));
        Serial.println(animationRegistry[index].name);

        // Try to create a simple fallback animation (first in registry)
        if (index != 0 && animationRegistry.size() > 0) {
            Serial.println(F("Falling back to first animation"));
            currentAnimation = animationRegistry[0].createFn(leds, numLeds);
            currentPatternIndex = 0; // Update the pattern index to match
        }
    }

    // Set the brightness for the new animation if it exists
    if (currentAnimation) {
        currentAnimation->setBrightness(brightness);

        // Save the current pattern to preferences
        systemManager.pushSavedNumber(PREF_PATTERN_KEY, currentPatternIndex);

        Serial.print(F("Animation created: "));
        Serial.println(animationRegistry[currentPatternIndex].name);
    } else {
        Serial.println(F("ERROR: Could not create any animation!"));
    }
}
