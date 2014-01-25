//Chux
//http://atlas-scientific.com/_files/_datasheets/_sensor/ENV-TEMP_1.4.pdf
#ifndef TempProbe_h
#define TempProbe_h

#include "Arduino.h"
#include "Subject.h"
//#define DEBUG_HW
#define DEBUG
#define DEBUG_SWEEP_TEMP

class TempProbe : public Subject<TempProbe>
{
  public:
    enum e_scale
    {
      FAHRENHEIT,
      CELCIUS
    };
    enum e_updatedParam
    {
      IDLE,
      SCALE,
      TEMP
    };
    double m_tempF;
    double m_tempC;
    int m_tempProbeReading;
    int m_tempProbePin;
    int m_digitalMappedTempProbePin;
    e_updatedParam m_updatedParam;
    TempProbe(int tempProbePin);
    void read();
    void setScale(e_scale scale);
    e_scale getScale();
    int tempFInt();
    int tempCInt();
    void pause();
    void unpause();
    void tick();
    double convertReadingToF(int tempProbeReading);
    double convertReadingToF(double tempC);
    double convertReadingToC(int tempProbeReading);
    
  private:
    e_scale m_activeScale;
    bool m_paused;
    int m_readCount;
    long m_candidateTempProbeReading;
    long m_delayTime;
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