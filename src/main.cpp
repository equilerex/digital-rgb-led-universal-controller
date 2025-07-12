/**
 * ESP32 Basic Wearable FastLED Controller
 * 
 * Main controller code for LED wearables
 * 
 * IMPORTANT: This is the only file that should call FastLED.show() during normal operation.
 * All FastLED.show() calls have been centralized here to prevent timer resets.
 */

#include <Arduino.h>
#include "system/SystemManager.h"
#include "animations/AnimationManager.h"
#include "config/Config.h"

// Include OLED manager directly in main if needed
#if ENABLE_OLED 
#include "display/OLEDManager.h"
OLEDManager oledManager;  // Create a global instance
#endif

// Global system components
SystemManager systemManager;

// Timing variables
unsigned long lastUpdate = 0;
unsigned long debugPrint = 0;

void setup() {
  // Initialize serial first for debugging
  Serial.begin(115200);
  delay(1000); // Give serial time to initialize
  
  Serial.println(F("-------------------------Main.cpp setup------------------------------"));
  
  // Initialize the system
  Serial.println(F("Calling systemManager.begin()..."));
  systemManager.begin();
  Serial.println(F("systemManager.begin() completed"));
  
  Serial.println(F("Setting up input manager..."));
  // Initialize input manager
  systemManager.getInputManager().begin(&systemManager);
  Serial.println(F("Input manager setup complete"));

  // Add a delay to help stability
  delay(100);

  // Initialize the animation system (this will load brightness and numLeds from preferences)
  Serial.println(F("Setting up animation system..."));
  systemManager.setupAnimationSystem();
  Serial.println(F("Animation system setup complete"));
  
  // Add a delay to help stability
  delay(1000);

  // Initialize OLED display (if enabled)
  #if ENABLE_OLED
    Serial.println(F("Setting up OLED manager..."));
    oledManager.setSystemManager(&systemManager);
    oledManager.begin(); 
    Serial.println(F("OLED manager setup complete"));
  #endif
  
  Serial.println(F("Setup complete. Running main loop..."));
}

void loop() {
  // Print a heartbeat message every few seconds
  EVERY_N_SECONDS(5) {
    Serial.println(F("Main loop running"));
  }

  // Update inputs (handles button presses)
  systemManager.updateInputs();

  // Update and show LEDs in a non-blocking way
  systemManager.updateLeds();

#if ENABLE_OLED
  // Update OLED display periodically
  EVERY_N_MILLISECONDS(250) {
    oledManager.update();
  }
#endif

  // Small delay to prevent CPU overload
  delay(5);
}
