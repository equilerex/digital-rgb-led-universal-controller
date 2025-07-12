 # ESP32-C3 0.42 OLED

## 🔧 Hardware Overview

* **Chip & Flash**: ESP32‑C3FN4/FH4 with built‑in 4_MB SPI flash, dual-mode Wi‑Fi (2.4_GHz b/g/n) & Bluetooth 5.0, ceramic antenna ([Emalliab][1]).
* **Power**: USB‑C input (3.3–6_V). VIN pin reportedly supports 3.7–6_V ([Emalliab][2]).
* **Buttons**:

  * BO0: enter download mode (hold, plug in USB, release)
  * RST: reset ([Emalliab][1]).
* **On‑board components**:

  * Built‑in blue LED on GPIO_8
  * 0.42″ OLED display (SSD1306 controller; “weird” layout: screen is 72_×_40_px center‑mapped in a 128_×_64 buffer) ([Emalliab][3], [Emalliab][2]).
* **Pinout** (approximate):

  * I²C: SDA = GPIO_5, SCL = GPIO_6, I²C addr = 0x3C ([Emalliab][1]).
  * Alternate I²C on GPIO_8,_9 (likely secondary bus) ([Emalliab][1]).
  * UART: GPIO_21/20; SPI: broken out; ADC on GPIO_0–5 ([Emalliab][1]).
* **Note**: This OLED’s buffer offset: xOffset = 30, yOffset = 12. Draw within a 72×40 region to avoid clipping ([Emalliab][2]).

---

## 📚 Software Setup

### Arduino IDE (U8g2)

1. Install **ESP32 Arduino core** (via Board Manager URL) ([GitHub][4]).
2. Select **“ESP32C3 Dev Module”** as board type ([Emalliab][1]).
3. Install **U8g2** library via Library Manager ([Emalliab][1]).
4. Use this initialization code:

```cpp
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /*no reset pin*/, /*SCL=*/6, /*SDA=*/5);
int w = 72, h = 40, xOff = 30, yOff = 12;

void setup() {
  delay(1000);
  u8g2.begin();
  u8g2.setContrast(255);
  u8g2.setBusClock(400000);
  u8g2.setFont(u8g2_font_ncenB10_tr);
}

void loop() {
  u8g2.clearBuffer();
  u8g2.drawFrame(xOff, yOff, w, h);
  u8g2.setCursor(xOff + 15, yOff + 25);
  u8g2.printf("%dx%d", w, h);
  u8g2.sendBuffer();
}
```

([Electrical Engineering Stack Exchange][5])

### MicroPython (optional)

Use either SSD1306 or SH1106 drivers:

```python
from machine import Pin, I2C
from ssd1306 import SSD1306_I2C  # or SH1106

i2c = I2C(0, scl=Pin(6), sda=Pin(5), freq=400000)
oled = SSD1306_I2C(72, 40, i2c)
oled.text("hello", 0, 0)
oled.show()
```

([Electrical Engineering Stack Exchange][5])

---

## 🛠️ PlatformIO Configuration

Use **esp32‑c3‑devkitm‑1** or **esp32‑c3‑devkitc‑02** as board in `platformio.ini`:

```ini
[env:esp32-c3-devkitm-1]
platform = espressif32
board = esp32-c3-devkitm-1
framework = arduino
build_flags = -DARDUINO_USB_CDC_ON_BOOT=1
```

* Enables USB CDC for `Serial.print()` ([PlatformIO Community][6], [Reddit][7]).
* Upload protocol defaults to `esptool`; external debuggers supported ([PlatformIO Documentation][8]).
* For uploading: hold BOOT while pressing RST once, then release BOOT ([Emalliab][2]).

---

## ⚠️ Tips & Gotchas

* **Unique SSD1306 buffer**: Only 72×40 renders; perform offsetting to display correctly.
* **CDC Initialization**: Add `-DARDUINO_USB_CDC_ON_BOOT=1` to build flags for reliable `Serial.print()` ([Emalliab][1], [Reddit][7]).
* **Strapping Pins**: Some GPIOs (0,2,5,12,15) have pull-ups/downs on boot—check datasheet if using them ([Emalliab][9]).

---

## ✅ Quick Setup Checklist

| Step | Description                                                               |
| ---- | ------------------------------------------------------------------------- |
| ✅    | Wire SDA → GPIO_5, SCL → GPIO_6                                           |
| ✅    | Install ESP32 core & U8g2 lib                                             |
| ✅    | Set board = ESP32C3 Dev Module (Arduino IDE) or appropriate in PlatformIO |
| ✅    | Add `build_flags` for PlatformIO if using serial                          |
| ✅    | Use provided Arduino or MicroPython code                                  |
| ✅    | Hold BOOT + RST once, then upload                                         |

---

Source:
[1]: https://emalliab.wordpress.com/2025/02/12/esp32-c3-0-42-oled/?utm_source=chatgpt.com "ESP32-C3 0.42 OLED - Kevin's Blog - WordPress.com" 