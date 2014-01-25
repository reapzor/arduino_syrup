//Chux
#ifndef AnalogHelper_h
#define AnalogHelper_h

#include "Arduino.h"

namespace AnalogHelper
{

  static int getDigitalAnalogPin(int pin)
  {
    switch(pin)
    {
      case 0:
        return A0;
      case 1:
        return A1;
      case 2:
        return A2;
      case 3:
        return A3;
      case 4:
        return A4;
      default:
        return -1;
    }
  }

}

#endif