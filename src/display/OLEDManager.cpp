/**
 * OLED Display Manager Implementation
 *
 * This class manages the OLED display, handling initialization and updates based on system state.
 * The update logic is separated into distinct drawing functions for different modes/screens,
 * making it easier to identify and modify specific display behaviors.
 *
 * Key improvements:
 * - Meaningful variable and function names.
 * - Modular structure: Separate methods for each display mode (brightness adjustment, LED count adjustment, normal operation).
 * - Main/normal screen uses top/middle/bottom row structure for clarity and easy customization.
 * - Removed redundant checks and unnecessary try-catch blocks for simplicity (assuming standard Arduino environment).
 * - Added support for future changes, e.g., easy addition of new modes or row customizations.
 */
#include "OLEDManager.h"
#include <Arduino.h>
#include "../config/Config.h"
#include "../animations/AnimationManager.h"
#include "../system/SystemManager.h"
#include "../config/PinConfig.h"

// Display constants
#define OLED_RESET U8X8_PIN_NONE
#define OLED_SDA_C3 5
#define OLED_SCL_C3 6 
// Screen row positions
// real size 128x64 - 72 = 56x40
#define X_OFFSET 28  // c3 has some off space on the left

OLEDManager::OLEDManager() :
    u8g2(U8G2_R0, OLED_RESET, OLED_SCL, OLED_SDA),
    available(false),
    systemManager(nullptr) {}

void OLEDManager::begin() {
    Serial.println(F("OLED initialization starting..."));

    u8g2.begin();
    Serial.println(F("OLED display initialized"));

    u8g2.setContrast(128);
    u8g2.setBusClock(400000);

    screenW = u8g2.getDisplayWidth();
    screenH = u8g2.getDisplayHeight();

    xOffset = screenW * 0.15;  // 5% from left
    yOffset = screenH * 0.00;  // No Y offset for now
    MAIN_SCREEN_TOP_ROW_Y = screenH * 0.55;
    MAIN_SCREEN_MID_ROW_Y = screenH * 0.75;
    MAIN_SCREEN_BOT_ROW_Y = screenH * 0.95;

    displayBootMessage("Booting...");
    available = true;

    Serial.println(F("OLED ready"));
}

void OLEDManager::update() {
    bool skipOledUpdate = false;

    if (!available) {
        EVERY_N_SECONDS(5) {
            Serial.println(F("OLED not available"));
        }
        skipOledUpdate = true;
    }

    if (!systemManager) {
        if (!skipOledUpdate) {
            drawSystemErrorScreen();
        }
        skipOledUpdate = true;
    }

    static unsigned long lastUpdateTime = 0;
    unsigned long currentTime = millis();

    if (!skipOledUpdate && (currentTime - lastUpdateTime < 200)) {
        skipOledUpdate = true;
    }
    u8g2.clearBuffer();

    if (!skipOledUpdate) {
        lastUpdateTime = currentTime;

        if (systemManager == nullptr || systemManager->getAnimationManager() == nullptr) {
            displayBootMessage("Booting...");
        } else if (!systemManager->getAnimationManager()->isReady()) {
            drawStartupScreen();
        } else {
            try {
                AnimationManager* animManager = systemManager->getAnimationManager();
                InputManager* inputManager = &systemManager->getInputManager();

                bool isBrightnessMode = inputManager ? inputManager->isBrightnessMode() : false;
                bool isLedCountMode = inputManager ? inputManager->isLedCountMode() : false;

                uint8_t numLeds = animManager->getNumLeds();
                uint8_t brightness = animManager->getBrightness();
                uint8_t patternIndex = animManager->getCurrentPatternIndex();
                const char* patternName = animManager->getCurrentAnimationName();
                bool inShuffleMode = animManager->inShuffleMode();

                if (isBrightnessMode) {
                    drawBrightnessAdjustmentScreen(brightness);
                } else if (isLedCountMode) {
                    drawLedCountAdjustmentScreen(numLeds);
                } else {
                    drawNormalOperationScreen(patternIndex, patternName, brightness, numLeds, inShuffleMode);
                }

            } catch (...) {
                Serial.println(F("ERROR: Exception caught while getting animation data"));
            }


            u8g2.sendBuffer();
            EVERY_N_SECONDS(20) { Serial.println(F("DEBUG: OLEDManager::update() completed successfully")); }
        }
    }
}

// Screen drawing methods
void OLEDManager::displayBootMessage(const char* message) {
    drawCenteredText("Booting...", MAIN_SCREEN_TOP_ROW_Y, u8g2_font_5x7_mr);
    drawCenteredText("Jo's blinky", MAIN_SCREEN_MID_ROW_Y, u8g2_font_8x13_mr);
    char ver[16];
    snprintf(ver, sizeof(ver), "V: %s", VERSION);
    drawCenteredText(ver, MAIN_SCREEN_BOT_ROW_Y, u8g2_font_6x10_mr);
    u8g2.sendBuffer();
}

void OLEDManager::drawStartupScreen() {
    drawCenteredText("Starting...", MAIN_SCREEN_MID_ROW_Y, u8g2_font_6x10_mr);
    u8g2.sendBuffer();
}

void OLEDManager::drawSystemErrorScreen() {
    drawCenteredText("SYS ERROR", MAIN_SCREEN_TOP_ROW_Y, u8g2_font_6x10_mr);
    drawCenteredText("Check manager", MAIN_SCREEN_MID_ROW_Y, u8g2_font_6x10_mr);
    u8g2.sendBuffer();
}

void OLEDManager::drawAnimationErrorScreen() {
    drawCenteredText("ANIM ERROR", MAIN_SCREEN_TOP_ROW_Y, u8g2_font_6x10_mr);
    drawCenteredText("No animation", MAIN_SCREEN_MID_ROW_Y, u8g2_font_6x10_mr);
    u8g2.sendBuffer();
}

void OLEDManager::drawBrightnessAdjustmentScreen(uint16_t brightness) {
    u8g2.drawDisc(xOffset + 5, MAIN_SCREEN_MID_ROW_Y - 5, 3);
    u8g2.drawLine(xOffset + 5, MAIN_SCREEN_MID_ROW_Y - 2,
                  xOffset + 5, MAIN_SCREEN_MID_ROW_Y + 1);
    u8g2.drawLine(xOffset + 3, MAIN_SCREEN_MID_ROW_Y + 1,
                  xOffset + 7, MAIN_SCREEN_MID_ROW_Y + 1);

    int brightnessPercent = map(brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS, 10, 100);
    char brightText[20];
    snprintf(brightText, sizeof(brightText), "%d%% BRIGHT", brightnessPercent);
    drawCenteredText(brightText, MAIN_SCREEN_MID_ROW_Y + 10, u8g2_font_6x10_mr);
}

void OLEDManager::drawLedCountAdjustmentScreen(uint16_t numLeds) {
    char ledText[20];
    snprintf(ledText, sizeof(ledText), "%d LEDs", numLeds);
    drawCenteredText(ledText, MAIN_SCREEN_MID_ROW_Y, u8g2_font_6x10_mr);
}

void OLEDManager::drawNormalOperationScreen(uint16_t patternIndex, const char* patternName,
                                           uint16_t brightness, uint16_t numLeds, bool inShuffleMode) {
    char topRow[40];
    if (inShuffleMode) {
        switch (patternIndex) {
            case 0: snprintf(topRow, sizeof(topRow), "R Shuffle"); break;
            case 1: snprintf(topRow, sizeof(topRow), "5s Shuffle"); break;
            case 2: snprintf(topRow, sizeof(topRow), "10S Shuffle"); break;
            case 3: snprintf(topRow, sizeof(topRow), "5m Shuffle"); break;
            default: snprintf(topRow, sizeof(topRow), "Shuffle");
        }
    } else {
        snprintf(topRow, sizeof(topRow), "%s", patternName);
    }
    u8g2.setFont(u8g2_font_5x7_mr);
    u8g2.setDrawColor(0); // Set draw color to background (usually 0 for black)
    u8g2.setDrawColor(1); // Set draw color back to foreground
    drawLeftText(topRow, MAIN_SCREEN_TOP_ROW_Y, u8g2_font_6x10_mr);
    char middleRow[20];
    if (inShuffleMode) {
        snprintf(middleRow, sizeof(middleRow), "%s", patternName);
        drawLeftText(middleRow, MAIN_SCREEN_MID_ROW_Y, u8g2_font_6x10_mr);
    } else {
        snprintf(middleRow, sizeof(middleRow), "#%d", patternIndex);
        drawCenteredText(middleRow, MAIN_SCREEN_MID_ROW_Y, u8g2_font_8x13_mr);
    }

    char bottomRow[30];
    int brightnessPercent = map(brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS, 20, 100);
    snprintf(bottomRow, sizeof(bottomRow), "B:%d%% L:%d", brightnessPercent, numLeds);
    drawCenteredText(bottomRow, MAIN_SCREEN_BOT_ROW_Y, u8g2_font_6x10_mr);
}

void OLEDManager::drawCenteredText(const char* text, int y, const uint8_t* font) {
    u8g2.setFont(font);
    int x = (screenW - u8g2.getStrWidth(text)) / 2;
    u8g2.drawStr(x, y, text);
}


void OLEDManager::drawProgressBar(int x, int y, int width, int height, int percentage) {
    percentage = constrain(percentage, 0, 100);
    int filledWidth = (width * percentage) / 100;

    u8g2.drawFrame(x, y, width, height);
    if (filledWidth > 0) {
    }
}

void OLEDManager::setSystemManager(SystemManager* sysManager) {
    systemManager = sysManager;
    Serial.println(F("OLEDManager: SystemManager reference set"));
}

void OLEDManager::drawLeftText(const char* text, int y, const uint8_t* font) {

    u8g2.setFont(font);

    u8g2.drawStr(X_OFFSET, y, text);
}
