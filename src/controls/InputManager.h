/**
 * Input Manager
 * 
 * Handles button input and long press modes
 */

#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <OneButton.h>
#include "../config/Config.h"
// Forward declarations
class SystemManager;

class InputManager {
public:
    // Initialize the input manager
    InputManager();
    
    // Begin the input manager
    void begin(SystemManager* sysManager);
    
    // Update the input manager (call in loop)
    void update();
    
    // Check if we're in brightness adjustment mode
    bool isBrightnessMode();
    
    // Check if we're in LED count adjustment mode
    bool isLedCountMode();

    
    // Check if we're in any long press mode
    bool isLongPressMode() const { return longPressMode; }

private:
    int buttonPin;
    OneButton button;
    SystemManager* systemManager;
    
    // Static instance pointer for callbacks
    static InputManager* instance;
    
    // Button state variables
    bool brightnessMode;
    bool ledCountUpMode;
    bool ledCountDownMode;
    bool longPressMode;
    unsigned long longPressStartTime;
    
    // Callback functions for button events
    static void onClickHandler();
    static void onLongPressStartHandler();
    static void onLongPressStopHandler();
    
    // Handle button click
    void handleClick();
    
    // Handle long press start
    void handleLongPressStart();
    
    // Handle long press stop
    void handleLongPressStop();
    
    // Cycle the long press mode
    void cycleLongPress();
};

#endif // INPUT_MANAGER_H
