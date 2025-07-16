/* button_demoReel100
 *
 * Originally By: Mark Kriegsman
 * Modified for EEPROM and button usage by: Andrew Tuline
 * Previous iteration: Joosep koivistik https://github.com/equilerex
 * Updated for modern FastLED: July 2025
 * ESP32-C3 SuperMini adaptation: July 2025
 * Button-controlled brightness: July 2025
 * Date: July, 2025
 *
 * Instructions:
 *
 * Program reads display mode and brightness from memory and displays it.
 * Single click: Change to the next pattern
 * Hold 3-4 seconds: Enter brightness cycling mode (20%-100%)
 * Hold 5-9 seconds: Reduce LED count by 50 (minimum 50 LEDs)
 * Hold 10+ seconds: Increase LED count by 50 (maximum 1000 LEDs)
 *
 * There's also debug output to the serial monitor . . just to be sure.
 *
 * Requirements:
 *
 * OneButton library (install via Library Manager)
 * FastLED 3.7+ library
 * ESP32-C3 SuperMini development board
 *
 * ESP32-C3 SuperMini Pin Configuration:
 * - GPIO9: Built-in BOOT button (used for all controls)
 * - GPIO8: LED data pin
 * - GPIO2: Built-in LED for status indication
 *
 */


//--------------------[ Dependencies ]-------------------------------------------
// ESP32-C3 compatibility check
#if !defined(ESP32) && !defined(CONFIG_IDF_TARGET_ESP32C3)
#warning "This code is optimized for ESP32-C3. Other ESP32 variants may work but pin assignments may need adjustment."
#endif


#include <Preferences.h>  // ESP32 replacement for EEPROM
#include <FastLED.h>
#include <OneButton.h>    // Professional button library
#define qsubd(x, b)  ((x>b)?wavebright:0)                     // A digital unsigned subtraction macro. if result <0, then => 0. Otherwise, take on fixed value.
#define qsuba(x, b)  ((x>b)?x-b:0)                            // Unsigned subtraction macro. if result <0, then => 0.
 
#include <U8g2lib.h>
#define OLED_RESET U8X8_PIN_NONE  // Reset pin
#define OLED_SDA 5
#define OLED_SCL 6


U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, OLED_RESET, OLED_SCL, OLED_SDA);
int width = 72;  // Actual OLED display width
int height = 40; // Actual OLED display height
int xOffset = 30;  // = (128-72)/2 - Center horizontally
int yOffset = 12;  // = (64-40)/2 - Center vertically
 


//--------------------[ Fixed definitions - cannot change on the fly. ]-------------------------------------------

// ESP32-C3 pin configuration
#define buttonPin 9                                           // GPIO9 - Built-in BOOT button (active LOW)
OneButton button(buttonPin, true);                           // Create OneButton instance (true = active LOW)
#define LED_DT 8                                              // GPIO8 - Data pin to connect to the LED strip
#define BUILTIN_LED 2                                         // GPIO2 - Built-in LED for status indication
bool enableBrightnessPin = 0;                                // No brightness potentiometer - using button control
bool enableColorPin = 0;                                     // No color potentiometer - using button control
uint16_t numLeds = 200;                                      // Default value

byte* heat = nullptr;   // Pointer for dynamic heat array

// Define LED_TYPE and COLOR_ORDER before they are used
#define COLOR_ORDER GRB                                       // It's GRB for WS2812 and BGR for APA102.
#define LED_TYPE WS2812                                       // Using APA102, WS2812, WS2801. Don't forget to modify LEDS.addLeds to suit.


                                      // Clock pin for WS2801 or APA102.
#define NUM_LEDS 1000                                          // Number of LED's.

//--------------------[ Setup #1 (ignore) ]-------------------------------------------
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))            // Definition for the array of routines to display.
Preferences preferences;                                      // ESP32 preferences object for persistent storage
CRGB  leds[NUM_LEDS];                                          // Initialize our LED array.
uint8_t gCurrentPatternNumber = 0;                            // Index number of which pattern is current

// Button-controlled brightness variables
bool brightnessMode = false;                                  // Whether we're in brightness adjustment mode
bool ledCountUpMode = false;                                  // Whether we're in LED count up adjustment mode
bool ledCountDownMode = false;                                // Whether we're in LED count down adjustment mode
bool longPressMode = false;                                   // Whether we're in any long press mode
unsigned long longPressStartTime = 0;                         // When long press mode started

// Add variables for numLeds adjustment
bool oledAvailable = false;                                  // Track if OLED is available
bool showNumLedsDisplay = false;                             // Whether to show LED length display
unsigned long numLedsDisplayTimeout = 0;                     // When to stop showing LED length display

// Variables for brightness display timing
bool showBrightnessDisplay = false;
unsigned long brightnessDisplayTimeout = 0;
#define BRIGHTNESS_DISPLAY_DURATION 3000                     // Show brightness for 3 seconds after adjustment
#define NUMLEDS_DISPLAY_DURATION 3000                        // Show LED length for 3 seconds after adjustment


//--------------------[ Global variables - can be changed on the fly. ]-------------------------------------------

 // Overall brightness definition. It can be changed on the fly.
uint8_t max_bright = 128;
uint8_t min_bright = 25;                                      // 20% of 255 (minimum brightness)
uint8_t max_bright_limit = 255;                               // 100% brightness (maximum)

// rotating "base color" used by many of the patterns
uint8_t gHue = 0;
// Standard hue
uint8_t thisdelay = 20;
// A delay value for the sequence(s)
uint8_t thishue = 140;

// rainbow mode creates scope issues if put below with all the rest
// FastLED's built-in rainbow generator.
void rainbow() {
  fill_rainbow(leds, numLeds, gHue, 7);
} // rainbow()



//--------------------[ define patterns to loop through (add new or remove existing) ]-------------------------------------------
// Define function type for patterns with no parameters
typedef void (*SimplePatternList)();

// Pattern names for logging and display
const char* patternNames[] = {
  "Rainbow with Glitter",
  "Rainbow March", 
  "Sinelon",
  "Two Sin",
  "Pop Fade",
  "Confetti",
  "Juggle",
  "BPM",
  "Three Sin Two",
  "Rainbow",
  "Red Purple Blue",
  "Green Yellow Red",
  "Green Blue",
  "Orange",
  "Purple",
  "Noise16",
  "Meteor Rain",
  "Fire Effect",
  "Twinkle Stars",
  "Color Waves",
  "Comet Effect",
  "Plasma Effect",
  "Breathing Light",
  "Aurora",
  "Lava Lamp",
  "Color Meditation",
  "Ocean Waves",
  "Moonlight",
  "Heartbeat",
  "Forest Canopy",
  "Strobe Pulse",
  "Beat Scanner",
  "Energy Ripple",
  "Color Slam",
  "Hyper Spin",
  "Beat Trails",
  "Strobe Wave",
  "Beat Drop"
};

SimplePatternList gPatterns[] = { rainbowWithGlitter, rainbow_march, sinelon, two_sin, pop_fade, confetti, juggle, bpm, threeSinTwo, rainbow, redPurpleBlue, greenYellowRed, greenBlue, orange, purple, noise16,
  meteorRain,  fireEffect,  twinkleStars,  colorWaves,  cometEffect,  plasmaEffect,  // Add semicolon here
  breathing,
  aurora,
  lavaLamp,
  colorMeditation,
  oceanWaves,
  moonlight,
  heartbeat,
  forestCanopy,
  strobePulse,
     beatScanner,
     energyRipple,
     colorSlam,
     hyperSpin,
     beatTrails,
     strobeWave,
     beatDrop
};

void setup() {
  // Initialize serial port for debugging.
  Serial.begin(115200);

  // Soft startup to ease the flow of electrons.
  delay(1000);
  
  // Now we can print to serial
  Serial.println(F("\n=== LED Controller Startup ==="));

  // Set mode switch pin with internal pullup (ESP32-C3 BOOT button is active LOW)
  pinMode(buttonPin, INPUT_PULLUP);

  // Initialize built-in LED for status indication
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);

  numLeds = preferences.getUShort("numLeds", 200); // Load saved numLeds, default to 200
  if (numLeds < 50) numLeds = 50;                  // Safety check for minimum value
  if (numLeds > 1000) numLeds = 1000;              // Safety check for maximum value


  // setup fastled library
  FastLED.addLeds<LED_TYPE, LED_DT, COLOR_ORDER>(leds, numLeds);  // Use this for WS2812
  FastLED.setBrightness(max_bright);


  // Initialize preferences and pull saved settings from memory
  preferences.begin("ledcontroller", false);

  // Restore numLeds setting
  numLeds = preferences.getUShort("numLeds", 200);           // Load saved numLeds, default to 200
  if (numLeds < 50) numLeds = 50;                            // Safety check for minimum value
  if (numLeds > 1000) numLeds = 1000;                        // Safety check for maximum value

  // Restore other settings
  gCurrentPatternNumber = preferences.getUChar("pattern", 0);  // Default to first pattern
  max_bright = preferences.getUChar("brightness", 128);        // Load saved brightness, default to 128

  // A safety in case the stored values have illegal values.
  if (gCurrentPatternNumber >= ARRAY_SIZE(gPatterns)) gCurrentPatternNumber = 0;
  if (max_bright < min_bright) max_bright = min_bright;
  if (max_bright > max_bright_limit) max_bright = max_bright_limit;

  FastLED.setBrightness(max_bright);                           // Apply loaded brightness

  // Enhanced startup logging
  Serial.println(F("ESP32-C3 Digital RGB LED Universal Controller"));
  Serial.println(F("Version: July 2025"));
  Serial.println(F("Author: Joosep Koivistik"));
  Serial.println(F("==============================="));
  
  // Hardware configuration
  Serial.println(F("Hardware Configuration:"));
  Serial.print(F("  LED Data Pin: GPIO"));
  Serial.println(LED_DT);
  Serial.print(F("  Button Pin: GPIO"));
  Serial.println(buttonPin);
  Serial.print(F("  Built-in LED: GPIO"));
  Serial.println(BUILTIN_LED);
  Serial.print(F("  LED Type: "));
  Serial.println(F("WS2812"));
  Serial.print(F("  Color Order: "));
  Serial.println(F("GRB"));
  Serial.println();
  
  // Current settings
  Serial.println(F("Current Settings:"));
  Serial.print(F("  Number of LEDs: "));
  Serial.println(numLeds);
  Serial.print(F("  Current Pattern: "));
  Serial.print(gCurrentPatternNumber);
  Serial.print(F(" ("));
  if (gCurrentPatternNumber < ARRAY_SIZE(patternNames)) {
    Serial.print(patternNames[gCurrentPatternNumber]);
  } else {
    Serial.print(F("Unknown"));
  }
  Serial.println(F(")"));
  Serial.print(F("  Total Patterns Available: "));
  Serial.println(ARRAY_SIZE(gPatterns));
  Serial.print(F("  Brightness: "));
  Serial.print(max_bright);
  Serial.print(F("/"));
  Serial.print(max_bright_limit);
  Serial.print(F(" ("));
  Serial.print((max_bright * 100) / max_bright_limit);
  Serial.println(F("%)"));
  Serial.print(F("  Min Brightness: "));
  Serial.print(min_bright);
  Serial.print(F(" ("));
  Serial.print((min_bright * 100) / max_bright_limit);
  Serial.println(F("%)"));
  Serial.println();
  
  // Control instructions
  Serial.println(F("Control Instructions:"));
  Serial.println(F("  Single click = Next pattern"));
  Serial.println(F("  Hold 3+ seconds = Brightness control mode"));
  Serial.println(F("  Built-in LED = Status indicator"));
  Serial.println(F("===============================\n"));

  // Brief LED flash to indicate successful startup
  digitalWrite(BUILTIN_LED, HIGH);
  delay(100);
  digitalWrite(BUILTIN_LED, LOW);

  // Setup OneButton callbacks
  button.attachClick(nextPattern);                           // Single click = next pattern
  button.attachLongPressStart(enterLongPressMode);          // Long press = brightness mode
  button.attachLongPressStop(exitLongPressMode);             // Release long press = exit brightness or LED count mode
  button.setPressTicks(2000);

  // Initialize OLED display
  u8g2.begin();
  u8g2.setContrast(255);     // set contrast to maximum
  u8g2.setBusClock(400000);  // 400kHz I2C (do not change!)
  
  // Display startup message
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_8x13_tr ); 
  u8g2.drawStr(xOffset, yOffset + 30, "Jo's blinky");
  u8g2.drawStr(xOffset, yOffset + 50, "Boot...");
  u8g2.sendBuffer(); 
  delay(2000);
  oledAvailable = true;
}



void loop() {
  // Tick the button state machine
  button.tick();

  // Handle brightness cycling when in brightness mode
  if (longPressMode) {
    cycleLongPress();
  } 
  

  // Call the current pattern function to update the 'leds' array
  // Different patterns have different optimal update rates
  EVERY_N_MILLISECONDS(50) {
    gPatterns[gCurrentPatternNumber]();
  }

  // Slowly cycle the "base color" through the rainbow
  EVERY_N_MILLISECONDS(20) {
    gHue++;
  }

  // Update display - show the LED colors
  FastLED.show();

  // Update OLED display occasionally
  EVERY_N_MILLISECONDS(500) { // Update more frequently (twice per second)
    if (oledAvailable) {
      updateOledDisplay();
    }
  }
} 

// Function to update OLED display
 

void updateOledDisplay() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_4x6_tr); // Use a smaller font that fits the tiny display
  
  // Pattern number and name
  char patternBuf[40];
  char statusBuf[30];
  char indBuf[30];
  char brightBuf[30];
  char stripBuf[30];
  
  if (brightnessMode) {
    // Draw a small lightbulb icon
    u8g2.drawDisc(xOffset + 5, yOffset + 20, 3);  // Bulb circle
    u8g2.drawLine(xOffset + 5, yOffset + 23, xOffset + 5, yOffset + 26); // Bulb base
    u8g2.drawLine(xOffset + 3, yOffset + 26, xOffset + 7, yOffset + 26); // Base bottom
    int brightPec = map(max_bright, min_bright, max_bright_limit, 10, 100);
    
    snprintf(brightBuf, sizeof(brightBuf), "%d%% BRIGHT", brightPec);
    u8g2.setFont(u8g2_font_8x13_tr);    
    u8g2.drawStr(xOffset + 15, yOffset + 35, brightBuf); // Moved text right to make room for icon
  } 
  else if (ledCountDownMode || ledCountUpMode) { 
    snprintf(stripBuf, sizeof(stripBuf), "%d LED's", numLeds);   
    u8g2.setFont(u8g2_font_8x13_tr);    
    u8g2.drawStr(xOffset + 0, yOffset + 35, stripBuf); // Changed from indBuf to stripBuf
  } 
  else {
    // Format the strings
    snprintf(patternBuf, sizeof(patternBuf), "#%d: %s", 
             gCurrentPatternNumber, 
             (gCurrentPatternNumber < ARRAY_SIZE(patternNames)) ? 
                patternNames[gCurrentPatternNumber] : "Unknown");
    
    // Format the strings
    snprintf(indBuf, sizeof(indBuf), "# %d", 
             gCurrentPatternNumber);
    
    snprintf(statusBuf, sizeof(statusBuf), "B:%d%% L:%d", 
             map(max_bright, min_bright, max_bright_limit, 20, 100),
             numLeds); 
    
    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.drawStr(xOffset + 0, yOffset + 20, patternBuf);

    u8g2.setFont(u8g2_font_8x13_tr);   
    u8g2.drawStr(xOffset + 0, yOffset + 35, indBuf);
    
    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.drawStr(xOffset + 0, yOffset + 50, statusBuf);
  }
  
  // Send the buffer to the display
  u8g2.sendBuffer();
}

// Button callback functions
void nextPattern() {
  if (!longPressMode) {
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
    preferences.putUChar("pattern", gCurrentPatternNumber);

    // Enhanced pattern change logging
    Serial.print(F("Pattern changed to: "));
    Serial.print(gCurrentPatternNumber);
    Serial.print(F(" ("));
    if (gCurrentPatternNumber < ARRAY_SIZE(patternNames)) {
      Serial.print(patternNames[gCurrentPatternNumber]);
    } else {
      Serial.print(F("Unknown"));
    }
    Serial.println(F(")"));
  }
}

void enterLongPressMode() {
  brightnessMode = true;
  longPressMode = true;
  longPressStartTime = millis();
  Serial.println(F("Button long press started: Entering brightness mode"));
}

void exitLongPressMode() {
  // Get total press duration
  unsigned long pressDuration = millis() - longPressStartTime;
  longPressStartTime = 0;
  
  // Temporarily dim LEDs during transitions
  uint8_t originalBrightness = FastLED.getBrightness();
  FastLED.setBrightness(0);

  if (brightnessMode) {
    // Going up from min to max
    if (max_bright == 255) {
      max_bright = 5;  // Reset to minimum
    } else {
      max_bright += 51;  // 20% of 255
      if (max_bright > 255) max_bright = 255;  // Cap at max
    }
    FastLED.setBrightness(max_bright);
    
    // Save the brightness setting
    preferences.putUChar("brightness", max_bright);
    Serial.print(F("Brightness set to: "));
    Serial.println(max_bright);
  } else if (ledCountUpMode || ledCountDownMode) {
    // Ensure numLeds is within bounds
    if (numLeds < 50) numLeds = 50;         // Minimum LED count
    if (numLeds > 1000) numLeds = 1000;     // Maximum LED count
    
    // Update FastLED with new count
    FastLED.addLeds<LED_TYPE, LED_DT, COLOR_ORDER>(leds, numLeds);
    FastLED.setBrightness(max_bright);
    
    // Save the LED count setting
    preferences.putUShort("numLeds", numLeds);
    Serial.print(F("LED count set to: "));
    Serial.println(numLeds);
  } else {
    // Just restore brightness if no special mode was active
    FastLED.setBrightness(max_bright);
  }

  // Reset all mode flags
  brightnessMode = false;
  ledCountUpMode = false;
  ledCountDownMode = false;
  longPressMode = false;
}

// Cycle brightness while in brightness mode
void cycleLongPress() {
  unsigned long elapsed = millis() - longPressStartTime; 
  static uint8_t tempBrightness = 0;
  
 
  // If we've been holding for more than 10 seconds, switch to LED count up mode
  if (elapsed >= 6000) {
    // Turn off other modes
    ledCountDownMode = false;
    
    // Enter LED count up mode
    ledCountUpMode = true;
    
    // Temporarily dim LEDs during transition
    tempBrightness = FastLED.getBrightness();
    FastLED.setBrightness(tempBrightness / 4);  // Dim to 25%

    // Increase LED count by 100
    numLeds += 100;
    if (numLeds > 1000) numLeds = 3000;  // Cap at maximum
    
    // Update LEDs with new count
    FastLED.addLeds<LED_TYPE, LED_DT, COLOR_ORDER>(leds, numLeds);
    
    // Restore brightness gradually
    FastLED.setBrightness(tempBrightness);
    
    Serial.print(F("LED count increased to: "));
    Serial.println(numLeds); 
  }  else if (elapsed >= 4000 ) {
    // Turn off other modes
    brightnessMode = false;  
    // Enter LED count down mode
    ledCountDownMode = true;
    
    // Temporarily dim LEDs during transition
    tempBrightness = FastLED.getBrightness();
    FastLED.setBrightness(tempBrightness / 4);  // Dim to 25%
    
    // Decrease LED count by 50
    numLeds -= 50;
    if (numLeds < 50) numLeds = 50;  // Minimum of 50 LEDs
    
    // Update LEDs with new count
    FastLED.addLeds<LED_TYPE, LED_DT, COLOR_ORDER>(leds, numLeds);
    
    // Restore brightness gradually
    FastLED.setBrightness(tempBrightness);
    
    Serial.print(F("LED count decreased to: "));
    Serial.println(numLeds);
      
  } 
} 

 

//--------------------[ helper functions ]------------------------------------------------------------------------------

// adds little random white flashes
void addGlitter(fract8 chanceOfGlitter) {

  if(random8() < chanceOfGlitter) {
    leds[ random16(numLeds) ] += CRGB::White;
  }

}

//--------------------[ modes are below here ]------------------------------------------------------------------------------

// Meteor Rain Animation - Non-blocking version
static int meteorPosition = 0;
static unsigned long lastMeteorUpdate = 0;

// Forward declaration
void meteorRainImpl(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean randomDecay);

// Wrapper function that conforms to SimplePatternList signature (no parameters)
void meteorRain() {
  byte red = 0xFF, green = 0x00, blue = 0x00;
  // Call the actual implementation with default parameters
  meteorRainImpl(255, 255, 255, 10, 64, true);
}

// Implementation function with all parameters
void meteorRainImpl(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean randomDecay) {
  unsigned long currentMillis = millis();
  
  // Update meteor position only every 30ms for smooth animation without blocking
  if (currentMillis - lastMeteorUpdate >= 30) {
    lastMeteorUpdate = currentMillis;

    // Fade all pixels
    for (int j = 0; j < numLeds; j++) {
      if ((!randomDecay) || (random8() > 64)) {
        leds[j].fadeToBlackBy(meteorTrailDecay);
      }
    }
    
    // Draw meteor at current position
    for (int j = 0; j < meteorSize; j++) {
      if ((meteorPosition - j < numLeds) && (meteorPosition - j >= 0)) {
        leds[meteorPosition - j] = CRGB(scale8(red, max_bright), scale8(green, max_bright), scale8(blue, max_bright));
      }
    }
    
    // Advance the meteor position
    meteorPosition++;

    // Reset position when meteor has moved completely off-screen
    if (meteorPosition >= numLeds + meteorSize) {
      meteorPosition = 0;
    }
    
    // Note: FastLED.show() is called in the main loop, not here
  }
}

// Fire Effect Animation - Non-blocking version
void fireEffect() {
  static byte* heat = new byte[numLeds]();

  // No need for additional timing here since this is called on a timer from the main loop

  // Simulate cooling
  for (int i = 0; i < numLeds; i++) {
    heat[i] = qsub8(heat[i], random8(0, ((55 * 10) / numLeds) + 2));
  }

  // Heat diffusion
  for (int k = numLeds - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }

  // Randomly ignite new sparks
  if (random8() < 120) {
    int y = random8(7);
    heat[y] = qadd8(heat[y], random8(160, 255));
  }

  // Map heat to colors
  for (int j = 0; j < numLeds; j++) {
    byte colorIndex = scale8(heat[j], max_bright);
    leds[j] = ColorFromPalette(HeatColors_p, colorIndex);
  }
  // FastLED.show() is called in the main loop
}

// Twinkle Stars Animation - Non-blocking version
void twinkleStars() {
  // No need for additional timing here since this is called on a timer from the main loop

  // Fade existing stars
  fadeToBlackBy(leds, numLeds, 10);

  // Add new stars randomly
  int pos = random16(numLeds);
  leds[pos] += CHSV(random8(64, 192), 200, scale8(255, max_bright));

  // FastLED.show() is called in the main loop
}

// Color Waves Animation
void colorWaves() {
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  uint8_t brightdepth = scale8(beatsin88(341, 96, 224), max_bright);
  uint16_t brightnessthetainc16 = beatsin88(203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis;
  sLastMillis = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88(400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  for (uint16_t i = 0; i < numLeds; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;
    uint16_t h16_128 = hue16 >> 7;
    if (h16_128 & 0x100) {
      hue8 = 255 - hue8;
    }

    uint8_t bright = scale8(qadd8(brightdepth, sin8(brightnesstheta16)), max_bright);
    brightnesstheta16 += brightnessthetainc16;

    leds[i] = CHSV(hue8, 255, bright);
  }
  FastLED.show();
}

// Comet Effect Animation
void cometEffect() {
  static int pos = 0;
  static int direction = 1;

  fadeToBlackBy(leds, numLeds, 20);
  leds[pos] = CHSV(gHue, 255, scale8(255, max_bright));

  pos += direction;
  if (pos >= numLeds - 1 || pos <= 0) {
    direction *= -1;
    pos = constrain(pos, 0, numLeds - 1); // Ensure pos stays within bounds
  }

  FastLED.show();
  delay(30);
}

// Plasma Effect Animation
void plasmaEffect() {
  for (int i = 0; i < numLeds; i++) {
    int x = i * 10;
    int y = millis() / 10;
    int colorIndex = sin8(x + y) + cos8(x - y);
    leds[i] = ColorFromPalette(RainbowColors_p, colorIndex, scale8(255, max_bright));
  }
  FastLED.show();
  delay(20);
}



// Built-in FastLED rainbow, plus some random sparkly glitter.
void rainbowWithGlitter() {
  rainbow();
  addGlitter(80);

}



// Random colored speckles that blink in and fade smoothly.
void confetti() {
  fadeToBlackBy(leds, numLeds, 10);
  int pos = random16(numLeds);
  leds[pos] += CHSV(gHue + random8(64), 200, 255);

}


// A colored dot sweeping back and forth, with fading trails.
void sinelon() {

  fadeToBlackBy(leds, numLeds, 20);
  int pos = beatsin16(13,0,numLeds-1);
  leds[pos] += CHSV(gHue, 255, 192);

}


// Colored stripes pulsing at a defined Beats-Per-Minute.
void bpm() {

  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);

  for(int i = 0; i < numLeds; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }

}



// Eight colored dots, weaving in and out of sync with each other.
void juggle() {
  fadeToBlackBy(leds, numLeds, 20);
  byte dothue = 0;

  for(int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,numLeds-1)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}


// noise16  -----------------------------------------------------------------------
uint32_t  x,hue_time;                                         // x & hue_time values
uint8_t octaves=2;       //2                                  // how many octaves to use for the brightness
uint8_t hue_octaves=3;   //3                                  // how many octaves to use for the hue
int xscale=57771;        //57771                              // the 'distance' between points on the x axis
uint32_t hxy = 43213;    //43213                              // not sure about this
int hue_scale=20;        //1                                  // the 'distance' between points for the hue noise
int hue_speed = 1;       //31                                 // how fast we move through hue noise
uint8_t x_speed = 0;     //331                                // adjust this value to move along the x axis between frames
int8_t hxyinc = 3;       //3

void noise16() {
  thisdelay=20; hxyinc = random16(1,15); octaves=random16(1,3); hue_octaves=random16(1,5); hue_scale=random16(10, 50);  x=random16(); xscale=random16(); hxy= random16(); hue_time=random16(); hue_speed=random16(1,3); x_speed=random16(1,30);

  fill_noise16(leds, numLeds, octaves, x, xscale, hue_octaves, hxy, hue_scale, hue_time);

    for (int i=0; i<numLeds; i++) {                            // We can filter, we can invert, we can do all sorts of things.
  //    leds[i].r = 0;                                            // Filter out red
      leds[i].g = 0;                                            // Filter out green
  //    leds[i].b = 0;                                            // Filter out blue
  //    leds[i].g =qsub(leds[i].g, 40);                           // Set limits
    }
}





// pop_fade variables ----------------------------------------------------------------------
int ranamount = 50;                                           // The higher the number, lowers the chance for a pixel to light up.
// uint8_t thisdelay = 50;                                       // Standard delay value.
bool boolcolours = 1;                                         // True = random, False = array of colours (up to 10)
uint8_t numcolours = 2;                                       // Number of colours in the array to display.
unsigned long colours[10] = {0xff0000, 0x00ff00, 0x0000ff, 0xffffff};  // Just assign the first 3 of these selectable colours.
uint8_t maxbar = 1;                                           // Maximum # of pixels to turn on in a row
uint8_t fadeval = 224;                                         // Maximum # of pixels to turn on in a row
uint8_t buttonDown = 0;                                        // Fade rate

void pop_fade() {
  // reset shared variables specific to this guy
  thisdelay=40; colours[0]=0x13b0f2; numcolours=1; boolcolours=0; maxbar=1;

  unsigned long thiscolour;
  if (ranamount >numLeds) ranamount = numLeds;              // Make sure we're at least utilizing ALL the LED's.
  int idex = random16(0, ranamount);

  if (idex < numLeds) {                                      // Only the lowest probability twinkles will do.
    boolcolours ? thiscolour = random(0, 0xffffff) : thiscolour =  colours[random16(0, numcolours)];
    int barlen = random16(1,maxbar);
    for (int i = 0; i <barlen; i++)
      if (idex+i < numLeds) leds[idex+i] = thiscolour;       // Make sure we don't overshoot the array.
  }
     nscale8(leds,numLeds,fadeval);                          // Fade the entire array. Or for just a few LED's, use  nscale8(&leds[2], 5, fadeval);
}

// sin mode specific variables ---------------
//two
uint8_t wavebright = 128;                                     // You can change the brightness of the waves/bars rolling across the screen. Best to make them not as bright as the sparkles.
uint8_t thathue = 80;                                        // You can change the starting hue for other wave.
uint8_t thisrot = 0;                                          // You can change how quickly the hue rotates for this wave. Currently 0.
uint8_t thatrot = 0;                                          // You can change how quickly the hue rotates for the other wave. Currently 0.
uint8_t allsat = 255;                                         // I like 'em fully saturated with colour.
int8_t thisspeed = 1;                                         // You can change the speed, and can use negative values.
int8_t thatspeed = 1;                                         // You can change the speed, and use negative values.
uint8_t allfreq = 32;                                         // You can change the frequency, thus overall width of bars.
int thisphase = 0;                                            // Phase change value gets calculated.
int thatphase = 0;                                            // Phase change value gets calculated.
uint8_t thiscutoff = 192;                                     // You can change the cutoff value to display this wave. Lower value = longer wave.
uint8_t thatcutoff = 192;                                     // You can change the cutoff value to display that wave. Lower value = longer wave.
// three
int wave1=0;
int wave2=0;
int wave3=0;
uint8_t inc1 = 2;
uint8_t inc2 = 1;
uint8_t inc3 = -3;
uint8_t lvl1 = 80;
uint8_t lvl2 = 80;
uint8_t lvl3 = 80;
uint8_t mul1 = 20;
uint8_t mul2 = 25;
uint8_t mul3 = 22;




// Two_sin()


void two_sin() {
  // reset shared variables specific to this guy
  thisdelay=10; thiscutoff=96; thatcutoff=96; thisrot=1;

  thisphase += thisspeed; thatphase += thatspeed;
  thishue = thishue + thisrot;                                                // Hue rotation is fun for thiswave.
  thathue = thathue + thatrot;                                                // It's also fun for thatwave.

  for (int k=0; k<numLeds-1; k++) {
    int thisbright = qsuba(cubicwave8((k*allfreq)+thisphase), thiscutoff);     // qsub sets a minimum value called thiscutoff. If < thiscutoff, then bright = 0. Otherwise, bright = 128 (as defined in qsub)..
    int thatbright = qsuba(cubicwave8((k*allfreq)+128+thatphase), thatcutoff); // This wave is 180 degrees out of phase (with the value of 128).

    leds[k] = CHSV(thishue, allsat, thisbright);                              // Assigning hues and brightness to the led array.
    leds[k] += CHSV(thathue, allsat, thatbright);
  }
}

// three_sin  ---------------------------------------------------------------------

// Forward declaration
void three_sin();

void threeSinTwo() {
  // reset shared variables specific to this guy
  thisdelay=20; mul1=5; mul2=8; mul3=7;
  three_sin();
}

void three_sin() {
  wave1 += inc1;
  wave2 += inc2;
  wave3 += inc3;
  for (int k=0; k<numLeds; k++) {
    leds[k].r = qsub8(sin8(mul1*k + wave1), lvl1);         // Another fixed frequency, variable phase sine wave with lowered level
    leds[k].g = qsub8(sin8(mul2*k + wave2), lvl2);         // A fixed frequency, variable phase sine wave with lowered level
    leds[k].b = qsub8(sin8(mul3*k + wave3), lvl3);         // A fixed frequency, variable phase sine wave with lowered level
  }
}

// rainbow_march variables -----------------------------------------------------------------

uint8_t deltahue = 1;
void rainbow_march() {
  // reset shared variables specific to this guy
  thisdelay=10; deltahue=2; thisrot=5;
  fill_rainbow(leds, numLeds, thishue, deltahue);               // I don't change deltahue on the fly as it's too fast near the end of the strip.
}


// Eight colored dots, weaving in and out of sync with each other.

void redPurpleBlue() {
  if (enableColorPin == 0) {
    thishue = 255;
  }
  fill_solid(leds, numLeds, CRGB(thishue, 0, 255-thishue));
}

void greenYellowRed() {
  if (enableColorPin == 0) {                                      
    thishue = 255;
  }
  fill_solid(leds, numLeds, CRGB(255-thishue, thishue, 0));
}

void greenBlue() {
  if (enableColorPin == 0) {
    thishue = 255;
  }
  fill_solid(leds, numLeds, CRGB(0, 255-thishue, thishue));
}

void orange() {    
  fill_solid(leds, numLeds, CRGB(255, 100, 0));
}

void purple() {    
  fill_solid(leds, numLeds, CRGB(255, 0, 255));
}

void breathing() {
  static float breath = 0;
  breath += 0.01;
  if (breath > TWO_PI) breath = 0;

  uint8_t brightness = beatsin8(6, min_bright, max_bright, 0, 0); // 6 BPM
  fill_solid(leds, numLeds, CHSV(140, 150, brightness)); // Soft blue
}

void aurora() {
  fadeToBlackBy(leds, numLeds, 5);
  static uint16_t t = 0;
  t += 5;

  for (int i = 0; i < numLeds; i++) {
    uint16_t noise = inoise8(i * 30, t);
    leds[i] += CHSV(140 + noise / 4, 200 - noise / 2, noise);
  }
}

void lavaLamp() {
  static uint16_t x = 0;
  x += 10;

  for (int i = 0; i < numLeds; i++) {
    uint8_t noise = inoise8(i * 50, x);
    uint8_t hue = map(noise, 0, 255, 10, 30); // Orange-red range
    leds[i] = CHSV(hue, 255, noise);
  }
  EVERY_N_SECONDS(30) {
    FastLED.setBrightness(random8(min_bright + 20, max_bright - 20));
  }
}

void colorMeditation() {
  static uint8_t hue = 0;
  EVERY_N_MINUTES(5) {
    hue = random8();
  }

  // Gradient that shifts imperceptibly
  fill_gradient(leds, numLeds,
              CHSV(hue, 255, max_bright),
              CHSV(hue + 30, 200, max_bright - 30),
              fl::FORWARD_HUES);

  EVERY_N_MILLISECONDS(100) {
    hue++;
  }
}

void oceanWaves() {
  static uint16_t offset = 0;
  offset += 3;

  for (int i = 0; i < numLeds; i++) {
    uint8_t wave = sin8(i * 10 + offset);
    leds[i] = CHSV(135, 200, wave); // Seafoam green
  }
  fadeLightBy(leds, numLeds, 20);
}

void moonlight() {
  fadeToBlackBy(leds, numLeds, 1);

  EVERY_N_SECONDS(2) {
    // Occasionally add sparkles
    if (random8() < 30) {
      uint8_t pos = random16(numLeds);
      leds[pos] = CRGB(220, 220, 255); // Moonlight white
    }
  }

  // Base soft blue light
  fill_solid(leds, numLeds, CRGB(25, 35, 80));
}

void heartbeat() {
  static uint8_t pulse = 0;
  pulse = beatsin8(40, min_bright, max_bright); // ~72 BPM

  CRGB color = CRGB::Red;
  color.fadeToBlackBy(220 - pulse);
  fill_solid(leds, numLeds, color);
}

void forestCanopy() {
  static uint16_t t = 0;
  t += 5;

  for (int i = 0; i < numLeds; i++) {
    uint8_t green = inoise8(i * 20, t);
    uint8_t blue = inoise8(i * 20 + 10000, t);
    leds[i] = CRGB(0, green/2, blue/3);
  }
  fadeLightBy(leds, numLeds, 5);
}

void strobePulse() {
  static uint8_t pulse = 0;
  pulse = beatsin8(120, 50, 255); // 120 BPM

  if (pulse > 200) {
    fill_solid(leds, numLeds, CHSV(random8(), 255, 255));
  } else {
    fill_solid(leds, numLeds, CRGB::Black);
  }
}

void beatScanner() {
  static uint8_t pos = 0;
  EVERY_N_MILLISECONDS(20) {
    pos = beatsin16(60, 0, numLeds-1); // 120 BPM

    fadeToBlackBy(leds, numLeds, 50);
    leds[pos] += CHSV(gHue, 255, 255);
    leds[(numLeds-1)-pos] += CHSV(gHue+128, 255, 255);

    gHue += 2;
  }
}

void energyRipple() {
  static uint8_t center = 0;
  EVERY_N_MILLISECONDS(50) {
    center = random16(numLeds);
  }

  EVERY_N_MILLISECONDS(20) {
    fadeToBlackBy(leds, numLeds, 30);
    uint8_t pulse = beatsin8(180, 50, 255); // Fast pulse

    for (int i = 0; i < 20; i++) {
      leds[(center + i) % numLeds] += CHSV(gHue + i*12, 255, pulse);
      leds[(center - i + numLeds) % numLeds] += CHSV(gHue + i*12, 255, pulse);
    }
    gHue += 3;
  }
}

void colorSlam() {
  static uint8_t lastBeat = 0;
  uint8_t beat = beatsin8(60, 0, 100); // 120 BPM

  if (beat < 10 && lastBeat >= 10) {
    // Beat hit - flash new color
    fill_solid(leds, numLeds, CHSV(random8(), 255, 255));
  } else {
    // Decay after hit
    fadeToBlackBy(leds, numLeds, 30);
  }
  lastBeat = beat;
}

void hyperSpin() {
  static uint16_t angle = 0;
  angle += beatsin16(60, 10, 30); // Speed varies with beat

  for (int i = 0; i < numLeds; i++) {
    uint8_t colorIndex = (i * 10) - angle;
    leds[i] = ColorFromPalette(RainbowStripeColors_p, colorIndex, beatsin8(120, 200, 255), LINEARBLEND);
  }
}

void beatTrails() {
  EVERY_N_MILLISECONDS(20) {
    // Add sparks on beat
    if (random8() < beatsin8(60, 10, 50)) {
      uint8_t pos = random16(numLeds);
      leds[pos] = CHSV(gHue + random8(64), 255, 255);
    }

    // Fade and shift
    fadeToBlackBy(leds, numLeds, 20);
    for (int i = numLeds-1; i > 0; i--) {
      leds[i] |= leds[i-1];
    }
    gHue += 2;
  }
}

void strobeWave() {
  static uint16_t offset = 0;
  offset += beatsin16(90, 5, 20); // Fast motion

  uint8_t strobe = beatsin8(180, 0, 255); // Strobe effect

  for (int i = 0; i < numLeds; i++) {
    if (strobe > 200) {
      leds[i] = CHSV((i * 10) + offset, 255, 255);
    } else if (strobe < 50) {
      leds[i] = CRGB::Black;
    } else {
      leds[i] = ColorFromPalette(HeatColors_p, (i * 5) + offset, 255, LINEARBLEND);
    }
  }
}

void beatDrop() {
  static uint8_t dropCounter = 0;

  EVERY_N_MILLISECONDS(50) {
    if (dropCounter == 0) {
      // Build anticipation
      fill_solid(leds, numLeds, CHSV(gHue, 255, beatsin8(30, 50, 150)));
    } else if (dropCounter < 10) {
      // Flash white during drop
      fill_solid(leds, numLeds, CRGB(255,255,255));
    } else {
      // Post-drop color explosion
      fadeToBlackBy(leds, numLeds, 30);
      for (int i = 0; i < 5; i++) {
        uint16_t pos = random16(numLeds);
        leds[pos] = CHSV(gHue + random16(64), 255, 255);
      }
    }

    dropCounter = (dropCounter + 1) % 30;
    if (dropCounter == 0) gHue += 64;
  }
}