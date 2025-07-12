/**
 * Input Manager Implementation
 */

#include "InputManager.h"
#include "../system/SystemManager.h"
#include "../animations/AnimationManager.h"
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
    button.setPressMs(1000);  // Long press detected after 1 second

    Serial.println(F("InputManager ready..."));
}

void InputManager::update() {
    // Update button state
    button.tick();
    
    // Debug print every 5 seconds
    EVERY_N_SECONDS(5) {
        Serial.println(F("InputManager update tick"));
    }
    
    // Handle brightness cycling when in long press mode
    if (longPressMode) {
        cycleLongPress();
    } 
}

void InputManager::handleClick() { 
    Serial.println(F("handleClick called"));
    if (systemManager && systemManager->getAnimationManager()) {
        // Directly trigger pattern change
        Serial.println(F("Button clicked, changing pattern"));
        systemManager->getAnimationManager()->nextPattern();
    } else {
        Serial.println(F("ERROR: systemManager or animationManager is null in handleClick"));
    }
}

void InputManager::handleLongPressStart() {
    brightnessMode = true;
    longPressMode = true;
    longPressStartTime = millis();  
    Serial.println(F("Button long press started: Entering brightness mode"));
}

void InputManager::handleLongPressStop() { 
    brightnessMode = false;
    longPressMode = false;
    ledCountDownMode = false;
    ledCountUpMode = false;
    longPressStartTime = 0;
}

void InputManager::cycleLongPress() {
    unsigned long elapsed = millis() - longPressStartTime; 
    uint16_t numLeds = systemManager->getAnimationManager()->getNumLeds();

    // If we've been holding for more than 5 seconds, switch to LED count up mode
    if (elapsed >= 5000) {
        // Turn off other modes
        ledCountDownMode = false;
        
        // Enter LED count up mode
        ledCountUpMode = true;
        
        // Increase LED count by 100, with safe bounds checking
        if (numLeds <= MAX_LEDS - 100) {
            numLeds += 100;
        } else {
            numLeds = MAX_LEDS; // Cap at maximum
        }
        systemManager->getAnimationManager()->updateLedCount(numLeds);
        
        Serial.print(F("LED count increased to: "));
        Serial.println(numLeds); 
    } else if (elapsed >= 3000) {
        // Turn off other modes
        brightnessMode = false;  
        // Enter LED count down mode
        ledCountDownMode = true;
        
        // Decrease LED count by 50, with safe bounds checking
        if (numLeds > 50) {
            numLeds -= 50;
        } else {
            numLeds = 1; // Minimum safe value
        }
        systemManager->getAnimationManager()->updateLedCount(numLeds);
        
        Serial.print(F("LED count decreased to: "));
        Serial.println(numLeds);
    }  
}
 
