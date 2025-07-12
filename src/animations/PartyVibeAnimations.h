/**
 * Party Vibe Animations
 * 
 * Collection of upbeat, party-friendly animations
 */

#ifndef PARTY_VIBE_ANIMATIONS_H
#define PARTY_VIBE_ANIMATIONS_H

#include "AnimationBase.h"

// Rainbow With Glitter Animation
class RainbowWithGlitterAnimation : public Animation {
public:
    RainbowWithGlitterAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        gHue(0) {}
    
    void update() override {
        // FastLED's built-in rainbow generator
        fill_rainbow(leds, numLeds, gHue, 7);
        
        // Add glitter
        addGlitter(80);
        
        EVERY_N_MILLISECONDS(20) {
            gHue++;
        }
    }
    
    const char* getName() override { return "Rainbow with Glitter"; }
    
    AnimationCategory getCategory() override { return PARTY_VIBE; }

private:
    uint8_t gHue;
};

// Rainbow March Animation
class RainbowMarchAnimation : public Animation {
public:
    RainbowMarchAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        thishue(0),
        deltahue(2) {}
    
    void update() override {
        EVERY_N_MILLISECONDS(10) {
            fill_rainbow(leds, numLeds, thishue, deltahue);
            thishue += 5; // Speed of color rotation
        }
    }
    
    const char* getName() override { return "Rainbow March"; }
    
    AnimationCategory getCategory() override { return PARTY_VIBE; }

private:
    uint8_t thishue;
    uint8_t deltahue;
};

// Confetti Animation
class ConfettiAnimation : public Animation {
public:
    ConfettiAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        gHue(0) {}
    
    void update() override {
        // Random colored speckles that blink in and fade smoothly
        fadeToBlackBy(leds, numLeds, 10);
        int pos = random16(numLeds);
        leds[pos] += CHSV(gHue + random8(64), 200, brightness);
        
        EVERY_N_MILLISECONDS(20) {
            gHue++;
        }
    }
    
    const char* getName() override { return "Confetti"; }
    
    AnimationCategory getCategory() override { return PARTY_VIBE; }

private:
    uint8_t gHue;
};

// BPM Animation
class BpmAnimation : public Animation {
public:
    BpmAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        gHue(0) {}
    
    void update() override {
        // Colored stripes pulsing at a defined Beats-Per-Minute
        uint8_t BeatsPerMinute = 62;
        CRGBPalette16 palette = PartyColors_p;
        uint8_t beat = beatsin8(BeatsPerMinute, 64, brightness);

        for(int i = 0; i < numLeds; i++) {
            leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
        }
        
        EVERY_N_MILLISECONDS(20) {
            gHue++;
        }
    }
    
    const char* getName() override { return "BPM"; }
    
    AnimationCategory getCategory() override { return PARTY_VIBE; }

private:
    uint8_t gHue;
};

// Twinkle Stars Animation
class TwinkleStarsAnimation : public Animation {
public:
    TwinkleStarsAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds) {}
    
    void update() override {
        // Fade existing stars
        fadeToBlackBy(leds, numLeds, 10);

        // Add new stars randomly
        int pos = random16(numLeds);
        leds[pos] += CHSV(random8(64, 192), 200, brightness);
    }
    
    const char* getName() override { return "Twinkle Stars"; }
    
    AnimationCategory getCategory() override { return PARTY_VIBE; }
};

// Color Waves Animation
class ColorWavesAnimation : public Animation {
public:
    ColorWavesAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        sPseudotime(0),
        sLastMillis(0),
        sHue16(0) {}
    
    void update() override {
        uint8_t brightdepth = scale8(beatsin88(341, 96, 224), brightness);
        uint16_t brightnessthetainc16 = beatsin88(203, (25 * 256), (40 * 256));
        uint8_t msmultiplier = beatsin88(147, 23, 60);

        uint16_t hue16 = sHue16;
        uint16_t hueinc16 = beatsin88(113, 1, 3000);

        uint16_t ms = millis();
        uint16_t deltams = ms - sLastMillis;
        sLastMillis = ms;
        sPseudotime += deltams * msmultiplier;
        sHue16 += deltams * beatsin88(400, 5, 9);
        uint16_t brightnesstheta16 = sPseudotime;

        for (uint16_t i = 0; i < numLeds; i++) {
            hue16 += hueinc16;
            uint8_t hue8 = hue16 / 256;
            uint16_t h16_128 = hue16 >> 7;
            if (h16_128 & 0x100) {
                hue8 = 255 - hue8;
            }

            uint8_t bright = scale8(qadd8(brightdepth, sin8(brightnesstheta16)), brightness);
            brightnesstheta16 += brightnessthetainc16;

            leds[i] = CHSV(hue8, 255, bright);
        }
    }
    
    const char* getName() override { return "Color Waves"; }
    
    AnimationCategory getCategory() override { return PARTY_VIBE; }

private:
    uint16_t sPseudotime;
    uint16_t sLastMillis;
    uint16_t sHue16;
};

#endif // PARTY_VIBE_ANIMATIONS_H
