//Chux
#ifndef Button_h
#define Button_h

#include "Arduino.h"
#include "Subject.h"
//#define DEBUG_HW
//#define DEBUG

template <class T>
class Button : public Subject<T>
{
  public:
    enum e_buttonState
    {
      UNDEF,
      ON,
      OFF
    };
    
    Button(int triggerPin) : Subject<T>()
    {
      m_triggerPin = triggerPin;
      pinMode(m_triggerPin, INPUT_PULLUP);
      m_buttonState = UNDEF;
    }
    
    ~Button() {}
    int m_triggerPin;
    e_buttonState m_buttonState;
    
    void read()
    {
      //triggerPin: Low = ON/DEPRESSED | High = OFF/RELEASED
      //Since we use pullup, the button needs to lead to ground.
      int triggerPinValue = digitalRead(m_triggerPin);
      #ifdef DEBUG_HW
        Serial.print(F("Button: "));
        Serial.print(m_triggerPin);
        Serial.print(F(" : "));
        Serial.println(triggerPinValue);
      #endif
      //Button is ON
      if (triggerPinValue == LOW && m_buttonState != ON) {
        m_buttonState = ON;
        #ifdef DEBUG
          Serial.print(F("Button "));
          Serial.print(m_triggerPin);
          Serial.println(F(" ON"));
        #endif
        this->notify();
      }
      //Button is OFF
      else if (triggerPinValue == HIGH && m_buttonState != OFF) {
        m_buttonState = OFF;
        #ifdef DEBUG
          Serial.print(F("Button "));
          Serial.print(m_triggerPin);
          Serial.println(F(" OFF"));
        #endif
        this->notify();
      }
    }
    
    void prime()
    {
      m_delayTime = (long)millis();
    }
    
    void tick()
    {
      if ((long)millis() - m_delayTime >= 0) {
        m_delayTime += READ_DELAY;
        read();
      }
    }
    
  private:
    long m_delayTime;
    static const int READ_DELAY = 7;
    
};

#endif