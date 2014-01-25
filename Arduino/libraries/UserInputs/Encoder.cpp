//Chux
#include "Arduino.h"
#include "Encoder.h"

Encoder::Encoder(int triggerPin, int directionPin)
{
  m_triggerPin = triggerPin;
  m_directionPin = directionPin;
  pinMode(triggerPin, INPUT_PULLUP);
  pinMode(directionPin, INPUT_PULLUP);
  directionState = UNDEF;
  m_paused = false;
  m_delayTime = 0;
}

void Encoder::read()
{
  if (m_paused) {
    return;
  }
  int triggerPinValue = digitalRead(m_triggerPin);
  int directionPinValue = digitalRead(m_directionPin);
  //triggerPin - 0 = trigger hit. 1 = reset
  //directionPin - Changes from 0 to 1 as backward when trigger
  // is low. Changes from 1 to 0 as forward when trigger is low.
  //When we see the trigger, we check directionPin once, 
  // we don't care about the second state it moves to
  // before the trigger is reset.
  #ifdef DEBUG_HW
    Serial.print(triggerPinValue);
    Serial.print(F(" A|B "));
    Serial.println(directionPinValue);
  #endif
  if (triggerPinValue == HIGH && directionPinValue == LOW) {
    //We have detected an invalid state. Bail.
    #ifdef DEBUG_HW
      Serial.println();Serial.println();
      Serial.println(F("HIT INVALID STATE"));
      Serial.println();Serial.println();
    #endif
    return; 
  }
  //If the trigger hit, begin processing the value
  // if the current direction hasnt been caught yet.
  if (triggerPinValue == LOW 
      && directionState == IDLE) {
    //Low is a forward turn.
    if (directionPinValue == LOW) {
      directionState = FORWARD;
    }
    //High is a backward turn.
    else {
      directionState = BACKWARD;
    }
    #ifdef DEBUG
      Serial.print(F("EncState "));
      Serial.println(directionState);
    #endif
    notify();
  }
  //Trigger pin is reset, reset direction if needed.
  else if (triggerPinValue == HIGH) {
    if (directionState != IDLE) {
      directionState = IDLE;
      #ifdef DEBUG_HW
        Serial.print(F("EncState "));
        Serial.println(directionState);
      #endif
      notify();
    }
  }
}

void Encoder::pause()
{
  m_paused = true;
}

void Encoder::unpause()
{
  m_paused = false;
}

void Encoder::tick()
{
  if ((long)millis() - m_delayTime >= 0) {
    m_delayTime = (long)millis() + READ_DELAY;
    read();
  }
}
