/* button_demoReel100
 *
 * Originally By: Mark Kriegsman
 * Modified for EEPROM and button usage by: Andrew Tuline
 * current iteration: Joosep koivistik https://github.com/equilerex
 * Date: August, 2019
 *
 * Instructions:
 * 
 * Program reads display mode from EEPROM and displays it.
 * Click to change to the next mode.
 * Hold button for > 1 second to write the current mode to EEPROM.
 * Double click to reset to mode 0.
 * 
 * There's also debug output to the serial monitor . . just to be sure.
 * 
 * Requirements:
 * 
 * Jeff Saltzman's jsbutton.h routine is included  Please ensure that it is 
 * downloaded alongside button_demoReel100.ino and in the same directory.
 * 
 * 
 */


//--------------------[ Dependencies ]------------------------------------------- 
#include "EEPROM.h"
#include "FastLED.h"
#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif
#define qsubd(x, b)  ((x>b)?wavebright:0)                     // A digital unsigned subtraction macro. if result <0, then => 0. Otherwise, take on fixed value.
#define qsuba(x, b)  ((x>b)?x-b:0)                            // Unsigned subtraction macro. if result <0, then => 0.


//--------------------[ Fixed definitions - cannot change on the fly. ]------------------------------------------- 

// pin configuration
#define buttonPin 12                                          // input pin to use as a digital input
#include "jsbutton.h"                                         // Nice button routine by Jeff Saltzman
#define LED_DT 10                                             // Data pin to connect to the strip.
#define brightnessPin A1                                       // analogue pin for brightness pot (comment out if you dont have it)
#define colorPin A0                                            // analogue pin for colo pot (comment out if you dont have it)
bool enableBrightnessPin = 1;                                // if your controller has no brightness potentiometer, set to 0
bool enableColorPin = 1;                                     // if your controller has no color potentiometer, set to 0

#define LED_CK 11                                             // Clock pin for WS2801 or APA102.
#define COLOR_ORDER GRB                                       // It's GRB for WS2812 and BGR for APA102.
#define LED_TYPE WS2812                                       // Using APA102, WS2812, WS2801. Don't forget to modify LEDS.addLeds to suit.
#define NUM_LEDS 150                                          // Number of LED's.


//--------------------[ Setup #1 (ignore) ]------------------------------------------- 
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))            // Definition for the array of routines to display.
int eepaddress = 0;                                           //Mode and EEPROM variables (savig across restarts) 
struct CRGB leds[NUM_LEDS];                                   // Initialize our LED array.
uint8_t gCurrentPatternNumber = 0;                            // Index number of which pattern is current         
uint8_t previousColorPotVal = 0;                              // avoid overriding loop values when color pot is not touched  
uint8_t previousBrightnessPotVal = 0;                              // avoid overriding loop values when brightness pot is not touched  


 
//--------------------[ Global variables - can be changed on the fly. ]------------------------------------------- 

 // Overall brightness definition. It can be changed on the fly.
uint8_t max_bright = 128;       
                              
// rotating "base color" used by many of the patterns
uint8_t gHue = 0;      
// Standard hue
uint8_t thisdelay = 20; 
// A delay value for the sequence(s)                                       
uint8_t thishue = 140;                                                                    

// rainbow mode creates scope issues if put below with all the rest
// FastLED's built-in rainbow generator.
void rainbow() {
  fill_rainbow(leds, NUM_LEDS, gHue, 7);                      
} // rainbow()                      



//--------------------[ define patterns to loop through (add new or remove existing) ]------------------------------------------- 
typedef void (*SimplePatternList[])();  


SimplePatternList gPatterns = { rainbowWithGlitter, rainbow_march, sinelon, two_sin, pop_fade, confetti, juggle, bpm, threeSinTwo, rainbow, redPurpleBlue, greenYellowRed, greenBlue, orange, purple, noise16};            

 

//--------------------[ setup #2 (ignore) ]------------------------------------------- 
void setup() {
  
  // Initialize serial port for debugging.
  Serial.begin(115200);              
                           
  // Soft startup to ease the flow of electrons.
  delay(1000);                                               
  
  // Set mode switch pin
  pinMode(buttonPin, INPUT);                                 
  digitalWrite(buttonPin, HIGH );
  previousBrightnessPotVal = analogRead(brightnessPin)/4;
  previousColorPotVal = analogRead(colorPin)/4;

  // setup fastled library
  //  LEDS.addLeds<LED_TYPE, LED_DT, LED_CK, COLOR_ORDER>(leds, NUM_LEDS);  // Use this for WS2801 or APA102
  LEDS.addLeds<LED_TYPE, LED_DT, COLOR_ORDER>(leds, NUM_LEDS);  // Use this for WS2812
  FastLED.setBrightness(max_bright);
  
  // FastLED Power management set at 5V, 500mA.
  set_max_power_in_volts_and_milliamps(5, 500);              

  // pull saved mode from memory
  gCurrentPatternNumber = EEPROM.read(eepaddress);
  // A safety in case the EEPROM has an illegal value.
  if (gCurrentPatternNumber > ARRAY_SIZE(gPatterns)) gCurrentPatternNumber = 0;   

  // log
  Serial.print("Starting ledMode: ");
  Serial.println(gCurrentPatternNumber);

}  



void loop() {

  readbutton();

  if (enableBrightnessPin==1) {
    readBrightness();
  }
  
  if (enableColorPin==1) {
    readColor();
  }
  
  EVERY_N_MILLISECONDS(50) {
    gPatterns[gCurrentPatternNumber]();                       // Call the current pattern function once, updating the 'leds' array    
  }

  EVERY_N_MILLISECONDS(20) {                                  // slowly cycle the "base color" through the rainbow
    gHue++;
  }

  FastLED.show();

}





//--------------------[ helper functions ]------------------------------------------------------------------------------

// Read the button and increase the mode
void readbutton() {

  uint8_t b = checkButton();

  // Just a click event to advance to next pattern
  if (b == 1) {
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
    Serial.println(gCurrentPatternNumber);
  }

  // A double-click event to reset to 0 pattern
  if (b == 2) {
    gCurrentPatternNumber = 0;
    Serial.println(gCurrentPatternNumber);
  }

  // A hold event to write current pattern to EEPROM
  if (b == 3) {
    EEPROM.write(eepaddress,gCurrentPatternNumber);
    // Serial.print("Writing: ");
    // Serial.println(gCurrentPatternNumber);
  }

}

// potentiometer to set brightness
void readBrightness() {
  uint8_t newBrightValue = analogRead(brightnessPin)/4;
   
  if( abs(newBrightValue-previousBrightnessPotVal) > 5) {
    previousBrightnessPotVal = newBrightValue;
    max_bright = newBrightValue;
    FastLED.setBrightness(max_bright);
  }
}

// potentiometer to affect color
void readColor() { 
  uint8_t newColorValue = analogRead(colorPin)/4; 
  if(abs(newColorValue-previousColorPotVal) > 10) {
    previousColorPotVal = newColorValue;
    gHue = newColorValue;
    thishue = newColorValue;
    // Serial.print("newColorValue");
    // Serial.println(newColorValue);
  }
}

// adds little random white flashes
void addGlitter(fract8 chanceOfGlitter) {

  if(random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }

}




//--------------------[ modes are below here ]------------------------------------------------------------------------------
 
// Built-in FastLED rainbow, plus some random sparkly glitter.
void rainbowWithGlitter() {
  rainbow();                                                 
  addGlitter(80);
  
}



// Random colored speckles that blink in and fade smoothly.
void confetti() {                
  fadeToBlackBy(leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV(gHue + random8(64), 200, 255);
  
}


// A colored dot sweeping back and forth, with fading trails.
void sinelon() {                                              

  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = beatsin16(13,0,NUM_LEDS-1);
  leds[pos] += CHSV(gHue, 255, 192);
  
}


// Colored stripes pulsing at a defined Beats-Per-Minute.
void bpm() {                                                 

  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
  
  for(int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
  
}



// Eight colored dots, weaving in and out of sync with each other.
void juggle() {      
  fadeToBlackBy(leds, NUM_LEDS, 20);
  byte dothue = 0;
  
  for(int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,NUM_LEDS-1)] |= CHSV(dothue, 200, 255);
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

  fill_noise16(leds, NUM_LEDS, octaves, x, xscale, hue_octaves, hxy, hue_scale, hue_time); 
  
    for (int i=0; i<NUM_LEDS; i++) {                            // We can filter, we can invert, we can do all sorts of things.
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
  if (ranamount >NUM_LEDS) ranamount = NUM_LEDS;              // Make sure we're at least utilizing ALL the LED's.
  int idex = random16(0, ranamount);

  if (idex < NUM_LEDS) {                                      // Only the lowest probability twinkles will do.
    boolcolours ? thiscolour = random(0, 0xffffff) : thiscolour =  colours[random16(0, numcolours)];     
    int barlen = random16(1,maxbar);
    for (int i = 0; i <barlen; i++) 
      if (idex+i < NUM_LEDS) leds[idex+i] = thiscolour;       // Make sure we don't overshoot the array.
  }
     nscale8(leds,NUM_LEDS,fadeval);                          // Fade the entire array. Or for just a few LED's, use  nscale8(&leds[2], 5, fadeval);
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
  
  for (int k=0; k<NUM_LEDS-1; k++) {
    int thisbright = qsuba(cubicwave8((k*allfreq)+thisphase), thiscutoff);     // qsub sets a minimum value called thiscutoff. If < thiscutoff, then bright = 0. Otherwise, bright = 128 (as defined in qsub)..
    int thatbright = qsuba(cubicwave8((k*allfreq)+128+thatphase), thatcutoff); // This wave is 180 degrees out of phase (with the value of 128).

    leds[k] = CHSV(thishue, allsat, thisbright);                              // Assigning hues and brightness to the led array.
    leds[k] += CHSV(thathue, allsat, thatbright);                      
  }
} 

// three_sin  ---------------------------------------------------------------------



void threeSinTwo() {
  // reset shared variables specific to this guy
  thisdelay=20; mul1=5; mul2=8; mul3=7;
  three_sin();
}
void three_sin() {
  wave1 += inc1;
  wave2 += inc2;
  wave3 += inc3;
  for (int k=0; k<NUM_LEDS; k++) {
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
  fill_rainbow(leds, NUM_LEDS, thishue, deltahue);               // I don't change deltahue on the fly as it's too fast near the end of the strip.
}


// Eight colored dots, weaving in and out of sync with each other.

void redPurpleBlue() {      
  if (enableColorPin == 0) {                                   
    thishue = 255;
  };
 fill_solid(leds, NUM_LEDS,CRGB(thishue,0,255-thishue)); LEDS.show();
 LEDS.show();
}
void greenYellowRed() {  
  if (enableColorPin == 0) {                                      
   thishue = 255;
  };
 fill_solid(leds, NUM_LEDS,CRGB(255-thishue,thishue,0)); LEDS.show();
 LEDS.show();
}
void greenBlue() {    
  if (enableColorPin == 0) {
    thishue = 255;
  };  
 fill_solid(leds, NUM_LEDS,CRGB(0,255-thishue,thishue)); LEDS.show();
 LEDS.show();
}

void orange() {    
  if (enableColorPin == 0) { 
   fill_solid(leds, NUM_LEDS,CRGB(255,100,0)); LEDS.show();
   LEDS.show();
  } else {
      
  };
}

void purple() {    
  if (enableColorPin == 0) { 
   fill_solid(leds, NUM_LEDS,CRGB(255,0,255)); LEDS.show();
   LEDS.show();
  } else {
      
  };
}
