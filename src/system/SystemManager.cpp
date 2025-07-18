/**
 * System Manager Implementation
 */
#include <Arduino.h>
#include "../config/PinConfig.h"
#include "SystemManager.h"
#include "../animations/AnimationManager.h"
#include <esp_task_wdt.h>

SystemManager::SystemManager() : animationManager(nullptr), leds(new CRGB[MAX_LEDS]) {
    memset(leds, 0, sizeof(CRGB) * MAX_LEDS);
}

SystemManager::~SystemManager() {
    if (animationManager) {
        delete animationManager;
        animationManager = nullptr;
    }
    delete[] leds;
}

void SystemManager::begin() {
    Serial.println(F("SystemManager Starting..."));
    initPreferences();
    initHardware();
    // Create animation manager
    Serial.println(F("Creating AnimationManager..."));
    animationManager = new AnimationManager(*this, leds);
    if (!animationManager) {
        Serial.println(F("ERROR: Failed to create AnimationManager"));
        return;
    }
    delay(500);  // Allow time for hardware to settle
    animationManager->begin();
    Serial.print(F("Total animations: "));
    Serial.println(animationManager->getPatternCount());
    Serial.print(F("Selected animation: "));
    Serial.println(animationManager->getCurrentAnimationName());

    Serial.println(F("SystemManager ready"));
}

void SystemManager::initPreferences() {
    preferences.begin(Config::PREF_NAMESPACE, false);
    Serial.println(F("Preferences initialized"));
}

void SystemManager::initHardware() {
    pinMode(BUILTIN_LED_PIN, OUTPUT);
    digitalWrite(BUILTIN_LED_PIN, LOW);
    Serial.println(F("Hardware initialized"));
}

void SystemManager::updateLeds() {
    if (!animationManager) {
        return; // Fail silently to avoid blocking with repeated error messages
    }

    unsigned long currentMillis = millis();
    static unsigned long lastSuccessfulUpdate = 0;

    if (currentMillis - lastLedUpdate >= ANIMATION_UPDATE_INTERVAL) {
        lastLedUpdate = currentMillis;
        
        // Non-blocking animation update
        animationManager->update();
        lastSuccessfulUpdate = currentMillis;
    }

    // Reduce timeout logging frequency to avoid blocking
    if (currentMillis - lastSuccessfulUpdate > 10000) { // Increased to 10 seconds
        EVERY_N_SECONDS(30) { Serial.println(F("WARNING: Animation update timeout")); }
        lastSuccessfulUpdate = currentMillis;
    }
}

void SystemManager::update() {
    inputManager.update();
    updateLeds();

    // Handle watchdog reset in a non-blocking manner
    #if defined(WATCHDOG_C3_WORKAROUND)
    static unsigned long lastWDTFeed = 0;
    unsigned long currentMillis = millis();
    if (currentMillis - lastWDTFeed > 1000) {
        esp_task_wdt_reset();
        lastWDTFeed = currentMillis;
    }
    #endif
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
    // Defer preferences write to avoid blocking main loop
    static unsigned long lastWrite = 0;
    if (millis() - lastWrite > 1000) { // Throttle writes to once per second
        preferences.putString(key, value);
        lastWrite = millis();
    }
}

void SystemManager::pushSavedByte(const char* key, uint8_t value) {
    // Defer preferences write to avoid blocking main loop
    static unsigned long lastWrite = 0;
    if (millis() - lastWrite > 1000) { // Throttle writes to once per second
        preferences.putUChar(key, value);
        lastWrite = millis();
    }
}

void SystemManager::pushSavedNumber(const char* key, uint16_t value) {
    // Defer preferences write to avoid blocking main loop
    static unsigned long lastWrite = 0;
    if (millis() - lastWrite > 1000) { // Throttle writes to once per second
        preferences.putUShort(key, value);
        lastWrite = millis();
    }
}

void SystemManager::handleNextPattern() {
    if (!animationManager) {
        Serial.println(F("ERROR: Animation manager null"));
        return;
    }
    animationManager->nextPattern();
}

void SystemManager::setCurrentPattern(uint16_t value) {
    if (!animationManager) {
        Serial.println(F("ERROR: Animation manager null"));
        return;
    }
    animationManager->setCurrentPattern(value);
}

void SystemManager::setBrightness(uint16_t value) {
    if (!animationManager) {
        Serial.println(F("ERROR: Animation manager null"));
        return;
    }
    animationManager->setBrightness(value);
}

void SystemManager::setNumLeds(uint16_t count) {
    if (!animationManager) {
        Serial.println(F("ERROR: Animation manager null"));
        return;
    }
    animationManager->setNumLeds(count);
}

uint8_t SystemManager::getBrightness() const {
    return animationManager ? animationManager->getBrightness() : 0;
}

uint16_t SystemManager::getNumLeds() const {
    return animationManager ? animationManager->getNumLeds() : 0;
}