/**
 * OLED Display Manager Implementation
 */
#include "OLEDManager.h"
#include <Arduino.h>
#include <Wire.h>
#include "../config/Config.h"
#include "../animations/AnimationManager.h"
#include "../controls/InputManager.h"
#include "../system/SystemManager.h"

// Display constants
#define OLED_RESET U8X8_PIN_NONE
#define OLED_SDA 5 // Primary I2C pins
#define OLED_SCL 6
#define OLED_SDA_ALT 8 // Alternate I2C pins
#define OLED_SCL_ALT 9
#define OLED_WIDTH 72
#define OLED_HEIGHT 40
#define OLED_X_OFFSET 30
#define OLED_Y_OFFSET 12

// Screen row positions
#define MAIN_SCREEN_TOP_ROW_Y 22
#define MAIN_SCREEN_MID_ROW_Y 35
#define MAIN_SCREEN_BOT_ROW_Y 48

OLEDManager::OLEDManager()
    : u8g2(U8G2_R0, OLED_RESET, OLED_SCL, OLED_SDA),
      available(false),
      systemManager(nullptr)
{}

void OLEDManager::begin() {
    Serial.println(F("OLED initialization starting..."));

    // Try primary I2C pins (GPIO 5/6)
    Serial.println(F("Trying primary I2C pins (SDA=5, SCL=6)..."));
    Wire.begin(OLED_SDA, OLED_SCL);
    Wire.setClock(100000); // 100kHz for stability
    Wire.beginTransmission(0x3C); // SSD1306 default address
    if (Wire.endTransmission() == 0) {
        Serial.println(F("I2C device found at address 0x3C"));
    } else {
        Serial.println(F("[ERROR] No I2C device found on primary pins! Trying alternate pins (SDA=8, SCL=9)..."));
        // Try alternate I2C pins (GPIO 8/9)
        Wire.begin(OLED_SDA_ALT, OLED_SCL_ALT);
        Wire.setClock(100000);
        Wire.beginTransmission(0x3C);
        if (Wire.endTransmission() == 0) {
            Serial.println(F("I2C device found at address 0x3C on alternate pins"));
            u8g2.setI2CAddress(0x3C); // Update address
        } else {
            Serial.println(F("[CRITICAL] No I2C devices found on either pin set! Check wiring, pull-ups (4.7kΩ), or OLED."));
            return;
        }
    }

    if (!u8g2.begin()) {
        Serial.println(F("[CRITICAL] OLED u8g2.begin() failed! Check I2C wiring or pins."));
        return;
    }
    Serial.println(F("OLED display initialized"));

    u8g2.setContrast(128);
    u8g2.setBusClock(400000);
    u8g2.clearBuffer();

    // Simple boot animation like old version
    for (int i = 0; i < 3; i++) {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tr);
        drawCenteredText("Jo's blinky", 15, u8g2_font_6x10_tr);
        u8g2.setFont(u8g2_font_4x6_tr);
        drawCenteredText("1.69+", 25, u8g2_font_4x6_tr);
        char dots[4] = ".";
        for (int j = 0; j <= i; j++) dots[j] = '.';
        dots[i+1] = '\0';
        drawCenteredText(dots, 35, u8g2_font_4x6_tr);
        u8g2.sendBuffer();
        delay(500);
    }

    available = true;
    Serial.println(F("OLED ready"));
}

void OLEDManager::update() {
    if (!available) {
        EVERY_N_SECONDS(5) { Serial.println(F("OLED not available")); }
        return;
    }

    static unsigned long lastUpdateTime = 0;
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime < 200) {
        return;
    }
    lastUpdateTime = currentTime;

    u8g2.clearBuffer();

    if (systemManager == nullptr || systemManager->getAnimationManager() == nullptr) {
        drawSystemErrorScreen();
    } else if (!systemManager->getAnimationManager()->isReady()) {
        drawStartupScreen();
    } else {
        bool isBrightnessMode = false;
        bool isLedCountMode = false;
        uint16_t numLeds = 0;
        uint16_t brightness = 0;
        uint16_t patternIndex = 0;

        AnimationManager* animManager = systemManager->getAnimationManager();
        InputManager* inputManager = &systemManager->getInputManager();
        if (inputManager != nullptr) {
            isBrightnessMode = inputManager->isBrightnessMode();
            isLedCountMode = inputManager->isLedCountMode();
        }

        numLeds = animManager->getNumLeds();
        brightness = animManager->getBrightness();
        patternIndex = animManager->getCurrentPatternIndex();
        const char* patternName = animManager->getCurrentPatternName();
        if (!patternName) patternName = "Unknown";

        if (isBrightnessMode) {
            drawBrightnessAdjustmentScreen(brightness);
        } else if (isLedCountMode) {
            drawLedCountAdjustmentScreen(numLeds);
        } else {
            drawNormalOperationScreen(patternIndex, patternName, brightness, numLeds);
        }
    }

    u8g2.sendBuffer();
    EVERY_N_SECONDS(5) { Serial.println(F("DEBUG: OLEDManager::update() completed successfully")); }
}

void OLEDManager::displayBootMessage(const char* message) {
    if (!available) return;

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_8x13_tr);
    drawCenteredText("Jo's blinky", OLED_Y_OFFSET + 20, u8g2_font_8x13_tr);
    drawCenteredText(message, OLED_Y_OFFSET + 40, u8g2_font_8x13_tr);
    u8g2.sendBuffer();
}

void OLEDManager::drawStartupScreen() {
    drawCenteredText("Starting...", MAIN_SCREEN_MID_ROW_Y, u8g2_font_8x13_tr);
    u8g2.sendBuffer();
}

void OLEDManager::drawSystemErrorScreen() {
    drawCenteredText("SYS ERROR", MAIN_SCREEN_TOP_ROW_Y, u8g2_font_8x13_tr);
    drawCenteredText("Check manager", MAIN_SCREEN_MID_ROW_Y, u8g2_font_8x13_tr);
    u8g2.sendBuffer();
}

void OLEDManager::drawAnimationErrorScreen() {
    drawCenteredText("ANIM ERROR", MAIN_SCREEN_TOP_ROW_Y, u8g2_font_8x13_tr);
    drawCenteredText("No animation", MAIN_SCREEN_MID_ROW_Y, u8g2_font_8x13_tr);
    u8g2.sendBuffer();
}

void OLEDManager::drawBrightnessAdjustmentScreen(uint16_t brightness) {
    u8g2.drawDisc(OLED_X_OFFSET + 5, MAIN_SCREEN_MID_ROW_Y - 5, 3);
    u8g2.drawLine(OLED_X_OFFSET + 5, MAIN_SCREEN_MID_ROW_Y - 2,
                  OLED_X_OFFSET + 5, MAIN_SCREEN_MID_ROW_Y + 1);
    u8g2.drawLine(OLED_X_OFFSET + 3, MAIN_SCREEN_MID_ROW_Y + 1,
                  OLED_X_OFFSET + 7, MAIN_SCREEN_MID_ROW_Y + 1);

    int brightnessPercent = map(brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS, 10, 100);
    char brightText[20];
    snprintf(brightText, sizeof(brightText), "%d%% BRIGHT", brightnessPercent);
    drawCenteredText(brightText, MAIN_SCREEN_MID_ROW_Y + 10, u8g2_font_8x13_tr);
}

void OLEDManager::drawLedCountAdjustmentScreen(uint16_t numLeds) {
    char ledText[20];
    snprintf(ledText, sizeof(ledText), "%d LEDs", numLeds); // Fixed: Use ledText size
    drawCenteredText(ledText, MAIN_SCREEN_MID_ROW_Y, u8g2_font_8x13_tr);
}

void OLEDManager::drawNormalOperationScreen(uint16_t patternIndex, const char* patternName,
                                           uint16_t brightness, uint16_t numLeds) {
    char topRow[40];
    if (patternIndex == 0) {
        snprintf(topRow, sizeof(topRow), "Shuffle");
    } else {
        snprintf(topRow, sizeof(topRow), "%s", patternName);
    }
    u8g2.setFont(u8g2_font_5x7_tr);
    drawCenteredText(topRow, MAIN_SCREEN_TOP_ROW_Y, u8g2_font_5x7_tr);

    char middleRow[20];
    if (patternIndex == 0) {
        snprintf(middleRow, sizeof(middleRow), "%s", patternName);
        u8g2.setFont(u8g2_font_5x7_tr);
    } else {
        snprintf(middleRow, sizeof(middleRow), "#%d", patternIndex);
        u8g2.setFont(u8g2_font_8x13_tr);
    }
    drawCenteredText(middleRow, MAIN_SCREEN_MID_ROW_Y, u8g2_font_8x13_tr);

    char bottomRow[30];
    int brightnessPercent = map(brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS, 20, 100);
    snprintf(bottomRow, sizeof(bottomRow), "B:%d%% L:%d", brightnessPercent, numLeds);

    u8g2.setFont(u8g2_font_4x6_tr);
    drawCenteredText(bottomRow, MAIN_SCREEN_BOT_ROW_Y, u8g2_font_4x6_tr);
}

void OLEDManager::drawCenteredText(const char* text, int y, const uint8_t* font) {
    u8g2.setFont(font);
    int16_t textWidth = u8g2.getStrWidth(text);
    int16_t x = (128 - textWidth) / 2;
    u8g2.drawStr(x, y, text);
}

void OLEDManager::drawProgressBar(int x, int y, int width, int height, int percentage) {
    percentage = constrain(percentage, 0, 100);
    int filledWidth = (width * percentage) / 100;

    u8g2.drawFrame(x, y, width, height);
    if (filledWidth > 0) {
        u8g2.drawBox(x, y, filledWidth, height);
    }
}

void OLEDManager::setSystemManager(SystemManager* sysManager) {
    systemManager = sysManager;
    Serial.println(F("OLEDManager: SystemManager reference set"));
}