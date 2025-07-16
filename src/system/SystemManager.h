/**
* System Manager
 * Handles overall system initialization and configuration
 */

#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include <FastLED.h>
#include "../config/Config.h"
#include "../controls/InputManager.h"

// Forward declaration
class AnimationManager;

class SystemManager {
public:
    SystemManager();
    ~SystemManager();

    void begin();
    void update();
    const char* getVersionInfo() const { return VERSION_INFO; }
    InputManager& getInputManager() { return inputManager; }
    AnimationManager* getAnimationManager() const { return animationManager; }

    void handleNextPattern();
    void setCurrentPattern(uint16_t value);
    void setBrightness(uint16_t value);
    void setNumLeds(uint16_t count);
    uint8_t getBrightness() const;
    uint16_t getNumLeds() const;

    String getSavedString(const char* key, const char* defaultValue);
    uint8_t getSavedByte(const char* key, uint8_t defaultValue);
    uint16_t getSavedNumber(const char* key, uint16_t defaultValue);
    void pushSavedString(const char* key, const String& value);
    void pushSavedByte(const char* key, uint8_t value);
    void pushSavedNumber(const char* key, uint16_t value);

private:
    static constexpr const char* VERSION_INFO = "v1.0.0";
    Preferences preferences;
    InputManager inputManager;
    AnimationManager* animationManager;
    CRGB* leds;
    unsigned long lastLedUpdate = 0;
    unsigned long lastLedShow = 0;
    unsigned long lastInputDebug = 0;

    void initPreferences();
    void initHardware();
    void updateLeds();
};

#endif // SYSTEM_MANAGER_H