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
  Wire.begin();        // required before u8g2.begin() on ESP32
  u8g2.begin();
}
```

### 🚩 Must Know:

* `Wire.begin()` **must be called before `u8g2.begin()`** on ESP32 if using custom I2C pins.
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

Would you like this as a markdown file or inline `.cpp` code comments for documentation reuse?
