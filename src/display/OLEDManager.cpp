/**
 * OLED Display Manager Implementation
 */

#include "OLEDManager.h"
#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "../config/Config.h"
#include "../animations/AnimationManager.h"
#include "../system/SystemManager.h"

OLEDManager::OLEDManager() : 
    u8g2(U8G2_R0, OLED_RESET, OLED_SCL, OLED_SDA),
    available(false) {}

void OLEDManager::begin() {
    Serial.println(F("OLED initialization starting..."));
    
    // Initialize Wire before u8g2.begin() (required for ESP32)
    Wire.begin(OLED_SDA, OLED_SCL);
    delay(50);
    
    // Initialize OLED display
    u8g2.begin();
    Serial.println(F("OLED display begin() called"));
    
    // Use lower contrast to reduce power requirements
    u8g2.setContrast(128);     // set contrast to 50% (128) instead of maximum (255)
    
    // Use lower I2C bus clock to improve stability
    u8g2.setBusClock(400000);  // 400kHz I2C (standard I2C speed)
    
    // Display startup message
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_8x13_tr); 
    
    // Calculate centered text position
    int16_t titleWidth = u8g2.getStrWidth("Jo's blinky");
    int16_t titleX = (128 - titleWidth) / 2;
    u8g2.drawStr(titleX, OLED_Y_OFFSET + 30, "Jo's blinky");
    u8g2.drawStr(OLED_X_OFFSET, OLED_Y_OFFSET + 50, "Boot...");
    u8g2.sendBuffer(); 
    
    Serial.println(F("OLED initialization complete"));
    available = true;
}

    void OLEDManager::update() {
    // Check if OLED is available and system is properly set up
    if (!available || !systemManager || !systemManager->getAnimationManager()) {
        EVERY_N_SECONDS(5) {
            Serial.println(F("OLED not ready yet..."));
        }
        return;
    }
    
    // Safely get animation manager once
    AnimationManager* animManager = systemManager->getAnimationManager();
    if (!animManager) {
        return;
    }
    
    // Get data from animation manager
    uint16_t numLedsActual = animManager->getNumLeds();
    uint16_t brightnessActual = animManager->getBrightness();
    uint16_t patternNumber = animManager->getCurrentPatternIndex();
    const char* patternNameActual = animManager->getCurrentPatternName();
    bool brightnessMode = systemManager->getInputManager().isBrightnessMode();
    bool ledCountMode = systemManager->getInputManager().isLedCountMode();
    
    // Clear buffer and prepare for drawing
    u8g2.clearBuffer();
    
    // Buffer declarations - prepare these before drawing operations
    char patternBuf[40] = {0};
    char statusBuf[30] = {0};
    char indBuf[30] = {0};
    char brightBuf[30] = {0};
    char stripBuf[30] = {0};
    
    // Different display modes based on current system state
    if (brightnessMode) {
        // Draw a small lightbulb icon
        u8g2.drawDisc(OLED_X_OFFSET + 5, OLED_Y_OFFSET + 20, 3);  // Bulb circle
        u8g2.drawLine(OLED_X_OFFSET + 5, OLED_Y_OFFSET + 23, OLED_X_OFFSET + 5, OLED_Y_OFFSET + 26); // Bulb base
        u8g2.drawLine(OLED_X_OFFSET + 3, OLED_Y_OFFSET + 26, OLED_X_OFFSET + 7, OLED_Y_OFFSET + 26); // Base bottom
        
        int brightPec = map(brightnessActual, MIN_BRIGHTNESS, MAX_BRIGHTNESS, 10, 100);
        snprintf(brightBuf, sizeof(brightBuf), "%d%% BRIGHT", brightPec);
        
        u8g2.setFont(u8g2_font_8x13_tr);    
        u8g2.drawStr(OLED_X_OFFSET + 15, OLED_Y_OFFSET + 35, brightBuf); 
    } 
    else if (ledCountMode) { 
        snprintf(stripBuf, sizeof(stripBuf), "%d LED's", numLedsActual); 
        
        u8g2.setFont(u8g2_font_8x13_tr);
        // Center the LED count text
        int16_t stripWidth = u8g2.getStrWidth(stripBuf);
        int16_t stripX = (128 - stripWidth) / 2;
        u8g2.drawStr(stripX, OLED_Y_OFFSET + 35, stripBuf);
    } 
    else {
        // Format the strings before drawing
        if (patternNameActual) {
            snprintf(patternBuf, sizeof(patternBuf), "#%d: %s", patternNumber, patternNameActual);
        } else {
            snprintf(patternBuf, sizeof(patternBuf), "#%d: Unknown", patternNumber);
        }
        
        snprintf(indBuf, sizeof(indBuf), "# %d", patternNumber);
        
        snprintf(statusBuf, sizeof(statusBuf), "B:%d%% L:%d", 
                map(brightnessActual, MIN_BRIGHTNESS, MAX_BRIGHTNESS, 20, 100),
                numLedsActual); 
        
        // Draw all elements
        u8g2.setFont(u8g2_font_4x6_tr);
        u8g2.drawStr(OLED_X_OFFSET, OLED_Y_OFFSET + 20, patternBuf);
    
        u8g2.setFont(u8g2_font_8x13_tr);   
        u8g2.drawStr(OLED_X_OFFSET, OLED_Y_OFFSET + 35, indBuf);
        
        u8g2.setFont(u8g2_font_4x6_tr);
        u8g2.drawStr(OLED_X_OFFSET, OLED_Y_OFFSET + 50, statusBuf);
    }
    
    // Send the buffer to the display
    u8g2.sendBuffer();
} 
 // Helper utility functions for OLED drawing
 
 void OLEDManager::drawCenteredText(const char* text, int y, const uint8_t* font) {
    // Set the specified font
    u8g2.setFont(font);
    
    // Calculate the width of the text
    int16_t textWidth = u8g2.getStrWidth(text);
    
    // Calculate the X position to center the text (display width is 128)
    int16_t x = (128 - textWidth) / 2;
    
    // Draw the text at the calculated position
    u8g2.drawStr(x, y, text);
 }
 
 void OLEDManager::drawProgressBar(int x, int y, int width, int height, int percentage) {
    // Ensure percentage is within valid range
    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;
    
    // Calculate the filled width based on percentage
    int filledWidth = (width * percentage) / 100;
    
    // Draw the outline frame
    u8g2.drawFrame(x, y, width, height);
    
    // Draw the filled portion if there is any
    if (filledWidth > 0) {
        u8g2.drawBox(x, y, filledWidth, height);
    }
 }