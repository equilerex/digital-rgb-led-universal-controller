/**
 * System Manager
 * 
 * Handles overall system initialization and configuration
 */

#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include <FastLED.h>
#include "../config/Config.h"
#include "../controls/InputManager.h"

// Forward declaration to resolve circular dependency
class AnimationManager;

class SystemManager {
public:
    // Constructor
    SystemManager();
    
    // Destructor
    ~SystemManager();
    
    // Initialize the system
    void begin();

    // Update all system components
    void update();
    
    // Get version info
    const char* getVersionInfo() const { return VERSION_INFO; }
    
    // Get input manager reference
    InputManager& getInputManager() { return inputManager; }
    
    // Get animation manager reference (will be null until initialized)
    AnimationManager* getAnimationManager() const { return animationManager; }
    
    // Set up the animation system
    void setupAnimationSystem();

    // Update inputs
    void updateInputs();

    // Update LEDs in a non-blocking way
    void updateLeds();
    
    // Preference management methods
    String getSavedString(const char* key, const char* defaultValue);
    uint8_t getSavedByte(const char* key, uint8_t defaultValue);
    uint16_t getSavedNumber(const char* key, uint16_t defaultValue);
    void pushSavedString(const char* key, const String& value);
    void pushSavedByte(const char* key, uint8_t value);
    void pushSavedNumber(const char* key, uint16_t value);

private:
    // Version info
    static constexpr const char* VERSION_INFO = "v1.0.0";
    
    // Preferences storage
    Preferences preferences;
    
    // Input manager
    InputManager inputManager;
    
    // Animation manager (pointer to allow delayed initialization)
    AnimationManager* animationManager;
    
    // LED timing variables
    unsigned long lastLedUpdate = 0;
    unsigned long lastLedShow = 0;
    
    // Input timing variables
    unsigned long lastInputDebug = 0;
    
    // Initialize preferences storage
    void initPreferences();
    
    // Initialize hardware components
    void initHardware();
};

#endif // SYSTEM_MANAGER_H
