//Chux
#include "SyrupDisplayManager.h"

SyrupDisplayManager::SyrupDisplayManager(LCDController *lcd,
  TempProbe *tempProbe, ValveController *valve, Stats *stats,
  OverrideManager *overrideManager, ToggleButton *toggleButton,
  SyrupSettingsManager *settingsManager, Encoder *encoder,
  TempValveManager *tempValveManager) :
    m_pLCD(lcd), m_pTempProbe(tempProbe), m_pValve(valve), m_pStats(stats),
    m_pOverrideManager(overrideManager), m_pToggleButton(toggleButton),
    m_pSettingsManager(settingsManager), m_pEncoder(encoder),
    m_pTempValveManager(tempValveManager),
    Observer<TempProbe>(), Observer<ValveController>(), Observer<Stats>(),
    Observer<ToggleButton>(), Observer<OverrideManager>(), Observer<THRESEditor>(),
    Observer<TempValveManager>()
{
  m_transitioning = false;
  m_nextTransition = UNDEF;
  m_currentTransitionDelay = 0;
  m_currentDrawDelay = 0;
  m_toggleButtonHoldDelay = 0;
  m_toggleButtonCountDelay = 0;
  m_shouldDraw = false;
  m_displayState = UNDEF;
  m_prepForTransition = false;
  m_pTHRESEditor = NULL;
  m_editModeBlinkTime = 0;
  m_editModeBlinkOn = false;
  m_editModeBlinkRow = 0;
  m_editModeBlinkOffset = 0;
}

SyrupDisplayManager::~SyrupDisplayManager()
{
  //cancelThresEditMode();
  //We will never die, and we want to save 100 bytes of progmem...
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
char* SyrupDisplayManager::s_scale = "Scale";

char* SyrupDisplayManager::s_last = "LAST";
char* SyrupDisplayManager::s_duration = "DUR";

char* SyrupDisplayManager::s_daysChar = "D";
char* SyrupDisplayManager::s_hoursChar = "H";
char* SyrupDisplayManager::s_minutesChar = "M";
char* SyrupDisplayManager::s_secondsChar = "s";
char* SyrupDisplayManager::s_zeroChar = "0";

char* SyrupDisplayManager::s_space = " ";

char* SyrupDisplayManager::s_average = "AVG";

char* SyrupDisplayManager::s_count = "COUNT";

char* SyrupDisplayManager::s_valveCaps = "VALVE";
char* SyrupDisplayManager::s_overrideValveOverride = "OVERRIDE";

char* SyrupDisplayManager::s_savedLineOne = "SETTINGS";
char* SyrupDisplayManager::s_savedLineTwo = "SAVED";
char* SyrupDisplayManager::s_canceledLineTwo = "CANCELED";

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
      Serial.print(F("TRANS: "));
      Serial.println(state);
    #endif
    m_nextTransition = state;
    return;
  }
  if (m_displayState == state) {
    #ifdef DEBUG
      Serial.print(F("DUPE: "));
      Serial.println(state);
    #endif
    return;
  }
  if (state == WELCOME) {
    m_currentTransitionDelay = (long)millis() + WELCOME_TRANSITION_DELAY;
    m_nextTransition = WELCOME_TRANSITION_STATE;
    m_transitioning = true;
  }
  if (state == SAVED || state == CANCELED) {
    m_currentTransitionDelay = (long)millis() + SETTING_SAVED_TRANSITION_DELAY;
    //Note: You must called setState(SAVED); and another state immediately after for it
    //  to transition to: setState(SAVED); setState(THRES);
    //Otherwise it will sit there forever and not auto transition.
    //Which is a valid state, I guess... If you want...
    m_nextTransition = state;
    m_transitioning = true;
  }
  cancelThresEditMode();
  #ifdef DEBUG
    Serial.print(F("LCD STATE: "));
    Serial.println(state);
  #endif
  m_displayState = state;
  draw();
}



/*
###############################
###  OBSERVER NOTIFCATIONS  ###
###############################
*/
//THRESEDITOR
void SyrupDisplayManager::update(THRESEditor *thresEditor)
{
  char value[7];
  *value = LCDController::NULL_TERMINATOR;
  switch(thresEditor->m_editItem)
  {
    case THRESEditor::UPPER_THRES:
      appendTempString(value, thresEditor->m_upperThreshold);
      m_pLCD->edit(0, 6, value);
      break;
    case THRESEditor::LOWER_THRES:
      appendTempString(value, thresEditor->m_lowerThreshold);
      m_pLCD->edit(1, 8, value);
      break;
    case THRESEditor::TEMP_SCALE:
      appendTempScaleSymbol(value, thresEditor->m_tempScale);
      m_pLCD->edit(1, 23, value);
      break;
  }
}

//TEMP PROBE
void SyrupDisplayManager::update(TempProbe *tempProbe)
{
  switch (m_displayState)
  {
    case MAIN:
      char tempStr[8];
      *tempStr = LCDController::NULL_TERMINATOR;
      appendTempStringMain(tempStr);
      m_pLCD->edit(0, 6, tempStr);
      break;
  }
}

//VALVE CONTROLLER
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
      offset = 7;
      row = 1;
      break;
    case VALVE_OVERRIDE:
      offset = 13;
      row = 1;
      break;
    default:
      return;
  }
  m_pLCD->edit(row, offset, valveStr);
}

//STATISTICS
void SyrupDisplayManager::update(Stats *stats)
{
  switch (stats->m_updatedStatsValue)
  {
    case Stats::CURRENT_DURATION:
      if (m_displayState == MAIN) {
        char row2Time[8];
        *row2Time = LCDController::NULL_TERMINATOR;
        appendDurationString(row2Time, stats->m_currentDuration, true, false);
        m_pLCD->edit(1, 17, row2Time);
        break;
      }
    case Stats::LAST_DURATION_CLOSED:
      if (m_displayState == LAST_DUR){
        char durationString[7];
        *durationString = LCDController::NULL_TERMINATOR;
        appendDurationString(durationString, m_pStats->m_lastDurationClosed, false, true);
        m_pLCD->edit(1, 17, durationString);
      }
      break;
    case Stats::LAST_DURATION_OPEN:
      if (m_displayState == LAST_DUR){
        char durationString[7];
        *durationString = LCDController::NULL_TERMINATOR;
        appendDurationString(durationString, m_pStats->m_lastDurationOpen, false, true);
        m_pLCD->edit(0, 15, durationString);
      }
      break;
    case Stats::RESET:
      switch (m_displayState)
      {
        case LAST_DUR:
        case AVERAGE_DUR:
        case COUNT:
          shouldDraw();
          break;
      }
      break;
  }
}

//TOGGLE BUTTON
void SyrupDisplayManager::update(ToggleButton *toggleButton)
{
  if (toggleButton->m_buttonState == ToggleButton::ON) {
    m_prepForTransition = true;
    m_toggleButtonHoldDelay = (long)millis()+TOGGLE_BUTTON_ON_DURATION;
    //This probably would be better somewhere else, but progmem is happy having it here
    //Same goes for the other 'THRES' observer(which is coupled with button and tick)
    if (m_displayState == THRES) {
      if ((long)millis()-m_toggleButtonCountDelay >= 0 && m_toggleButtonPressCount >= 1) {
        m_toggleButtonPressCount = 0;
      }
      if (m_toggleButtonPressCount == 0) {
        m_toggleButtonCountDelay = (long)millis()+TOGGLE_BUTTON_COUNT_DURATION;
      }
      m_toggleButtonPressCount++;
      if (m_toggleButtonPressCount >= TOGGLE_BUTTON_COUNT) {
        m_toggleButtonPressCount = 0;
        cancelThresEditMode();
        setState(CANCELED);
        setState(THRES);
      }
    }
  }
  else {
    if (m_prepForTransition && m_pTHRESEditor == NULL) {
      transitionToNextState();
    }
    m_prepForTransition = false;
    m_toggleButtonHoldDelay = 0;
  }
}

//TEMP VALVE MANAGER
void SyrupDisplayManager::update(TempValveManager *tempValveManager)
{
  //Serial.println(tempValveManager->m_thresholdRegion);
}

//OVERRIDE MANAGER
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
  registerObservers();
  setState(WELCOME);
}

void SyrupDisplayManager::tick()
{
  if (m_transitioning && (long)millis()-m_currentTransitionDelay >= 0) {
    m_currentTransitionDelay = 0;
    m_transitioning = false;
    e_displayState displayState = m_nextTransition;
    m_nextTransition = UNDEF;
    setState(displayState);
  }
  if (m_prepForTransition) {
    if ((long)millis()-m_toggleButtonHoldDelay >= 0) {
      m_prepForTransition = false;
      switch (m_displayState)
      {
        case THRES:
          if (m_pTHRESEditor && m_pTHRESEditor->isInEditMode()) {
            m_pTHRESEditor->save();
            cancelThresEditMode();
            setState(SAVED);
            setState(THRES);
          }
          else {
            m_pTHRESEditor = new THRESEditor(m_pSettingsManager, m_pEncoder, m_pToggleButton);
            m_pTHRESEditor->attach(this);
            m_pTHRESEditor->enterEditMode();
            m_editModeBlinkTime = (long)millis();
          }        
          break;
        case LAST_DUR:
        case AVERAGE_DUR:
        case COUNT:
          m_pStats->reset();
          break;
      }
    }
  }
  if (m_pTHRESEditor != NULL && m_displayState == THRES &&
      (long)millis()-m_editModeBlinkTime >= 0) {
    switch(m_pTHRESEditor->m_editItem)
    {
      case THRESEditor::UPPER_THRES:
        editModeBlinkDraw(0, 4);
        break;
      case THRESEditor::LOWER_THRES:
        editModeBlinkDraw(1, 6);
        break;
      case THRESEditor::TEMP_SCALE:
        editModeBlinkDraw(1, 21);
        break;
    }
    m_editModeBlinkTime += EDIT_MODE_BLINK_TIME;
  }
  if (m_shouldDraw && (long)millis()-m_currentDrawDelay >= 0) {
    m_shouldDraw = false;
    draw();
  }
}



/*
#################
###  DRAWING  ###
#################
*/
void SyrupDisplayManager::editModeBlinkDraw(int row, int offset)
{
  if (m_editModeBlinkOn) {
    if (m_editModeBlinkRow != row || m_editModeBlinkOffset != offset) {
      m_pLCD->edit(m_editModeBlinkRow, m_editModeBlinkOffset, s_colon);
    }
    m_editModeBlinkOn = false;
    m_pLCD->edit(row, offset, s_colon);
  }
  else {
    m_editModeBlinkOn = true;
    m_pLCD->edit(row, offset, s_space);
  }
  m_editModeBlinkRow = row;
  m_editModeBlinkOffset = offset;
}

void SyrupDisplayManager::shouldDraw()
{
  m_currentDrawDelay = (long)millis() + DELAY_BETWEEN_POSSIBLE_SHOULD_DRAWS;
  m_shouldDraw = true;
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
    case THRES:
      drawThres();
      break;
    case SAVED:
      drawSettingSaved();
      break;
    case CANCELED:
      drawSettingCanceled();
      break;
    case LAST_DUR:
      drawDuration();
      break;
    default:
      m_pLCD->clear();
      m_pLCD->write(1, s_welcomeLineOne);      
      break;
  }
}

void SyrupDisplayManager::drawDuration()
{
  char durationString[17];
  strcpy(durationString, s_last);
  strcat(durationString, s_space);
  strcat(durationString, s_duration);
  strcat(durationString, s_space);
  strcat(durationString, s_valveStateOpen);
  strcat(durationString, s_colon);
  m_pLCD->write(0, durationString);
  
  strcpy(durationString, s_last);
  strcat(durationString, s_space);
  strcat(durationString, s_duration);
  strcat(durationString, s_space);
  strcat(durationString, s_valveStateClosed);
  strcat(durationString, s_colon);
  m_pLCD->write(1, durationString);
  
  *durationString = LCDController::NULL_TERMINATOR;
  appendDurationString(durationString, m_pStats->m_lastDurationOpen, false, true);
  m_pLCD->edit(0, 15, durationString);
  
  *durationString = LCDController::NULL_TERMINATOR;
  appendDurationString(durationString, m_pStats->m_lastDurationClosed, false, true);
  m_pLCD->edit(1, 17, durationString);
  
}

void SyrupDisplayManager::drawSettingSaved()
{
  m_pLCD->clear();
  m_pLCD->edit(0, 8, s_savedLineOne);
  m_pLCD->edit(1, 9, s_savedLineTwo);
}

void SyrupDisplayManager::drawSettingCanceled()
{
  m_pLCD->clear();
  m_pLCD->edit(0, 7, s_savedLineOne);
  m_pLCD->edit(1, 8, s_canceledLineTwo);
}

void SyrupDisplayManager::drawThres()
{
  char row[15];
  strcpy(row, s_valveStateOpen);
  strcat(row, s_colon);
  strcat(row, s_space);
  appendTempString(row, m_pSettingsManager->m_settings.m_upperThreshold);
  m_pLCD->write(0, row);
  
  strcpy(row, s_valveStateClosed);
  strcat(row, s_colon);
  strcat(row, s_space);      
  appendTempString(row, m_pSettingsManager->m_settings.m_lowerThreshold);
  m_pLCD->write(1, row);
  
  int scaleStrOffset = 16;
  strcpy(row, s_scale);
  strcat(row, s_colon);
  strcat(row, s_space);
  appendTempScaleSymbol(row, m_pSettingsManager->m_settings.m_tempScale);
  m_pLCD->edit(1, scaleStrOffset, row);
  
  m_pLCD->edit(0, scaleStrOffset, s_savedLineOne);
}

void SyrupDisplayManager::drawValveOverride()
{
  m_pLCD->clear();

  char row[15];
  strcpy(row, s_valveCaps);
  strcat(row, s_space);
  strcat(row, s_overrideValveOverride);
  m_pLCD->edit(0, 5, row);
  
  strcpy(row, s_mainValve);
  strcat(row, s_colon);
  strcat(row, s_space);
  appendValveStateString(row);
  m_pLCD->edit(1, 6, row);
}

void SyrupDisplayManager::drawWelcome()
{
  m_pLCD->write(0, s_welcomeLineOne);
  m_pLCD->write(1, s_welcomeLineTwo);
}

void SyrupDisplayManager::drawMain()
{
  char row[18];
  //sprintf(row1, "%s%s%s", s_mainTemp, s_colon, s_space);
  strcpy(row, s_mainTemp);
  strcat(row, s_colon);
  strcat(row, s_space);
  appendTempStringMain(row);
  m_pLCD->write(0, row);
  m_pLCD->edit(0, 19, s_mainTime);
  
  strcpy(row, s_mainValve);
  strcat(row, s_colon);
  strcat(row, s_space);
  appendValveStateString(row);
  m_pLCD->write(1, row);
  
  *row = LCDController::NULL_TERMINATOR;
  appendDurationString(row, m_pStats->m_currentDuration, true, false);
  m_pLCD->edit(1, 17, row);
}

void SyrupDisplayManager::appendTempString(char *string, float temp)
{
  int charLength = 6;
  char tempStr[7];
  dtostrf(temp, 4, 2, tempStr);
  strcat(string, tempStr);
  appendSpaces(string, charLength, tempStr);
}



/*
#################
### APPENDING ###
#################
*/
void SyrupDisplayManager::appendTempStringMain(char *string)
{
  int charLength = 7;
  char tempStr[8];
  if (m_pTempProbe->m_tempProbeReading != 0) {
    switch(m_pSettingsManager->m_settings.m_tempScale)
    {
      case TempProbe::FAHRENHEIT:
        dtostrf(m_pTempProbe->m_tempF, 4, 2, tempStr);
        strcat(tempStr, s_tempDegreeF);
        break;
      case TempProbe::CELCIUS:
        dtostrf(m_pTempProbe->m_tempC, 4, 2, tempStr);
        strcat(tempStr, s_tempDegreeC);
        break;
    }
  }
  else {
    strcpy(tempStr, s_calculating);
    strcat(tempStr, s_period);
    strcat(tempStr, s_period);
    strcat(tempStr, s_period);
  }
  strcat(string, tempStr);
  appendSpaces(string, charLength, tempStr);
}

void SyrupDisplayManager::appendTempScaleSymbol(char* string, TempProbe::e_scale scale)
{
  switch(scale)
  {
    case TempProbe::FAHRENHEIT:
      strcat(string, s_tempDegreeF);
      break;
    case TempProbe::CELCIUS:
      strcat(string, s_tempDegreeC);
      break;
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

void SyrupDisplayManager::appendDurationString(char* string, unsigned long time,
  bool rightOriented, bool useMinuteDisplay)
{
  int charLength = 7;
  long dayTime = 86400;
  int hourTime = 3600;
  int minuteTime = 60;
  int integerBase = 10;
  char durationStr[charLength+1];
  *durationStr = LCDController::NULL_TERMINATOR;
  int minutes = 0;
  int hours = 0;
  int days = 0;
  int seconds = 0;
  bool ignoreSeconds = false;
  bool ignoreMinutes = false;
  char timePart[5];
  while (time >= dayTime) {
    time -= dayTime;
    days++;
  }
  while (time >= hourTime) {
    time -= hourTime;
    hours++;
  }
  while (time >= minuteTime) {
    time -= minuteTime;
    minutes++;
  }
  seconds = time;
  if (days < 100) {
    if (days > 0) {
      itoa(days, timePart, integerBase);
      strcat(durationStr, timePart);
      strcat(durationStr, s_daysChar);
      if (hours > 0) {
        strcat(durationStr, s_space);
      }
      ignoreMinutes = true;
      ignoreSeconds = true;
    }
    if (hours > 0) {
      itoa(hours, timePart, integerBase);
      strcat(durationStr, timePart);
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
      itoa(minutes, timePart, integerBase);
      strcat(durationStr, timePart);
      if (!ignoreSeconds) {
        if (useMinuteDisplay) {
          strcat(durationStr, s_minutesChar);
          strcat(durationStr, s_space);
        }
        else {
          strcat(durationStr, s_colon);
        }
      }
    }
    if (!ignoreSeconds) {
      if (seconds < 10) {
        strcat(durationStr, s_zeroChar);
      }
      itoa(seconds, timePart, integerBase);
      strcat(durationStr, timePart);
      if (minutes == 0 || useMinuteDisplay) {
        strcat(durationStr, s_secondsChar);
      }
    }
  }
  else {
    itoa(days, timePart, integerBase);
    strcat(durationStr, timePart);
    strcat(durationStr, s_daysChar);
  }
  if (rightOriented) {
    appendSpaces(string, charLength, durationStr);
    strcat(string, durationStr);
  }
  else {
    strcat(string, durationStr);
    appendSpaces(string, charLength, durationStr);
  }
}

void SyrupDisplayManager::appendSpaces(char* string, int length, char* baseString)
{
  int spacesNeeded = length-strlen(baseString);
  for (int x = 0; x < spacesNeeded; x++) {
    strcat(string, s_space);
  }
}



/*
#################
###  HELPERS  ###
#################
*/
void SyrupDisplayManager::registerObservers()
{
  m_pTempProbe->attach(this);
  m_pValve->attach(this);
  m_pStats->attach(this);
  m_pOverrideManager->attach(this);
  m_pToggleButton->attach(this);
  m_pTempValveManager->attach(this);
}

void SyrupDisplayManager::unregisterObservers()
{
  m_pTempProbe->detach(this);
  m_pValve->detach(this);
  m_pStats->detach(this);
  m_pOverrideManager->detach(this);
  m_pToggleButton->detach(this);
  m_pTempValveManager->detach(this);
}

void SyrupDisplayManager::cancelThresEditMode()
{
  if (m_pTHRESEditor != NULL) {
    m_pTHRESEditor->leaveEditMode();
    m_pTHRESEditor->detach(this);
    delete(m_pTHRESEditor);
    m_pTHRESEditor = NULL;
  }
 // if (m_editModeBlinkOn) {
    //editModeBlinkDraw(m_editModeBlinkRow, m_editModeBlinkOffset);
  //}
}

