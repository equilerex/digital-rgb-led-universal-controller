/**
 * Input Manager Implementation
 */

#include "InputManager.h"
#include "../system/SystemManager.h"
#include <FastLED.h>
#include "../config/Config.h"

// Initialize static instance pointer
InputManager* InputManager::instance = nullptr;

// Static callback handlers
void InputManager::onClickHandler() {
    Serial.println(F("onClickHandler called"));
    if (instance) {
        Serial.println(F("instance exists, calling handleClick"));
        instance->handleClick();
    } else {
        Serial.println(F("ERROR: instance is null in onClickHandler"));
    }
}

void InputManager::onLongPressStartHandler() {
    if (instance) instance->handleLongPressStart();
}

void InputManager::onLongPressStopHandler() {
    if (instance) instance->handleLongPressStop();
}

InputManager::InputManager() :
    buttonPin(BUTTON_PIN),
    button(buttonPin, true),  // true = active LOW
    systemManager(nullptr),
    brightnessMode(false),
    ledCountUpMode(false),
    ledCountDownMode(false),
    longPressMode(false),
    longPressStartTime(0)
{
    // Set this instance as the singleton
    instance = this;
}

void InputManager::begin(SystemManager* sysManager) {
    Serial.println(F("InputManager Starting..."));
    // Store reference to system manager
    systemManager = sysManager;
    
    // Ensure instance pointer is set (safety check)
    instance = this;
    
    // Setup button with callbacks
    button.attachClick(onClickHandler);
    button.attachLongPressStart(onLongPressStartHandler);
    button.attachLongPressStop(onLongPressStopHandler);
    button.setPressMs(1000);  // Long press detected after 1 second (recommended API)

    Serial.println(F("InputManager ready..."));
}

void InputManager::update() {
    // Update button state
    button.tick();
    
    // Handle brightness cycling when in long press mode
    if (longPressMode) {
        cycleLongPress();
    } 
}

void InputManager::handleClick() { 
    // First check if system manager is valid
    if (systemManager == nullptr) {
        Serial.println(F("ERROR: systemManager is null in handleClick"));
        return;
    }

    // Call the system manager to handle the pattern change
    systemManager->handleNextPattern();
}

bool InputManager::isBrightnessMode() {
    return brightnessMode;
}
bool InputManager::isLedCountMode() {
    return ledCountUpMode || ledCountDownMode;
}

void InputManager::handleLongPressStart() {
    brightnessMode = true;
    longPressMode = true;
    longPressStartTime = millis();

    Serial.println(F("Button long press started: Entering brightness mode"));
}

void InputManager::handleLongPressStop() {
    // if released before proceeding to led mode, we wanted to change brightness
    if (brightnessMode) {
        systemManager->setBrightness(systemManager->getBrightness() + ADJUST_BRIGHTNESS_INCREMENT);
    } else if (ledCountUpMode) {
        systemManager->setNumLeds(systemManager->getNumLeds() + ADJUST_NUM_LEDS_INCREMENT);
    } else if (ledCountDownMode) {
        systemManager->setNumLeds(systemManager->getNumLeds() - ADJUST_NUM_LEDS_INCREMENT);
    } else {
        systemManager->setCurrentPattern(systemManager->getNumLeds() + ADJUST_NUM_LEDS_INCREMENT); // Increase
    }
    brightnessMode = false;
    longPressMode = false;
    ledCountDownMode = false;
    ledCountUpMode = false;
    longPressStartTime = 0;
}

void InputManager::cycleLongPress() {
    // Failsafe check for system manager
    if (systemManager == nullptr) {
        Serial.println(F("ERROR: systemManager is null in cycleLongPress"));
        longPressMode = false; // Exit long press mode to prevent further issues
        return;
    }
    
    unsigned long elapsed = millis() - longPressStartTime; 

    // Mode selection based on how long the button has been held
     if (elapsed >= SHUFFLE_BUTTON_HOLDTIME) {
        ledCountDownMode = false;
        ledCountUpMode = false;

    } else if (elapsed >= LED_COUNT_UP_HOLDTIME) {
        ledCountDownMode = false;
        ledCountUpMode = true;


    } 
    else if (elapsed >= LED_COUNT_DOWN_HOLDTIME) {
        // Turn off other modes
        brightnessMode = false;
        ledCountDownMode = true;


    }
}
