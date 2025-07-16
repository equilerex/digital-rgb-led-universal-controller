#ifndef ANIMATION_BASE_H
#define ANIMATION_BASE_H

#include <FastLED.h>
#include <vector>

// Define qsuba macro if not already defined
#ifndef qsuba
#define qsuba(x, b) ((x > b) ? x - b : 0) // Unsigned subtraction macro
#endif

// Forward declaration of AnimationInfo
struct AnimationInfo;


// Global registry (extern here, def in .cpp)
extern std::vector<AnimationInfo> globalAnimationRegistry;

// Animation base class
class Animation {
public:
    Animation(CRGB* ledArray, uint16_t numLeds, const char* animName = "Unnamed")
        : leds(ledArray),
          numLeds(numLeds),
          name(animName),
          brightness(128),
          colorModifier(128) {}

    virtual ~Animation() {}

    virtual void update() = 0;
    virtual const char* getName() const { return name; }
    virtual void setBrightness(uint8_t value) { brightness = value; }
    virtual void setColorModifier(uint8_t value) { colorModifier = value; }

protected:
    CRGB* leds;
    uint16_t numLeds;
const char* name;
    uint8_t brightness;
    uint8_t colorModifier;

    void addGlitter(fract8 chanceOfGlitter) {
        if (random8() < chanceOfGlitter) {
            leds[random16(numLeds)] += CRGB::White;
        }
    }
};

// AnimationInfo struct for registry
#ifndef ANIMATION_INFO_DEFINED
#define ANIMATION_INFO_DEFINED
struct AnimationInfo {
    const char* name;
    Animation* (*createFn)(CRGB*, uint16_t);
};
#endif

// Template for automatic registration
template <typename T>
struct Registrar {
    Registrar(const char* name) {
        globalAnimationRegistry.push_back({
            name,
            [](CRGB* leds, uint16_t numLeds) -> Animation* {
                return new T(leds, numLeds);
            }
        });
    }
};

#endif // ANIMATION_BASE_H
