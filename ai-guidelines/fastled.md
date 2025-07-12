Here's a **concise but thorough reference of the FastLED library API** and **must-know behaviors, quirks, and caveats** — ideal for using as an LLM instruction set (e.g., GitHub Copilot prompt seed or system message).

---

## 🚀 FastLED Library – Essential API Reference & Must-Knows

### ✅ **Core Concepts**

* **LED Array Initialization**: `CRGB leds[NUM_LEDS];`

    * Must be statically allocated (not dynamically resized at runtime).
    * Can be global or static in a class.
    * Cannot be re-created after `FastLED.addLeds()` is called.

* **Setup**:

  ```cpp
  FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  ```

* **Show the LEDs**:

  ```cpp
  FastLED.show();  // Pushes buffer to the LED strip
  ```

---

### 🎨 **Modifying LEDs**

* **Set individual LEDs**:

  ```cpp
  leds[i] = CRGB::Red;
  leds[i].setRGB(r, g, b);
  leds[i].fadeToBlackBy(amount);  // Fades LED by % (0-255)
  ```

* **Clear the strip**:

  ```cpp
  FastLED.clear();  // Clears internal buffer
  FastLED.show();   // Required to apply the clearing
  ```

  ⚠️ **Caveat**: If used early in boot without a valid strip or power, it can appear to "kill" animation until next update.

---

### 🔁 **Animations & Effects**

* **Simple looping animation**:

  ```cpp
  EVERY_N_MILLISECONDS(50) {
    // Animate here
    FastLED.show();
  }
  ```

* **Useful macros**:

    * `EVERY_N_MILLISECONDS(ms) { }`
    * `EVERY_N_SECONDS(s) { }`

* **Noise, sine, palette helpers**:

  ```cpp
  uint8_t noise = inoise8(x, y);  // Perlin noise
  uint8_t sine = beatsin8(BPM, low, high);
  ```

* **HSV support**:

  ```cpp
  leds[i] = CHSV(hue, sat, val);
  ```

---

### 🔧 **Brightness & Power Management**

* `FastLED.setBrightness(x);` // 0–255
* `FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);` // auto-dim if needed
* **Note**: Brightness affects `.show()`, not the raw buffer

---

### ⚠️ Gotchas / Must-Knows

| Issue                                     | Explanation                                                                 |
| ----------------------------------------- | --------------------------------------------------------------------------- |
| `leds[]` must be static                   | You **cannot realloc** or recreate `leds[]` after `FastLED.addLeds()`       |
| `.clear()` + `.show()` removes all pixels | Make sure you **update buffer again** afterward or animation appears frozen |
| No dynamic length change                  | You must re-flash to change strip length                                    |
| Global buffer required                    | If inside a class, `CRGB* leds` must point to a static/global buffer        |
| `.show()` must be called explicitly       | FastLED doesn’t auto-flush changes                                          |
| Watch for blocking delays                 | Use `EVERY_N_` macros or `millis()` timing, **not** `delay()` in animations |
| Power draw limits                         | Especially important for large strips — auto-dim with `setMaxPower...`      |
| No multi-strip built-in                   | You must manage each strip's `CRGB[]` and `.addLeds()` call manually        |

---

### 🔄 **Palette Support**

* Define:

  ```cpp
  CRGBPalette16 palette = PartyColors_p;
  ```

* Use:

  ```cpp
  leds[i] = ColorFromPalette(palette, index, brightness);
  ```

---

### 🧪 **Debug Helpers**

```cpp
Serial.print(leds[0].r);  // Inspect color components
```

---

### 📋 Copilot Prompt Seed (Summary Form)

> You're coding an ESP32 LED animation using FastLED. `leds[]` is a statically declared CRGB array. You can't change its size at runtime. Use `FastLED.addLeds<>()` in `setup()`, then animate in `loop()` using `EVERY_N_MILLISECONDS()`. Call `FastLED.show()` to push changes. Avoid `delay()`. Don't call `FastLED.clear()` unless followed by new pixel values and `.show()`. Use `CHSV()` for hue-based color, or `ColorFromPalette()` for smoother effects. Never reassign the `leds` array. Prefer global buffers and low-CPU drawing.

---

Would you like this turned into a markdown file or code comment block for easy reuse?
