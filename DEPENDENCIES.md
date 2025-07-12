# ESP32 Basic Wearable FastLED - Dependencies

This document lists all dependencies required for this project and provides installation instructions.

## Required Libraries

| Library Name | Version | Repository/Source | Required |
|-------------|---------|-------------------|----------|
| FastLED | 3.7.0 | https://github.com/FastLED/FastLED | Yes |
| OneButton | 2.0.3 | https://github.com/mathertel/OneButton | Yes |
| U8g2 | 2.35.9 | https://github.com/olikraus/u8g2 | Optional* |

*U8g2 is only required if you want to use an OLED display with your wearable. To disable OLED functionality, set `ENABLE_OLED` to 0 in `Config.h`.

### U8g2 Installation Note

When using PlatformIO, the U8g2 library should be installed via the PlatformIO Library Registry:

### Method 1: Using PlatformIO

The easiest way to install all dependencies is to use PlatformIO. The project's `platformio.ini` file already includes all required libraries:
