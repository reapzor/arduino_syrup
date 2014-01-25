//Chux
#ifndef SyrupDisplayManager_h
#define SyrupDisplayManager_h

#include "Arduino.h"

#include "LCDController.h"
#include "Observer.h"
#include "TempProbe.h"
#include "ValveController.h"
#include "Stats.h"

//#define DEBUG_OBSERVERS
#define DEBUG

class SyrupDisplayManager : public Observer<TempProbe>, public Observer<ValveController>,
  public Observer<Stats>
{
  public:
    enum e_displayState
    {
      MAIN,
      THRES,
      LAST_DUR,
      AVERAGE_DUR,
      COUNT,
      SYS_INFO,
      LCD_INFO,
      VALVE_OVERRIDE,
      SAVED,
      THRES_EDIT,
      LCD_EDIT,
      WELCOME,
      OFF,
      UNDEF
    };
    static const int m_validStates = 7;
    
    static char* s_welcomeLineOne;
    static char* s_welcomeLineTwo;
    
    static char* s_mainTemp;
    static char* s_mainValve;
    static char* s_mainTime;
    
    static char* s_tempDegree;
    static char* s_valveStateOpen;
    static char* s_valveStateClosed;
    
    static char* s_thresTempOpen;
    static char* s_thresTempClose;
    
    static char* s_seenTempMax;
    static char* s_seenTempMin;
    
    static char* s_lastDurClose;
    static char* s_lastDurOpen;
    
    static char* s_secondsChar;
    static char* s_minutesChar;
    static char* s_hoursChar;
    
    static char* s_averageDurClose;
    static char* s_averageDurOpen;
    
    static char* s_countOpen;
    static char* s_countClose;
    
    static char* s_overrideValveLineOne;
    static char* s_overrideValveLineTwo;
    
    static char* s_savedLineOne;
    static char* s_savedLineTwo;
    
    static char* s_systemUptime;
    static char* s_systemFreeMem;
    
    void tick();
    void setState(e_displayState state);
    void draw();
    
    e_displayState m_displayState;
    
    void update(TempProbe *tempProbe);
    void update(ValveController *valve);
    void update(Stats *stats);
    
    SyrupDisplayManager(LCDController *lcd, TempProbe *tempProbe,
      ValveController *valve, Stats *stats);
    ~SyrupDisplayManager();
  
  private:
    bool m_shouldDraw;
    char convertTimeToString();
    char convertTempToString();
    LCDController *m_pLCD;
    TempProbe *m_pTempProbe;
    ValveController *m_pValve;
    Stats *m_pStats;
    static const int WELCOME_TRANSITION_DELAY = 5000;
    static const int SETTING_SAVED_TRANSITION_DELAY = 2000;
    static const int DELAY_BETWEEN_POSSIBLE_SHOULD_DRAWS = 300;
    long m_currentDrawDelay;
    static const e_displayState WELCOME_TRANSITION_STATE = MAIN;
    bool m_transitioning;
    e_displayState m_nextTransition;
    long m_currentTransitionDelay;
    void drawWelcome();
    void drawMain();
    
    

};
    
#endif