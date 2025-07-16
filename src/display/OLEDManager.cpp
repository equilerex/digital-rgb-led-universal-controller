/**
 * OLED Display Manager Implementation
 */

#include "OLEDManager.h"
#include <Arduino.h>
#include "../config/Config.h"
#include "../animations/AnimationManager.h"
#include "../system/SystemManager.h"


#include <U8g2lib.h>
#define OLED_RESET U8X8_PIN_NONE  // Reset pin
#define OLED_SDA 5                      // SDA pin for I2C
#define OLED_SCL 6                      // SCL pin for I2C
#define OLED_WIDTH 72                   // Actual OLED display width
#define OLED_HEIGHT 40                  // Actual OLED display height
#define OLED_X_OFFSET 30                // = (128-72)/2 - Center horizontally
#define OLED_Y_OFFSET 12                // = (64-40)/2 - Center vertically


OLEDManager::OLEDManager() :
    u8g2(U8G2_R0, OLED_RESET, OLED_SCL, OLED_SDA),
    available(false) {}

void OLEDManager::begin() {
    Serial.println(F("OLED initialization starting..."));

    // Initialize OLED display
    u8g2.begin();
    Serial.println(F("OLED display begin() called"));

    // Use lower contrast to reduce power requirements
    u8g2.setContrast(128);     // set contrast to 50% (128) instead of maximum (255)

    // Use lower I2C bus clock to improve stability
    u8g2.setBusClock(400000);  // 400kHz I2C (standard I2C speed)

    // Display startup message
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_4x6_tr);

    // Calculate centered text position
    int16_t titleWidth = u8g2.getStrWidth("Jo's blinky");
    int16_t titleX = (128 - titleWidth) / 2;
    u8g2.drawStr(titleX, OLED_Y_OFFSET + 30, "Jo's blinky");
    u8g2.drawStr(OLED_X_OFFSET, OLED_Y_OFFSET + 50, "Boot...");
    u8g2.sendBuffer();

    Serial.println(F("OLED initialization complete"));
    available = true;
}

void OLEDManager::begin(SystemManager* sysManager) {
    // Set the system manager reference
    setSystemManager(sysManager);
    
    // Call the main begin method
    begin();
}

    void OLEDManager::update() {
    // Check if OLED is available
    if (!available) {
        EVERY_N_SECONDS(5) {
            Serial.println(F("OLED not available..."));
        }

        // Don't try to use the OLED if it's not available
        return;
    }

    // Always handle errors before accessing hardware
    try {
        // Display error message on OLED
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_4x6_tr);

    // Check if animation manager is available
    if (!systemManager->getAnimationManager()) {
        EVERY_N_SECONDS(5) {
            Serial.println(F("Leds loading"));
        }
        // Display waiting message on OLED
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_4x6_tr);
        drawCenteredText("Starting up...", OLED_Y_OFFSET + 30, u8g2_font_4x6_tr);
        u8g2.sendBuffer();
        return;
    }

    // Check if system manager is properly set up
    if (!systemManager) {
        EVERY_N_SECONDS(5) {
            Serial.println(F("System manager not available..."));
        }
        // Display error message on OLED
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_8x13_tr);
        drawCenteredText("System Error", OLED_Y_OFFSET + 30, u8g2_font_8x13_tr);
        u8g2.sendBuffer();
        return;
    }

    // Check if animation manager is available
    if (!systemManager->getAnimationManager()) {
        EVERY_N_SECONDS(5) {
            Serial.println(F("Animation manager not available..."));
        }
        // Display waiting message on OLED
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_8x13_tr);
        drawCenteredText("Starting up...", OLED_Y_OFFSET + 30, u8g2_font_8x13_tr);
        u8g2.sendBuffer();
        return;
    }

    // Safely get animation manager once
    AnimationManager* animManager = systemManager->getAnimationManager();
    // This is redundant with the check above, but keeping for code robustness
    if (!animManager) {
        return;
    }

    // Check if AnimationManager is fully initialized and ready
    if (!animManager->isReady()) {
        // Display initialization message while waiting for AnimationManager
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_8x13_tr);
        drawCenteredText("Initializing...", OLED_Y_OFFSET + 30, u8g2_font_8x13_tr);
        u8g2.sendBuffer();
        return;
    }

    // Get data from animation manager with failsafe defaults
    uint16_t numLedsActual = 0;
    uint16_t brightnessActual = 0;
    uint16_t patternNumber = 0;
    const char* patternNameActual = "Unknown";
    bool brightnessMode = false;
    bool ledCountMode = false;

    // Safely get values with null checks
    try {
        numLedsActual = animManager->getNumLeds();
        brightnessActual = animManager->getBrightness();
        patternNumber = animManager->getCurrentPatternIndex();
        const char* tempName = animManager->getCurrentPatternName();
        if (tempName) {
            patternNameActual = tempName;
        }

        // Only try to access input manager if it's available
        // Using &systemManager->getInputManager() to get a pointer rather than the object itself
        InputManager* inputManager = &systemManager->getInputManager();
        if (inputManager != nullptr) {
            brightnessMode = inputManager->isBrightnessMode();
            ledCountMode = inputManager->isLedCountMode();
        }
    } catch (...) {
        // Catch any exceptions to prevent crashes
        Serial.println(F("ERROR: Exception caught while getting animation data"));
    }

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

        // Handle shuffle mode display differently
        if (patternNumber == 0) {
            // In shuffle mode, show the current shuffle animation name
            const char* shuffleName = animManager->getCurrentPatternName();
            snprintf(indBuf, sizeof(indBuf), "%s", shuffleName);
        } else {
            snprintf(indBuf, sizeof(indBuf), "# %d", patternNumber);
        }

        snprintf(statusBuf, sizeof(statusBuf), "B:%d%% L:%d",
                map(brightnessActual, MIN_BRIGHTNESS, MAX_BRIGHTNESS, 20, 100),
                numLedsActual);

        // Draw all elements
        u8g2.setFont(u8g2_font_4x6_tr);
        u8g2.drawStr(OLED_X_OFFSET, OLED_Y_OFFSET + 20, patternBuf);

        // Use smaller font for shuffle mode
        if (patternNumber == 0) {
            u8g2.setFont(u8g2_font_4x6_tr);  // Smaller font for shuffle animation name
        } else {
            u8g2.setFont(u8g2_font_8x13_tr);  // Normal font for pattern number
        }
        u8g2.drawStr(OLED_X_OFFSET, OLED_Y_OFFSET + 35, indBuf);

        u8g2.setFont(u8g2_font_4x6_tr);
        u8g2.drawStr(OLED_X_OFFSET, OLED_Y_OFFSET + 50, statusBuf);
    }

        // Send the buffer to the display
        u8g2.sendBuffer();

        // Log completion of update
        EVERY_N_SECONDS(5) {
            Serial.println(F("DEBUG: OLEDManager::update() completed successfully"));
        }
    } catch (const std::exception& e) {
        Serial.println(F("ERROR: Exception in OLEDManager::update()"));
    } catch (...) {
        Serial.println(F("ERROR: Unknown exception in OLEDManager::update()"));
    }
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

 void OLEDManager::displayBootMessage(const char* message) {
    // Only display if OLED is available
    if (!available) {
        Serial.println(F("DEBUG: Cannot display boot message, OLED not available"));
        return;
    }

    // Display a simple boot message
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_8x13_tr);

    // Draw title
    drawCenteredText("Jo's blinky", OLED_Y_OFFSET + 30, u8g2_font_8x13_tr);

    // Draw message
    drawCenteredText(message, OLED_Y_OFFSET + 50, u8g2_font_8x13_tr);

    // Send to display
    u8g2.sendBuffer();

    Serial.print(F("DEBUG: Boot message displayed: "));
    Serial.println(message);
 }

void OLEDManager::setSystemManager(SystemManager* sysManager) {
    systemManager = sysManager;
    Serial.println(F("OLEDManager: SystemManager reference set"));
}