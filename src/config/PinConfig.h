#pragma once

#ifndef PINCONFIG_H
#define PINCONFIG_H

// --- ESP32-C3 DevKit ---
#if defined(ARDUINO_ESP32C3_DEV)
  #ifndef WATCHDOG_C3_WORKAROUND
    #define WATCHDOG_C3_WORKAROUND
  #endif
  #ifndef OLED_SDA
    #define OLED_SDA 5
  #endif
  #ifndef OLED_SCL
    #define OLED_SCL 6
  #endif
  #ifndef LED_DATA_PIN
    #define LED_DATA_PIN 8
  #endif
  #ifndef BUTTON_1_PIN
    #define BUTTON_1_PIN 9
  #endif

// --- ESP32-S3 DevKitC-1 ---
#elif defined(ARDUINO_ESP32S3_DEV)
 
  #ifndef OLED_SDA
    #define OLED_SDA 8
  #endif
  #ifndef OLED_SCL
    #define OLED_SCL 9
  #endif
  #ifndef LED_DATA_PIN
    #define LED_DATA_PIN 4
  #endif
  #ifndef BUTTON_1_PIN
    #define BUTTON_1_PIN 0
  #endif

// --- ESP32-D1 Mini (LOLIN32) ---
#elif defined(ARDUINO_LOLIN32)
  #ifndef OLED_SDA
    #define OLED_SDA 21
  #endif
  #ifndef OLED_SCL
    #define OLED_SCL 22
  #endif
  #ifndef LED_DATA_PIN
    #define LED_DATA_PIN 4
  #endif
  #ifndef BUTTON_1_PIN
    #define BUTTON_1_PIN 13
  #endif

// --- Classic ESP32 DevKit V1 ---
#elif defined(ARDUINO_ESP32_DEV)
  #ifndef OLED_SDA
    #define OLED_SDA 21
  #endif
  #ifndef OLED_SCL
    #define OLED_SCL 22
  #endif
  #ifndef LED_DATA_PIN
    #define LED_DATA_PIN 4
  #endif
  #ifndef BUTTON_1_PIN
    #define BUTTON_1_PIN 13
  #endif

#endif // end per-board conditions

#endif // PINCONFIG_H
