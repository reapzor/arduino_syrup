//Chux
#ifndef ToggleButton_h
#define ToggleButton_h

#include "Arduino.h"
#include "Subject.h"
#include "Button.h"

//#define DEBUG_HW
//#define DEBUG

class ToggleButton : public Button<ToggleButton>
{
  public:
    ToggleButton(int pin) : Button<ToggleButton>(pin) {}
    
};

#endif