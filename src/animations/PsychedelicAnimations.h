/**
 * Psychedelic Animations
 * 
 * Collection of trippy, mind-bending animations
 */

#ifndef PSYCHEDELIC_ANIMATIONS_H
#define PSYCHEDELIC_ANIMATIONS_H

#include "AnimationBase.h"

// Two Sin Animation
class TwoSinAnimation : public Animation {
public:
    TwoSinAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        wavebright(128),
        thathue(80),
        thisrot(1),
        thatrot(0),
        allsat(255),
        thisspeed(1),
        thatspeed(1),
        allfreq(32),
        thisphase(0),
        thatphase(0),
        thiscutoff(96),
        thatcutoff(96) {}
    
    void update() override {
        thisphase += thisspeed; 
        thatphase += thatspeed;
        uint8_t thishue = 0; // Local variable to avoid conflict
        uint8_t thathue = thishue + 128; // 180 degrees out of phase
        
        thishue = thishue + thisrot;                                  // Hue rotation is fun for thiswave.
        thathue = thathue + thatrot;                                  // It's also fun for thatwave.

        for (int k=0; k<numLeds-1; k++) {
            int thisbright = qsuba(cubicwave8((k*allfreq)+thisphase), thiscutoff);      // qsub sets a minimum value called thiscutoff. If < thiscutoff, then bright = 0. Otherwise, bright = 128 (as defined in qsub)..
            int thatbright = qsuba(cubicwave8((k*allfreq)+128+thatphase), thatcutoff);  // This wave is 180 degrees out of phase (with the value of 128).

            leds[k] = CHSV(thishue, allsat, thisbright);                               // Assigning hues and brightness to the led array.
            leds[k] += CHSV(thathue, allsat, thatbright);
        }
    }
    
    const char* getName() override { return "Two Sin"; }
    
    AnimationCategory getCategory() override { return PSYCHEDELIC; }

private:
    uint8_t wavebright;
    uint8_t thathue;
    uint8_t thisrot;
    uint8_t thatrot;
    uint8_t allsat;
    int8_t thisspeed;
    int8_t thatspeed;
    uint8_t allfreq;
    int thisphase;
    int thatphase;
    uint8_t thiscutoff;
    uint8_t thatcutoff;
};

// Three Sin Two Animation
class ThreeSinTwoAnimation : public Animation {
public:
    ThreeSinTwoAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        wave1(0),
        wave2(0),
        wave3(0),
        inc1(2),
        inc2(1),
        inc3(-3),
        lvl1(80),
        lvl2(80),
        lvl3(80),
        mul1(5),
        mul2(8),
        mul3(7) {}
    
    void update() override {
        wave1 += inc1;
        wave2 += inc2;
        wave3 += inc3;
        
        for (int k=0; k<numLeds; k++) {
            leds[k].r = qsub8(sin8(mul1*k + wave1), lvl1);         // Another fixed frequency, variable phase sine wave with lowered level
            leds[k].g = qsub8(sin8(mul2*k + wave2), lvl2);         // A fixed frequency, variable phase sine wave with lowered level
            leds[k].b = qsub8(sin8(mul3*k + wave3), lvl3);         // A fixed frequency, variable phase sine wave with lowered level
        }
    }
    
    const char* getName() override { return "Three Sin Two"; }
    
    AnimationCategory getCategory() override { return PSYCHEDELIC; }

private:
    int wave1;
    int wave2;
    int wave3;
    uint8_t inc1;
    uint8_t inc2;
    uint8_t inc3;
    uint8_t lvl1;
    uint8_t lvl2;
    uint8_t lvl3;
    uint8_t mul1;
    uint8_t mul2;
    uint8_t mul3;
};

// Pop Fade Animation
class PopFadeAnimation : public Animation {
public:
    PopFadeAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        ranamount(50),
        boolcolours(false),
        numcolours(1),
        maxbar(1),
        fadeval(224) {
        // Initialize colors array
        colours[0] = 0x13b0f2; // Cyan
    }
    
    void update() override {
        unsigned long thiscolour;
        if (ranamount > numLeds) ranamount = numLeds;              // Make sure we're at least utilizing ALL the LED's.
        int idex = random16(0, ranamount);

        if (idex < numLeds) {                                      // Only the lowest probability twinkles will do.
            boolcolours ? thiscolour = random(0, 0xffffff) : thiscolour = colours[random16(0, numcolours)];
            int barlen = random16(1, maxbar);
            for (int i = 0; i < barlen; i++)
                if (idex+i < numLeds) leds[idex+i] = thiscolour;   // Make sure we don't overshoot the array.
        }
        nscale8(leds, numLeds, fadeval);                           // Fade the entire array.
    }
    
    const char* getName() override { return "Pop Fade"; }
    
    AnimationCategory getCategory() override { return PSYCHEDELIC; }

private:
    int ranamount;
    bool boolcolours;
    uint8_t numcolours;
    unsigned long colours[10];
    uint8_t maxbar;
    uint8_t fadeval;
};

// Plasma Effect Animation
class PlasmaEffectAnimation : public Animation {
public:
    PlasmaEffectAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds) {}
    
    void update() override {
        static unsigned long plasmaTime = 0;
        plasmaTime += 20;
        
        for (int i = 0; i < numLeds; i++) {
            int x = i * 10;
            int y = plasmaTime / 10;
            int colorIndex = sin8(x + y) + cos8(x - y);
            leds[i] = ColorFromPalette(RainbowColors_p, colorIndex, brightness);
        }
    }
    
    const char* getName() override { return "Plasma Effect"; }
    
    AnimationCategory getCategory() override { return PSYCHEDELIC; }
};

// Lava Lamp Animation
class LavaLampAnimation : public Animation {
public:
    LavaLampAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        x(0),
        lastBrightnessChange(0) {}
    
    void update() override {
        x += 10;

        for (int i = 0; i < numLeds; i++) {
            uint8_t noise = inoise8(i * 50, x);
            uint8_t hue = map(noise, 0, 255, 10, 30); // Orange-red range
            leds[i] = CHSV(hue, 255, noise);
        }
        
        // Occasionally change brightness for "lava lamp" feel
        unsigned long currentMillis = millis();
        if (currentMillis - lastBrightnessChange > 30000) { // 30 seconds
            FastLED.setBrightness(random8(MIN_BRIGHTNESS + 20, brightness - 20));
            lastBrightnessChange = currentMillis;
        }
    }
    
    const char* getName() override { return "Lava Lamp"; }
    
    AnimationCategory getCategory() override { return PSYCHEDELIC; }

private:
    uint16_t x;
    unsigned long lastBrightnessChange;
};

#endif // PSYCHEDELIC_ANIMATIONS_H
