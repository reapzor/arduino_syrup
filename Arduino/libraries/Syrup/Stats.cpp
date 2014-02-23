//Chux
#include "Stats.h"
#include <MemoryFree.h>

Stats::Stats(TempValveManager *tempValveManager, TempProbe *tempProbe) :
    m_pTempValveManager(tempValveManager), m_pTempProbe(tempProbe),
    Subject<Stats>(), Observer<TempValveManager>(), Observer<TempProbe>()
{
  reset();
  m_uptime = 0;
  m_freeMem = 0;
  m_currentDuration = 0;
}

void Stats::reset()
{
  m_tempMin = 1023; // :(
  m_tempMax = 0;
  m_lastDurationClosed = 0;
  m_lastDurationOpen = 0;
  m_averageDurationClosed = 0;
  m_averageDurationOpen = 0;
  m_countClosed = 0;
  m_countOpen = 0;
  m_nextMinute = 0;
  m_nextSecond = 0;
  resetNextSecond();
  resetNextMinute();
  sendNotify(RESET);
}

void Stats::registerObservers()
{
  m_pTempProbe->attach(this);
  m_pTempValveManager->attach(this);
}

void Stats::unregisterObservers()
{
  m_pTempProbe->detach(this);
  m_pTempValveManager->detach(this);
}

void Stats::update(TempValveManager *tempValveManager)
{
  switch(tempValveManager->m_taskRun)
  {
    case TempValveManager::UPPER_TASK:
      m_lastDurationClosed = m_currentDuration;
      sendNotify(LAST_DURATION_CLOSED);
      m_countOpen++;
      #ifdef DEBUG_OBSERVERS
        Serial.print(F("VALVE OBSERVER CHANGE! COUNT OPEN: "));
        Serial.println(m_countOpen);
      #endif
      sendNotify(COUNT_OPEN);
      m_currentDuration = 0;
      resetNextSecond();
      sendNotify(CURRENT_DURATION);
      if (m_countClosed == 0) {
        m_averageDurationClosed = 0;
      }
      else {
        m_averageDurationClosed = ((m_averageDurationClosed * (m_countClosed-1))
          + m_lastDurationClosed) / m_countClosed;
        sendNotify(AVERAGE_DURATION_CLOSED);
      }
      break;
    case TempValveManager::LOWER_TASK:
      m_lastDurationOpen = m_currentDuration;
      sendNotify(LAST_DURATION_OPEN);
      m_countClosed++;
      #ifdef DEBUG_OBSERVERS
        Serial.print(F("VALVE OBSERVER CHANGE! COUNT CLOSED: "));
        Serial.println(m_countClosed);
      #endif
      sendNotify(COUNT_CLOSED);
      m_currentDuration = 0;
      resetNextSecond();
      sendNotify(CURRENT_DURATION);
      if (m_countOpen == 0) {
        m_averageDurationOpen = 0;
      }
      else {
        m_averageDurationOpen = ((m_averageDurationOpen * (m_countOpen-1))
          + m_lastDurationOpen) / m_countOpen;
        sendNotify(AVERAGE_DURATION_OPEN);
      }
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
  m_updatedStatsValue = statsValue;
  notify();
  m_updatedStatsValue = IDLE;
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
  m_nextSecond = millis();
  setNextSecond();
}

void Stats::resetNextMinute()
{
  m_nextMinute = millis();
  setNextMinute();
}

void Stats::tick()
{
  if ((long)(millis()-m_nextSecond) >= 0) {
    setNextSecond();
    m_currentDuration++;
    sendNotify(CURRENT_DURATION);
  }  
  if ((long)(millis()-m_nextMinute) >= 0) {
    setNextMinute();
    m_uptime++;
    #ifdef DEV
      Serial.print(F("Up: "));
      Serial.println(m_uptime);
    #endif
    sendNotify(UPTIME);    
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
