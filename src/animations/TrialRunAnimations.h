/**
 * Trial Run Animations
 * 
 * Placeholder for experimental animations that are still being developed
 * Add your experimental animations here before moving them to their permanent categories
 */

#ifndef TRIAL_RUN_ANIMATIONS_H
#define TRIAL_RUN_ANIMATIONS_H

#include "AnimationBase.h"

// Example placeholder for a new trial animation
// Uncomment and implement when you have a new animation to test
/*
class NewExperimentalAnimation : public Animation {
public:
    NewExperimentalAnimation(CRGB* ledArray, uint16_t numLeds) : 
        Animation(ledArray, numLeds) {}
    
    void update() override {
        // Your experimental animation code here
    }
    
    const char* getName() override { return "New Experimental"; }
    
    AnimationCategory getCategory() override { return TRIAL_RUNS; }
};
*/

#endif // TRIAL_RUN_ANIMATIONS_H
