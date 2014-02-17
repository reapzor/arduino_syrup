//Chux
#ifndef Stats_h
#define Stats_h

#include "Arduino.h"
#include "Subject.h"
#include <TempProbe.h>
#include <ValveController.h>
#include "Observer.h"

#define DEV
//#define DEBUG_OBSERVERS
//#define DEBUG
    
class Stats : public Subject<Stats>, public Observer<ValveController>,
    public Observer<TempProbe>
{
  public:
    enum e_statsValues
    {
      IDLE,
      TEMP_MIN,
      TEMP_MAX,
      CURRENT_DURATION,
      LAST_DURATION_CLOSED,
      LAST_DURATION_OPEN,
      AVERAGE_DURATION_CLOSED,
      AVERAGE_DURATION_OPEN,
      COUNT_CLOSED,
      COUNT_OPEN,
      UPTIME,
      FREE_MEM
    };
    
    e_statsValues m_updatedStatsValue;
    float m_tempMin;
    float m_tempMax;
    unsigned long m_currentDuration;
    unsigned long m_lastDurationClosed;
    unsigned long m_lastDurationOpen;
    unsigned long m_averageDurationClosed;
    unsigned long m_averageDurationOpen;
    int m_countClosed;
    int m_countOpen;
    //Holds value in minutes.
    unsigned long m_uptime;
    int m_freeMem;
    
    void registerObservers();
    void unregisterObservers();
    void prime();
    void tick();
    
    void update(ValveController *valve);
    void update(TempProbe *tempProbe);
    
    void reset();
    
    Stats(ValveController *valveController, TempProbe *tempProbe);
    ~Stats();
    
  private:
    static const int ONE_SECOND = 1000;
    static const long ONE_MINUTE = 60000;
    ValveController *m_pValveController;
    TempProbe *m_pTempProbe;
    long m_nextMinute;
    long m_nextSecond;
    void resetNextSecond();
    void resetNextMinute();
    void setNextSecond();
    void setNextMinute();
    void updateTempMinMax(float temp);
    void updateFreeMem();
    void sendNotify(e_statsValues statsValue);
    void sendNotify(e_statsValues statsValue, bool shouldReturnToIdle);

};

#endif