/**
 * System Manager Implementation
 */

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

    // Test FastLED
    FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, DEFAULT_NUM_LEDS);
    fill_solid(leds, 5, CRGB::Red);
    FastLED.setBrightness(50);
    FastLED.show();
    delay(1000);
    if (leds[0] != CRGB::Red) {
        Serial.println(F("ERROR: LED test failed - check wiring/pin"));
    } else {
        Serial.println(F("LED test passed - red on first 5 LEDs"));
    }
    fill_solid(leds, MAX_LEDS, CRGB::Black);
    FastLED.show();

    // Create animation manager
    Serial.println(F("Creating AnimationManager..."));
    animationManager = new AnimationManager(*this, leds);
    if (!animationManager) {
        Serial.println(F("ERROR: Failed to create AnimationManager"));
        return;
    }

    animationManager->begin();
    Serial.print(F("Total animations: "));
    Serial.println(animationManager->getPatternCount());
    Serial.print(F("Selected animation: "));
    Serial.println(animationManager->getCurrentPatternName());

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
        EVERY_N_SECONDS(5) { Serial.println(F("ERROR: Animation manager null")); }
        return;
    }

    unsigned long currentMillis = millis();
    static unsigned long lastSuccessfulUpdate = 0;

    if (currentMillis - lastLedUpdate >= ANIMATION_UPDATE_INTERVAL) {
        lastLedUpdate = currentMillis;
        EVERY_N_SECONDS(10) { Serial.println(F("Updating animation...")); }
        animationManager->update();
        lastSuccessfulUpdate = currentMillis;
        EVERY_N_SECONDS(10) { Serial.println(F("Animation update complete")); }
    }

    if (currentMillis - lastSuccessfulUpdate > 5000) {
        EVERY_N_SECONDS(5) { Serial.println(F("WARNING: Animation update timeout")); }
        lastSuccessfulUpdate = currentMillis;
    }
}

void SystemManager::update() {
    EVERY_N_SECONDS(10) { Serial.println(F("Updating inputs...")); }
    inputManager.update();
    updateLeds();
    #if defined(WATCHDOG_C3_WORKAROUND)
    static unsigned long lastWDTFeed = 0;
    if (millis() - lastWDTFeed > 1000) {
        esp_task_wdt_reset();
        lastWDTFeed = millis();
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
    Serial.print(F("Saving: ")); Serial.print(key); Serial.print(F(" = ")); Serial.println(value);
    preferences.putString(key, value);
}

void SystemManager::pushSavedByte(const char* key, uint8_t value) {
    Serial.print(F("Saving: ")); Serial.print(key); Serial.print(F(" = ")); Serial.println(value);
    preferences.putUChar(key, value);
}

void SystemManager::pushSavedNumber(const char* key, uint16_t value) {
    Serial.print(F("Saving: ")); Serial.print(key); Serial.print(F(" = ")); Serial.println(value);
    preferences.putUShort(key, value);
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