/**
 * Slow and Soothing Animations
 * 
 * Collection of calming, slow-paced animations
 */

#ifndef SLOW_AND_SOOTHING_ANIMATIONS_H
#define SLOW_AND_SOOTHING_ANIMATIONS_H

#include "AnimationBase.h"

// Define FORWARD_HUES if not already defined
#ifndef FORWARD_HUES
#define FORWARD_HUES 1
#endif

// Breathing Light Animation
class BreathingAnimation : public Animation {
public:
    BreathingAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        breath(0) {
        // Validate LED array and numLeds
        if (!ledArray || numLeds == 0) {
            Serial.println(F("WARNING: BreathingAnimation initialized with invalid LEDs"));
        }
    }
    
    void update() override {
        // Failsafe: check for null LED array or zero LEDs
        if (!leds || numLeds == 0) {
            return;
        }
        
        // Increment breath value (controls the breathing effect)
        breath += 0.01;
        if (breath > TWO_PI) breath = 0;

        // Calculate brightness using beatsin8 (built-in FastLED sine function)
        // This ensures brightness is within valid range
        uint8_t breathBrightness = beatsin8(6, MIN_BRIGHTNESS, constrain(brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS), 0, 0); // 6 BPM
        
        // Apply the same color to all LEDs
        fill_solid(leds, numLeds, CHSV(140, 150, breathBrightness)); // Soft blue
    }
    
    const char* getName() override { return "Breathing"; }
    
    AnimationCategory getCategory() override { return SLOW_AND_SOOTHING; }

private:
    float breath;
};

// Aurora Animation
class AuroraAnimation : public Animation {
public:
    AuroraAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        t(0) {}
    
    void update() override {
        fadeToBlackBy(leds, numLeds, 5);
        t += 5;

        for (int i = 0; i < numLeds; i++) {
            uint16_t noise = inoise8(i * 30, t);
            leds[i] += CHSV(140 + noise / 4, 200 - noise / 2, noise);
        }
    }
    
    const char* getName() override { return "Aurora"; }
    
    AnimationCategory getCategory() override { return SLOW_AND_SOOTHING; }

private:
    uint16_t t;
};

// Color Meditation Animation
class ColorMeditationAnimation : public Animation {
public:
    ColorMeditationAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        hue(0),
        lastHueChange(0) {}
    
    void update() override {
        // Change hue very slowly - once every 5 minutes
        unsigned long currentMillis = millis();
        if (currentMillis - lastHueChange > 300000) { // 5 minutes
            hue = random8();
            lastHueChange = currentMillis;
        }

        // Handle single LED case to prevent division by zero
        if (numLeds <= 1) {
            if (numLeds == 1) {
                leds[0] = CHSV(hue, 240, brightness);
            }
            return;
        }
    
        // Gradient that shifts imperceptibly
        for (int i = 0; i < numLeds; i++) {
            uint8_t progress = 255 * i / (numLeds - 1);
            // Blend between two colors based on position
            leds[i] = CHSV(
                map8(progress, hue, hue + 30),
                map8(progress, 255, 200),
                map8(progress, brightness, brightness - 30)
            );
        }

        // Imperceptibly shift hue
        EVERY_N_MILLISECONDS(100) {
            hue++;
        }
    }
    
    const char* getName() override { return "Color Meditation"; }
    
    AnimationCategory getCategory() override { return SLOW_AND_SOOTHING; }

private:
    uint8_t hue;
    unsigned long lastHueChange;
};

// Ocean Waves Animation
class OceanWavesAnimation : public Animation {
public:
    OceanWavesAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        offset(0) {}
    
    void update() override {
        offset += 3;

        for (int i = 0; i < numLeds; i++) {
            uint8_t wave = sin8(i * 10 + offset);
            leds[i] = CHSV(135, 200, wave); // Seafoam green
        }
        fadeLightBy(leds, numLeds, 20);
    }
    
    const char* getName() override { return "Ocean Waves"; }
    
    AnimationCategory getCategory() override { return SLOW_AND_SOOTHING; }

private:
    uint16_t offset;
};

// Moonlight Animation
class MoonlightAnimation : public Animation {
public:
    MoonlightAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds) {}
    
    void update() override {
        fadeToBlackBy(leds, numLeds, 1);

        EVERY_N_SECONDS(2) {
            // Occasionally add sparkles
            if (random8() < 30) {
                uint8_t pos = random16(numLeds);
                leds[pos] = CRGB(220, 220, 255); // Moonlight white
            }
        }

        // Base soft blue light
        fill_solid(leds, numLeds, CRGB(25, 35, 80));
    }
    
    const char* getName() override { return "Moonlight"; }
    
    AnimationCategory getCategory() override { return SLOW_AND_SOOTHING; }
};

// Forest Canopy Animation
class ForestCanopyAnimation : public Animation {
public:
    ForestCanopyAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        t(0) {}
    
    void update() override {
        t += 5;

        for (int i = 0; i < numLeds; i++) {
            uint8_t green = inoise8(i * 20, t);
            uint8_t blue = inoise8(i * 20 + 10000, t);
            leds[i] = CRGB(0, green/2, blue/3);
        }
        fadeLightBy(leds, numLeds, 5);
    }
    
    const char* getName() override { return "Forest Canopy"; }
    
    AnimationCategory getCategory() override { return SLOW_AND_SOOTHING; }

private:
    uint16_t t;
};

#endif // SLOW_AND_SOOTHING_ANIMATIONS_H
