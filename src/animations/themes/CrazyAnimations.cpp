#include <Arduino.h>
#include <FastLED.h>
#include "../AnimationBase.h"
#include "../../config/Config.h"
#include <fl/colorutils.h>
#include <fl/colorutils_misc.h>

class CosmicBeastOfManyMoods : public Animation {
private:
    uint8_t gHue;
    uint16_t t;
    uint32_t seed;
    CRGBPalette16 currentPalette;
    CRGBPalette16 altPalette;
    uint8_t mood;
    uint8_t modeStep;
    unsigned long lastMoodSwitch;
    unsigned long lastChaosEvent;

    void blendPalettes() {
        nblendPaletteTowardPalette(currentPalette, altPalette, 5);
    }

    void chooseNewMood() {
        mood = random8(4);
        switch (mood) {
            case 0: altPalette = PartyColors_p; break;
            case 1: altPalette = ForestColors_p; break;
            case 2: altPalette = OceanColors_p; break;
            case 3: altPalette = CRGBPalette16(CHSV(random8(), 200, 255), CHSV(random8(), 255, 255), CHSV(random8(), 255, 200), CHSV(random8(), 150, 255)); break;
        }
        lastMoodSwitch = millis();
    }

    void glitterStorm(uint8_t intensity) {
        for (int i = 0; i < intensity; i++) {
            leds[random16(numLeds)] += CHSV(random8(), 200, 255);
        }
    }

    void chaosEvent() {
        switch (random8(4)) {
            case 0: fill_rainbow(leds, numLeds, gHue); break;
            case 1: for (int i = 0; i < numLeds; i++) leds[i] = CHSV(gHue + i * 5, 255, beatsin8(12, 120, 255)); break;
            case 2: glitterStorm(30); break;
            case 3: fill_solid(leds, numLeds, CRGB::Purple); break;
        }
        lastChaosEvent = millis();
    }

    void noiseLayer() {
        for (uint16_t i = 0; i < numLeds; i++) {
            uint8_t index = inoise8(i * 10, millis() / 4 + seed);
            leds[i] += ColorFromPalette(currentPalette, index + gHue, 128);
        }
    }

    void pulseLayer() {
        uint8_t wave = beatsin8(10, 0, 255);
        for (uint16_t i = 0; i < numLeds; i++) {
            if (i % (wave / 32 + 1) == 0)
                leds[i] += CHSV(gHue + i * 3, 255, wave);
        }
    }

    void sparkleLayer() {
        if (random8() < 50) leds[random16(numLeds)] += CRGB::White;
    }

public:
    CosmicBeastOfManyMoods(CRGB* ledArray, uint16_t numLeds): Animation(ledArray, numLeds, "Cosmic Beast of Many Moods"), gHue(0), t(0), seed(random16()), mood(0), modeStep(0), lastMoodSwitch(0), lastChaosEvent(0) {
        currentPalette = RainbowColors_p;
        altPalette = LavaColors_p;
    }

    void update() override {
        EVERY_N_MILLISECONDS(50) {
            gHue++;
            t++;
        }

        blendPalettes();

        if (millis() - lastMoodSwitch > 20000) {
            chooseNewMood();
        }

        if (millis() - lastChaosEvent > random16(5000, 10000)) {
            chaosEvent();
        }

        fadeToBlackBy(leds, numLeds, 20);

        // Core effect layering
        noiseLayer();
        pulseLayer();
        sparkleLayer();

        if (random8() < 10) glitterStorm(5);
    }
};

static Registrar<CosmicBeastOfManyMoods> cosmicBeastOfManyMoodsRegistrator("Cosmic Beast of Many Moods");

// TomorrowlandStageAnimation - Simulates Tomorrowland main stage: vibrant pulsing waves, laser beams, pyro flashes, LED patterns
class TomorrowlandStageAnimation : public Animation {
private:
    uint8_t gHue = 0;
    uint16_t noiseOffset = 0;
    uint8_t laserPos = 0;
    bool laserActive = false;
    uint8_t pyroChance = 5; // % chance per frame for pyro flash
    CRGBPalette16 festivalPalette = CRGBPalette16(
        CRGB::Purple, CRGB::HotPink, CRGB::Cyan, CRGB::Lime,
        CRGB::BlueViolet, CRGB::Magenta, CRGB::Turquoise, CRGB::GreenYellow,
        CRGB::Indigo, CRGB::DeepPink, CRGB::SkyBlue, CRGB::Chartreuse,
        CRGB::DarkViolet, CRGB::Fuchsia, CRGB::DodgerBlue, CRGB::SpringGreen
    );
public:TomorrowlandStageAnimation(CRGB* ledArray, uint16_t numLeds) : Animation(ledArray, numLeds, "Tomorrowland Stage") {}
    void update() override {
        EVERY_N_MILLISECONDS(20) { gHue++; noiseOffset += 2; } // Smooth shift
        fadeToBlackBy(leds, numLeds, 15); // Trails and fade

        // Base LED waves (noise for organic movement)
        for (int i = 0; i < numLeds; i++) {
            uint8_t noise = inoise8(i * 15 + noiseOffset, gHue * 2);
            uint8_t bright = sin8(noise) / 2 + 100; // Pulsing brightness
            leds[i] = ColorFromPalette(festivalPalette, noise + gHue, bright, LINEARBLEND);
        }

        // Laser beams: Fast moving white lines with fade
        if (!laserActive && random8() < 10) { laserActive = true; laserPos = 0; }
        if (laserActive) {
            if (laserPos < numLeds) {
                leds[laserPos] = CRGB::White; // Bright laser
                if (laserPos > 0) leds[laserPos - 1].fadeToBlackBy(100); // Short trail
                laserPos += 3; // Speed
            } else {
                laserActive = false;
            }
        }

        // Pyro flashes: Random bright white bursts fading quickly
        if (random8() < pyroChance) {
            uint16_t pos = random16(numLeds);
            leds[pos] = CRGB::White;
            addGlitter(50); // Extra sparkles during pyro
            EVERY_N_MILLISECONDS(50) { fadeToBlackBy(leds, numLeds, 150); } // Quick fade
        }

        // Symmetry patterns: Mirror effect for stage-like feel
        for (int i = 0; i < numLeds / 2; i++) {
            leds[numLeds - 1 - i] = leds[i]; // Mirror
        }
    }
};
static Registrar<TomorrowlandStageAnimation> tomorrowlandStageRegistrator("Tomorrowland Stage");

class GlitchedCyberAnimation : public Animation {
private:
  uint8_t gHue = 0;
  uint8_t glitchDensity = 20;
  CRGBPalette16 neonPalette = CRGBPalette16(CRGB::HotPink, CRGB::Cyan, CRGB::Purple, CRGB::Lime);
public:
GlitchedCyberAnimation(CRGB* ledArray, uint16_t numLeds) : Animation(ledArray, numLeds, "Glitched Cyber") {}
  void update() override {
    EVERY_N_MILLISECONDS(50) { gHue++; }
    fadeToBlackBy(leds, numLeds, 30); // Quick fade for motion
    fill_solid(leds, numLeds, CRGB::Black); // Base dark
    for (int i = 0; i < numLeds; i++) {
      if (random8() < glitchDensity) {
        uint8_t glitchType = random8(3);
        if (glitchType == 0) { // Pixel glitch
          leds[i] = ColorFromPalette(neonPalette, gHue + random8(64), 255);
        } else if (glitchType == 1) { // Shift segment
          if (i + 5 < numLeds) {
            leds[i + random8(5)] = ColorFromPalette(neonPalette, gHue, 200);
          }
        } else { // Noise burst
          leds[i] += CHSV(gHue, 255, random8(100, 255));
        }
      }
    }
    blur1d(leds, numLeds, 50); // Smear glitches
  }
};
static Registrar<GlitchedCyberAnimation> glitchedCyberRegistrator("Glitched Cyber");

// Playa Chaos Carnival - Silly, overengineered Burning Man animation: dust storms, mutant vehicles, hugs, fairy dust, thunder, mood shifts
class PlayaChaosCarnivalAnimation : public Animation {
private:
    uint8_t gHue = 0; // Global hue shift for base colors
    uint8_t chaosFactor = 0; // Builds over time for more randomness
    uint8_t currentMood = 0; // 0: Fiery Playa, 1: Psychedelic Dust, 2: Mystic Hug, 3: Wild Carnival
    unsigned long lastMoodChange = 0;
    unsigned long lastThunder = 0;
    unsigned long lastHug = 0;
    uint16_t dustNoiseOffset = 0;
    uint8_t artCarPos = 0;
    uint8_t artCarSpeed = 2;
    bool thunderActive = false;
    bool hugActive = false;
    CRGBPalette16 moodPalettes[4] = {
        CRGBPalette16(CRGB::Black, CRGB::Red, CRGB::Orange, CRGB::Yellow), // Fiery
        CRGBPalette16(CRGB::Black, CRGB::Purple, CRGB::Blue, CRGB::Indigo), // Psychedelic
        CRGBPalette16(CRGB::Black, CRGB::Pink, CRGB::HotPink, CRGB::DeepPink), // Mystic Hug
        CRGBPalette16(CRGB::Black, CRGB::Green, CRGB::Cyan, CRGB::Lime) // Carnival
    };

    // Internal method: Apply base dust storm noise layer
    void applyDustStorm() {
        dustNoiseOffset += random8(1, 3); // Random speed variation
        for (int i = 0; i < numLeds; i++) {
            uint8_t noise = inoise8(i * (10 + chaosFactor / 10), dustNoiseOffset);
            uint8_t bright = sin8(noise) / 2 + 50; // Medium brightness
            leds[i] = ColorFromPalette(moodPalettes[currentMood], noise + gHue, bright, LINEARBLEND);
        }
    }

    // Internal method: Chase "mutant vehicles" with trails
    void runArtCars() {
        if (random8() < 20 + chaosFactor) { // More cars as chaos increases
            artCarPos = (artCarPos + artCarSpeed) % numLeds;
            leds[artCarPos] = CHSV(gHue + 128, 255, 200); // Bright opposing color
            for (int trail = 1; trail < 5; trail++) { // Trail loop
                if (artCarPos >= trail) {
                    leds[artCarPos - trail].fadeToBlackBy(50 * trail); // Fading trail
                }
            }
            artCarSpeed = random8(1, 4); // Random speed change
        }
    }

    // Internal method: Trigger "hug bursts" - warm pulses
    void triggerHugBurst() {
        if (hugActive) {
            for (int i = 0; i < numLeds; i++) {
                leds[i].r = min(255, leds[i].r + random8(50, 100)); // Warm red/orange boost
                leds[i].fadeLightBy(200 - chaosFactor); // Fade based on chaos
            }
            if (random8() < 10) hugActive = false; // Random end
        } else if (millis() - lastHug > random16(10000, 30000)) { // Random trigger
            hugActive = true;
            lastHug = millis();
        }
    }

    // Internal method: Sprinkle "fairy dust" sparkles
    void sprinkleFairyDust() {
        uint8_t dustDensity = 5 + chaosFactor / 5; // More dust with chaos
        for (uint8_t d = 0; d < dustDensity; d++) {
            if (random8() < 30) {
                uint16_t pos = random16(numLeds);
                leds[pos] += CHSV(random8(), 200, random8(100, 200)); // Random sparkle
                leds[pos].fadeLightBy(150); // Subtle fade
            }
        }
    }

    // Internal method: Occasional "thunderclap" flash
    void thunderClap() {
        if (thunderActive) {
            fill_solid(leds, numLeds, CRGB::White); // Bright flash
            EVERY_N_MILLISECONDS(50) { fadeToBlackBy(leds, numLeds, 200); } // Quick fade
            if (random8() < 20) thunderActive = false;
        } else if (millis() - lastThunder > random16(20000, 60000)) {
            thunderActive = true;
            lastThunder = millis();
        }
    }

    // Internal method: Morph moods and chaos
    void morphMood() {
        // WARNING: random16(90000) overflows uint16_t, max value is 65535. Consider using a lower max.
        if (millis() - lastMoodChange > random16(30000, 60000)) { // Random mood shift, reduced max to avoid overflow warning
            currentMood = random8(4);
            chaosFactor = min(50, chaosFactor + random8(5, 10)); // Build chaos
            gHue += random8(64, 128); // Hue jump
            lastMoodChange = millis();
            randomSeed(millis() + chaosFactor); // Reseed for more randomness
        }
    }

public:
PlayaChaosCarnivalAnimation(CRGB* ledArray, uint16_t numLeds) : Animation(ledArray, numLeds, "Playa Chaos Carnival") {
        randomSeed(millis()); // Initial seed for randomness
    }
    void update() override {
        fadeToBlackBy(leds, numLeds, 10 + chaosFactor / 5); // Base fade, increases with chaos

        // Core loop: Apply layers with side effects
        morphMood(); // Check for mood/chaos changes
        applyDustStorm(); // Background noise
        for (uint8_t loop = 0; loop < 3 + chaosFactor / 10; loop++) { // Overengineered multi-loop for density
            runArtCars(); // Chasing vehicles
            sprinkleFairyDust(); // Sparkles
        }
        triggerHugBurst(); // Pulses
        thunderClap(); // Flashes

        // Global side effect: Random blend tweak
        if (random8() < chaosFactor) {
            nblend(leds[random16(numLeds)], CRGB::Black, random8(50)); // Random dim spot
        }

        EVERY_N_MILLISECONDS(50) { gHue++; } // Slow hue drift
    }
};
static Registrar<PlayaChaosCarnivalAnimation> playaChaosCarnivalRegistrator("Playa Chaos Carnival");

class SpaceWizardsAndLizards : public Animation {
private:
    uint8_t gHue;
    uint16_t t;
    uint32_t wizardSeed;
    CRGBPalette16 wizardPalette;
    CRGBPalette16 lizardPalette;
    bool isWizardPhase;
    unsigned long lastPhaseSwitch;

    void switchPhase() {
        isWizardPhase = !isWizardPhase;
        lastPhaseSwitch = millis();
        wizardSeed = random16();
    }

    void castWizardSpell() {
        for (uint16_t i = 0; i < numLeds; i++) {
            uint8_t noise = inoise8(i * 15, millis() / 3 + wizardSeed);
            uint8_t brightness = sin8(noise + gHue);
            leds[i] = ColorFromPalette(wizardPalette, noise, brightness);
            if (random8() < 8) leds[i] += CRGB::White;
        }
    }

    void summonLizardAura() {
        fadeToBlackBy(leds, numLeds, 25);
        for (uint16_t i = 0; i < numLeds; i++) {
            if (i % 7 == 0) {
                uint8_t index = (gHue + i * 2 + t) % 255;
                leds[i] += ColorFromPalette(lizardPalette, index, 180);
            }
        }
        if (random8() < 30) {
            uint16_t burstPos = beatsin16(5, 0, numLeds - 1);
            leds[burstPos] += CRGB::Green;
            leds[(burstPos + 1) % numLeds] += CRGB::Yellow;
        }
    }

public:
    SpaceWizardsAndLizards(CRGB* ledArray, uint16_t numLeds): Animation(ledArray, numLeds, "Space Wizards & Lizards"), gHue(0), t(0), wizardSeed(random16()), isWizardPhase(true), lastPhaseSwitch(0) {
        wizardPalette = CRGBPalette16(
            CHSV(180, 255, 255), CHSV(200, 255, 200), CHSV(160, 255, 255), CHSV(180, 200, 255),
            CHSV(190, 255, 255), CHSV(170, 255, 180), CHSV(210, 200, 255), CHSV(160, 180, 200),
            CHSV(180, 255, 255), CHSV(200, 255, 200), CHSV(160, 255, 255), CHSV(180, 200, 255),
            CHSV(190, 255, 255), CHSV(170, 255, 180), CHSV(210, 200, 255), CHSV(160, 180, 200));

        lizardPalette = CRGBPalette16(
            CHSV(85, 255, 255), CHSV(100, 200, 255), CHSV(120, 255, 200), CHSV(90, 255, 180),
            CHSV(110, 255, 255), CHSV(100, 255, 255), CHSV(130, 255, 180), CHSV(85, 180, 200),
            CHSV(85, 255, 255), CHSV(100, 200, 255), CHSV(120, 255, 200), CHSV(90, 255, 180),
            CHSV(110, 255, 255), CHSV(100, 255, 255), CHSV(130, 255, 180), CHSV(85, 180, 200));
    }

    void update() override {
        EVERY_N_MILLISECONDS(40) {
            gHue++;
            t++;
        }

        if (millis() - lastPhaseSwitch > 15000) {
            switchPhase();
        }

        if (isWizardPhase) {
            castWizardSpell();
        } else {
            summonLizardAura();
        }
    }
};

static Registrar<SpaceWizardsAndLizards> spaceWizardsAndLizardsRegistrator("Space Wizards Lizards");

class JuggleAnimation : public Animation {
  public:
    JuggleAnimation(CRGB* ledArray, uint16_t numLeds): Animation(ledArray, numLeds, "Juggle") {}
    void update() override {
        fadeToBlackBy(leds, numLeds, 20);
        byte dothue = 0;
        for(int i = 0; i < 8; i++) {
            leds[beatsin16(i+7, 0, numLeds-1)] |= CHSV(dothue, 200, brightness);
            dothue += 32;
        }
    }
};
static Registrar<JuggleAnimation> juggleRegistrator("Juggle");

class SinelonAnimation : public Animation {
  private:
    uint8_t gHue;
  public:
    SinelonAnimation(CRGB* ledArray, uint16_t numLeds): Animation(ledArray, numLeds, "Sinelon"), gHue(0) {}
    void update() override {
        fadeToBlackBy(leds, numLeds, 20);
        int pos = beatsin16(13, 0, numLeds-1);
        leds[pos] += CHSV(gHue, 255, brightness);
        EVERY_N_MILLISECONDS(20) { gHue++; }
    }
};
static Registrar<SinelonAnimation> sinelonRegistrator("Sinelon");

class FireTribeWonderland : public Animation {
private:
    uint8_t gHue;
    uint16_t t;
    CRGBPalette16 softFlamePalette;
    CRGBPalette16 sunsetGlowPalette;
    CRGBPalette16 warmDreamPalette;
    uint8_t morphStage;
    unsigned long lastMorph;

    void morphPalette() {
        morphStage = (morphStage + 1) % 3;
        lastMorph = millis();
    }

    CRGBPalette16 getCurrentPalette() {
        switch (morphStage) {
            case 0: return softFlamePalette;
            case 1: return sunsetGlowPalette;
            default: return warmDreamPalette;
        }
    }

    void drawLayeredWaves(CRGBPalette16 palette) {
        for (uint16_t i = 0; i < numLeds; i++) {
            uint8_t offset = sin8(i * 3 + t / 2);
            uint8_t index = inoise8(i * 12, t + offset * 2);
            leds[i] = ColorFromPalette(palette, index + gHue, 200, LINEARBLEND);
        }
    }

    void overlayPulse() {
        uint8_t pulse = beatsin8(4, 40, 120);
        for (uint16_t i = 0; i < numLeds; i++) {
            leds[i].fadeLightBy(255 - pulse);
        }
    }

    void sprinkleAmber() {
        if (random8() < 40) {
            leds[random16(numLeds)] += CHSV(random8(20, 40), 255, random8(100, 200));
        }
    }

public:
    FireTribeWonderland(CRGB* ledArray, uint16_t numLeds): Animation(ledArray, numLeds, "Fire Tribe Wonderland"), gHue(0), t(0), morphStage(0), lastMorph(0) {

        softFlamePalette = CRGBPalette16(
            CHSV(0, 255, 100), CHSV(10, 255, 180), CHSV(20, 200, 255), CHSV(25, 180, 255),
            CHSV(30, 180, 200), CHSV(40, 150, 150), CHSV(20, 255, 255), CHSV(10, 200, 180),
            CHSV(0, 255, 100), CHSV(10, 255, 180), CHSV(20, 200, 255), CHSV(25, 180, 255),
            CHSV(30, 180, 200), CHSV(40, 150, 150), CHSV(20, 255, 255), CHSV(10, 200, 180));

        sunsetGlowPalette = CRGBPalette16(
            CHSV(5, 255, 255), CHSV(15, 200, 255), CHSV(25, 150, 230), CHSV(35, 180, 200),
            CHSV(10, 150, 220), CHSV(20, 180, 240), CHSV(30, 160, 255), CHSV(40, 180, 255),
            CHSV(5, 255, 255), CHSV(15, 200, 255), CHSV(25, 150, 230), CHSV(35, 180, 200),
            CHSV(10, 150, 220), CHSV(20, 180, 240), CHSV(30, 160, 255), CHSV(40, 180, 255));

        warmDreamPalette = CRGBPalette16(
            CHSV(0, 0, 100), CHSV(10, 30, 150), CHSV(20, 50, 200), CHSV(30, 80, 220),
            CHSV(40, 100, 255), CHSV(20, 80, 240), CHSV(10, 50, 220), CHSV(5, 30, 180),
            CHSV(0, 0, 100), CHSV(10, 30, 150), CHSV(20, 50, 200), CHSV(30, 80, 220),
            CHSV(40, 100, 255), CHSV(20, 80, 240), CHSV(10, 50, 220), CHSV(5, 30, 180));
    }

    void update() override {
        EVERY_N_MILLISECONDS(45) {
            gHue++;
            t++;
        }

        if (millis() - lastMorph > 25000) {
            morphPalette();
        }

        CRGBPalette16 palette = getCurrentPalette();

        fadeToBlackBy(leds, numLeds, 10);
        drawLayeredWaves(palette);
        overlayPulse();
        sprinkleAmber();
    }
};

static Registrar<FireTribeWonderland> fireTribeWonderlandRegistrator("Fire Tribe Wonderland");

class CosmicChaosAnimation : public Animation {
private:
    // State machine for multi-effect chaos
    enum ChaosState {
        QUANTUM_SWIRL,
        NEBULA_BURST,
        CHROMATIC_WORMHOLES,
        PLASMA_TSUNAMI
    } currentState;

    // Core variables
    uint8_t gHue;
    uint16_t noiseSeed;
    uint8_t fractalDepth;
    CRGBPalette16 cosmicPalette;

    // Sub-effect controllers
    struct QuantumParams {
        uint8_t waveDensity;
        int8_t gravityVector;
        uint16_t entropyField;
        uint8_t timeDilation;
    } quantumParams;

    // Wormhole physics simulator (because why not?)
    class Wormhole {
    public:
        float position;
        float velocity;
        float mass;
        CRGB trail[5];
        uint8_t hue;
        QuantumParams* quantumParamsPtr;
        uint16_t* numLedsPtr;

        Wormhole(QuantumParams* qp, uint16_t* nl) : position(random16()), velocity(random8(200)-100),
                    mass(random8(50,200)), hue(random8()), quantumParamsPtr(qp), numLedsPtr(nl) {
            for(auto& c : trail) c = CHSV(hue, 255, 128);
        }

        void updatePhysics() {
            velocity += quantumParamsPtr->gravityVector * (mass/100.0);
            position = fmod(position + velocity/1000.0, *numLedsPtr + 50) - 25;
            if(position < -24) position += *numLedsPtr + 50;
        }
    };

    // Fix: declare as vector of Wormhole
    std::vector<Wormhole> spacetimeAnomalies;

    // Move recursiveGlitter as a member function
    void recursiveGlitter(uint8_t depth, fract8 chance) {
        if(depth == 0) return;

        if(random8() < chance) {
            const uint16_t pos = random16(numLeds);
            leds[pos] += CHSV(gHue + random8(64), 200, 255);
            recursiveGlitter(depth - 1, chance / 2);
        }
    }

    // Fractal plasma generator
    void generateFractalPlasma(uint8_t depth, uint16_t x, uint16_t width) {
        if(depth == 0) return;

        const uint16_t mid = x + width/2;
        const uint8_t val = inoise8(noiseSeed + x * depth, millis() >> 4);

        if(mid < numLeds) {
            leds[mid] = ColorFromPalette(cosmicPalette, val, 255, LINEARBLEND);
            generateFractalPlasma(depth-1, x, width/2);
            generateFractalPlasma(depth-1, mid, width/2);
        }
    }

    // Palette morphing system
    void evolveCosmicPalette() {
        static CRGBPalette16 targetPalette = PartyColors_p;
        static uint8_t paletteBlend = 0;

        EVERY_N_MILLISECONDS(quantumParams.timeDilation * 50) {
            nblendPaletteTowardPalette(cosmicPalette, targetPalette, 12);

            if(paletteBlend++ > 128) {
                targetPalette = CRGBPalette16(
                    CHSV(random8(), 255, 255),
                    CHSV(random8(), random8(128,255), 255),
                    CHSV(random8(), 100, random8(128,255)),
                    CHSV(random8(), 255, 255)
                );
                paletteBlend = 0;
            }
        }
    }

    // State transition handler
    void quantumStateShift() {
        static uint32_t lastChange = 0;
        const uint32_t chaosInterval = 15000 + random8() * 500;

        if(millis() - lastChange > chaosInterval) {
            currentState = static_cast<ChaosState>((currentState + 1 + random8(2)) % 4); // <-- fixed missing parenthesis

            // Randomize physics parameters
            quantumParams = {
                random8(3,15),
                static_cast<int8_t>(random8(2) ? 1 : -1), // explicit cast for narrowing conversion
                random16(),
                random8(5,25)
            };

            // Create spacetime anomalies
            spacetimeAnomalies.clear();
            for(int i = 0; i < (3 + random8(5)); i++) {
                spacetimeAnomalies.emplace_back(&quantumParams, &numLeds);
            }

            lastChange = millis();
            noiseSeed = random16();
            fractalDepth = random8(3,7);
        }
    }

    // STATE MACHINE COMPONENTS
    void quantumSwirl() {
        const uint8_t swirlSpeed = beatsin8(12, 3, 18);
        const uint8_t baseHue = gHue + beatsin8(15, 0, 96);

        for(int i = 0; i < numLeds; i++) {
            uint8_t hue = baseHue + inoise8(i * quantumParams.waveDensity, noiseSeed);
            uint8_t bri = sin8(i * 3 + millis()/20);
            leds[i] = CHSV(hue, 240, bri);
        }

        recursiveGlitter(4, 30);
    }

    void nebulaBurst() {
        fadeToBlackBy(leds, numLeds, 32);

        // Generate fractal plasma
        generateFractalPlasma(fractalDepth, 0, numLeds);

        // Add dimensional rifts
        EVERY_N_MILLISECONDS(100) {
            const uint16_t pos = random16(numLeds);
            const uint8_t width = random8(3,15);
            for(int i = 0; i < width; i++) {
                if(pos + i < numLeds) {
                    leds[pos+i] = CRGB::White;
                }
            }
        }

        // Animate wormholes
        for(auto& wormhole : spacetimeAnomalies) {
            wormhole.updatePhysics();
            for(int i = 0; i < 5; i++) {
                int pos = static_cast<int>(wormhole.position) - i;
                if(pos >= 0 && pos < numLeds) {
                    leds[pos] = wormhole.trail[i];
                }
            }
        }
    }

public:
    CosmicChaosAnimation(CRGB* ledArray, uint16_t numLeds): Animation(ledArray, numLeds, "Cosmic Chaos"),
          gHue(0),
          currentState(QUANTUM_SWIRL),
          noiseSeed(5338),
          fractalDepth(5),
          cosmicPalette(OceanColors_p) {

        quantumParams = {8, 1, 1337, 15};
        spacetimeAnomalies.emplace_back(&quantumParams, &this->numLeds);
    }

    void update() override {
        // Core timing system
        EVERY_N_MILLISECONDS(20) { gHue++; }
        EVERY_N_SECONDS(30) { noiseSeed += random16(32768); }

        // Run state machine
        quantumStateShift();
        evolveCosmicPalette();

        switch(currentState) {
            case QUANTUM_SWIRL: quantumSwirl(); break;
            case NEBULA_BURST: nebulaBurst(); break;
            case CHROMATIC_WORMHOLES: nebulaBurst(); break;
            case PLASMA_TSUNAMI: quantumSwirl(); break;
        }

        // Dimensional energy bleed-through
        if(random8() < 45) {
            recursiveGlitter(1, 80);
        }

        // Temporal distortion effect
        blur1d(leds, numLeds, beatsin8(10, 3, 15));
    }
};

static Registrar<CosmicChaosAnimation> cosmicChaosRegistrator("Cosmic Chaos");

// Trippy Hippie Magic Wonderland - Smooth, flowing rainbow waves, blooming lights, swirling patterns, gentle magic bursts
class TrippyHippieWonderlandAnimation : public Animation {
private:
    uint8_t gHue = 0; // Global hue for color drifting
    uint16_t flowOffset = 0; // Offset for wave flows
    uint8_t wonderFactor = 0; // Builds for more magic over time
    unsigned long lastBloom = 0;
    unsigned long lastSwirl = 0;
    unsigned long lastMagicBurst = 0;
    bool bloomActive = false;
    bool swirlActive = false;
    uint16_t bloomPos = 0;
    uint8_t bloomRadius = 0;
    CRGBPalette16 wonderPalette = CRGBPalette16(
        CRGB::Lavender, CRGB::LightPink, CRGB::LightSkyBlue, CRGB::PaleGreen,
        CRGB::Violet, CRGB::Pink, CRGB::SkyBlue, CRGB::MintCream,
        CRGB::Orchid, CRGB::HotPink, CRGB::Turquoise, CRGB::LimeGreen,
        CRGB::Purple, CRGB::DeepPink, CRGB::DodgerBlue, CRGB::SpringGreen
    );

    // Internal method: Base rainbow flow layer with noise
    void applyRainbowFlow() {
        flowOffset += 1 + wonderFactor / 20; // Slow to faster flow
        for (int i = 0; i < numLeds; i++) {
            uint8_t noise = inoise8(i * 10, flowOffset);
            uint8_t bright = sin8(noise) / 2 + 80; // Soft brightness
            leds[i] = ColorFromPalette(wonderPalette, noise + gHue, bright, LINEARBLEND);
        }
    }

    // Internal method: Blooming flowers - gentle color spreads
    void bloomFlowers() {
        if (bloomActive) {
            for (int r = 0; r < bloomRadius; r++) { // Nested loop for bloom expansion
                int left = bloomPos - r;
                int right = bloomPos + r;
                if (left >= 0) nblend(leds[left], CHSV(gHue + 64, 200, 150 - r * 10), 128); // Blend bloom
                if (right < numLeds) nblend(leds[right], CHSV(gHue + 64, 200, 150 - r * 10), 128);
            }
            bloomRadius += 1; // Expand
            if (bloomRadius > 20 + wonderFactor / 5) { bloomActive = false; } // End bloom
        } else if (millis() - lastBloom > random16(5000, 15000)) {
            bloomActive = true;
            bloomPos = random16(numLeds);
            bloomRadius = 1;
            lastBloom = millis();
            if (random8() < 30) gHue += random8(32, 96); // Side effect: Hue shift on bloom
        }
    }

    // Internal method: Swirling mandala patterns
    void swirlMandala() {
        if (swirlActive) {
            for (int i = 0; i < numLeds; i += 4 + wonderFactor / 10) { // Pattern loop with variable step
                leds[i] = CHSV(gHue + i, 180, 200);
                if (i + 1 < numLeds) leds[i + 1].fadeLightBy(100); // Swirl fade
                if (i + 2 < numLeds) leds[i + 2] = CHSV(gHue - i, 180, 150);
                if (i + 3 < numLeds) leds[i + 3].fadeLightBy(150);
            }
            if (random8() < 10) swirlActive = false; // Random end
        } else if (millis() - lastSwirl > random16(10000, 30000)) {
            swirlActive = true;
            lastSwirl = millis();
        }
    }

    // Internal method: Magic bursts - random color spreads with glitter
    void magicBursts() {
        if (millis() - lastMagicBurst > random16(20000, 60000)) {
            uint16_t burstPos = random16(numLeds);
            uint8_t burstHue = random8();
            for (int b = 0; b < 10; b++) { // Burst loop
                uint16_t pos = (burstPos + random16(-20, 20)) % numLeds;
                leds[pos] = CHSV(burstHue, 255, random8(150, 255));
                addGlitter(50); // Side effect: Add glitter during burst
            }
            lastMagicBurst = millis();
            wonderFactor = min(100, wonderFactor + random8(5, 15)); // Build wonder
            randomSeed(millis() + wonderFactor); // Reseed for creativity
        }
    }

public:
    TrippyHippieWonderlandAnimation(CRGB* ledArray, uint16_t numLeds) : Animation(ledArray, numLeds, "Trippy Hippie Wonderland") {
        randomSeed(millis()); // Seed for unique runs
    }
    void update() override {
        fadeToBlackBy(leds, numLeds, 5 + wonderFactor / 20); // Gentle fade, increases slightly

        // Layered effects with side effects
        applyRainbowFlow(); // Base layer
        bloomFlowers(); // Blooms
        swirlMandala(); // Swirls
        magicBursts(); // Bursts

        EVERY_N_MILLISECONDS(30) { gHue++; } // Slow hue drift
    }
};
static Registrar<TrippyHippieWonderlandAnimation> trippyHippieWonderlandRegistrator("Trippy Hippie Wonderland");
