/**
 * Project Configuration
 * 
 * Centralizes all configurable parameters for the project
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <FastLED.h>

// Safety settings
#define ENABLE_SAFE_MODE 1              // Set to 1 to enable extra safety measures
#define MAX_MILLIAMPS 1000              // Maximum current for FastLED power management (1A)

// OLED functionality setting
#define ENABLE_OLED 1                   

// Include U8X8 only if OLED is enabled
#if ENABLE_OLED
#include <U8g2lib.h>
#endif

// LED Configuration
#define LED_DATA_PIN 8                  // GPIO8 - Data pin to connect to the LED strip
#define COLOR_ORDER GRB                 // It's GRB for WS2812 and BGR for APA102
#define LED_TYPE WS2812                 // Using WS2812, WS2801, APA102, etc.
#define MAX_LEDS 1000                    // (is set to a fixed 1000 since fastled cannot change the value on runtime. DO NOT CHANGE THIS!)
#define DEFAULT_NUM_LEDS 100            // Default number of LEDs (reduced to safer value)

// Button Configuration
#define BUTTON_PIN 9                    // GPIO9 - Built-in BOOT button (active LOW)
#define BUILTIN_LED_PIN 2               // GPIO2 - Built-in LED for status indication

// OLED Display Configuration
// ENABLE_OLED is now defined at the top of the file
#if ENABLE_OLED
#define OLED_RESET U8X8_PIN_NONE        // Reset pin
#else
#define OLED_RESET 0                    // Dummy value when OLED is disabled
#endif
#define OLED_SDA 5                      // SDA pin for I2C
#define OLED_SCL 6                      // SCL pin for I2C
#define OLED_WIDTH 72                   // Actual OLED display width
#define OLED_HEIGHT 40                  // Actual OLED display height
#define OLED_X_OFFSET 30                // = (128-72)/2 - Center horizontally
#define OLED_Y_OFFSET 12                // = (64-40)/2 - Center vertically

// Brightness Configuration
#define MIN_BRIGHTNESS 5               // Minimum brightness (10% of 255)
#define MAX_BRIGHTNESS 255              // Maximum brightness (100%)
#define DEFAULT_BRIGHTNESS 128          // Default brightness (50%)

// Timing Configuration
#define ANIMATION_UPDATE_INTERVAL 50    // Update animations every 50ms
#define HUE_UPDATE_INTERVAL 20          // Update global hue every 20ms
#define BRIGHTNESS_DISPLAY_DURATION 3000 // Show brightness for 3 seconds
#define NUMLEDS_DISPLAY_DURATION 3000   // Show LED length for 3 seconds

// Input Mode Configuration
#define BRIGHTNESS_CHANGE_HOLDTIME 3000  // Hold 3 seconds to enter brightness mode
#define LED_COUNT_DOWN_HOLDTIME 4000     // Hold 4 seconds to enter LED count down mode
#define LED_COUNT_UP_HOLDTIME 6000       // Hold 6 seconds to enter LED count up mode

// Preferences Keys
#define PREF_NAMESPACE "jos_led_controller"
#define PREF_PATTERN_KEY "pattern"
#define PREF_BRIGHTNESS_KEY "brightness"
#define PREF_NUM_LEDS_KEY "numLeds"

// Animation Categories
enum AnimationCategory {
    SLOW_AND_SOOTHING,
    SOLID_COLORS,
    HIGH_BPM,
    PARTY_VIBE,
    PSYCHEDELIC,
    INTENSE,
    CRAZY,
    TRIAL_RUNS,
    NUM_CATEGORIES  // Must be last
};

#endif // CONFIG_H
