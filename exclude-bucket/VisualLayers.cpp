#include <Arduino.h>
#include <FastLED.h>
#include <deque>
#include "AnimationBase.h"
#include "../audio/AudioFeatures.h" // Assume this exists; if not, define dummy
#include "../audio/AudioSnapshot.h" // Assume this exists; if not, define dummy
#include "../config/Config.h"

// Dummy AudioFeatures if no real audio
struct DummyAudioFeatures {
    float energy = random8(0, 255) / 255.0f;
    float bass = random8(0, 255) / 255.0f;
    float treble = random8(0, 255) / 255.0f;
    float loudness = random8(0, 100) / 100.0f;
    float dynamics = random8(0, 100) / 100.0f;
    float volume = random8(0, 100) / 100.0f;
    float spectrumCentroid = random8(0, NUM_SAMPLES / 2);
    bool beatDetected = random8() < 50;
    float bpm = 120.0f + random8(-20, 20);
    float noiseFloor = 0.1f;
    int dominantBand = random8(0, NUM_SAMPLES / 2);
    int16_t* waveform = nullptr;
    int waveformSize = 0;
};

// Dummy snapshots
std::deque<AudioSnapshot> dummySnapshots;

// Layer implementations (adapted)

class EnergyPulseRiverLayer : public VisualLayer {
    float position = 0.0f;
    float speed = 0.0f;
    uint8_t hue = 0;

public:
    void update(const AudioFeatures& audio, const std::deque<AudioSnapshot>& snapshots) override {
        speed = audio.energy * 0.5f;
        position += speed;
        hue = (uint8_t)(audio.energy * 255);
    }

    void render(CRGB* leds, int count) override {
        for (int i = 0; i < count; ++i) {
            float phase = fmod(position + i * 0.1f, count);
            uint8_t bright = 128 + 127 * sin8((uint8_t)(phase));
            leds[i] += CHSV(hue, 255, bright);
        }
    }

    const char* getName() const override { return "EnergyPulseRiverLayer"; }
};
 









// Example for one more
class TrebleSparkleLayer : public VisualLayer {
private:
    float treble = 0.0f;

public:
    void update(const AudioFeatures& audio, const std::deque<AudioSnapshot>&) override {
        treble = audio.treble;
    }

    void render(CRGB* leds, int count) override {
        int numSparks = map(treble * 100, 0, 100, 0, 10);
        for (int i = 0; i < numSparks; ++i) {
            int pos = random(count);
            leds[pos] += CHSV(200 + random(55), 255, 180 + random(75));
        }
    }

    const char* getName() const override { return "TrebleSparkleLayer"; }
};