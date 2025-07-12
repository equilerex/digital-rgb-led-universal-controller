/**
 * Solid Color Animations
 * 
 * Collection of simple solid color animations
 */

#ifndef SOLID_COLOR_ANIMATIONS_H
#define SOLID_COLOR_ANIMATIONS_H

#include "AnimationBase.h"

// Static Rainbow Animation
class RainbowAnimation : public Animation {
public:
    RainbowAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        gHue(0) {}
    
    void update() override {
        fill_rainbow(leds, numLeds, gHue, 7);
        
        EVERY_N_MILLISECONDS(20) {
            gHue++;
        }
    }
    
    const char* getName() override { return "Rainbow"; }
    
    AnimationCategory getCategory() override { return SOLID_COLORS; }

private:
    uint8_t gHue;
};

// Red-Purple-Blue Animation
class RedPurpleBlueAnimation : public Animation {
public:
    RedPurpleBlueAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        hue(255) {}
    
    void update() override {
        hue = colorModifier; // Use colorModifier as the hue control
        fill_solid(leds, numLeds, CRGB(hue, 0, 255-hue));
    }
    
    const char* getName() override { return "Red Purple Blue"; }
    
    AnimationCategory getCategory() override { return SOLID_COLORS; }

private:
    uint8_t hue;
};

// Green-Yellow-Red Animation
class GreenYellowRedAnimation : public Animation {
public:
    GreenYellowRedAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        hue(255) {}
    
    void update() override {
        hue = colorModifier; // Use colorModifier as the hue control
        fill_solid(leds, numLeds, CRGB(255-hue, hue, 0));
    }
    
    const char* getName() override { return "Green Yellow Red"; }
    
    AnimationCategory getCategory() override { return SOLID_COLORS; }

private:
    uint8_t hue;
};

// Green-Blue Animation
class GreenBlueAnimation : public Animation {
public:
    GreenBlueAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds),
        hue(255) {}
    
    void update() override {
        hue = colorModifier; // Use colorModifier as the hue control
        fill_solid(leds, numLeds, CRGB(0, 255-hue, hue));
    }
    
    const char* getName() override { return "Green Blue"; }
    
    AnimationCategory getCategory() override { return SOLID_COLORS; }

private:
    uint8_t hue;
};

// Orange Animation
class OrangeAnimation : public Animation {
public:
    OrangeAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds) {}
    
    void update() override {
        fill_solid(leds, numLeds, CRGB(255, 100, 0));
    }
    
    const char* getName() override { return "Orange"; }
    
    AnimationCategory getCategory() override { return SOLID_COLORS; }
};

// Purple Animation
class PurpleAnimation : public Animation {
public:
    PurpleAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds) {}
    
    void update() override {
        fill_solid(leds, numLeds, CRGB(255, 0, 255));
    }
    
    const char* getName() override { return "Purple"; }
    
    AnimationCategory getCategory() override { return SOLID_COLORS; }
};

#endif // SOLID_COLOR_ANIMATIONS_H
