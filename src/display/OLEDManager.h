/**
 * OLED Display Manager
 * Handles all OLED display operations
 */
#ifndef OLEDMANAGER_H
#define OLEDMANAGER_H

#include <U8g2lib.h>
#include <Arduino.h>
#include "../system/SystemManager.h"
#include "../config/Config.h"
#include "../animations/AnimationManager.h"

class SystemManager;

class OLEDManager {
public:
    OLEDManager(); 
    void begin();
    void begin(SystemManager* systemManager);
    void update();
    void displayBootMessage(const char* message);
    void setSystemManager(SystemManager* sysManager);

private:
    void drawStartupScreen();
    void drawSystemErrorScreen();
    void drawAnimationErrorScreen();
    void drawBrightnessAdjustmentScreen(uint16_t brightness);
    void drawLedCountAdjustmentScreen(uint16_t numLeds);
    void drawNormalOperationScreen(uint16_t patternIndex, const char* patternName,
                                  uint16_t brightness, uint16_t numLeds);
    void drawCenteredText(const char* text, int y, const uint8_t* font = u8g2_font_8x13_tr);
    void drawProgressBar(int x, int y, int width, int height, int percentage);

    U8G2_SSD1309_128X64_NONAME0_F_HW_I2C u8g2;
    bool available;
    SystemManager* systemManager;
    uint16_t lastPatternNumber;
    unsigned long patternChangeTime;
};

#endif