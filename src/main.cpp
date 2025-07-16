/**
 * ESP32 Basic Wearable FastLED Controller
 */
#include <Arduino.h>
#include "system/SystemManager.h"
#include "config/Config.h"
#include <FastLED.h>
#include <esp_task_wdt.h>
#include <soc/rtc_wdt.h>
#include <WiFi.h>
#if ENABLE_OLED
#include "display/OLEDManager.h"
OLEDManager oledManager;
#endif

// Global system components
SystemManager systemManager;

// Timing variables
unsigned long lastUpdate = 0;
unsigned long lastShow = 0;

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println(F("Setup start"));
    WiFi.mode(WIFI_OFF);
    Serial.println(F("WiFi off"));

    #if defined(WATCHDOG_C3_WORKAROUND)
    esp_task_wdt_init(0xFFFFFFFF, false);
    esp_task_wdt_add(NULL);
    #else
    esp_task_wdt_init(0xFFFFFFFF, false);
    rtc_wdt_protect_off();
    rtc_wdt_disable();
    #endif

    // Test LED hardware
    CRGB testLeds[MAX_LEDS];
    FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(testLeds, DEFAULT_NUM_LEDS);
    fill_solid(testLeds, 5, CRGB::Red);
    FastLED.setBrightness(50);
    FastLED.show();
    delay(1000);  // Give time to see
    if (testLeds[0] != CRGB::Red) {
        Serial.println(F("ERROR: LED test failed - check wiring/pin"));
    } else {
        Serial.println(F("LED test passed - red on first 5 LEDs"));
    }
    fill_solid(testLeds, MAX_LEDS, CRGB::Black);
    FastLED.show();

    systemManager.begin();
    delay(200);

    #if ENABLE_OLED
    oledManager.begin(&systemManager);
    Serial.println(F("OLED manager setup complete"));
    #endif

    systemManager.getInputManager().begin(&systemManager);
    Serial.println(F("Input manager setup complete"));
    Serial.println(F("Setup complete. Running main loop..."));
}

void loop() {
    EVERY_N_SECONDS(5) { Serial.println(F("Main loop running")); }
    systemManager.update();

    #if ENABLE_OLED
    EVERY_N_MILLISECONDS(250) { oledManager.update(); }
    #endif

    if (millis() - lastShow >= ANIMATION_UPDATE_INTERVAL) {
        lastShow = millis();
        AnimationManager* animMgr = systemManager.getAnimationManager();
        if (animMgr && animMgr->isReady() && animMgr->getCurrentPatternIndex() < animMgr->getPatternCount()) {
            EVERY_N_SECONDS(10) { Serial.println(F("Calling FastLED.show()...")); }
            FastLED.show();
            EVERY_N_SECONDS(10) { Serial.println(F("FastLED.show() done")); }
        } else {
            EVERY_N_SECONDS(5) { Serial.println(F("Skipping FastLED.show() - no valid animation")); }
        }
        #if defined(WATCHDOG_C3_WORKAROUND)
        esp_task_wdt_reset();
        #endif
    }

    delay(5);
}