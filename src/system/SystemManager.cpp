/**
 * System Manager Implementation
 */

#include "SystemManager.h"
#include "../animations/AnimationManager.h"

SystemManager::SystemManager() : 
    animationManager(nullptr) {
}

SystemManager::~SystemManager() {
    // Clean up animation manager if it was created
    if (animationManager) {
        delete animationManager;
        animationManager = nullptr;
    }
}

void SystemManager::begin() {
    Serial.println(F("SystemManager Starting...")); 
    
    // Initialize preferences
    initPreferences();
    
    // Initialize hardware components
    initHardware();
    Serial.println(F("SystemManager ready...")); 
} 

void SystemManager::initPreferences() {
    // Initialize preferences storage
    preferences.begin("jos_led_controller", false);
    Serial.println(F("Preferences initialized"));
}

String SystemManager::getSavedString(const char* key, const char* defaultValue) {
    return preferences.getString(key, defaultValue);
}

uint8_t SystemManager::getSavedByte(const char* key, uint8_t defaultValue) {
    return preferences.getUChar(key, defaultValue);
}

uint16_t SystemManager::getSavedNumber(const char* key, uint16_t defaultValue) {
    return preferences.getUShort(key, defaultValue);
}

void SystemManager::pushSavedString(const char* key, const String& value) {
    Serial.print(F("Saving preference: "));
    Serial.print(key);
    Serial.print(F(" = "));
    Serial.println(value);
    preferences.putString(key, value);
}

void SystemManager::pushSavedByte(const char* key, uint8_t value) {
    Serial.print(F("Saving preference: "));
    Serial.print(key);
    Serial.print(F(" = "));
    Serial.println(value);
    preferences.putUChar(key, value);
}

void SystemManager::pushSavedNumber(const char* key, uint16_t value) {
    Serial.print(F("Saving preference: "));
    Serial.print(key);
    Serial.print(F(" = "));
    Serial.println(value);
    preferences.putUShort(key, value);
}

void SystemManager::initHardware() {
    // Initialize built-in LED for status indication
    pinMode(BUILTIN_LED_PIN, OUTPUT);
    digitalWrite(BUILTIN_LED_PIN, LOW);
    
    Serial.println(F("Hardware initialized"));
}

void SystemManager::setupAnimationSystem() {
    // Create animation manager if it doesn't exist
    if (!animationManager) {
        Serial.println(F("Creating AnimationManager..."));
        animationManager = new AnimationManager(*this);
        if (!animationManager) {
            Serial.println(F("ERROR: Failed to create AnimationManager!"));
            return;
        }
    } else {
        Serial.println(F("AnimationManager already exists, reusing instance"));
    }
    
    // Initialize the animation system
    Serial.println(F("Initializing animation system..."));
    animationManager->begin();
    
    // Get and print the count of registered animations
    Serial.print(F("Total animations: "));
    Serial.println(animationManager->getPatternCount());
    Serial.print(F("Selected animation: "));
    Serial.println(animationManager->getCurrentPatternName());
    Serial.println(F("Animation system setup complete"));
}

void SystemManager::updateLeds() {
    // Safety check
    if (!animationManager) {
        EVERY_N_SECONDS(10) {
            Serial.println(F("WARNING: Animation manager is null in updateLeds"));
        }
        return;
    }
    
    unsigned long currentMillis = millis();
    static unsigned long lastSuccessfulUpdate = 0;
    
    // Update animation calculations at the animation update rate
    // This calculation doesn't show LEDs yet, just updates the buffer
    if (currentMillis - lastLedUpdate >= ANIMATION_UPDATE_INTERVAL) {
        lastLedUpdate = currentMillis;
        
        // Debug print - add Serial.print before animation update
        EVERY_N_SECONDS(10) {
            Serial.println(F("Updating animation..."));
        }
        
        // Update with basic error recovery
        animationManager->update();
        lastSuccessfulUpdate = millis();

        // Debug print - add Serial.print after animation update
        EVERY_N_SECONDS(10) {
            Serial.println(F("Animation update complete"));
        }
    }
    
    // Check for update timeouts - if updates are taking too long, something might be wrong
    if (currentMillis - lastSuccessfulUpdate > 5000) {
        // If no successful updates for 5 seconds, try to reset the animation
        EVERY_N_SECONDS(5) {
            Serial.println(F("WARNING: No successful animation updates recently. Attempting recovery."));
        }
        
        // Recovery attempt here would be too complex - just note the problem
        lastSuccessfulUpdate = currentMillis; // Reset the timer to prevent spam
    }
    
    // Show LEDs at a controlled rate to not overwhelm the bus
    // This is what actually sends data to the LED strip
    if (currentMillis - lastLedShow >= 30) {  // 30ms = ~33fps
        lastLedShow = currentMillis;

        // Debug print before FastLED.show
        EVERY_N_SECONDS(10) {
            Serial.println(F("About to call FastLED.show()..."));
        }

        // Add a try block to catch any potential FastLED.show() issues
        // Note: This won't actually catch hardware exceptions, but it's good practice
        try {
            FastLED.show();
        } catch (...) {
            Serial.println(F("ERROR: Exception in FastLED.show()"));
        }

        // Debug print after FastLED.show
        EVERY_N_SECONDS(10) {
            Serial.println(F("FastLED.show() completed"));
        }
    }
}

void SystemManager::updateInputs() {
    // Debug print before updating input
    EVERY_N_SECONDS(10) {
        Serial.println(F("Updating input manager..."));
    }
    
    // Update the input manager to handle button presses
    inputManager.update();

    // Debug print after updating input
    EVERY_N_SECONDS(10) {
        Serial.println(F("Input manager update complete"));
    }
}

void SystemManager::update() {
    // Update inputs
    updateInputs();

    // Update LEDs
    updateLeds();
}
