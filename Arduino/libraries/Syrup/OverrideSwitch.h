//Chux
#ifndef OverrideSwitch_h
#define OverrideSwitch_h

#include "Arduino.h"
#include "Subject.h"
#include "Button.h"

//#define DEBUG_HW
#define DEBUG

class OverrideSwitch : public Button<OverrideSwitch>
{
  public:
    OverrideSwitch(int pin) : Button<OverrideSwitch>(pin) {}
    ~OverrideSwitch() {}
    
};

#endif