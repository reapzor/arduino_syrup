//Chux
#include "TempValveManager.h"

TempValveManager::TempValveManager(TempProbe *tempProbe, ValveController *valveController,
  SyrupSettingsManager *settingsManager) :
    m_pTempProbe(tempProbe), m_pValveController(valveController), 
    m_pSettingsManager(settingsManager),
    Observer<TempProbe>(), Subject<TempValveManager>()
{
  m_thresholdRegion = UNDEF;
  m_hasDoneUpperBoundsTask = false;
}
TempValveManager::~TempValveManager()
{
  //unregisterObservers();
  //Since it never dies
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
    return;
  }
  if (temp < (lowerThreshold - boundsThreshold) && m_thresholdRegion != BELOW) {
    m_thresholdRegion = BELOW;
    #ifdef DEBUG_THRES
      Serial.println(F("TVM: BELOW"));
    #endif
    tryLowerBoundsTask();
    notify();
    return;
  }
}

void TempValveManager::tryUpperBoundsTask()
{
  if (!m_hasDoneUpperBoundsTask || m_thresholdRegion == UNDEF) {
    doUpperBoundsTask();
    m_hasDoneUpperBoundsTask = true;
  }
}

void TempValveManager::tryLowerBoundsTask()
{
  if (m_hasDoneUpperBoundsTask || m_thresholdRegion == UNDEF) {
    doLowerBoundsTask();
    m_hasDoneUpperBoundsTask = false;
  }
}

void TempValveManager::doUpperBoundsTask()
{
  #ifdef DEBUG_BOUNDS
    Serial.println(F("TVM: UPPER BOUNDS"));
  #endif
  m_pValveController->openValve();
}

void TempValveManager::doLowerBoundsTask()
{
  #ifdef DEBUG_BOUNDS
    Serial.println(F("TVM: LOWER BOUNDS"));
  #endif
  m_pValveController->closeValve();
}

