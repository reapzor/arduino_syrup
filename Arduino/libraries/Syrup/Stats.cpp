//Chux
#include "Stats.h"
#include <MemoryFree.h>

Stats::Stats(ValveController *valveController, TempProbe *tempProbe) :
    m_pValveController(valveController), m_pTempProbe(tempProbe),
    Subject<Stats>(), Observer<ValveController>(), Observer<TempProbe>()
{
  reset();
}

Stats::~Stats()
{
 //unregisterObservers();
 //Since it never dies
}

void Stats::reset()
{
  m_tempMin = 32767; // :(
  m_tempMax = 0;
  m_currentDuration = 0;
  m_lastDurationClosed = 0;
  m_averageDurationClosed = 0;
  m_averageDurationOpen = 0;
  m_countClosed = 0;
  m_countOpen = 0;
  m_uptime = 0;
  m_freeMem = 0;
  m_nextMinute = 0;
  m_nextSecond = 0;
}

void Stats::registerObservers()
{
  m_pTempProbe->attach(this);
  m_pValveController->attach(this);
}

void Stats::unregisterObservers()
{
  m_pTempProbe->detach(this);
  m_pValveController->detach(this);
}

void Stats::update(ValveController *valve)
{
  switch(valve->m_valveState)
  {
    case ValveController::OPEN:
      m_lastDurationClosed = m_currentDuration;
      sendNotify(LAST_DURATION_CLOSED, false);
      m_countOpen++;
      #ifdef DEBUG_OBSERVERS
        Serial.print(F("VALVE OBSERVER CHANGE! COUNT OPEN: "));
        Serial.println(m_countOpen);
      #endif
      sendNotify(COUNT_OPEN, false);
      m_currentDuration = 0;
      resetNextSecond();
      sendNotify(CURRENT_DURATION, false);
      m_averageDurationClosed = ((m_averageDurationClosed * (m_countClosed-1))
        + m_lastDurationClosed) / m_countClosed;
      sendNotify(AVERAGE_DURATION_CLOSED);
      break;
    case ValveController::CLOSED:
      m_lastDurationOpen = m_currentDuration;
      sendNotify(LAST_DURATION_OPEN, false);
      m_countClosed++;
      #ifdef DEBUG_OBSERVERS
        Serial.print(F("VALVE OBSERVER CHANGE! COUNT CLOSED: "));
        Serial.println(m_countClosed);
      #endif
      sendNotify(COUNT_CLOSED, false);
      m_currentDuration = 0;
      resetNextSecond();
      sendNotify(CURRENT_DURATION, false);
      m_averageDurationOpen = ((m_averageDurationOpen * (m_countOpen-1))
        + m_lastDurationOpen) / m_countOpen;
      sendNotify(AVERAGE_DURATION_OPEN);
      break;
  }
}

void Stats::update(TempProbe *tempProbe)
{
  int temp = tempProbe->m_tempProbeReading;
  if (temp < m_tempMin) {
    m_tempMin = temp;
    sendNotify(TEMP_MIN);
  }
  if (temp > m_tempMax) {
    m_tempMax = temp;
    sendNotify(TEMP_MAX);
  }  
}

void Stats::sendNotify(e_statsValues statsValue)
{
  sendNotify(statsValue, true);
}

void Stats::sendNotify(e_statsValues statsValue, bool shouldReturnToIdle)
{
  m_updatedStatsValue = statsValue;
  notify();
  if (shouldReturnToIdle) {
    m_updatedStatsValue = IDLE;
  }
}

void Stats::prime()
{
  registerObservers();
  updateFreeMem();
  resetNextMinute();
  resetNextSecond();
}

void Stats::resetNextSecond()
{
  m_nextSecond = (long)millis();
  setNextSecond();
}

void Stats::resetNextMinute()
{
  m_nextMinute = (long)millis();
  setNextMinute();
}

void Stats::tick()
{
  if ((long)millis()-m_nextSecond >= 0) {
    setNextSecond();
    m_currentDuration++;
    sendNotify(CURRENT_DURATION);
  }  
  if ((long)millis()-m_nextMinute >= 0) {
    setNextMinute();
    m_uptime++;
    #ifdef DEV
      Serial.print(F("Up: "));
      Serial.println(m_uptime);
    #endif
    sendNotify(UPTIME, false);    
    updateFreeMem();
  }
}

void Stats::updateFreeMem()
{
  m_freeMem = freeMemory();
  #ifdef DEV
    Serial.print(F("Free: "));
    Serial.println(m_freeMem);
  #endif
  sendNotify(FREE_MEM);
}

void Stats::setNextSecond()
{
  m_nextSecond += ONE_SECOND;
}

void Stats::setNextMinute()
{
  m_nextMinute += ONE_MINUTE;
}
