#include <Arduino.h>
/**
 * ESP32 Basic Wearable FastLED Controller
 */
#define FASTLED_INTERNAL //remove annoying pragma messages
#include "system/SystemManager.h"
#include "animations/AnimationManager.h"
#include "config/Config.h"
#include "config/PinConfig.h"
#include <FastLED.h>
#include <esp_task_wdt.h>
#include <WiFi.h>
#include <Preferences.h>
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
    Serial.println(F("Setup start"));
    Serial.print(F("FastLED version: ")); Serial.println(FASTLED_VERSION);
    WiFi.mode(WIFI_OFF);

    // Check NVS health
    Preferences prefs;
    prefs.begin(Config::PREF_NAMESPACE, false);
    size_t freeEntries = prefs.freeEntries();
    Serial.print(F("[INFO] NVS free entries: ")); Serial.println(freeEntries);
    if (freeEntries < 10) {
        Serial.println(F("[WARNING] NVS nearly full; clearing to prevent errors."));
        prefs.clear();
    }
    prefs.end();

    #if defined(WATCHDOG_C3_WORKAROUND) || defined(WATCHDOG_S3_WORKAROUND)
        esp_task_wdt_init(0xFFFFFFFF, false);
        esp_task_wdt_add(NULL);
    #endif

    systemManager.begin();

    #if ENABLE_OLED
        oledManager.setSystemManager(&systemManager);
        oledManager.begin();
        Serial.println(F("OLED manager setup complete"));
    #endif

    // Test LED hardware post-AnimationManager begin
    AnimationManager* animMgr = systemManager.getAnimationManager();
    if (animMgr && animMgr->isReady()) {
        CRGB* leds = animMgr->getLEDs();
        fill_solid(leds, 5, CRGB::Red);
        FastLED.setBrightness(50);
        FastLED.show();
        delay(500);
        if (leds[0] != CRGB::Red) {
            Serial.println(F("ERROR: LED test failed - check wiring/pin or power supply"));
        } else {
            Serial.println(F("LED test passed - red on first 5 LEDs"));
        }
        fill_solid(leds, MAX_LEDS, CRGB::Black);
        FastLED.show();
    } else {
        Serial.println(F("ERROR: AnimationManager not ready for LED test"));
    }

    EVERY_N_SECONDS(10) {
        Serial.print(F("[INFO] LED buffer address: 0x"));
        Serial.println(reinterpret_cast<uintptr_t>(animMgr ? animMgr->getLEDs() : nullptr), HEX);
        Serial.print(F("[INFO] LED buffer size: "));
        Serial.println(sizeof(CRGB) * MAX_LEDS);
        Serial.println(F("[HEALTHY] If buffer address is in ESP32-C3 DRAM (0x3FC80000-0x3FCE0000) range, all good. Otherwise, caution!"));
        Serial.print(F("[INFO] Estimated power draw (300 LEDs, brightness "));
        Serial.print(animMgr ? animMgr->getBrightness() : 0);
        Serial.print(F("): ~"));
        Serial.print((300 * (animMgr ? animMgr->getBrightness() : 128) * 60) / 255);
        Serial.println(F(" mA"));
    }

    systemManager.getInputManager().begin(&systemManager);
    Serial.println(F("Input manager setup complete"));
    Serial.println(F("Setup complete. Running main loop..."));
}

void logHeapStackUsage() {
    static unsigned long lastLogTime = 0;
    unsigned long currentTime = millis();
    if (currentTime - lastLogTime > 5000) {
        size_t heapFree = ESP.getFreeHeap();
        size_t largestBlock = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
        UBaseType_t stackHighWater = uxTaskGetStackHighWaterMark(NULL);

        Serial.print(F("[INFO] Heap free: "));
        Serial.print(heapFree);
        Serial.println(F(" bytes"));
        Serial.print(F("[INFO] Largest heap block: "));
        Serial.print(largestBlock);
        Serial.println(F(" bytes"));
        Serial.print(F("[INFO] Stack high water mark: "));
        Serial.print(stackHighWater);
        Serial.println(F(" (higher is healthier, <200 is caution, <100 is critical!)"));

        if (heapFree < 10000) {
            Serial.println(F("[CRITICAL] Heap memory low! Risk of crashes or random bugs."));
        } else if (heapFree < 20000) {
            Serial.println(F("[CAUTION] Heap memory getting low. Consider optimizing."));
        }

        if (largestBlock < 5000) {
            Serial.println(F("[CAUTION] Largest heap block is small. May cause allocation failures."));
        }

        if (stackHighWater < 100) {
            Serial.println(F("[CRITICAL] Stack high water mark is very low! Risk of stack overflow."));
        } else if (stackHighWater < 200) {
            Serial.println(F("[CAUTION] Stack high water mark is getting low. Monitor for overflows."));
        }
        lastLogTime = currentTime;
    }
}

void loop() {
  // Print a heartbeat message every few seconds
  EVERY_N_SECONDS(5) {
    Serial.println(F("Main loop running"));
  }

    EVERY_N_SECONDS(60) { Serial.println(F("[INFO] Main loop running - system healthy if this repeats.")); }
    systemManager.update();

#if ENABLE_OLED
  // Update OLED display periodically
  EVERY_N_MILLISECONDS(250) {
    oledManager.update();
  }
#endif

    logHeapStackUsage();

    EVERY_N_SECONDS(10) {
        Serial.println(F("[INFO] About to call FastLED.show() - if you see freezes here, check wiring, power, or buffer issues."));
    }

    if (millis() - lastShow >= ANIMATION_UPDATE_INTERVAL) {
        lastShow = millis();
        AnimationManager* animMgr = systemManager.getAnimationManager();
        if (animMgr && animMgr->isReady()) {
            unsigned long startTime = micros();
            FastLED.show();
            unsigned long duration = micros() - startTime;
            //Serial.print(F("[DEBUG] FastLED.show() duration: "));
            //Serial.print(duration);
            //Serial.println(F(" microseconds"));

            EVERY_N_SECONDS(20) {
                CRGB* leds = animMgr->getLEDs();
                Serial.print(F("[DEBUG] Post-show sample LED[0]: R:"));
                Serial.print(leds[0].r);
                Serial.print(F(" G:"));
                Serial.print(leds[0].g);
                Serial.print(F(" B:"));
                Serial.println(leds[0].b);
            }
        } else {
            EVERY_N_SECONDS(10) { Serial.println(F("[CAUTION] AnimationManager not ready while loop active")); }
        }
        #if defined(WATCHDOG_C3_WORKAROUND)
        esp_task_wdt_reset();
        #endif
    }

    delay(5);
}