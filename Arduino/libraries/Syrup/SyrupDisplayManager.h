//Chux
#ifndef SyrupDisplayManager_h
#define SyrupDisplayManager_h

#include "Arduino.h"

#include "LCDController.h"
#include "Observer.h"
#include "TempProbe.h"
#include "ValveController.h"
#include "Stats.h"
#include "ToggleButton.h"
#include "OverrideManager.h"
#include "THRESEditor.h"
#include "SyrupSettingsManager.h"
#include "Encoder.h"

//#define DEBUG_OBSERVERS
#define DEBUG

class SyrupDisplayManager : public Observer<TempProbe>, public Observer<ValveController>,
  public Observer<Stats>, public Observer<ToggleButton>, public Observer<OverrideManager>, 
  public Observer<THRESEditor>
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
      CANCELED,
      THRES_EDIT,
      LCD_EDIT,
      WELCOME,
      OFF,
      UNDEF
    };
    
    static char* s_welcomeLineOne;
    static char* s_welcomeLineTwo;
    
    static char* s_mainTemp;
    static char* s_mainValve;
    static char* s_mainTime;
    
    static char* s_calculating;
    static char* s_period; 
    
    static char* s_tempDegreeF;
    static char* s_tempDegreeC;
    static char* s_valveStateOpen;
    static char* s_valveStateClosed;

    static char* s_seenMax;
    static char* s_seenMin;
    static char* s_seenObserved;
    static char* s_scale;
    
    static char* s_last;
    static char* s_duration;
    
    static char* s_daysChar;
    static char* s_hoursChar;
    static char* s_zeroChar;
    static char* s_secondsChar;
    
    static char* s_space;
    
    static char* s_average;

    static char* s_count;
    
    static char* s_valveCaps;
    static char* s_overrideValveOverride;
    
    static char* s_savedLineOne;
    static char* s_savedLineTwo;
    static char* s_canceledLineTwo;
    
    static char* s_systemUptime;
    static char* s_systemFreeMem;
    
    static char* s_colon;
    
    void prime();
    void tick();
    void setState(e_displayState state);
    void transitionToNextState();
    void draw();
    
    void registerObservers();
    void unregisterObservers();
    
    e_displayState m_displayState;
    
    void update(TempProbe *tempProbe);
    void update(ValveController *valve);
    void update(Stats *stats);
    void update(ToggleButton *toggleButton);
    void update(OverrideManager *overrideManager);
    void update(THRESEditor *thresEditor);
    
    SyrupDisplayManager(LCDController *lcd, TempProbe *tempProbe,
      ValveController *valve, Stats *stats, OverrideManager *overrideManager,
      ToggleButton *toggleButton, SyrupSettingsManager *settingsManager,
      Encoder *encoder);
    ~SyrupDisplayManager();
  
  private:
    bool m_shouldDraw;
    bool m_prepForTransition;
    void appendDurationStringLeftOriented(char* string, unsigned long time);
    void appendDurationStringRightOriented(char* string, unsigned long time);
    void appendDurationString(char* string, unsigned long time, bool rightOriented);
    void appendTempString(char* string, float temp);
    void appendTempStringMain(char* string);
    void appendValveStateString(char* string);
    void appendTempScaleSymbol(char* string, TempProbe::e_scale scale);
    LCDController *m_pLCD;
    TempProbe *m_pTempProbe;
    ValveController *m_pValve;
    Stats *m_pStats;
    OverrideManager *m_pOverrideManager;
    ToggleButton *m_pToggleButton;
    THRESEditor *m_pTHRESEditor;
    Encoder *m_pEncoder;
    SyrupSettingsManager *m_pSettingsManager;
    static const int TOGGLE_BUTTON_COUNT_DURATION = 2000;
    static const int TOGGLE_BUTTON_COUNT = 3;
    static const int TOGGLE_BUTTON_ON_DURATION = 3000;
    static const int WELCOME_TRANSITION_DELAY = 2000;
    static const int SETTING_SAVED_TRANSITION_DELAY = 3000;
    static const int DELAY_BETWEEN_POSSIBLE_SHOULD_DRAWS = 300;
    static const int EDIT_MODE_BLINK_TIME = 500;
    static const e_displayState WELCOME_TRANSITION_STATE = MAIN;
    bool m_transitioning;
    e_displayState m_nextTransition;
    long m_currentTransitionDelay;
    void drawWelcome();
    void drawMain();
    void drawValveOverride();
    void drawThres();
    void drawSettingSaved();
    void drawSettingCanceled();
    void cancelThresEditMode();
    void editModeBlinkDraw(int row, int offset);
    long m_toggleButtonHoldDelay;
    long m_toggleButtonCountDelay;
    long m_editModeBlinkTime;
    bool m_editModeBlinkOn;
    int m_editModeBlinkRow;
    int m_editModeBlinkOffset;
    int m_toggleButtonPressCount;

};
    
#endif