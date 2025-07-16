#include <Arduino.h>
#include <FastLED.h>
#include <fl/colorutils.h>
#include <fl/colorutils_misc.h>
#include "../AnimationBase.h"
#include "../../config/Config.h"


class RainbowWithGlitterAnimation : public Animation {
private:
    uint8_t gHue;
    void addGlitter(fract8 chanceOfGlitter) {
        if (random8() < chanceOfGlitter) {
            leds[random16(numLeds)] += CRGB::White;
        }
    }
public:
    RainbowWithGlitterAnimation(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "Rainbow with Glitter"), gHue(0) {}
    void update() override {
        fill_rainbow(leds, numLeds, gHue, 7);
        addGlitter(80);
        EVERY_N_MILLISECONDS(20) { gHue++; }
    }
};
static Registrar<RainbowWithGlitterAnimation> rainbowWithGlitterRegistrar("Rainbow with Glitter");

class LiquidDreamAnimation : public Animation {
private:
    // Dream state variables
    struct DreamState {
        float hue;             // Main hue (0-255)
        float hueDrift;        // Hue drift speed
        float pulseCenter;     // Center of breathing pulse (0-1)
        float pulseWidth;      // Width of pulse effect
        float energyFlow;      // Slow energy movement
        float phaseShift;      // Phase offset for waves
    } state;

    // Palette morphing system
    CRGBPalette256 currentPalette;
    CRGBPalette256 targetPalette;
    float paletteBlendProgress;

    // Wave parameters
    struct Wave {
        float amplitude;
        float frequency;
        float speed;
        float phase;
    } waveA, waveB, waveC;

    // Timeless helpers
    float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }

    float smoothstep(float edge0, float edge1, float x) {
        x = constrain((x - edge0) / (edge1 - edge0), 0.0, 1.0);
        return x * x * (3 - 2 * x);
    }

    // Create dreamy palette transitions
    void evolvePalette() {
        EVERY_N_MINUTES(7) {
            // Generate new target palette
            CHSV baseColor = CHSV(random8(), 180 + random8(40), 200);
            targetPalette = CRGBPalette256(
                baseColor,
                CHSV((baseColor.hue + 85) % 255, 200, 255),
                CHSV((baseColor.hue + 170) % 255, 160, 180),
                CHSV((baseColor.hue + 200) % 255, 220, 200)
            );
            paletteBlendProgress = 0.0;
        }

        // Gradually blend palettes over 90 seconds
        if(paletteBlendProgress < 1.0) {
            paletteBlendProgress += 0.0001;  // ~90s transition
            for(int i = 0; i < 256; i++) {
                currentPalette[i] = blend(
                    currentPalette[i],
                    targetPalette[i],
                    (uint8_t)(255 * smoothstep(0, 1, paletteBlendProgress))
                );
            }
        }
    }

    // Update dream state with slow drifts
    void evolveDreamState() {
        // Hours-long hue cycle (1 full cycle per ~2 hours)
        state.hue = fmod(state.hue + 0.0001, 256.0);

        // Slowly drifting parameters
        state.hueDrift = sin8(millis() / 42000.0) / 512.0;
        state.pulseCenter = 0.5 + 0.3 * sin8(millis() / 38000.0) / 255.0;
        state.pulseWidth = 0.2 + 0.1 * sin8(millis() / 57000.0) / 255.0;
        state.energyFlow = fmod(state.energyFlow + 0.00003, 1.0);
        state.phaseShift = sin8(millis() / 29000.0) / 255.0;

        // Wave evolution (changes every few minutes)
        EVERY_N_MINUTES(3) {
            waveA.amplitude = 0.3 + random8(20)/100.0;
            waveA.frequency = 0.01 + random8(10)/1000.0;
            waveA.speed = 0.00002 + random8(10)/1000000.0;

            waveB.amplitude = 0.2 + random8(15)/100.0;
            waveB.frequency = 0.015 + random8(15)/1000.0;
            waveB.speed = -0.00003 + random8(10)/1000000.0;

            waveC.amplitude = 0.15 + random8(10)/100.0;
            waveC.frequency = 0.025 + random8(20)/1000.0;
            waveC.speed = 0.000015 + random8(5)/1000000.0;
        }
    }

    // Calculate dreamy pixel value
    CRGB dreamPixel(uint16_t pos) {
        const float position = (float)pos / numLeds;

        // Three overlapping wave functions
        const float wave1 = waveA.amplitude *
            sin(2 * PI * (waveA.frequency * pos + millis() * waveA.speed));

        const float wave2 = waveB.amplitude *
            sin(2 * PI * (waveB.frequency * pos + millis() * waveB.speed + state.phaseShift));

        const float wave3 = waveC.amplitude *
            cos(2 * PI * (waveC.frequency * pos + millis() * waveC.speed));

        // Combined waveform value
        float waveValue = (wave1 + wave2 + wave3) / 3.0;

        // Breathing pulse effect
        const float pulse = exp(-pow((position - state.pulseCenter) / state.pulseWidth, 2));

        // Final position in the color spectrum
        const float huePosition = fmod(
            state.hue +
            state.hueDrift * pos +
            waveValue * 20 +
            pulse * 30 +
            state.energyFlow * 60,
            256.0
        );

        // Get color from smoothly evolving palette
        return ColorFromPalette(
            currentPalette,
            (uint8_t)huePosition,
            200 + 55 * pulse,  // Brighter at pulse center
            LINEARBLEND
        );
    }

public:
    LiquidDreamAnimation(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "Liquid Dream") {

        // Initial dream state
        state = {
            .hue = 0,
            .hueDrift = 0.001,
            .pulseCenter = 0.5,
            .pulseWidth = 0.3,
            .energyFlow = 0,
            .phaseShift = 0
        };

        // Initialize waves
        waveA = {0.3, 0.01, 0.00002, 0};
        waveB = {0.2, 0.015, -0.00003, 0};
        waveC = {0.15, 0.025, 0.000015, 0};

        // Create initial palette
        currentPalette = CRGBPalette256(
            CHSV(30, 200, 200),
            CHSV(90, 220, 220),
            CHSV(160, 180, 180),
            CHSV(210, 200, 200)
        );
        targetPalette = currentPalette;
        paletteBlendProgress = 1.0;
    }

    void update() override {
        // Update everything slowly
        evolveDreamState();
        evolvePalette();

        // Render each pixel with dreamy calculations
        for(int i = 0; i < numLeds; i++) {
            leds[i] = dreamPixel(i);
        }

        // Apply subtle blur for extra smoothness
        EVERY_N_SECONDS(5) {
            blur1d(leds, numLeds, 32);
        }
    }
};
static Registrar<LiquidDreamAnimation> liquidDreamRegistrar("Liquid Dream");

class DreamwaveAuroraAnimation : public Animation {
private:
    uint16_t x;
    uint16_t scale;
    CRGBPalette16 currentPalette;
    CRGBPalette16 targetPalette;
    uint8_t colorLoop;
    uint8_t brightness;

public:
    DreamwaveAuroraAnimation(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "Dreamwave Aurora"), x(0), scale(50), colorLoop(0), brightness(180) {
        currentPalette = PartyColors_p;
        targetPalette = OceanColors_p;
    }

    void update() override {
        // Smooth fade to next palette
        EVERY_N_MILLISECONDS(100) {
            nblendPaletteTowardPalette(currentPalette, targetPalette, 4);
        }

        // Occasionally switch palette
        EVERY_N_SECONDS(15) {
            switch (random8(4)) {
                case 0: targetPalette = LavaColors_p; break;
                case 1: targetPalette = CloudColors_p; break;
                case 2: targetPalette = OceanColors_p; break;
                case 3: targetPalette = ForestColors_p; break;
            }
        }

        // Fill strip with noise-driven colors
        for (uint16_t i = 0; i < numLeds; i++) {
            uint8_t noise = inoise8(i * scale, x);
            uint8_t index = noise + colorLoop;
            leds[i] = ColorFromPalette(currentPalette, index, brightness);
        }

        // Add subtle glitter on top
        if (random8() < 40) {
            leds[random16(numLeds)] += CRGB::White;
        }

        // Motion over time
        x += 1;
        colorLoop += 1;
    }
};
static Registrar<DreamwaveAuroraAnimation> dreamwaveAuroraRegistrar("Dreamwave Aurora");

class BreathingAnimation : public Animation {
private:
    float breath;
public:
    BreathingAnimation(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "Breathing"), breath(0) {}
    void update() override {
        if (!leds || numLeds == 0) return;
        breath += 0.01;
        if (breath > TWO_PI) breath = 0;
        uint8_t breathBrightness = beatsin8(6, MIN_BRIGHTNESS, constrain(brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS), 0, 0);
        fill_solid(leds, numLeds, CHSV(140, 150, breathBrightness));
    }
};
static Registrar<BreathingAnimation> breathingRegistrar("Breathing");

class AuroraAnimation : public Animation {
private:
    uint16_t t;
    CRGBPalette16 auroraPalette;
public:
    AuroraAnimation(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "Aurora"), t(0) {
        auroraPalette = CRGBPalette16(
            CRGB(0, 100, 0), CRGB(0, 150, 0), CRGB(0, 200, 0), CRGB(0, 255, 0),
            CRGB(100, 0, 200), CRGB(150, 0, 255), CRGB(200, 0, 255), CRGB(255, 0, 255),
            CRGB(0, 50, 150), CRGB(0, 100, 200), CRGB(0, 150, 255), CRGB(0, 200, 255),
            CRGB(0, 0, 0), CRGB(0, 0, 0), CRGB(0, 0, 0), CRGB(0, 0, 0)
        );
    }
    void update() override {
        EVERY_N_MILLISECONDS(50) { t++; }
        fadeToBlackBy(leds, numLeds, 10);
        for (int i = 0; i < numLeds; i++) {
            uint8_t noise = inoise8(i * 20, t);
            uint8_t index = map(noise, 0, 255, 0, 255);
            uint8_t bright = map(sin8(noise), 0, 255, 50, 150);
            leds[i] = ColorFromPalette(auroraPalette, index, bright, LINEARBLEND);
        }
    }
};
static Registrar<AuroraAnimation> auroraRegistrar("Aurora");

// ---------------------- Lava, Cyber, Aurora Storm -----------------------------
class LavaCyberAuroraStorm : public Animation {
private:
    uint8_t gHue;
    uint16_t noiseScale;
    uint8_t speed;
    CRGBPalette16 lavaPalette;
    CRGBPalette16 cyberPalette;
    CRGBPalette16 auroraPalette;
    uint8_t mood;
    unsigned long moodChangeTime;
    uint8_t thunderChance;
public:
    LavaCyberAuroraStorm(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "LavaCyberAuroraStorm"), gHue(0), noiseScale(20), speed(5), mood(0), moodChangeTime(0), thunderChance(5) {
        lavaPalette = CRGBPalette16(CRGB::Black, CRGB::Red, CRGB::Orange, CRGB::Yellow);
        cyberPalette = CRGBPalette16(CRGB::Black, CRGB::HotPink, CRGB::Purple, CRGB::Cyan);
        auroraPalette = CRGBPalette16(CRGB::Black, CRGB::Green, CRGB::Teal, CRGB::Purple);
    }
    void update() override {
        EVERY_N_SECONDS(random8(30, 60)) { mood = random8(3); } // Shift moods
        EVERY_N_MILLISECONDS(50) { gHue++; }
        fadeToBlackBy(leds, numLeds, 10); // Smooth fade

        for (int i = 0; i < numLeds; i++) {
            uint8_t noise = inoise8(i * noiseScale, millis() / speed + gHue);
            uint8_t bright = sin8(noise) / 2; // Low to medium
            CRGB color;
            switch (mood) {
                case 0: color = ColorFromPalette(lavaPalette, noise + gHue, bright); break;
                case 1: color = ColorFromPalette(cyberPalette, noise + gHue, bright); break;
                case 2: color = ColorFromPalette(auroraPalette, noise + gHue, bright); break;
            }
            nblend(leds[i], color, 128); // Blend layers
        }

        // Sparkles
        if (random8() < 20) {
            leds[random16(numLeds)] += CRGB::White; // Sprinkle sparkles
            leds[random16(numLeds)].fadeLightBy(200);
        }

        // Thunderstorm flash
        if (random8() < thunderChance) {
            fill_solid(leds, numLeds, CRGB::White);
            EVERY_N_MILLISECONDS(100) { fadeToBlackBy(leds, numLeds, 200); } // Quick flash fade
        }
    }
};
static Registrar<LavaCyberAuroraStorm> lavaCyberAuroraStormRegistrar("LavaCyberAuroraStorm");

class MoonlightAnimation : public Animation {
private:
    uint8_t starPositions[10];
    uint8_t starBright[10];
    uint8_t shootingPos;
    uint8_t shootingBright;
    bool shootingActive;
    CRGBPalette16 nightPalette;
public:
    MoonlightAnimation(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "Moonlight"), shootingPos(0), shootingBright(0), shootingActive(false) {
        nightPalette = CRGBPalette16(
            CRGB(0, 0, 20), CRGB(0, 0, 40), CRGB(0, 0, 60), CRGB(10, 0, 80),
            CRGB(20, 0, 100), CRGB(30, 0, 120), CRGB(40, 0, 140), CRGB(50, 0, 160),
            CRGB(60, 0, 180), CRGB(70, 0, 200), CRGB(80, 0, 220), CRGB(90, 0, 240),
            CRGB(100, 0, 255), CRGB(120, 20, 255), CRGB(140, 40, 255), CRGB(160, 60, 255)
        );
        for (uint8_t i = 0; i < 10; i++) {
            starPositions[i] = random16(numLeds);
            starBright[i] = random8(50, 100);
        }
    }
    void update() override {
        fadeToBlackBy(leds, numLeds, 5); // Subtle fade

        // Background noise clouds
        for (int i = 0; i < numLeds; i++) {
            uint8_t index = inoise8(i * 10, millis() / 100);
            uint8_t bright = qsuba(index, 200); // Low brightness
            leds[i] = ColorFromPalette(nightPalette, index, bright);
        }

        // Twinkling stars
        for (uint8_t i = 0; i < 10; i++) {
            starBright[i] = beatsin8(10 + i, 50, 150); // Varying twinkle
            leds[starPositions[i]] = CRGB::White;
            leds[starPositions[i]].fadeLightBy(255 - starBright[i]);
        }

        // Occasional shooting star
        EVERY_N_SECONDS(10) {
            if (!shootingActive && random8() < 50) {
                shootingActive = true;
                shootingPos = 0;
                shootingBright = 255;
            }
        }
        if (shootingActive) {
            if (shootingPos < numLeds) {
                leds[shootingPos] = CRGB::White;
                leds[shootingPos].fadeLightBy(255 - shootingBright);
                shootingPos += 2; // Speed
                shootingBright -= 20; // Fade tail
            } else {
                shootingActive = false;
            }
        }
    }
};
static Registrar<MoonlightAnimation> moonlightRegistrar("Moonlight");

class ForestCanopyAnimation : public Animation {
private:
    uint16_t t;
public:
    ForestCanopyAnimation(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "Forest Canopy"), t(0) {}
    void update() override {
        t += 5;
        for (int i = 0; i < numLeds; i++) {
            uint8_t green = inoise8(i * 20, t);
            uint8_t blue = inoise8(i * 20 + 10000, t);
            leds[i] = CRGB(0, green/2, blue/3);
        }
        fadeLightBy(leds, numLeds, 5);
    }
};
static Registrar<ForestCanopyAnimation> forestCanopyRegistrar("Forest Canopy");

class GentlePulseWaveAnimation : public Animation {
private:
    uint8_t gHue;
public:
    GentlePulseWaveAnimation(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "Gentle Pulse Wave"), gHue(0) {}
    void update() override {
        EVERY_N_MILLISECONDS(235) { gHue++; }
        fadeToBlackBy(leds, numLeds, 20);
        uint8_t pos = beatsin8(5, 0, numLeds - 1);
        uint8_t brightness = beatsin8(10, 50, 150);
        leds[pos] = CHSV(gHue, 200, brightness);
    }
};
static Registrar<GentlePulseWaveAnimation> gentlePulseWaveRegistrar("Gentle Pulse Wave");

class TwilightRippleAnimation : public Animation {
private:
    uint8_t gHue;
    CRGBPalette16 currentPalette;
public:
    TwilightRippleAnimation(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "Twilight Ripple"), gHue(0),
          currentPalette(CRGBPalette16(
              CRGB::Blue, CRGB::Purple, CRGB(0, 128, 128), CRGB(0, 100, 0),
              CRGB(0, 0, 128), CRGB(75, 0, 130), CRGB::Cyan, CRGB(34, 139, 34),
              CRGB::Blue, CRGB::Purple, CRGB(0, 128, 128), CRGB(0, 100, 0),
              CRGB(0, 0, 128), CRGB(75, 0, 130), CRGB::Cyan, CRGB(34, 139, 34))) {}
    void update() override {
        EVERY_N_MILLISECONDS(235) { gHue++; }
        fadeToBlackBy(leds, numLeds, 10);
        for (uint16_t i = 0; i < numLeds; i++) {
            uint8_t index = inoise8(i * 20, millis() / 50) + gHue;
            uint8_t brightness = qsuba(inoise8(i * 10, millis() / 40), 100);
            leds[i] = ColorFromPalette(currentPalette, index, brightness);
        }
    }
};
static Registrar<TwilightRippleAnimation> twilightRippleRegistrar("Twilight Ripple");

class StarlitDriftAnimation : public Animation {
private:
    uint8_t gHue;
    CRGBPalette16 currentPalette;
public:
    StarlitDriftAnimation(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "Starlit Drift"), gHue(0),
          currentPalette(CRGBPalette16(
              CRGB::Blue, CRGB::Purple, CRGB(70, 130, 180), CRGB(200, 255, 255),
              CRGB(0, 0, 128), CRGB(75, 0, 130), CRGB(135, 206, 235), CRGB::Gray,
              CRGB::Blue, CRGB::Purple, CRGB(70, 130, 180), CRGB(200, 255, 255),
              CRGB(0, 0, 128), CRGB(75, 0, 130), CRGB(135, 206, 235), CRGB::Gray)) {}
    void update() override {
        EVERY_N_MILLISECONDS(235) { gHue++; }
        fadeToBlackBy(leds, numLeds, 15);
        if (random8() < 20) {
            uint16_t pos = random16(numLeds);
            uint8_t brightness = beatsin8(8, 80, 120);
            leds[pos] = ColorFromPalette(currentPalette, gHue + random8(20), brightness);
        }
    }
};
static Registrar<StarlitDriftAnimation> starlitDriftRegistrar("Starlit Drift");


class EtherealPlasmaDrift : public Animation {
private:
    uint8_t gHue;              // Global hue for slow drift
    uint16_t noiseX;           // X coord for noise field
    uint16_t noiseY;           // Y coord for noise field (time-based)
    uint8_t pulseBeat;         // Precomputed pulse for efficiency
    CRGBPalette16 currentPalette;  // Fixed palette, morphed in-place

public:
    EtherealPlasmaDrift(CRGB* ledArray, uint16_t numLeds)
        : Animation(ledArray, numLeds, "Ethereal Plasma Drift") {
        // Init palette to soft blues/purples for start
        fill_gradient(currentPalette.entries, 16, CHSV(160, 255, 255), CHSV(220, 200, 180), fl::LONGEST_HUES);
    }

    void update() override {
        // Efficient timing: Increment counters once
        EVERY_N_MILLISECONDS(30) { gHue++; noiseY += 2; }  // Slow evolution
        EVERY_N_MILLISECONDS(50) { noiseX++; }             // Asymmetric noise motion
        pulseBeat = beatsin8(5, 64, 192);                  // Gentle pulse, precompute

        // Morph palette subtly for non-repetition (in-place, no alloc)
        static CRGB targetColor = CHSV(gHue + 128, 180, 200);  // Evolving target
        EVERY_N_MILLISECONDS(100) {
            for (uint8_t i = 0; i < 16; i++) {
                nblend(currentPalette[i], targetColor, 8);
            }
            targetColor = CHSV(gHue + random8(64), 180 + random8(76), 150 + random8(106));
        }

        // Core plasma: Perlin noise mapped to palette, O(N) loop
        for (uint16_t i = 0; i < numLeds; i++) {
            // 2D noise for depth, scaled to avoid repetition
            uint8_t noise = inoise8(i * 12 + noiseX, noiseY + sin8(i * 3) * 2);
            uint8_t bright = qadd8(noise / 2, pulseBeat / 2);  // Layer pulse, clamp to 255
            leds[i] = ColorFromPalette(currentPalette, noise + gHue / 2, bright, LINEARBLEND);
        }

        // Subtle glitter trails: Sparse, fixed probability, no extra arrays
        if (random8() < 20) {
            uint16_t pos = random16(numLeds);
            leds[pos] += CRGB(32, 32, 32);  // Soft white add, overflows safe in CRGB
            if (pos > 0) leds[pos - 1].fadeLightBy(64);  // Trail fade
        }

        // Final blur for smoothness: O(N), diffuses repetition
        blur1d(leds, numLeds, 32 + (pulseBeat / 8));  // Dynamic blur tied to pulse
    }
};

static Registrar<EtherealPlasmaDrift> etherealPlasmaDriftRegistrator("Ethereal Plasma Drift");