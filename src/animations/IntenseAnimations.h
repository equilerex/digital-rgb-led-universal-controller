/**
 * Intense Animations
 * 
 * Collection of high-energy, intense animations
 */

#ifndef INTENSE_ANIMATIONS_H
#define INTENSE_ANIMATIONS_H

#include "AnimationBase.h"

// Hyper Spin Animation
class HyperSpinAnimation : public Animation {
public:
    HyperSpinAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        angle(0) {}
    
    void update() override {
        angle += beatsin16(60, 10, 30); // Speed varies with beat

        for (int i = 0; i < numLeds; i++) {
            uint8_t colorIndex = (i * 10) - angle;
            leds[i] = ColorFromPalette(RainbowStripeColors_p, colorIndex, beatsin8(120, 200, brightness), LINEARBLEND);
        }
    }
    
    const char* getName() override { return "Hyper Spin"; }
    
    AnimationCategory getCategory() override { return INTENSE; }

private:
    uint16_t angle;
};

// Beat Trails Animation
class BeatTrailsAnimation : public Animation {
public:
    BeatTrailsAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        gHue(0) {}
    
    void update() override {
        EVERY_N_MILLISECONDS(20) {
            // Add sparks on beat
            if (random8() < beatsin8(60, 10, 50)) {
                uint8_t pos = random16(numLeds);
                leds[pos] = CHSV(gHue + random8(64), 255, brightness);
            }

            // Fade and shift
            fadeToBlackBy(leds, numLeds, 20);
            for (int i = numLeds-1; i > 0; i--) {
                leds[i] |= leds[i-1];
            }
            gHue += 2;
        }
    }
    
    const char* getName() override { return "Beat Trails"; }
    
    AnimationCategory getCategory() override { return INTENSE; }

private:
    uint8_t gHue;
};

// Strobe Wave Animation
class StrobeWaveAnimation : public Animation {
public:
    StrobeWaveAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        offset(0) {}
    
    void update() override {
        offset += beatsin16(90, 5, 20); // Fast motion

        uint8_t strobe = beatsin8(180, 0, 255); // Strobe effect

        for (int i = 0; i < numLeds; i++) {
            if (strobe > 200) {
                leds[i] = CHSV((i * 10) + offset, 255, brightness);
            } else if (strobe < 50) {
                leds[i] = CRGB::Black;
            } else {
                leds[i] = ColorFromPalette(HeatColors_p, (i * 5) + offset, brightness, LINEARBLEND);
            }
        }
    }
    
    const char* getName() override { return "Strobe Wave"; }
    
    AnimationCategory getCategory() override { return INTENSE; }

private:
    uint16_t offset;
};

// Energy Ripple Animation
class EnergyRippleAnimation : public Animation {
public:
    EnergyRippleAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        center(0),
        gHue(0) {}
    
    void update() override {
        EVERY_N_MILLISECONDS(50) {
            center = random16(numLeds);
        }

        EVERY_N_MILLISECONDS(20) {
            fadeToBlackBy(leds, numLeds, 30);
            uint8_t pulse = beatsin8(180, 50, brightness); // Fast pulse

            for (int i = 0; i < 20; i++) {
                leds[(center + i) % numLeds] += CHSV(gHue + i*12, 255, pulse);
                leds[(center - i + numLeds) % numLeds] += CHSV(gHue + i*12, 255, pulse);
            }
            gHue += 3;
        }
    }
    
    const char* getName() override { return "Energy Ripple"; }
    
    AnimationCategory getCategory() override { return INTENSE; }

private:
    uint8_t center;
    uint8_t gHue;
};

#endif // INTENSE_ANIMATIONS_H
