# Digital-rgb-led-universal-controller
My little universal controller library which i use for all of my projects running WS2812 / WS2812b strips.
Contains a well curated set of animations which fit most simple project straight out of the box


### Features
* Brightness control potentiometer
* color modifier potentiometer (works on most animations)
* mode button
    * 1 click to switch mode
    * Hold 3-4 seconds to adjust brightness (20%-100%)
    * Hold 5-9 seconds to reduce LED strip length by 50 (minimum 50 LEDs)
    * Hold 10+ seconds to increase LED strip length by 50 (maximum 1000 LEDs)

### Animations
(will try to upload a video soon)
* rainbowWithGlitter - scrolling rainbow with some random white glitter here and there
* rainbow_march - same as above but without clitter
* sinelon
* two_sin
* pop_fade
* confetti
* juggle,
* bpm
* threeSinTwo
# ESP32 Basic Wearable FastLED Controller

A versatile ESP32-based controller for LED wearables and projects using the FastLED library. This project offers multiple animation patterns, user control via buttons, optional OLED display support, and easy configuration.

![ESP32 Wearable Demo](images/demo.jpg)

## Features

- **Multiple Animation Categories:**
    - Slow & Soothing animations (Breathing, Aurora, Color Meditation, etc.)
    - Solid Colors
    - High BPM animations
    - Party Vibe animations
    - Psychedelic animations
    - And more!

- **Hardware Control:**
    - Single button interface for switching animations and entering different modes
    - Long-press functionality for adjusting brightness and LED count
    - Optional OLED display for status information

- **ESP32 Optimized:**
    - Designed for ESP32-C3 boards (can be adapted for other ESP32 variants)
    - Low power consumption for battery-powered wearables
    - Efficient memory usage
    - Enhanced watchdog timer handling to prevent boot loops
    - Ultra-safe LED initialization for reliability

## Watchdog and Safe Boot Changes

Recent updates have significantly improved stability and reliability:

1. **Ultra-Safe LED Initialization**:
    - Multi-stage initialization starting with just 1 LED
    - Progressive brightness ramping to prevent power surges
    - Extensive yield() calls to feed the watchdog timer
    - Multiple safety delays between critical operations

2. **Watchdog Timer Management**:
    - Optional watchdog disabling during critical initialization
    - Adjustable watchdog timeout settings in Config.h
    - Recovery mechanisms if the main loop takes too long

3. **Memory Optimization**:
    - LED arrays properly allocated on the heap
    - Smart memory management with proper cleanup
    - More conservative default values for LED count and brightness

4. **Emergency Recovery**:
    - System can detect when it's struggling and enter safe mode
    - Emergency shutdown procedure to reset to a known good state

## Hardware Requirements

- ESP32-C3 development board (or other ESP32 variants with minor modifications)
- WS2812B LED strip (or other FastLED compatible LEDs)
- Momentary pushbutton
- Optional:
    - SSD1306 OLED display (128x64)
    - Potentiometers for brightness/color adjustment

## Wiring Diagram

Basic wiring:
- Connect LED strip data pin to GPIO8 (configurable in Config.h)
- Connect button to GPIO9 (configurable in Config.h)
- Optional: Connect SSD1306 OLED display via I2C (SDA to GPIO5, SCL to GPIO6)

## Installation and Setup

### Using PlatformIO (Recommended)

1. **Install PlatformIO:**
    - Install [VS Code](https://code.visualstudio.com/)
    - Install the [PlatformIO extension](https://platformio.org/install/ide?install=vscode)

2. **Clone and Open the Project:**
   ```bash
   git clone https://github.com/yourusername/esp32-basic-wearable-fastled.git
   cd esp32-basic-wearable-fastled
   code .  # Open with VS Code
   ```

3. **Install Dependencies:**
   PlatformIO will automatically install the required libraries when you build.

4. **Configure the Project:**
    - Edit `src/config/Config.h` to match your hardware setup
    - Set `DEFAULT_NUM_LEDS` to your LED strip length
    - Set `LED_TYPE` to your LED strip type (default: WS2812)
    - Set `ENABLE_OLED` to 0 if you don't have an OLED display

5. **Build and Upload:**
    - Connect your ESP32 to your computer
    - Click the PlatformIO "Upload" button in VS Code
    - Or use the terminal commands:
      ```bash
      # First ensure all libraries are installed
      pio lib install
      
      # Then build and upload for your board:
      # For ESP32-C3 DevKitM-1:
      pio run -e esp32-c3-devkitm-1 -t upload

      # For ESP32-S3 DevKitC-1:
      pio run -e esp32-s3-devkitc-1 -t upload

      # For ESP32-D1 Mini (LOLIN32):
      pio run -e esp32-d1-mini -t upload

      # For ESP32 DevKit V1:
      pio run -e esp32-devkit-v1 -t upload
      ```
    - Replace the `-e <env>` argument with your board's environment name as listed in `platformio.ini`.

   # PlatformIO method to clean project build artifacts, such as object files and compiled code. This helps to remove outdated or unnecessary files generated during the build process, allowing for a clean build environment.
   pio run -t clean

6. **Monitor Serial Output (Optional):**
   ```bash
   pio device monitor
   ```

### Common PlatformIO Commands

note: if you want to remove (or add) any animations, just remove from or add names to `SimplePatternList` object

## Editing Pin Assignments

**Important:**  
FastLED requires the LED data pin to be a compile-time constant.  
You **cannot** change the LED pin using PlatformIO build flags or `platformio.ini`.  
Instead, you must edit the pin assignments directly in `src/config/PinConfig.h` using board-specific `#ifdef` blocks.

### Default Pin Assignments

Below are the default pins for each supported board as defined in `src/config/PinConfig.h`:

| Board                   | LED_DATA_PIN | BUTTON_1_PIN | OLED_SDA | OLED_SCL |
|-------------------------|--------------|--------------|----------|----------|
| ESP32-C3 DevKit         | 8            | 9            | 5        | 6        |
| ESP32-S3 DevKitC-1      | 4            | 0            | 8        | 9        |
| ESP32-D1 Mini (LOLIN32) | 4            | 13           | 21       | 22       |
| ESP32 DevKit V1         | 4            | 13           | 21       | 22       |

### How to Change Pins

1. **Open `src/config/PinConfig.h`**

2. **Find your board's section.**  
   For example, for ESP32 DevKit V1:
   ```cpp
   // --- Classic ESP32 DevKit V1 ---
   #elif defined(ARDUINO_ESP32_DEV)
     #ifndef OLED_SDA
       #define OLED_SDA 21
     #endif
     #ifndef OLED_SCL
       #define OLED_SCL 22
     #endif
     #ifndef LED_DATA_PIN
       #define LED_DATA_PIN 4
     #endif
     #ifndef BUTTON_1_PIN
       #define BUTTON_1_PIN 13
     #endif
   ```

3. **Edit the pin numbers as needed.**  
   For example, to change the LED data pin to GPIO 5:
   ```cpp
   #ifndef LED_DATA_PIN
     #define LED_DATA_PIN 5
   #endif
   ```

4. **Save the file and rebuild/upload your firmware.**

### Quick Reference

- **Upload for a specific board:**  
  `pio run -e <env> -t upload`  
  (Replace `<env>` with your board's environment name.)

- **Edit pins:**  
  Change pin defines in `src/config/PinConfig.h` under your board's section.

### Why not use PlatformIO build flags?

FastLED requires the LED data pin to be a static constant at compile time.  
PlatformIO build flags (`-D CONFIG_LED_DATA_PIN=...`) do **not** work for this purpose.  
Always set your pins in `PinConfig.h` using the correct board `#ifdef` block.

### Author

**Joosep Kõivistik** - [homepage](http://koivistik.com) |  [youtube](https://www.youtube.com/channel/UCqMFsfxrBrQIHnIKoJjqHTA) | |  [Instagram](https://www.instagram.com/joosepkoivistik/)


## Acknowledgments

Big thanks to:
* Mark Kriegsman (for all the lovely animations)
* [Andrew Tuline](https://github.com/atuline/FastLED-Demos) for the original repository

### Boards confirmed to work on:
