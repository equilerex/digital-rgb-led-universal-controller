/**
 * Crazy Animations
 * 
 * Collection of wild, unpredictable animations
 */

#ifndef CRAZY_ANIMATIONS_H
#define CRAZY_ANIMATIONS_H

#include "AnimationBase.h"

// Juggle Animation
class JuggleAnimation : public Animation {
public:
    JuggleAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds) {}
    
    void update() override {
        // Eight colored dots, weaving in and out of sync with each other
        fadeToBlackBy(leds, numLeds, 20);
        byte dothue = 0;

        for(int i = 0; i < 8; i++) {
            leds[beatsin16(i+7, 0, numLeds-1)] |= CHSV(dothue, 200, brightness);
            dothue += 32;
        }
    }
    
    const char* getName() override { return "Juggle"; }
    
    AnimationCategory getCategory() override { return CRAZY; }
};

// Sinelon Animation
class SinelonAnimation : public Animation {
public:
    SinelonAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        gHue(0) {}
    
    void update() override {
        // A colored dot sweeping back and forth, with fading trails
        fadeToBlackBy(leds, numLeds, 20);
        int pos = beatsin16(13, 0, numLeds-1);
        leds[pos] += CHSV(gHue, 255, brightness);
        
        EVERY_N_MILLISECONDS(20) {
            gHue++;
        }
    }
    
    const char* getName() override { return "Sinelon"; }
    
    AnimationCategory getCategory() override { return CRAZY; }

private:
    uint8_t gHue;
};

// Noise16 Animation
class Noise16Animation : public Animation {
public:
    Noise16Animation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        x(0),
        hue_time(0),
        octaves(2),
        hue_octaves(3),
        xscale(57771),
        hxy(43213),
        hue_scale(20),
        hue_speed(1),
        x_speed(0),
        hxyinc(3) {}
    
    void update() override {
        // Random settings for a truly crazy effect
        EVERY_N_SECONDS(5) {
            hxyinc = random16(1, 15); 
            octaves = random16(1, 3); 
            hue_octaves = random16(1, 5); 
            hue_scale = random16(10, 50);  
            x = random16(); 
            xscale = random16(); 
            hxy = random16(); 
            hue_time = random16(); 
            hue_speed = random16(1, 3); 
            x_speed = random16(1, 30);
        }

        fill_noise16(leds, numLeds, octaves, x, xscale, hue_octaves, hxy, hue_scale, hue_time);

        for (int i=0; i<numLeds; i++) {
            leds[i].g = 0; // Filter out green
        }
        
        // Move through the noise
        hue_time += hue_speed;
        x += x_speed;
    }
    
    const char* getName() override { return "Noise16"; }
    
    AnimationCategory getCategory() override { return CRAZY; }

private:
    uint32_t x, hue_time;
    uint8_t octaves;
    uint8_t hue_octaves;
    int xscale;
    uint32_t hxy;
    int hue_scale;
    uint8_t hue_speed;
    uint8_t x_speed;
    int8_t hxyinc;
};

// Meteor Rain Animation
class MeteorRainAnimation : public Animation {
public:
    MeteorRainAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        meteorPosition(0),
        lastMeteorUpdate(0) {}
    
    void update() override {
        unsigned long currentMillis = millis();
        
        // Update meteor position only every 30ms for smooth animation without blocking
        if (currentMillis - lastMeteorUpdate >= 30) {
            lastMeteorUpdate = currentMillis;

            // Fade all pixels
            for (int j = 0; j < numLeds; j++) {
                if (random8() > 64) {
                    leds[j].fadeToBlackBy(64);
                }
            }
            
            // Draw meteor at current position
            for (int j = 0; j < 10; j++) {
                if ((meteorPosition - j < numLeds) && (meteorPosition - j >= 0)) {
                    leds[meteorPosition - j] = CRGB(255, 255, 255);
                }
            }
            
            // Advance the meteor position
            meteorPosition++;

            // Reset position when meteor has moved completely off-screen
            if (meteorPosition >= numLeds + 10) {
                meteorPosition = 0;
            }
        }
    }
    
    const char* getName() override { return "Meteor Rain"; }
    
    AnimationCategory getCategory() override { return CRAZY; }

private:
    int meteorPosition;
    unsigned long lastMeteorUpdate;
};

// Fire Effect Animation
class FireEffectAnimation : public Animation {
public:
    FireEffectAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds) {
        // Initialize heat array dynamically
        heat = new byte[numLeds]();
    }
    
    ~FireEffectAnimation() {
        // Clean up dynamically allocated memory
        if (heat) delete[] heat;
    }
    
    void update() override {
        // Re-allocate heat array if numLeds has changed
        if (!heat || currentSize != numLeds) {
            if (heat) delete[] heat;
            heat = new byte[numLeds]();
            currentSize = numLeds;
        }
        
        // Simulate cooling
        for (int i = 0; i < numLeds; i++) {
            heat[i] = qsub8(heat[i], random8(0, ((55 * 10) / numLeds) + 2));
        }

        // Heat diffusion
        for (int k = numLeds - 1; k >= 2; k--) {
            heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
        }

        // Randomly ignite new sparks
        if (random8() < 120) {
            int y = random8(7);
            heat[y] = qadd8(heat[y], random8(160, 255));
        }

        // Map heat to colors
        for (int j = 0; j < numLeds; j++) {
            byte colorIndex = scale8(heat[j], brightness);
            leds[j] = ColorFromPalette(HeatColors_p, colorIndex);
        }
    }
    
    const char* getName() override { return "Fire Effect"; }
    
    AnimationCategory getCategory() override { return CRAZY; }

private:
    byte* heat = nullptr;
    uint16_t currentSize = 0;
};

// Comet Effect Animation
class CometEffectAnimation : public Animation {
public:
    CometEffectAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        pos(0),
        direction(1) {}
    
    void update() override {
        fadeToBlackBy(leds, numLeds, 20);
        leds[pos] = CHSV(gHue, 255, brightness);

        pos += direction;
        if (pos >= numLeds - 1 || pos <= 0) {
            direction *= -1;
            pos = constrain(pos, 0, numLeds - 1); // Ensure pos stays within bounds
        }
        
        EVERY_N_MILLISECONDS(20) {
            gHue++;
        }
    }
    
    const char* getName() override { return "Comet Effect"; }
    
    AnimationCategory getCategory() override { return CRAZY; }

private:
    int pos;
    int direction;
    uint8_t gHue = 0;
};

#endif // CRAZY_ANIMATIONS_H
