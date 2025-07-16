/**
* Project Configuration
 */
#ifndef CONFIG_H
#define CONFIG_H
#define WATCHDOG_C3_WORKAROUND 1
#include <FastLED.h>

#define ENABLE_SAFE_MODE 1
#define MAX_MILLIAMPS 1000
#define ENABLE_OLED 1

#if ENABLE_OLED
#include <U8g2lib.h>
#endif

#define LED_DATA_PIN 8
#define COLOR_ORDER GRB
#define LED_TYPE WS2812
#define MIN_LEDS 1
#define MAX_LEDS 1000
#define DEFAULT_NUM_LEDS 300

#define BUTTON_PIN 9
#define BUILTIN_LED_PIN 2
#define ADJUST_NUM_LEDS_INCREMENT 50
#define ADJUST_BRIGHTNESS_INCREMENT 25

#define MIN_BRIGHTNESS 25
#define MAX_BRIGHTNESS 255
#define DEFAULT_BRIGHTNESS 128

#define TARGET_FPS 60
#define ANIMATION_UPDATE_INTERVAL (1000 / TARGET_FPS)
#define HUE_UPDATE_INTERVAL 20
#define BRIGHTNESS_DISPLAY_DURATION 3000
#define NUMLEDS_DISPLAY_DURATION 3000
#define SHUFFLE_DURATION 3000

#define LED_COUNT_DOWN_HOLDTIME 2000
#define LED_COUNT_UP_HOLDTIME 4000
#define SHUFFLE_HOLDTIME 6000

// OLED pins
#define OLED_SDA 5
#define OLED_SCL 6
#define OLED_RESET U8X8_PIN_NONE

namespace Config {
    inline constexpr const char* PREF_NAMESPACE = "jos_led_controller";
    inline constexpr const char* PREF_PATTERN_KEY = "pattern";
    inline constexpr const char* PREF_BRIGHTNESS_KEY = "brightness";
    inline constexpr const char* PREF_NUM_LEDS_KEY = "numLeds";
}

enum AnimationCategory {
    SLOW_AND_SOOTHING,
    SOLID_COLORS,
    HIGH_BPM,
    PARTY_VIBE,
    PSYCHEDELIC,
    INTENSE,
    CRAZY,
    TRIAL_RUNS,
    NUM_CATEGORIES
};

#ifndef TWO_PI
#define TWO_PI 6.283185307179586
#endif

#endif // CONFIG_H