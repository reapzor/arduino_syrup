//Chux
#ifndef Button_h
#define Button_h

#include "Arduino.h"
#include "Subject.h"
//#define DEBUG_HW
#define DEBUG

class Button : public Subject<Button>
{
  public:
    enum e_buttonState
    {
      UNDEF,
      ON,
      OFF
    };
    Button(int triggerPin);
    int m_triggerPin;
    e_buttonState buttonState;
    void read();
    void pause();
    void unpause();
    void tick();
  private:
    bool m_paused;
    long m_delayTime;
    static const int READ_DELAY = 7;
};

#endif