//Chux
#include "TempValveManager.h"

TempValveManager::TempValveManager(TempProbe *tempProbe, ValveController *valveController) :
    m_pTempProbe(tempProbe), m_pValveController(valveController), 
    Observer<TempProbe>()
{
  m_upperThreshold = 999.0; // :(
  m_lowerThreshold = 0.0;
  
  m_thresholdRegion = UNDEF;
  m_hasDoneUpperBoundsTask = false;
}
TempValveManager::~TempValveManager()
{
  unregisterObservers();
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
  switch(tempProbe->getScale())
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
  if (temp >= (m_upperThreshold + boundsThreshold) && m_thresholdRegion != OVER) {
    m_thresholdRegion = OVER;
    #ifdef DEBUG_THRES
      Serial.println(F("TVM: OVER"));
    #endif
    tryUpperBoundsTask();
    return;
  }
  if (temp >= m_upperThreshold && temp < (m_upperThreshold + boundsThreshold)
      && m_thresholdRegion != UPPER) {
    m_thresholdRegion = UPPER;
    #ifdef DEBUG_THRES
      Serial.println(F("TVM: UPPER"));
    #endif
    tryUpperBoundsTask();
    return;
  }
  if (temp <= (m_upperThreshold - boundsThreshold) && m_thresholdRegion >= UPPER
      && m_thresholdRegion != UNDEF) {
    m_thresholdRegion = MEDIAN_DESCENDING;
    #ifdef DEBUG_THRES
      Serial.println(F("TVM: DESCENDING"));
    #endif
    return;
  }
  if (temp >= (m_lowerThreshold + boundsThreshold) && m_thresholdRegion < MEDIAN_ASCENDING
      && m_thresholdRegion != UNDEF) {
    m_thresholdRegion = MEDIAN_ASCENDING;
    #ifdef DEBUG_THRES
      Serial.println(F("TVM: ASCENDING"));
    #endif
    return;
  }
  if (temp >= (m_lowerThreshold - boundsThreshold) && temp <= m_lowerThreshold
      && m_thresholdRegion != LOWER) {
    m_thresholdRegion = LOWER;
    #ifdef DEBUG_THRES
      Serial.println(F("TVM: LOWER"));
    #endif
    tryLowerBoundsTask();
    return;
  }
  if (temp < (m_lowerThreshold - boundsThreshold) && m_thresholdRegion != BELOW) {
    m_thresholdRegion = BELOW;
    #ifdef DEBUG_THRES
      Serial.println(F("TVM: BELOW"));
    #endif
    tryLowerBoundsTask();
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

void TempValveManager::setUpperThreshold(float upperThreshold)
{
  m_upperThreshold = upperThreshold;
}

void TempValveManager::setLowerThreshold(float lowerThreshold)
{
  m_lowerThreshold = lowerThreshold;
}

float TempValveManager::getUpperThreshold()
{
  return m_upperThreshold;
}

float TempValveManager::getLowerThreshold()
{
  return m_lowerThreshold;
}