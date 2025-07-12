/**
 * OLED Display Manager
 * 
 * Handles all OLED display operations
 */

#ifndef OLED_MANAGER_H
#define OLED_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "../config/Config.h"

// Forward declarations to avoid circular dependencies
class SystemManager;
class AnimationManager;

class OLEDManager {
public:
    OLEDManager();
    
    // Initialize the display
    void begin();
    
    // Update the display
    void update();   
    
    // Set the system manager reference
    void setSystemManager(SystemManager* sysManager) { systemManager = sysManager; }
    
    // Check if OLED is available
    bool isAvailable() const { return available; }

private:
    // Utility functions for OLED drawing
    void drawCenteredText(const char* text, int y, const uint8_t* font);
    void drawProgressBar(int x, int y, int width, int height, int percentage);
    
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
    bool available;
    SystemManager* systemManager = nullptr; 
};

#endif // OLED_MANAGER_H
