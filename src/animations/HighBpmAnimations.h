/**
 * High BPM Animations
 * 
 * Collection of fast-paced animations with beat elements
 */

#ifndef HIGH_BPM_ANIMATIONS_H
#define HIGH_BPM_ANIMATIONS_H

#include "AnimationBase.h"

// Heartbeat Animation
class HeartbeatAnimation : public Animation {
public:
    HeartbeatAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds) {}
    
    void update() override {
        uint8_t pulse = beatsin8(40, MIN_BRIGHTNESS, brightness); // ~72 BPM

        CRGB color = CRGB::Red;
        color.fadeToBlackBy(220 - pulse);
        fill_solid(leds, numLeds, color);
    }
    
    const char* getName() override { return "Heartbeat"; }
    
    AnimationCategory getCategory() override { return HIGH_BPM; }
};

// Strobe Pulse Animation
class StrobePulseAnimation : public Animation {
public:
    StrobePulseAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds) {}
    
    void update() override {
        uint8_t pulse = beatsin8(120, 50, 255); // 120 BPM

        if (pulse > 200) {
            fill_solid(leds, numLeds, CHSV(random8(), 255, brightness));
        } else {
            fill_solid(leds, numLeds, CRGB::Black);
        }
    }
    
    const char* getName() override { return "Strobe Pulse"; }
    
    AnimationCategory getCategory() override { return HIGH_BPM; }
};

// Beat Scanner Animation
class BeatScannerAnimation : public Animation {
public:
    BeatScannerAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        pos(0),
        gHue(0) {}
    
    void update() override {
        EVERY_N_MILLISECONDS(20) {
            pos = beatsin16(60, 0, numLeds-1); // 60 BPM

            fadeToBlackBy(leds, numLeds, 50);
            leds[pos] += CHSV(gHue, 255, brightness);
            leds[(numLeds-1)-pos] += CHSV(gHue+128, 255, brightness);

            gHue += 2;
        }
    }
    
    const char* getName() override { return "Beat Scanner"; }
    
    AnimationCategory getCategory() override { return HIGH_BPM; }

private:
    uint8_t pos;
    uint8_t gHue;
};

// Color Slam Animation
class ColorSlamAnimation : public Animation {
public:
    ColorSlamAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        lastBeat(0) {}
    
    void update() override {
        uint8_t beat = beatsin8(60, 0, 100); // 60 BPM

        if (beat < 10 && lastBeat >= 10) {
            // Beat hit - flash new color
            fill_solid(leds, numLeds, CHSV(random8(), 255, brightness));
        } else {
            // Decay after hit
            fadeToBlackBy(leds, numLeds, 30);
        }
        lastBeat = beat;
    }
    
    const char* getName() override { return "Color Slam"; }
    
    AnimationCategory getCategory() override { return HIGH_BPM; }

private:
    uint8_t lastBeat;
};

// Beat Drop Animation
class BeatDropAnimation : public Animation {
public:
    BeatDropAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        dropCounter(0),
        gHue(0) {}
    
    void update() override {
        EVERY_N_MILLISECONDS(50) {
            if (dropCounter == 0) {
                // Build anticipation
                fill_solid(leds, numLeds, CHSV(gHue, 255, beatsin8(30, 50, 150)));
            } else if (dropCounter < 10) {
                // Flash white during drop
                fill_solid(leds, numLeds, CRGB(255,255,255));
            } else {
                // Post-drop color explosion
                fadeToBlackBy(leds, numLeds, 30);
                for (int i = 0; i < 5; i++) {
                    uint16_t pos = random16(numLeds);
                    leds[pos] = CHSV(gHue + random16(64), 255, brightness);
                }
            }

            dropCounter = (dropCounter + 1) % 30;
            if (dropCounter == 0) gHue += 64;
        }
    }
    
    const char* getName() override { return "Beat Drop"; }
    
    AnimationCategory getCategory() override { return HIGH_BPM; }

private:
    uint8_t dropCounter;
    uint8_t gHue;
};

#endif // HIGH_BPM_ANIMATIONS_H
