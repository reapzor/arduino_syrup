//Chux
#include "SyrupDisplayManager.h"

SyrupDisplayManager::SyrupDisplayManager(LCDController *lcd,
  TempProbe *tempProbe, ValveController *valve, Stats *stats,
  OverrideManager *overrideManager, ToggleButton *toggleButton) :
    m_pLCD(lcd), m_pTempProbe(tempProbe), m_pValve(valve), m_pStats(stats),
    m_pOverrideManager(overrideManager), m_pToggleButton(toggleButton),
    Observer<TempProbe>(), Observer<ValveController>(), Observer<Stats>(),
    Observer<ToggleButton>(), Observer<OverrideManager>()
{
  m_transitioning = false;
  m_nextTransition = UNDEF;
  m_currentTransitionDelay = 0;
  m_currentDrawDelay = 0;
  m_shouldDraw = false;
  m_displayState = UNDEF;
  m_prepForTransition = false;
}

SyrupDisplayManager::~SyrupDisplayManager()
{

}

char* SyrupDisplayManager::s_welcomeLineOne = "THE SYRUP-O-MATIC  V1.0";
char* SyrupDisplayManager::s_welcomeLineTwo = "CREATED BY CHUCK BENSON";

char* SyrupDisplayManager::s_mainTemp = "Temp";
char* SyrupDisplayManager::s_mainValve = "Valve";
char* SyrupDisplayManager::s_mainTime = "Time";

char* SyrupDisplayManager::s_calculating = "CALC";
char* SyrupDisplayManager::s_period = ".";

char* SyrupDisplayManager::s_tempDegreeF = "F";
char* SyrupDisplayManager::s_tempDegreeC = "C";
char* SyrupDisplayManager::s_valveStateOpen = "OPEN";
char* SyrupDisplayManager::s_valveStateClosed = "CLOSED";

char* SyrupDisplayManager::s_seenMax = "MAX";
char* SyrupDisplayManager::s_seenMin = "MIN";
char* SyrupDisplayManager::s_seenObserved = "OBSERVED";

char* SyrupDisplayManager::s_last = "LAST";
char* SyrupDisplayManager::s_duration = "DUR";

char* SyrupDisplayManager::s_daysChar = "D";
char* SyrupDisplayManager::s_hoursChar = "H";
char* SyrupDisplayManager::s_secondsChar = "s";
char* SyrupDisplayManager::s_zeroChar = "0";

char* SyrupDisplayManager::s_space = " ";

char* SyrupDisplayManager::s_average = "AVG";

char* SyrupDisplayManager::s_count = "COUNT";

char* SyrupDisplayManager::s_valveCaps = "VALVE";
char* SyrupDisplayManager::s_overrideValveOverride = "OVERRIDE";

char* SyrupDisplayManager::s_savedLineOne = "SETTING";
char* SyrupDisplayManager::s_savedLineTwo = "SAVED";

char* SyrupDisplayManager::s_systemUptime = "UPTIME";
char* SyrupDisplayManager::s_systemFreeMem = "FREE MEM";

char* SyrupDisplayManager::s_colon = ":";

void SyrupDisplayManager::transitionToNextState()
{
  if (m_displayState <= LCD_INFO) {
    if (m_displayState == LCD_INFO) {
      setState((e_displayState) 0);
      return;
    }
    setState((e_displayState) (m_displayState+1));
  }
}

void SyrupDisplayManager::setState(e_displayState state)
{
  if (m_transitioning) {
    #ifdef DEBUG
      Serial.println(F("TRANSITIONING!"));
    #endif
    m_nextTransition = state;
    return;
  }
  if (m_displayState == state) {
    #ifdef DEBUG
      Serial.print(F("ALREADY SET THIS DISPLAY STATE: "));
      Serial.println(state);
    #endif
    return;
  }
  if (state == WELCOME) {
    m_currentTransitionDelay = (long)millis() + WELCOME_TRANSITION_DELAY;
    m_nextTransition = WELCOME_TRANSITION_STATE;
    m_transitioning = true;
  }
  if (state == SAVED) {
    m_currentTransitionDelay = (long)millis() + SETTING_SAVED_TRANSITION_DELAY;
    //Note: You must called setState(SAVED); and another state immediately after for it
    //  to transition to: setState(SAVED); setState(THRES);
    //Otherwise it will sit there forever and not auto transition.
    //Which is a valid state, I guess... If you want...
    m_nextTransition = SAVED;
    m_transitioning = true;
  }
  #ifdef DEBUG
    Serial.print(F("SETTING LCD STATE TO: "));
    Serial.println(state);
  #endif
  m_displayState = state;
  draw();
}

void SyrupDisplayManager::update(TempProbe *tempProbe)
{
  switch (tempProbe->m_updatedParam)
  {
    case TempProbe::TEMP:
      switch (m_displayState)
      {
        case MAIN:
          char tempStr[8];
          *tempStr = LCDController::NULL_TERMINATOR;
          appendTempString(tempStr);
          int offset = strlen(s_mainTemp)+strlen(s_colon)+strlen(s_space);
          m_pLCD->edit(0, offset, tempStr);
          break;
      }
      break;
  }
}

void SyrupDisplayManager::update(ValveController *valve)
{
  char valveStr[7];
  *valveStr = LCDController::NULL_TERMINATOR;
  appendValveStateString(valveStr);
  int offset;
  int row;
  switch (m_displayState)
  {
    case MAIN:
      offset = strlen(s_mainValve)+strlen(s_colon)+strlen(s_space);
      row = 1;
      break;
    case VALVE_OVERRIDE:
      offset = 6+strlen(s_mainValve)+strlen(s_colon)+strlen(s_space);
      row = 1;
      break;
    default:
      return;
  }
  m_pLCD->edit(1, offset, valveStr);
}

void SyrupDisplayManager::update(Stats *stats)
{
  switch (stats->m_updatedStatsValue)
  {
    case Stats::CURRENT_DURATION:
      switch (m_displayState)
      {
        case MAIN:
            char row2Time[8];
            *row2Time = LCDController::NULL_TERMINATOR;
            appendDurationStringRightOriented(row2Time, stats->m_currentDuration);
            m_pLCD->edit(1, 24-7, row2Time);
            break;
      }
      break;
  }
}

void SyrupDisplayManager::update(ToggleButton *toggleButton)
{
  if (toggleButton->m_buttonState == ToggleButton::ON) {
    m_prepForTransition = true;
  }
  else {
    if (m_prepForTransition) {
      transitionToNextState();
      m_prepForTransition = false;
    }
  }
}

void SyrupDisplayManager::update(OverrideManager *overrideManager)
{
  if (overrideManager->isValveOverrideEnabled()) {
    m_pToggleButton->detach(this);
    setState(VALVE_OVERRIDE);
  }
  else {
    m_pToggleButton->attach(this);
    setState(MAIN);
  }
}

void SyrupDisplayManager::prime()
{
  m_currentDrawDelay = (long)millis();
}

void SyrupDisplayManager::tick()
{
  if (m_transitioning && (long)millis()-m_currentTransitionDelay >= 0) {
    m_currentTransitionDelay = 0;
    m_transitioning = false;
    m_displayState = m_nextTransition;
    m_nextTransition = UNDEF;
    m_shouldDraw = true;
  }
  if (m_shouldDraw && (long)millis()-m_currentDrawDelay >= 0) {
    m_currentDrawDelay += DELAY_BETWEEN_POSSIBLE_SHOULD_DRAWS;
    m_shouldDraw = false;
    draw();
  }
}

void SyrupDisplayManager::draw()
{
  switch (m_displayState)
  {
    case WELCOME:
      drawWelcome();
      break;
    case MAIN:
      drawMain();
      break;
    case VALVE_OVERRIDE:
      drawValveOverride();
      break;
    default:
      m_pLCD->clear();
      break;
  }
}

void SyrupDisplayManager::drawValveOverride()
{
  int row1Length = strlen(s_valveCaps)+strlen(s_space)+strlen(s_overrideValveOverride);
  char row1[row1Length+1];
  strcpy(row1, s_valveCaps);
  strcat(row1, s_space);
  strcat(row1, s_overrideValveOverride);
  m_pLCD->clear(0, 0, 5);
  m_pLCD->clear(0, 18, 6);
  m_pLCD->edit(0, 5, row1);
  
  int row2Length = strlen(s_mainValve)+strlen(s_colon)+strlen(s_space);
  char row2[row2Length+7];
  strcpy(row2, s_mainValve);
  strcat(row2, s_colon);
  strcat(row2, s_space);
  appendValveStateString(row2);
  m_pLCD->clear(1, 0, 6);
  m_pLCD->clear(1, 19, 5);
  m_pLCD->edit(1, 6, row2);
}

void SyrupDisplayManager::drawWelcome()
{
  m_pLCD->write(0, s_welcomeLineOne);
  m_pLCD->write(1, s_welcomeLineTwo);
}

void SyrupDisplayManager::drawMain()
{
  char row1[18];
  strcpy(row1, s_mainTemp);
  strcat(row1, s_colon);
  strcat(row1, s_space);
  appendTempString(row1);
  m_pLCD->write(0, row1);
  m_pLCD->edit(0, 19, s_mainTime);
  
  char row2[14];
  strcpy(row2, s_mainValve);
  strcat(row2, s_colon);
  strcat(row2, s_space);
  appendValveStateString(row2);
  m_pLCD->write(1, row2);
  char row2Time[9];
  *row2Time = LCDController::NULL_TERMINATOR;
  appendDurationStringRightOriented(row2Time, m_pStats->m_currentDuration);
  m_pLCD->edit(1, 24-strlen(row2Time), row2Time);
}

void SyrupDisplayManager::appendTempString(char *string)
{
  char tempStr[8];
  *tempStr = LCDController::NULL_TERMINATOR;
  if (m_pTempProbe->m_tempProbeReading == 0) {
    strcat(tempStr, s_calculating);
    strcat(tempStr, s_period);
    strcat(tempStr, s_period);
    strcat(tempStr, s_period);
  }
  else {
    char temp[7];
    switch(m_pTempProbe->getScale())
    {
      case TempProbe::FAHRENHEIT:
        dtostrf(m_pTempProbe->m_tempF, 4, 2, temp);
        strcat(tempStr, temp);    
        strcat(tempStr, s_tempDegreeF);
        break;
      case TempProbe::CELCIUS:
        dtostrf(m_pTempProbe->m_tempC, 4, 2, temp);
        strcat(tempStr, temp);      
        strcat(tempStr, s_tempDegreeC);
        break;
    }
  }
  strcat(string, tempStr);
  for (int x = strlen(tempStr); x < 7; x++) {
    strcat(string, s_space);
  }
}

void SyrupDisplayManager::appendValveStateString(char* string)
{
  switch (m_pValve->m_valveState)
  {
    case ValveController::OPEN:
      strcat(string, s_valveStateOpen);
      strcat(string, s_space);
      strcat(string, s_space);
      break;
    case ValveController::CLOSED:
    case ValveController::UNDEF:
      strcat(string, s_valveStateClosed);
      break;
  }
}

void SyrupDisplayManager::appendDurationStringLeftOriented(char* string, unsigned long time)
{
  appendDurationString(string, time, false);
}

void SyrupDisplayManager::appendDurationStringRightOriented(char* string, unsigned long time)
{
  appendDurationString(string, time, true);
}

void SyrupDisplayManager::appendDurationString(char* string, unsigned long time,
  bool rightOriented)
{
  char durationStr[8];
  *durationStr = LCDController::NULL_TERMINATOR;
  int minutes = 0;
  int hours = 0;
  int days = 0;
  int seconds = 0;
  bool ignoreSeconds = false;
  bool ignoreMinutes = false;
  while (time >= 86400) {
    time -= 86400;
    days++;
  }
  while (time >= 3600) {
    time -= 3600;
    hours++;
  }
  while (time >= 60) {
    time -= 60;
    minutes++;
  }
  seconds = time;
  if (days >= 100) {
    char dayStr[7];
    itoa(days, dayStr, 10);
    strcat(durationStr, dayStr);
    strcat(durationStr, s_daysChar);
  }
  else {
    if (days > 0) {
      char dayStr[3];
      itoa(days, dayStr, 10);
      strcat(durationStr, dayStr);
      strcat(durationStr, s_daysChar);
      if (hours > 0) {
        strcat(durationStr, s_space);
      }
      ignoreMinutes = true;
      ignoreSeconds = true;
    }
    if (hours > 0) {
      char hourStr[3];
      itoa(hours, hourStr, 10);
      strcat(durationStr, hourStr);
      if (ignoreMinutes) {
        strcat(durationStr, s_hoursChar);
      }
      else {
        if (seconds % 2 > 0) {
          strcat(durationStr, s_colon);
        }
        else {
          strcat(durationStr, s_space);
        }
      }
      ignoreSeconds = true;
    }
    if (!ignoreMinutes && (minutes > 0 || hours > 0)) {
      if (minutes < 10) {
        strcat(durationStr, s_zeroChar);
      }
      char minuteStr[3];
      itoa(minutes, minuteStr, 10);
      strcat(durationStr, minuteStr);
      if (!ignoreSeconds) {
        strcat(durationStr, s_colon);
      }
    }
    if (!ignoreSeconds) {
      if (seconds < 10) {
        strcat(durationStr, s_zeroChar);
      }
      char secondStr[3];
      itoa(seconds, secondStr, 10);
      strcat(durationStr, secondStr);
      if (minutes == 0) {
        strcat(durationStr, s_secondsChar);
      }
    }
  }
  if (rightOriented) {
    for (int x = 0; x < 7-strlen(durationStr); x++) {
      strcat(string, s_space);
    }
    strcat(string, durationStr);
  }
  else {
    strcat(string, durationStr);
    for (int x = strlen(durationStr); x < 7; x++) {
      strcat(string, s_space);
    }
  }
}


void SyrupDisplayManager::registerObservers()
{
  m_pTempProbe->attach(this);
  m_pValve->attach(this);
  m_pStats->attach(this);
  m_pOverrideManager->attach(this);
  m_pToggleButton->attach(this);
}

void SyrupDisplayManager::unregisterObservers()
{
  m_pTempProbe->detach(this);
  m_pValve->detach(this);
  m_pStats->detach(this);
  m_pOverrideManager->detach(this);
  m_pToggleButton->detach(this);
}

/*

long eventTimeout=(long)millis()+1000;
if((long)millis()-eventTimeout>=0) {
eventTimeout=(long)millis()+1000;
}
*/


