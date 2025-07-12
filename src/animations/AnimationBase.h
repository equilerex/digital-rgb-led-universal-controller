/**
 * Animation Base Class
 * 
 * Defines the interface for all animations
 */

#ifndef ANIMATION_BASE_H
#define ANIMATION_BASE_H

#include <FastLED.h>
#include "../config/Config.h"

// Define qsuba macro if not already defined
#ifndef qsuba
#define qsuba(x, b)  ((x>b)?x-b:0)  // Unsigned subtraction macro. if result <0, then => 0.
#endif

class Animation {
public:
    Animation(CRGB* ledArray, uint16_t numLeds) : 
        leds(ledArray), 
        numLeds(numLeds),
        brightness(DEFAULT_BRIGHTNESS),
        colorModifier(128) {}
    
    virtual ~Animation() {}
    
    // Run the animation - must be implemented by derived classes
    virtual void update() = 0;
    
    // Get the name of the animation
    virtual const char* getName() = 0;
    
    // Get the category of the animation
    virtual AnimationCategory getCategory() = 0;
    
    // Set the brightness for the animation
    virtual void setBrightness(uint8_t value) { brightness = value; }
    
    // Set the color modifier value
    virtual void setColorModifier(uint8_t value) { colorModifier = value; }

protected:
    CRGB* leds;              // Pointer to the LED array
    uint16_t numLeds;        // Number of LEDs
    uint8_t brightness;      // Brightness level
    uint8_t colorModifier;   // Color modifier value
    
    // Utility function for animations to add glitter effect
    void addGlitter(fract8 chanceOfGlitter) {
        if (random8() < chanceOfGlitter) {
            leds[random16(numLeds)] += CRGB::White;
        }
    }
};

#endif // ANIMATION_BASE_H
