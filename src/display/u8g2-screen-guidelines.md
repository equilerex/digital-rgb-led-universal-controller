Absolutely! Here's a **complete U8g2 OLED Display Library Reference & Best Practices Guide**, tailored for an LLM (like Copilot) to build U8g2-based screen UI for microcontrollers like ESP32.

---

# 📘 U8g2 Library – Base Instructions & Must-Knows

This guide outlines how to use the [U8g2](https://github.com/olikraus/u8g2) graphics library for OLED displays (SSD1306, SH1106, etc.) in microcontroller projects.

---

## ✅ **Basic Initialization**

```cpp
#include <U8g2lib.h>
#include <Wire.h>

// Hardware I2C constructor (most common)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_R0,             // rotation
  /* reset=*/U8X8_PIN_NONE,
  /* clock=*/SCL, 
  /* data=*/SDA
);

void setup() {
  Wire.begin();        // required before u8g2.begin() on ESP32 unless it has an onboard display already wired up.
  u8g2.begin();
}
```

### 🚩 Must Know:

* do not use `Wire.begin()` on esp32c3 - it conflicts with onboard display. for other boards,  **must be called before `u8g2.begin()`** on ESP32 if using custom I2C pins. (if its an integrated display, fx esp32c3 oled, this should not be called or it will break)
* You cannot reconfigure the display after construction; recreate the object if needed.
* Use the `*_F_` version (page-buffered, full RAM) for best performance unless you're extremely RAM-constrained.


---

## 🧰 **Core Drawing Workflow**

1. **Start frame**:

   ```cpp
   u8g2.clearBuffer(); // Clear internal RAM
   ```

2. **Draw something**:

   ```cpp
   u8g2.setFont(u8g2_font_6x10_tr);
   u8g2.drawStr(0, 20, "Hello OLED!");
   ```

3. **Render to screen**:

   ```cpp
   u8g2.sendBuffer();  // Push to display
   ```

> ⚠️ You must call `clearBuffer()` and `sendBuffer()` on every full redraw. U8g2 does not retain display state between frames.

---

## ✍️ **Text Handling**

* Set font:

  ```cpp
  u8g2.setFont(u8g2_font_ncenB08_tr);
  ```

* Draw text at x, y:

  ```cpp
  u8g2.drawStr(10, 42, "Hello World!");
  ```

* Dynamic text:

  ```cpp
  char buf[20];
  sprintf(buf, "FPS: %d", fps);
  u8g2.drawStr(0, 10, buf);
  ```

* Fonts:

    * `*_tr` = transparent (recommended)
    * `*_t0r` = single-byte
    * `u8g2_font_4x6_tr` = small
    * `u8g2_font_8x13_tr` = medium readable
    * [Font list here](https://github.com/olikraus/u8g2/wiki/fntlistall)

---

## 🔳 **Shapes & Graphics**

| Function                               | Description      |
| -------------------------------------- | ---------------- |
| `drawBox(x, y, w, h)`                  | Solid rectangle  |
| `drawFrame(x, y, w, h)`                | Hollow rectangle |
| `drawCircle(x, y, r)`                  | Outline circle   |
| `drawDisc(x, y, r)`                    | Solid circle     |
| `drawLine(x0, y0, x1, y1)`             | Straight line    |
| `drawTriangle(x0, y0, x1, y1, x2, y2)` | Triangle         |
| `drawPixel(x, y)`                      | Single pixel     |

---

## 💡 **Best Practices & Animation Tips**

### 🔄 Refresh Loop

* Structure your screen updates like this:

  ```cpp
  void loop() {
    u8g2.clearBuffer();
    
    // Draw elements here...
    
    u8g2.sendBuffer();
    delay(33);  // ~30 FPS
  }
  ```

### ⚠️ Avoid:

* Excessive font switching mid-frame
* Drawing outside of bounds
* Redrawing more than 1–2 times per frame without changes
* Using too many large fonts (RAM usage!)

---

## 🎨 **Design Tips for Pleasing UIs**

| Goal                | Tips                                                                           |
| ------------------- | ------------------------------------------------------------------------------ |
| Readability         | Use consistent spacing and alignment                                           |
| Information Density | Use small fonts like `4x6_tr` for technical data                               |
| Aesthetic           | Combine icon + text (e.g., `drawDisc` + label)                                 |
| Feedback            | Use animation (bouncing dots, blinking indicators) to show loading or activity |
| Audio-Reactiveness  | Map volume, bass, or BPM to bar heights, pulse radius, etc.                    |

---

## 📈 **Dynamic Layout Patterns**

* **Left-aligned list**:

  ```cpp
  int y = 10;
  for (int i = 0; i < 5; i++) {
    sprintf(buf, "Item %d", i + 1);
    u8g2.drawStr(0, y, buf);
    y += 10;
  }
  ```

* **Centered text**:

  ```cpp
  int16_t tx = (128 - u8g2.getStrWidth("Center!")) / 2;
  u8g2.drawStr(tx, 32, "Center!");
  ```

---

## 🧪 Example Snippet

```cpp
u8g2.clearBuffer();

u8g2.setFont(u8g2_font_8x13_tr);
u8g2.drawStr(0, 10, "Status:");
u8g2.drawBox(0, 12, map(analogRead(A0), 0, 1023, 0, 128), 10);

u8g2.setFont(u8g2_font_4x6_tr);
sprintf(buf, "FPS: %d", fps);
u8g2.drawStr(100, 64, buf);

u8g2.sendBuffer();
```

---

## ✅ Copilot Prompt Seed (Short Form)

> You're coding a microcontroller OLED UI using U8g2. Use `u8g2.clearBuffer()` and `u8g2.sendBuffer()` every frame. Use small fonts like `u8g2_font_4x6_tr` or `u8g2_font_8x13_tr` for status text. Draw shapes (box, disc, line) to visualize audio or status. Avoid blocking delays, and use `Wire.begin()` before `u8g2.begin()`. Animate using `millis()` or external state. Keep screens clean, centered, and reactive.

---


esp32c3-oled-supermini.md
``markdown``
    
    #include <Arduino.h>
    #include <WS2812FX.h>
    #include <U8g2lib.h>
    
    #ifdef U8X8_HAVE_HW_SPI
    #include <SPI.h>
    #endif
    #ifdef U8X8_HAVE_HW_I2C
    #include <Wire.h>
    #endif
    #define LED_COUNT 1
    #define LED_PIN 2
    #define SDA_PIN 5
    #define SCL_PIN 6
    
    
    WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);
    
    /*
    U8g2lib Example Overview:
    Frame Buffer Examples: clearBuffer/sendBuffer. Fast, but may not work with all Arduino boards because of RAM consumption
    Page Buffer Examples: firstPage/nextPage. Less RAM usage, should work with all Arduino boards.
    U8x8 Text Only Example: No RAM usage, direct communication with display controller. No graphics, 8x8 Text only.
    
    */
    
    // Please UNCOMMENT one of the contructor lines below
    // U8g2 Contructor List (Frame Buffer)
    // The complete list is available here: https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
    // Please update the pin numbers according to your setup. Use U8X8_PIN_NONE if the reset pin is not connected
    //U8G2_NULL u8g2(U8G2_R0);	// null device, a 8x8 pixel display which does nothing
    //U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 12, /* dc=*/ 4, /* reset=*/ 6);	// Arduboy (Production, Kickstarter Edition)
    //U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1306_128X64_NONAME_F_3W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* reset=*/ 8);
    //U8G2_SSD1306_128X64_NONAME_F_3W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* reset=*/ 8);
    //U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
    //U8G2_SSD1306_128X64_ALT0_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // same as the NONAME variant, but may solve the "every 2nd line skipped" problem
    //U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* reset=*/ 8);
    //U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display
    //U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 16, /* data=*/ 17, /* reset=*/ U8X8_PIN_NONE);   // ESP32 Thing, pure SW emulated I2C
    //U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 16, /* data=*/ 17);   // ESP32 Thing, HW I2C with pin remapping
    //U8G2_SSD1306_128X64_NONAME_F_6800 u8g2(U8G2_R0, 13, 11, 2, 3, 4, 5, 6, A4, /*enable=*/ 7, /*cs=*/ 10, /*dc=*/ 9, /*reset=*/ 8);
    //U8G2_SSD1306_128X64_NONAME_F_8080 u8g2(U8G2_R0, 13, 11, 2, 3, 4, 5, 6, A4, /*enable=*/ 7, /*cs=*/ 10, /*dc=*/ 9, /*reset=*/ 8);
    //U8G2_SSD1306_128X64_VCOMH0_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);	// same as the NONAME variant, but maximizes setContrast() range
    //U8G2_SSD1306_128X64_ALT0_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);	// same as the NONAME variant, but may solve the "every 2nd line skipped" problem
    //U8G2_SSD1306_102X64_EA_OLEDS102_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);	// same as the NONAME variant, but may solve the "every 2nd line skipped" problem
    //U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
    //U8G2_SH1106_128X64_VCOMH0_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);		// same as the NONAME variant, but maximizes setContrast() range
    //U8G2_SH1106_128X64_WINSTAR_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);		// same as the NONAME variant, but uses updated SH1106 init sequence
    //U8G2_SH1106_128X32_VISIONOX_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
    //U8G2_SH1106_128X32_VISIONOX_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SH1106_72X40_WISE_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SH1107_64X128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SH1107_128X128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SH1107_128X128_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 8);
    //U8G2_SH1107_PIMORONI_128X128_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 8);
    //U8G2_SH1107_SEEED_128X128_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);
    //U8G2_SH1107_SEEED_128X128_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
    //U8G2_SH1107_SEEED_96X96_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SH1108_160X160_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SH1122_256X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);				// Enable U8G2_16BIT in u8g2.h
    //U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 21, /* data=*/ 20, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather M0 Basic Proto + FeatherWing OLED
    //U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather ESP8266/32u4 Boards + FeatherWing OLED
    //U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);  // Adafruit ESP8266/32u4/ARM Boards + FeatherWing OLED
    //U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   // pin remapping with ESP8266 HW I2C
    //U8G2_SSD1306_128X32_WINSTAR_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   // pin remapping with ESP8266 HW I2C
    //U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // EastRising 0.66" OLED breakout board, Uno: A4=SDA, A5=SCL, 5V powered
    //U8G2_SSD1306_48X64_WINSTAR_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   
    //U8G2_SSD1306_64X32_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
    //U8G2_SSD1306_64X32_1F_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
    //U8G2_SSD1306_96X16_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // EastRising 0.69" OLED
    U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // EastRising 0.42" OLED
    //U8G2_SSD1320_160X132_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  
    //U8G2_SSD1320_160X132_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8); 	
    //U8G2_SSD1322_NHD_256X64_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);	// Enable U8G2_16BIT in u8g2.h
    //U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);	// Enable U8G2_16BIT in u8g2.h
    //U8G2_SSD1322_NHD_128X64_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1322_NHD_128X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1325_NHD_128X64_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1325_NHD_128X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD0323_OS128064_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD0323_OS128064_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1326_ER_256X32_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);         // experimental driver for ER-OLED018-1
    //U8G2_SSD1327_SEEED_96X96_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);	// Seeedstudio Grove OLED 96x96
    //U8G2_SSD1327_SEEED_96X96_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);	// Seeedstudio Grove OLED 96x96
    //U8G2_SSD1327_EA_W128128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1327_EA_W128128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1327_EA_W128128_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 5, /* data=*/ 4, /* reset=*/ U8X8_PIN_NONE);
    //U8G2_SSD1327_EA_W128128_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);  /* Uno: A4=SDA, A5=SCL, add "u8g2.setBusClock(400000);" into setup() for speedup if possible */
    //U8G2_SSD1327_MIDAS_128X128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1327_MIDAS_128X128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1327_MIDAS_128X128_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); /* Uno: A4=SDA, A5=SCL, add "u8g2.setBusClock(400000);" into setup() for speedup if possible */
    //U8G2_SSD1327_WS_128X128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1327_WS_128X128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1327_VISIONOX_128X96_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1327_VISIONOX_128X96_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1329_128X96_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1329_128X96_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1329_96X96_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1329_96X96_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1329_96X96_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ A4, /* dc=*/ A2, /* reset=*/ U8X8_PIN_NONE); //  Adafruit Feather 32u4 Basic Proto
    //U8G2_SSD1305_128X32_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1305_128X32_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1305_128X32_ADAFRUIT_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1305_128X32_ADAFRUIT_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1305_128X64_ADAFRUIT_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1305_128X64_ADAFRUIT_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1305_128X64_RAYSTAR_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1305_128X64_RAYSTAR_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_SSD1309_128X64_NONAME0_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  
    //U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  
    //U8G2_SSD1309_128X64_NONAME2_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  
    //U8G2_SSD1309_128X64_NONAME2_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  
    //U8G2_SSD1316_128X32_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  
    //U8G2_SSD1316_128X32_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8); 	
    //U8G2_SSD1317_96X96_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  // not tested, not confirmed
    //U8G2_SSD1317_96X96_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8); 	// not tested, not confirmed
    //U8G2_SSD1318_128X96_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  
    //U8G2_SSD1318_128X96_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_LD7032_60X32_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 11, /* data=*/ 12, /* cs=*/ 9, /* dc=*/ 10, /* reset=*/ 8);	// SW SPI Nano Board
    //U8G2_LD7032_60X32_F_4W_SW_I2C u8g2(U8G2_R0, /* clock=*/ 11, /* data=*/ 12, /* reset=*/ U8X8_PIN_NONE);	// NOT TESTED!
    //U8G2_LD7032_60X32_ALT_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 11, /* data=*/ 12, /* cs=*/ 9, /* dc=*/ 10, /* reset=*/ 8);	// SW SPI Nano Board
    //U8G2_LD7032_60X32_ALT_F_4W_SW_I2C u8g2(U8G2_R0, /* clock=*/ 11, /* data=*/ 12, /* reset=*/ U8X8_PIN_NONE);	// NOT TESTED!
    //U8G2_UC1701_EA_DOGS102_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_UC1701_EA_DOGS102_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_PCD8544_84X48_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  // Nokia 5110 Display
    //U8G2_PCD8544_84X48_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8); 		// Nokia 5110 Display
    //U8G2_PCF8812_96X65_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);	// Could be also PCF8814
    //U8G2_PCF8812_96X65_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);						// Could be also PCF8814
    //U8G2_HX1230_96X68_F_3W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* reset=*/ 8);
    //U8G2_HX1230_96X68_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_KS0108_128X64_F u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 18, /*dc=*/ 17, /*cs0=*/ 14, /*cs1=*/ 15, /*cs2=*/ U8X8_PIN_NONE, /* reset=*/  U8X8_PIN_NONE); 	// Set R/W to low!
    //U8G2_KS0108_ERM19264_F u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 18, /*dc=*/ 17, /*cs0=*/ 14, /*cs1=*/ 15, /*cs2=*/ 16, /* reset=*/  U8X8_PIN_NONE); 	// Set R/W to low!
    //U8G2_HD44102_100X64_F u8g2(U8G2_R0, 4, 5, 6, 7, 8, 9, 10, 11, /*enable=*/ 2, /*dc=*/ 3, /*cs0=*/ A0, /*cs1=*/ A1, /*cs2=*/ A2, /* reset=*/  U8X8_PIN_NONE); 	// Set R/W to low!
    //U8G2_T7932_150X32_F u8g2(U8G2_R0, 4, 5, 6, 7, 8, 9, 10, 11, /*enable=*/ 2, /*dc=*/ 3, /*cs0=*/ A0, /*cs1=*/ A1, /*cs2=*/ A2, /* reset=*/  U8X8_PIN_NONE); 	// Set R/W to low!
    //U8G2_ST7920_192X32_F_8080 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 18, /*cs=*/ U8X8_PIN_NONE, /*dc=*/ 17, /*reset=*/ U8X8_PIN_NONE);
    //U8G2_ST7920_192X32_F_SW_SPI u8g2(U8G2_R0, /* clock=*/ 18 /* A4 */ , /* data=*/ 16 /* A2 */, /* CS=*/ 17 /* A3 */, /* reset=*/ U8X8_PIN_NONE);
    //U8G2_ST7920_128X64_F_8080 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 18 /* A4 */, /*cs=*/ U8X8_PIN_NONE, /*dc/rs=*/ 17 /* A3 */, /*reset=*/ 15 /* A1 */);	// Remember to set R/W to 0
    //U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, /* clock=*/ 18 /* A4 */ , /* data=*/ 16 /* A2 */, /* CS=*/ 17 /* A3 */, /* reset=*/ U8X8_PIN_NONE);
    //U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* CS=*/ 10, /* reset=*/ 8);
    //U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, /* clock=*/ 14, /* data=*/ 13, /* CS=*/ 15, /* reset=*/ 16); // Feather HUZZAH ESP8266, E=clock=14, RW=data=13, RS=CS
    //U8G2_ST7920_128X64_F_HW_SPI u8g2(U8G2_R0, /* CS=*/ 10, /* reset=*/ 8);
    //U8G2_ST7920_128X64_F_HW_SPI u8g2(U8G2_R0, /* CS=*/ 15, /* reset=*/ 16); // Feather HUZZAH ESP8266, E=clock=14, RW=data=13, RS=CS
    //U8G2_ST7528_ERC16064_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7528_ERC16064_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7565_EA_DOGM128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7565_EA_DOGM128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7565_64128N_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7565_64128N_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7565_EA_DOGM132_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ U8X8_PIN_NONE);	// DOGM132 Shield
    //U8G2_ST7565_EA_DOGM132_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ U8X8_PIN_NONE);	// DOGM132 Shield
    //U8G2_ST7565_ZOLEN_128X64_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7565_ZOLEN_128X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7565_LM6059_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);		// Adafruit ST7565 GLCD
    //U8G2_ST7565_LM6059_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);		// Adafruit ST7565 GLCD
    //U8G2_ST7565_KS0713_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);		// KS0713 controller
    //U8G2_ST7565_KS0713_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);			// KS0713 controller
    //U8G2_ST7565_LX12864_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7565_LX12864_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7565_ERC12864_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7565_ERC12864_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7565_ERC12864_ALT_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8); // contrast improved version for ERC12864
    //U8G2_ST7565_ERC12864_ALT_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  // contrast improved version for ERC12864
    //U8G2_ST7565_NHD_C12832_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7565_NHD_C12832_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7565_NHD_C12864_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7565_NHD_C12864_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7565_JLX12864_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7565_JLX12864_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7567_PI_132X64_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 7, /* dc=*/ 9, /* reset=*/ 8);  // Pax Instruments Shield, LCD_BL=6
    //U8G2_ST7567_PI_132X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 7, /* dc=*/ 9, /* reset=*/ 8);  // Pax Instruments Shield, LCD_BL=6
    //U8G2_ST7567_JLX12864_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 7, /* dc=*/ 9, /* reset=*/ 8);  
    //U8G2_ST7567_JLX12864_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 7, /* dc=*/ 9, /* reset=*/ 8);  
    //U8G2_ST7567_OS12864_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 7, /* dc=*/ 9, /* reset=*/ 8);  
    //U8G2_ST7567_OS12864_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 7, /* dc=*/ 9, /* reset=*/ 8);  
    //U8G2_ST7567_ENH_DG128064_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7567_ENH_DG128064_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7567_ENH_DG128064I_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7567_ENH_DG128064I_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7567_64X32_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
    //U8G2_ST7567_HEM6432_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
    //U8G2_ST7571_128X128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7571_128X128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7571_128X96_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7571_128X96_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7586S_ERC240160_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST7586S_YMC240160_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST75256_JLX172104_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST75256_JLX172104_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST75256_JLX19296_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST75256_JLX19296_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST75256_JLX256128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);	// Enable U8g2 16 bit mode for this display
    //U8G2_ST75256_JLX256128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);		// Enable U8g2 16 bit mode for this display
    //U8G2_ST75256_WO256X128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);	// Enable U8g2 16 bit mode for this display
    //U8G2_ST75256_WO256X128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);		// Enable U8g2 16 bit mode for this display
    //U8G2_ST75256_JLX256128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 9, /* data=*/ 8, /* cs=*/ 7, /* dc=*/ 6, /* reset=*/ 5);  // MKR Zero, Enable U8g2 16 bit mode for this display
    //U8G2_ST75256_JLX256128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 7, /* dc=*/ 6, /* reset=*/ 5);  // MKR Zero, Enable U8g2 16 bit mode for this display
    //U8G2_ST75256_JLX256160_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);	// Enable U8g2 16 bit mode for this display
    //U8G2_ST75256_JLX256160_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);		// Enable U8g2 16 bit mode for this display
    //U8G2_ST75256_JLX256160M_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);	// Enable U8g2 16 bit mode for this display
    //U8G2_ST75256_JLX256160M_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);		// Enable U8g2 16 bit mode for this display
    //U8G2_ST75256_JLX256160_ALT_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);	// Enable U8g2 16 bit mode for this display
    //U8G2_ST75256_JLX256160_ALT_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);		// Enable U8g2 16 bit mode for this display
    //U8G2_ST75256_JLX240160_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST75256_JLX240160_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_ST75256_JLX25664_F_2ND_HW_I2C u8g2(U8G2_R0, /* reset=*/ 8);	// Due, 2nd I2C, enable U8g2 16 bit mode for this display
    //U8G2_ST75320_JLX320240_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);	// Enable U8g2 16 bit mode for this display
    //U8G2_ST75320_JLX320240_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);		// Enable U8g2 16 bit mode for this display
    //U8G2_NT7534_TG12864R_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  
    //U8G2_NT7534_TG12864R_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  
    //U8G2_ST7588_JLX12864_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ 5);  
    //U8G2_ST7588_JLX12864_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 5);
    //U8G2_IST3020_ERC19264_F_6800 u8g2(U8G2_R0, 44, 43, 42, 41, 40, 39, 38, 37,  /*enable=*/ 28, /*cs=*/ 32, /*dc=*/ 30, /*reset=*/ 31); // Connect WR pin with GND
    //U8G2_IST3020_ERC19264_F_8080 u8g2(U8G2_R0, 44, 43, 42, 41, 40, 39, 38, 37,  /*enable=*/ 29, /*cs=*/ 32, /*dc=*/ 30, /*reset=*/ 31); // Connect RD pin with 3.3V
    //U8G2_IST3020_ERC19264_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_IST7920_128X128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  // Round display
    //U8G2_IST7920_128X128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  // Round display
    //U8G2_LC7981_160X80_F_6800 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 18, /*cs=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // Connect RW with GND
    //U8G2_LC7981_160X160_F_6800 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 18, /*cs=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // Connect RW with GND
    //U8G2_LC7981_240X128_F_6800 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 18, /*cs=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // Connect RW with GND
    //U8G2_LC7981_240X64_F_6800 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 18, /*cs=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // Connect RW with GND
    //U8G2_SED1520_122X32_F u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*dc=*/ A0, /*e1=*/ A3, /*e2=*/ A2, /* reset=*/  A4); 	// Set R/W to low!
    //U8G2_T6963_240X128_F_8080 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable/wr=*/ 17, /*cs/ce=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // Connect RD with +5V, FS0 and FS1 with GND
    //U8G2_T6963_256X64_F_8080 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable/wr=*/ 17, /*cs/ce=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // Connect RD with +5V, FS0 and FS1 with GND
    //U8G2_T6963_160X80_F_8080 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable/wr=*/ 17, /*cs/ce=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // Connect RD with +5V, FS0 and FS1 with GND
    //U8G2_T6963_128X64_F_8080 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable/wr=*/ 17, /*cs/ce=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // Connect RD with +5V, FS0 and FS1 with GND
    //U8G2_T6963_128X64_ALT_F_8080 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable/wr=*/ 17, /*cs/ce=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // Connect RD with +5V, FS0 and FS1 with GND
    //U8G2_SED1330_240X128_F_8080 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 17, /*cs=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // Connect RD with +5V, FG with GND
    //U8G2_SED1330_240X128_F_6800 u8g2(U8G2_R0, 13, 11, 2, 3, 4, 5, 6, A4, /*enable=*/ 7, /*cs=*/ 10, /*dc=*/ 9, /*reset=*/ 8); // A0 is dc pin!
    //U8G2_RA8835_NHD_240X128_F_8080 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 17, /*cs=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // Connect /RD = E with +5V, enable is /WR = RW, FG with GND, 14=Uno Pin A0
    //U8G2_RA8835_NHD_240X128_F_6800 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7,  /*enable=*/ 17, /*cs=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // A0 is dc pin, /WR = RW = GND, enable is /RD = E
    //U8G2_UC1601_128X32_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_UC1601_128X64_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_UC1604_JLX19264_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_UC1604_JLX19264_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  
    //U8G2_UC1608_ERC24064_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  // SW SPI, Due ERC24064-1 Test Setup
    //U8G2_UC1608_DEM240064_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  // SW SPI, Due ERC24064-1 Test Setup
    //U8G2_UC1608_ERC240120_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_UC1608_240X128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  // SW SPI, Due ERC24064-1 Test Setup
    //U8G2_UC1610_EA_DOGXL160_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/  U8X8_PIN_NONE);
    //U8G2_UC1610_EA_DOGXL160_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/  U8X8_PIN_NONE);
    //U8G2_UC1611_EA_DOGM240_F_2ND_HW_I2C u8g2(U8G2_R0, /* reset=*/ 8);	// Due, 2nd I2C, DOGM240 Test Board
    //U8G2_UC1611_EA_DOGM240_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);   // Due, SW SPI, DOGXL240 Test Board
    //U8G2_UC1611_EA_DOGXL240_F_2ND_HW_I2C u8g2(U8G2_R0, /* reset=*/ 8);	// Due, 2nd I2C, DOGXL240 Test Board
    //U8G2_UC1611_EA_DOGXL240_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);   // Due, SW SPI, DOGXL240 Test Board
    //U8G2_UC1611_EW50850_F_8080 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7,  /*enable=*/ 18, /*cs=*/ 3, /*dc=*/ 16, /*reset=*/ 17); // 240x160, Connect RD/WR1 pin with 3.3V, CS is aktive high
    //U8G2_UC1611_CG160160_F_8080 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7,  /*enable=*/ 18, /*cs=*/ 3, /*dc=*/ 16, /*reset=*/ 17); // Connect WR1 and CD1 pin with 3.3V, connect CS0 with cs, WR0 with enable, CD with dc
    //U8G2_UC1611_IDS4073_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);	// Enable U8g2 16Bit Mode
    //U8G2_UC1611_IDS4073_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);		// Enable U8g2 16Bit Mode
    //U8G2_UC1617_JLX128128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_UC1617_JLX128128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_UC1638_192X96_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_UC1638_192X96_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_UC1638_192X96_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* reset=*/ 8);  // u8g2 test board: I2C clock/data is same as SPI, , I2C default address is 0x78
    //U8G2_SSD1606_172X72_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);		// eInk/ePaper Display
    //U8G2_SSD1607_200X200_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);	// eInk/ePaper Display, original LUT from embedded artists
    //U8G2_SSD1607_GD_200X200_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);	// Good Display
    //U8G2_SSD1607_WS_200X200_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);	// Waveshare
    //U8G2_IL3820_296X128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);	// WaveShare 2.9 inch eInk/ePaper Display, enable 16 bit mode for this display!
    //U8G2_IL3820_V2_296X128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);	// ePaper Display, lesser flickering and faster speed, enable 16 bit mode for this display!
    //U8G2_MAX7219_64X8_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 11, /* data=*/ 12, /* cs=*/ 10, /* dc=*/ U8X8_PIN_NONE, /* reset=*/ U8X8_PIN_NONE);
    //U8G2_MAX7219_32X8_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 11, /* data=*/ 12, /* cs=*/ 10, /* dc=*/ U8X8_PIN_NONE, /* reset=*/ U8X8_PIN_NONE);
    //U8G2_MAX7219_8X8_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 11, /* data=*/ 12, /* cs=*/ 10, /* dc=*/ U8X8_PIN_NONE, /* reset=*/ U8X8_PIN_NONE);
    //U8G2_LS013B7DH03_128X128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ U8X8_PIN_NONE, /* reset=*/ 8);	// there is no DC line for this display
    //U8G2_LS027B7DH01_400X240_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ U8X8_PIN_NONE, /* reset=*/ 8);	// there is no DC line for this display, SPI Mode 2
    //U8G2_LS027B7DH01_M0_400X240_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ U8X8_PIN_NONE, /* reset=*/ 8);	// there is no DC line for this display, SPI Mode 0
    //U8G2_LS013B7DH05_144X168_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ U8X8_PIN_NONE, /* reset=*/ 8);	// there is no DC line for this display
    //U8G2_ST7511_AVD_320X240_F_8080 u8g2(U8G2_R0, 13, 11, 2, 3, 4, 5, 6, A4, /*enable/WR=*/ 7, /*cs=*/ 10, /*dc=*/ 9, /*reset=*/ 8); // Enable U8g2 16Bit Mode and connect RD pin with 3.3V/5V
    //U8G2_S1D15721_240X64_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    //U8G2_S1D15721_240X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
    
    
    // End of constructor list
    
    
    void u8g2_prepare(void) {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setFontRefHeightExtendedText();
    u8g2.setDrawColor(1);
    u8g2.setFontPosTop();
    u8g2.setFontDirection(0);
    }
    
    void u8g2_box_frame(uint8_t a) {
    u8g2.drawStr( 0, 0, "drawBox");
    u8g2.drawBox(5,10,20,10);
    u8g2.drawBox(10+a,15,30,7);
    u8g2.drawStr( 0, 30, "drawFrame");
    u8g2.drawFrame(5,10+30,20,10);
    u8g2.drawFrame(10+a,15+30,30,7);
    }
    
    void u8g2_disc_circle(uint8_t a) {
    u8g2.drawStr( 0, 0, "drawDisc");
    u8g2.drawDisc(10,18,9);
    u8g2.drawDisc(24+a,16,7);
    u8g2.drawStr( 0, 30, "drawCircle");
    u8g2.drawCircle(10,18+30,9);
    u8g2.drawCircle(24+a,16+30,7);
    }
    
    void u8g2_r_frame(uint8_t a) {
    u8g2.drawStr( 0, 0, "drawRFrame/Box");
    u8g2.drawRFrame(5, 10,40,30, a+1);
    u8g2.drawRBox(50, 10,25,40, a+1);
    }
    
    void u8g2_string(uint8_t a) {
    u8g2.setFontDirection(0);
    u8g2.drawStr(30+a,31, " 0");
    u8g2.setFontDirection(1);
    u8g2.drawStr(30,31+a, " 90");
    u8g2.setFontDirection(2);
    u8g2.drawStr(30-a,31, " 180");
    u8g2.setFontDirection(3);
    u8g2.drawStr(30,31-a, " 270");
    }
    
    void u8g2_line(uint8_t a) {
    u8g2.drawStr( 0, 0, "drawLine");
    u8g2.drawLine(7+a, 10, 40, 55);
    u8g2.drawLine(7+a*2, 10, 60, 55);
    u8g2.drawLine(7+a*3, 10, 80, 55);
    u8g2.drawLine(7+a*4, 10, 100, 55);
    }
    
    void u8g2_triangle(uint8_t a) {
    uint16_t offset = a;
    u8g2.drawStr( 0, 0, "drawTriangle");
    u8g2.drawTriangle(14,7, 45,30, 10,40);
    u8g2.drawTriangle(14+offset,7-offset, 45+offset,30-offset, 57+offset,10-offset);
    u8g2.drawTriangle(57+offset*2,10, 45+offset*2,30, 86+offset*2,53);
    u8g2.drawTriangle(10+offset,40+offset, 45+offset,30+offset, 86+offset,53+offset);
    }
    
    void u8g2_ascii_1() {
    char s[2] = " ";
    uint8_t x, y;
    u8g2.drawStr( 0, 0, "ASCII page 1");
    for( y = 0; y < 6; y++ ) {
    for( x = 0; x < 16; x++ ) {
    s[0] = y*16 + x + 32;
    u8g2.drawStr(x*7, y*10+10, s);
    }
    }
    }
    
    void u8g2_ascii_2() {
    char s[2] = " ";
    uint8_t x, y;
    u8g2.drawStr( 0, 0, "ASCII page 2");
    for( y = 0; y < 6; y++ ) {
    for( x = 0; x < 16; x++ ) {
    s[0] = y*16 + x + 160;
    u8g2.drawStr(x*7, y*10+10, s);
    }
    }
    }
    
    void u8g2_extra_page(uint8_t a)
    {
    u8g2.drawStr( 0, 0, "Unicode");
    u8g2.setFont(u8g2_font_unifont_t_symbols);
    u8g2.setFontPosTop();
    u8g2.drawUTF8(0, 24, "☀ ☁");
    switch(a) {
    case 0:
    case 1:
    case 2:
    case 3:
    u8g2.drawUTF8(a*3, 36, "☂");
    break;
    case 4:
    case 5:
    case 6:
    case 7:
    u8g2.drawUTF8(a*3, 36, "☔");
    break;
    }
    }
    
    #define cross_width 24
    #define cross_height 24
    static const unsigned char cross_bits[] U8X8_PROGMEM  = {
    0x00, 0x18, 0x00, 0x00, 0x24, 0x00, 0x00, 0x24, 0x00, 0x00, 0x42, 0x00,
    0x00, 0x42, 0x00, 0x00, 0x42, 0x00, 0x00, 0x81, 0x00, 0x00, 0x81, 0x00,
    0xC0, 0x00, 0x03, 0x38, 0x3C, 0x1C, 0x06, 0x42, 0x60, 0x01, 0x42, 0x80,
    0x01, 0x42, 0x80, 0x06, 0x42, 0x60, 0x38, 0x3C, 0x1C, 0xC0, 0x00, 0x03,
    0x00, 0x81, 0x00, 0x00, 0x81, 0x00, 0x00, 0x42, 0x00, 0x00, 0x42, 0x00,
    0x00, 0x42, 0x00, 0x00, 0x24, 0x00, 0x00, 0x24, 0x00, 0x00, 0x18, 0x00, };
    
    #define cross_fill_width 24
    #define cross_fill_height 24
    static const unsigned char cross_fill_bits[] U8X8_PROGMEM  = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x18, 0x64, 0x00, 0x26,
    0x84, 0x00, 0x21, 0x08, 0x81, 0x10, 0x08, 0x42, 0x10, 0x10, 0x3C, 0x08,
    0x20, 0x00, 0x04, 0x40, 0x00, 0x02, 0x80, 0x00, 0x01, 0x80, 0x18, 0x01,
    0x80, 0x18, 0x01, 0x80, 0x00, 0x01, 0x40, 0x00, 0x02, 0x20, 0x00, 0x04,
    0x10, 0x3C, 0x08, 0x08, 0x42, 0x10, 0x08, 0x81, 0x10, 0x84, 0x00, 0x21,
    0x64, 0x00, 0x26, 0x18, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
    
    #define cross_block_width 14
    #define cross_block_height 14
    static const unsigned char cross_block_bits[] U8X8_PROGMEM  = {
    0xFF, 0x3F, 0x01, 0x20, 0x01, 0x20, 0x01, 0x20, 0x01, 0x20, 0x01, 0x20,
    0xC1, 0x20, 0xC1, 0x20, 0x01, 0x20, 0x01, 0x20, 0x01, 0x20, 0x01, 0x20,
    0x01, 0x20, 0xFF, 0x3F, };
    
    void u8g2_bitmap_overlay(uint8_t a) {
    uint8_t frame_size = 28;
    
    u8g2.drawStr(0, 0, "Bitmap overlay");
    
    u8g2.drawStr(0, frame_size + 12, "Solid / transparent");
    u8g2.setBitmapMode(false /* solid */);
    u8g2.drawFrame(0, 10, frame_size, frame_size);
    u8g2.drawXBMP(2, 12, cross_width, cross_height, cross_bits);
    if(a & 4)
    u8g2.drawXBMP(7, 17, cross_block_width, cross_block_height, cross_block_bits);
    
    u8g2.setBitmapMode(true /* transparent*/);
    u8g2.drawFrame(frame_size + 5, 10, frame_size, frame_size);
    u8g2.drawXBMP(frame_size + 7, 12, cross_width, cross_height, cross_bits);
    if(a & 4)
    u8g2.drawXBMP(frame_size + 12, 17, cross_block_width, cross_block_height, cross_block_bits);
    }
    
    void u8g2_bitmap_modes(uint8_t transparent) {
    const uint8_t frame_size = 24;
    
    u8g2.drawBox(0, frame_size * 0.5, frame_size * 5, frame_size);
    u8g2.drawStr(frame_size * 0.5, 50, "Black");
    u8g2.drawStr(frame_size * 2, 50, "White");
    u8g2.drawStr(frame_size * 3.5, 50, "XOR");
    
    if(!transparent) {
    u8g2.setBitmapMode(false /* solid */);
    u8g2.drawStr(0, 0, "Solid bitmap");
    } else {
    u8g2.setBitmapMode(true /* transparent*/);
    u8g2.drawStr(0, 0, "Transparent bitmap");
    }
    u8g2.setDrawColor(0);// Black
    u8g2.drawXBMP(frame_size * 0.5, 24, cross_width, cross_height, cross_bits);
    u8g2.setDrawColor(1); // White
    u8g2.drawXBMP(frame_size * 2, 24, cross_width, cross_height, cross_bits);
    u8g2.setDrawColor(2); // XOR
    u8g2.drawXBMP(frame_size * 3.5, 24, cross_width, cross_height, cross_bits);
    }
    
    uint8_t draw_state = 0;
    
    void draw(void) {
    u8g2_prepare();
    switch(draw_state >> 3) {
    case 0: u8g2_box_frame(draw_state&7); break;
    case 1: u8g2_disc_circle(draw_state&7); break;
    case 2: u8g2_r_frame(draw_state&7); break;
    case 3: u8g2_string(draw_state&7); break;
    case 4: u8g2_line(draw_state&7); break;
    case 5: u8g2_triangle(draw_state&7); break;
    case 6: u8g2_ascii_1(); break;
    case 7: u8g2_ascii_2(); break;
    case 8: u8g2_extra_page(draw_state&7); break;
    case 9: u8g2_bitmap_modes(0); break;
    case 10: u8g2_bitmap_modes(1); break;
    case 11: u8g2_bitmap_overlay(draw_state&7); break;
    }
    }
    
    
    void setup(void) {
    ws2812fx.init();
    ws2812fx.setBrightness(255);
    ws2812fx.setSpeed(1000);
    ws2812fx.setColor(0x007BFF);
    ws2812fx.setMode(FX_MODE_STATIC);
    ws2812fx.start();
    Wire.begin(SDA_PIN, SCL_PIN);   
    u8g2.begin();
    }
    
    void loop(void) {
    
    
    ws2812fx.service();
    // picture loop
    u8g2.clearBuffer();
    draw();
    u8g2.sendBuffer();
    
    // increase the state
    draw_state++;
    if ( draw_state >= 12*8 )
    draw_state = 0;
    
    // deley between each page
    delay(100);
    
    }
``````