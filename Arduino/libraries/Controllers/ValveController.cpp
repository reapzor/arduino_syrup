//Chux
#include "ValveController.h"

ValveController::ValveController(int valvePin)
{
  m_valveSignalPin = valvePin;
  m_suggestedState = UNDEF;
  m_delayTime = 0;
  pinMode(valvePin, OUTPUT);
  digitalWrite(m_valveSignalPin, LOW);
  m_valveState = CLOSED;
  m_stateIsForced = false;
}

void ValveController::openValve()
{
  changeValveState(false, OPEN);
}

void ValveController::closeValve()
{
  changeValveState(false, CLOSED);
}

void ValveController::forceOpenValve()
{
  changeValveState(true, OPEN);
}

void ValveController::forceCloseValve()
{
  changeValveState(true, CLOSED);
}

void ValveController::swapValveState()
{
  swapValveState(false);
}

void ValveController::forceSwapValveState()
{
  swapValveState(true);
}

void ValveController::swapValveState(bool force)
{
  switch (m_valveState)
  {
    case OPEN:
      changeValveState(force, CLOSED);
      break;
    case CLOSED:
      changeValveState(force, OPEN);
      break;
  }
}

void ValveController::releaseForcedState()
{
  if (!m_stateIsForced) {
    return;
  }
  m_stateIsForced = false;
  forceExpireDelay();
  changeValveToSuggestedState();
}

void ValveController::changeValveState(bool ignoreDelay, e_valveState state)
{
  if (!ignoreDelay && m_stateIsForced) {
    #ifdef DEBUG
      Serial.print(F("Told to change valve state on pin "));
      Serial.print(m_valveSignalPin);
      Serial.print(F(" while in a forced state. State: "));
      Serial.println(state);
    #endif
    if (m_suggestedState != state) {
      m_suggestedState = state;
    }
    return;
  }
  if (!ignoreDelay && !delayExpired()) {
    #ifdef DEBUG
      Serial.print(F("Told to change valve state on pin "));
      Serial.print(m_valveSignalPin);
      Serial.print(F(" before delay finish. State:"));
      Serial.println(state);
    #endif
    if (m_suggestedState != state) {
      m_suggestedState = state;
    }
    return;
  }
  if (ignoreDelay && !m_stateIsForced) {
    #ifdef DEBUG
      Serial.print(F("Forcing change of valve state on pin "));
      Serial.println(m_valveSignalPin);
    #endif
    if (m_suggestedState == UNDEF) {
      m_suggestedState = m_valveState;
    }
    m_stateIsForced = true;
  }  
  m_valveState = state;
  resetDelay();
  switch (state)
  {
    case OPEN:
      digitalWrite(m_valveSignalPin, HIGH);
      break;
    case CLOSED:
      digitalWrite(m_valveSignalPin, LOW);
      break;
  }
  #ifdef DEV
    Serial.print(F("VALVE ON PIN "));
    Serial.print(m_valveSignalPin);
    Serial.print(F(" SET TO STATE: "));
    Serial.println(state);
  #endif
  notify();
}

void ValveController::resetDelay()
{
  m_delayTime = (long)millis() + STATE_CHANGE_DELAY;
}

bool ValveController::delayExpired()
{
  return ((long)millis()-m_delayTime >= 0);
}

void ValveController::forceExpireDelay()
{
  m_delayTime = 0;
}

void ValveController::tick()
{
  if (m_suggestedState == UNDEF || m_stateIsForced
      || !delayExpired()) {
    return;
  }
  changeValveToSuggestedState();
}

void ValveController::changeValveToSuggestedState()
{
  switch(m_suggestedState)
  {
    case OPEN:
      openValve();
      break;
    case CLOSED:
      closeValve();
      break;
    case UNDEF:
      return;
    default:
      break;
  }
  m_suggestedState = UNDEF;
}

