/**
 * Animation Manager
 * 
 * Manages all animations, their categories, and FastLED hardware
 */

#ifndef ANIMATION_MANAGER_H
#define ANIMATION_MANAGER_H

#include <vector>
#include <map>
#include <functional>
#include <FastLED.h>
#include "AnimationBase.h"
#include "../config/Config.h"
// Forward declaration to avoid circular include
class SystemManager;

// Define a factory function type that creates animations
typedef Animation* (*CreateAnimationFn)(CRGB*, uint16_t);

// Structure to hold animation metadata
// Note: This only stores information about animations, not the actual instances
// Instances are created on-demand when an animation is selected
struct AnimationInfo {
    const char* name;
    AnimationCategory category;
    CreateAnimationFn createFn;
};

class AnimationManager {
public:
    // Initialize the animation manager
    AnimationManager(SystemManager& systemManager);
    
    // Clean up animations
    ~AnimationManager();
    
    // Initialize FastLED hardware and animations
    void begin();
    
    // Update the current animation
    void update();
    
    // This method is deprecated - FastLED.show() should only be called in main.cpp
    void show() { 
        // Intentionally empty - FastLED.show() is now called only in main.cpp
    }
    
    // Get LED array
    CRGB* getLEDs() { return leds; }
    
    // Get number of LEDs
    uint16_t getNumLeds() const { return numLeds; }

    // Get current brightness
    uint8_t getBrightness() const { return brightness; }
    
    // Switch to the next animation
    void nextPattern();
    
    // Get the current animation name
    const char* getCurrentPatternName();
    
    // Get the total number of animations
    uint8_t getPatternCount();
    
    // Get the current pattern index
    uint8_t getCurrentPatternIndex();
    
    // Set the current pattern by index
    void setCurrentPattern(uint8_t index);
    

    
    // Update the LED count (used when numLeds changes)
    void updateLedCount(uint16_t count);
    
    // Alias for updateLedCount to maintain compatibility
    void updateNumLeds(uint16_t count) { updateLedCount(count); }
    

    // Set brightness for all animations and save to preferences
    void setBrightness(uint8_t value);
    
    // Initialize all animations - should be called after constructor
    void initializeAnimations() {
        // Log initialization
        Serial.println(F("Initializing animation registry:"));
        
        // Check if registry is already populated
        if (animationRegistry.empty()) {
            // Register animations if not already done in begin()
            registerAnimations();
        }
        
        // List all registered animations
        for (uint8_t i = 0; i < animationRegistry.size(); i++) {
            Serial.print(F("  - "));
            Serial.println(animationRegistry[i].name);
        }
        
        Serial.print(F("Total animations in registry: "));
        Serial.println(animationRegistry.size());
        
        // No need to manually set the pattern - begin() already did this with proper validation
    }
    
    // Get animations by category
    std::vector<AnimationInfo> getAnimationsByCategory(AnimationCategory category);
    
    // Get current animation category name
    const char* getCurrentCategoryName();
    
    // Get category name by index
    const char* getCategoryName(AnimationCategory category);

private:
    SystemManager& systemManager;      // Reference to system manager
    CRGB leds[MAX_LEDS];              // Statically allocated LED array
    uint16_t numLeds;                 // Number of LEDs
    uint8_t brightness;               // Brightness value
    uint8_t currentPatternIndex;      // Index of the current pattern
    std::vector<AnimationInfo> animationRegistry; // Animation metadata
    Animation* currentAnimation;      // Currently active animation instance
    
    // Initialize the LED strip
    void initLEDs();
    
    // Register all animations
    void registerAnimations();
    
    // Create an animation instance
    void createAnimation(uint8_t index);
    
    // Clean up current animation
    void cleanupCurrentAnimation();
    
    // Map of category names
    const char* categoryNames[NUM_CATEGORIES] = {
        "Slow & Soothing",
        "Solid Colors",
        "High BPM",
        "Party Vibe",
        "Psychedelic",
        "Intense",
        "Crazy",
        "Trial Runs"
    };
};

#endif // ANIMATION_MANAGER_H
