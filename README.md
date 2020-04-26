# Digital-rgb-led-universal-controller
My little universal controller library which i use for all of my projects running WS2812 / WS2812b strips. 
Contains a well curated set of animations which fit most simple project straight out of the box

Forked from [Andrew Tuline](https://github.com/atuline/FastLED-Demos/tree/master/button_demoReel100) who forked it from Mark Kriegsman.
Ive modified the original code extensively, slimmed it down to only things i need, optimized some of the code, added options for brightness
and color potentiometers, etc.

 
### Features
* Brightness control potentiometer
* color modifier potentiometer (works on most animations)
* mode button
    * 1 click to switch mode
    * Double click to reset mode
    * hold button to set the current animation to the default upon reset or reboot

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
* rainbow - static rainbow, no animation
* redPurpleBlue- solid color that you can change from red to purple to blue using the color modifier potentiometer (2 of the)
* greenYellowRed - solid color that you can change from green to yellow to red using the color modifier potentiometer (2 of the)
* greenBlue - solid color that you can change from green to blue using the color modifier potentiometer (2 of the)
* orange - solid color
* purple - solid color
* noise16 - you better not have epilepsy

 
### Installing

1. Hook it up by the schematic:
[wiring](/schematic.jpg)
2. make sure you have arduino editor: https://www.arduino.cc
3. if you decide to not include brightness potentiometer in your build, open the .ino file and change `set enableBrightnessPin = 1` to `set enableBrightnessPin = 0` 
3. if you decide to not include brightness color modifier in your build, open the .ino file and change `set enableBrightnessPin = 1` to `set enableBrightnessPin = 0`
4. set LED_TYPE to reflect your led strip type (WS2812 by default)
5. set NUM_LEDS to the number of your led's  (150 by default)
6. upload the code
6. have fun

note: if you want to remove (or add) any animations, just remove from or add names to `SimplePatternList` object
  
### Author

**Joosep Kõivistik** - [homepage](http://koivistik.com) |  [youtube](https://www.youtube.com/channel/UCqMFsfxrBrQIHnIKoJjqHTA) | |  [Instagram](https://www.instagram.com/joosepkoivistik/)


## Acknowledgments

Big thanks to:
* Mark Kriegsman (for all the lovely animations)
* [Andrew Tuline](https://github.com/atuline/FastLED-Demos) for the original repository
 
