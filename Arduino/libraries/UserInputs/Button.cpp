//Chux
#include "Arduino.h"
#include "Button.h"

Button::Button(int triggerPin)
{
  m_triggerPin = triggerPin;
  pinMode(m_triggerPin, INPUT_PULLUP);
  buttonState = UNDEF;
  m_paused = false;
}

void Button::read()
{
  if (m_paused) {
    return;
  }
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
  if (triggerPinValue == LOW && buttonState != ON) {
    buttonState = ON;
    #ifdef DEBUG
      Serial.print(F("Button "));
      Serial.print(m_triggerPin);
      Serial.println(F(" ON"));
    #endif
    notify();
  }
  //Button is OFF
  else if (triggerPinValue == HIGH && buttonState != OFF) {
    buttonState = OFF;
    #ifdef DEBUG
      Serial.print(F("Button "));
      Serial.print(m_triggerPin);
      Serial.println(F(" OFF"));
    #endif
    notify();
  }
}

void Button::pause()
{
  m_paused = true;
}

void Button::unpause()
{
  m_paused = false;
}

void Button::tick()
{
  if ((long)millis() - m_delayTime >= 0) {
    m_delayTime = (long)millis() + READ_DELAY;
    read();
  }
}
