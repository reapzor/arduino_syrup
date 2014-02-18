//Chux
#ifndef Encoder_h
#define Encoder_h

#include "Arduino.h"
#include "Subject.h"
//#define DEBUG_HW
//#define DEBUG

class Encoder : public Subject<Encoder>
{
  public:
    enum e_direction
    {
      UNDEF,
      IDLE,
      FORWARD,
      BACKWARD
    };
    Encoder(int triggerPin, int directionPin);
    int m_triggerPin;
    int m_directionPin;
    e_direction m_direction;
    void read();
    void pause();
    void unpause();
    void prime();
    void tick();
  private:
    bool m_paused;
    long m_delayTime;
    static const int READ_DELAY = 1;
};

#endif