//Chux
//http://atlas-scientific.com/_files/_datasheets/_sensor/ENV-TEMP_1.4.pdf
#ifndef TempProbe_h
#define TempProbe_h

#include "Arduino.h"
#include "Subject.h"

//#define DEBUG_HW
//#define DEBUG
#define DEBUG_SWEEP_TEMP

class TempProbe : public Subject<TempProbe>
{
  public:
    enum e_scale
    {
      FAHRENHEIT,
      CELCIUS
    };
    float m_tempF;
    float m_tempC;
    int m_tempProbeReading;
    int m_tempProbePin;
    int m_digitalMappedTempProbePin;
    TempProbe(int tempProbePin);
    void read();
    int tempFInt();
    int tempCInt();
    void prime();
    void tick();
    float convertReadingToF(int tempProbeReading);
    float convertReadingToF(float tempC);
    float convertReadingToC(int tempProbeReading);
    
  private:
    int m_readCount;
    unsigned long m_candidateTempProbeReading;
    unsigned long m_delayTime;
    #ifdef DEBUG_SWEEP_TEMP
      int db_sweepMax;
      int db_sweepMin;
      int db_currentTemp;
      bool db_sweepDirection;
      static const int READS = 50;
    #else
      static const int READS = 200;
    #endif
    static const int READ_DELAY = 20;
    
};

#endif