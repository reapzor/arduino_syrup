//Chux
#ifndef Stats_h
#define Stats_h

#include "Arduino.h"
#include "Subject.h"
#include <TempValveManager.h>
#include "Observer.h"

//#define DEV
//#define DEBUG_OBSERVERS
//#define DEBUG
    
class Stats : public Subject<Stats>, public Observer<TempValveManager>,
    public Observer<TempProbe>
{
  public:
    enum e_statsValues
    {
      IDLE,
      RESET,
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
    int m_tempMin;
    int m_tempMax;
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
    
    void update(TempValveManager *tempValveManager);
    void update(TempProbe *tempProbe);
    
    void reset();
    
    Stats(TempValveManager *tempValveManager, TempProbe *tempProbe);
    
  private:
    static const int ONE_SECOND = 1000;
    static const long ONE_MINUTE = 60000;
    TempValveManager *m_pTempValveManager;
    TempProbe *m_pTempProbe;
    unsigned long m_nextMinute;
    unsigned long m_nextSecond;
    void resetNextSecond();
    void resetNextMinute();
    void setNextSecond();
    void setNextMinute();
    void updateFreeMem();
    void sendNotify(e_statsValues statsValue);
    
};

#endif