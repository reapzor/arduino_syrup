//Chux
#include "TempProbe.h"
#include "AnalogHelper.h"

using namespace AnalogHelper;

TempProbe::TempProbe(int tempProbePin)
{
  m_tempProbePin = tempProbePin;
  m_digitalMappedTempProbePin = getDigitalAnalogPin(tempProbePin);
  digitalWrite(m_digitalMappedTempProbePin, LOW);
  m_readCount = 0;
  m_candidateTempProbeReading = 0;
  m_tempProbeReading = 0;
  m_tempF = 0.0;
  m_tempC = 0.0;
  m_delayTime = 0;
  #ifdef DEBUG_SWEEP_TEMP
    db_sweepDirection = false;
    db_sweepMax = 160;
    db_sweepMin = 130;
    db_currentTemp = 140;
  #endif
}

void TempProbe::read()
{
  int sensorReading = analogRead(m_tempProbePin);
  #ifdef DEBUG_HW
    Serial.print(F("TempSensor: "));
    Serial.println(sensorReading);
  #endif
  if (m_readCount < READS-1) {
    m_readCount++;
    m_candidateTempProbeReading += sensorReading;
  }
  else {
    #ifdef DEBUG_SWEEP_TEMP
      if (db_sweepDirection) {
        db_currentTemp--;
        if (db_currentTemp <= db_sweepMin) {
          db_sweepDirection = false;
        }
      }
      else {
        db_currentTemp++;
        if (db_currentTemp >= db_sweepMax) {
          db_sweepDirection = true;
        }
      }
      int averageReading = db_currentTemp;
    #else
      int averageReading = (m_candidateTempProbeReading + sensorReading) / READS;
    #endif
    m_candidateTempProbeReading = 0;
    m_readCount = 0;
    if (averageReading != m_tempProbeReading) {
      m_tempProbeReading = averageReading;
      m_tempC = convertReadingToC(averageReading);
      m_tempF = convertReadingToF(m_tempC);     
      #ifdef DEBUG
        Serial.print(F("Average Temp Changed: "));
        Serial.print(averageReading);
        Serial.print(F("ADC | "));
        Serial.print(m_tempF);
        Serial.print(F("F | "));
        Serial.print(m_tempC);
        Serial.println(F("C."));
      #endif
      notify();
    }
  }
}

float TempProbe::convertReadingToC(int tempProbeReading)
{
  float sensorReadingMiliVolts = (float)tempProbeReading;
  sensorReadingMiliVolts *= .0048;
  sensorReadingMiliVolts *= 1000;
  return (0.0512 * sensorReadingMiliVolts) - 20.5128;
}

float TempProbe::convertReadingToF(float tempC)
{
  return ((tempC * 9) / 5) + 32;     
}

float TempProbe::convertReadingToF(int tempProbeReading)
{
  return convertReadingToF(convertReadingToC(tempProbeReading));
}

int TempProbe::tempFInt()
{
  return (int) m_tempF;
}

int TempProbe::tempCInt()
{
  return (int) m_tempC;
}

void TempProbe::prime()
{
  m_delayTime = (long)millis();
}

void TempProbe::tick()
{
  if ((long)millis()-m_delayTime >= 0) {
    m_delayTime += READ_DELAY;
    read();
  }
}