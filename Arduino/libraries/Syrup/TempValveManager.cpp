//Chux
#include "TempValveManager.h"

TempValveManager::TempValveManager(TempProbe *tempProbe, ValveController *valveController,
  SyrupSettingsManager *settingsManager) :
    m_pTempProbe(tempProbe), m_pValveController(valveController), 
    m_pSettingsManager(settingsManager),
    Observer<TempProbe>(), Subject<TempValveManager>()
{
  m_thresholdRegion = UNDEF;
  m_taskSwitch = false;
  m_taskRun = NO_TASK;
}

void TempValveManager::registerObservers()
{
  m_pTempProbe->attach(this);
}

void TempValveManager::unregisterObservers()
{
  m_pTempProbe->detach(this);
}

void TempValveManager::update(TempProbe *tempProbe)
{
  switch(m_pSettingsManager->m_settings.m_tempScale)
  {
    case TempProbe::FAHRENHEIT:
      updateThreshold(tempProbe->m_tempF, BOUNDS_THRESHOLD_F);
      break;
    case TempProbe::CELCIUS:
      updateThreshold(tempProbe->m_tempC, BOUNDS_THRESHOLD_C);
      break;
  }
}

void TempValveManager::updateThreshold(float temp, float boundsThreshold)
{
  float upperThreshold = m_pSettingsManager->m_settings.m_upperThreshold;
  float lowerThreshold = m_pSettingsManager->m_settings.m_lowerThreshold;
  if (temp >= (upperThreshold + boundsThreshold) && m_thresholdRegion != OVER) {
    m_thresholdRegion = OVER;
    #ifdef DEBUG_THRES
      Serial.println(F("TVM: OVER"));
    #endif
    tryUpperBoundsTask();
    notify();
    if (m_taskRun != NO_TASK) {
      m_taskRun = NO_TASK;
    }
    return;
  }
  if (temp >= upperThreshold && temp < (upperThreshold + boundsThreshold)
      && m_thresholdRegion != UPPER) {
    m_thresholdRegion = UPPER;
    #ifdef DEBUG_THRES
      Serial.println(F("TVM: UPPER"));
    #endif
    tryUpperBoundsTask();
    notify();
    if (m_taskRun != NO_TASK) {
      m_taskRun = NO_TASK;
    }    
    return;
  }
  if (temp <= (upperThreshold - boundsThreshold) && m_thresholdRegion >= UPPER
      && m_thresholdRegion != UNDEF) {
    m_thresholdRegion = MEDIAN_DESCENDING;
    #ifdef DEBUG_THRES
      Serial.println(F("TVM: DESCENDING"));
    #endif
    notify();
    return;
  }
  if (temp >= (lowerThreshold + boundsThreshold) && m_thresholdRegion < MEDIAN_ASCENDING
      && m_thresholdRegion != UNDEF) {
    m_thresholdRegion = MEDIAN_ASCENDING;
    #ifdef DEBUG_THRES
      Serial.println(F("TVM: ASCENDING"));
    #endif
    notify();
    return;
  }
  if (temp >= (lowerThreshold - boundsThreshold) && temp <= lowerThreshold
      && m_thresholdRegion != LOWER) {
    m_thresholdRegion = LOWER;
    #ifdef DEBUG_THRES
      Serial.println(F("TVM: LOWER"));
    #endif
    tryLowerBoundsTask();
    notify();
    if (m_taskRun != NO_TASK) {
      m_taskRun = NO_TASK;
    }
    return;
  }
  if (temp < (lowerThreshold - boundsThreshold) && m_thresholdRegion != BELOW) {
    m_thresholdRegion = BELOW;
    #ifdef DEBUG_THRES
      Serial.println(F("TVM: BELOW"));
    #endif
    tryLowerBoundsTask();
    notify();
    if (m_taskRun != NO_TASK) {
      m_taskRun = NO_TASK;
    }
    return;
  }
}

void TempValveManager::tryUpperBoundsTask()
{
  if (!m_taskSwitch) {
    doUpperBoundsTask();
    m_taskSwitch = true;
  }
}

void TempValveManager::tryLowerBoundsTask()
{
  if (m_taskSwitch) {
    doLowerBoundsTask();
    m_taskSwitch = false;
  }
}

void TempValveManager::doUpperBoundsTask()
{
  #ifdef DEBUG_BOUNDS
    Serial.println(F("TVM: UPPER BOUNDS"));
  #endif
  if (m_pValveController->m_valveState == ValveController::CLOSED) {
    m_pValveController->openValve();
    m_taskRun = UPPER_TASK;
  }
}

void TempValveManager::doLowerBoundsTask()
{
  #ifdef DEBUG_BOUNDS
    Serial.println(F("TVM: LOWER BOUNDS"));
  #endif
  if (m_pValveController->m_valveState == ValveController::OPEN) {
    m_pValveController->closeValve();
    m_taskRun = LOWER_TASK;
  }
}

