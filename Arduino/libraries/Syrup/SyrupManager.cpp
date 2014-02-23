//Chux
#include "SyrupManager.h"

SyrupManager::SyrupManager(LCDController *lcd,
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

char* SyrupManager::s_welcomeLineOne = "THE SYRUP-O-MATIC  V1.0";
char* SyrupManager::s_welcomeLineTwo = "CREATED BY CHUCK BENSON";

char* SyrupManager::s_mainTemp = "Temp";
char* SyrupManager::s_mainValve = "Valve";

char* SyrupManager::s_calculating = "CALC...";

char* SyrupManager::s_tempDegreeF = "F";
char* SyrupManager::s_tempDegreeC = "C";
char* SyrupManager::s_valveStateOpen = "OPEN";
char* SyrupManager::s_valveStateClosed = "CLOSED";

char* SyrupManager::s_seenMax = "MAX";
char* SyrupManager::s_seenMin = "MIN";
char* SyrupManager::s_seenObserved = "SEEN";
char* SyrupManager::s_scale = "Scale";

char* SyrupManager::s_last = "LAST";
char* SyrupManager::s_duration = "DUR";

char* SyrupManager::s_daysChar = "D";
char* SyrupManager::s_hoursChar = "H";
char* SyrupManager::s_minutesChar = "m";
char* SyrupManager::s_secondsChar = "s";
char* SyrupManager::s_zeroChar = "0";

char* SyrupManager::s_space = " ";

char* SyrupManager::s_average = "AVG";

char* SyrupManager::s_count = "COUNT";

char* SyrupManager::s_raising = "Raising";
char* SyrupManager::s_dropping = "Dropping";
char* SyrupManager::s_upper = "Upper";
char* SyrupManager::s_lower = "Lower";
char* SyrupManager::s_over = "OVER!";
char* SyrupManager::s_under = "UNDER!";

char* SyrupManager::s_tempCaps = "TEMP";
char* SyrupManager::s_valveCaps = "VALVE";
char* SyrupManager::s_overrideValveOverride = "OVERRIDE";

char* SyrupManager::s_savedLineOne = "SETTINGS";
char* SyrupManager::s_savedLineTwo = "SAVED";
char* SyrupManager::s_canceledLineTwo = "CANCELED";

char* SyrupManager::s_systemUptime = "UPTIME";
char* SyrupManager::s_systemFreeMem = "FREE MEM";

char* SyrupManager::s_colon = ":";

void SyrupManager::transitionToNextState()
{
  if (m_displayState <= SYS_INFO) {
    if (m_displayState == SYS_INFO) {
      setState((e_displayState) 0);
      return;
    }
    setState((e_displayState) (m_displayState+1));
  }
}

void SyrupManager::setState(e_displayState state)
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
void SyrupManager::update(THRESEditor *thresEditor)
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
void SyrupManager::update(TempProbe *tempProbe)
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
void SyrupManager::update(ValveController *valve)
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
void SyrupManager::update(Stats *stats)
{
  switch (stats->m_updatedStatsValue)
  {
    case Stats::CURRENT_DURATION:
      if (m_displayState == MAIN) {
        char row2Time[8];
        *row2Time = LCDController::NULL_TERMINATOR;
        appendDurationString(row2Time, stats->m_currentDuration, true, true);
        m_pLCD->edit(1, 17, row2Time);
      }
      break;
    case Stats::LAST_DURATION_CLOSED:
      if (m_displayState == LAST_DUR){
        char durationString[7];
        *durationString = LCDController::NULL_TERMINATOR;
        appendDurationString(durationString, stats->m_lastDurationClosed, false, false);
        m_pLCD->edit(1, 17, durationString);
      }
      break;
    case Stats::LAST_DURATION_OPEN:
      if (m_displayState == LAST_DUR){
        char durationString[7];
        *durationString = LCDController::NULL_TERMINATOR;
        appendDurationString(durationString, stats->m_lastDurationOpen, false, false);
        m_pLCD->edit(0, 15, durationString);
      }
      break;
    case Stats::AVERAGE_DURATION_OPEN:
      if (m_displayState == AVERAGE_DUR) {
        char durationString[7];
        *durationString = LCDController::NULL_TERMINATOR;
        appendDurationString(durationString, stats->m_averageDurationOpen, false, false);
        m_pLCD->edit(0, 14, durationString);
      }
      break;
    case Stats::AVERAGE_DURATION_CLOSED:
      if (m_displayState == AVERAGE_DUR) {
        char durationString[7];
        *durationString = LCDController::NULL_TERMINATOR;
        appendDurationString(durationString, stats->m_averageDurationClosed, false, false);
        m_pLCD->edit(1, 16, durationString);
      }
      break;
    case Stats::COUNT_OPEN:
      if (m_displayState == COUNT) {
        char countStr[6];
        itoa(stats->m_countOpen, countStr, 10);
        appendSpaces(countStr, 5, countStr);
        m_pLCD->edit(0, 12, countStr);
      }
      break;
    case Stats::COUNT_CLOSED:
      if (m_displayState == COUNT) {
        char countStr[6];
        itoa(stats->m_countClosed, countStr, 10);
        appendSpaces(countStr, 5, countStr);
        m_pLCD->edit(1, 14, countStr);
      }
      break;
    case Stats::UPTIME:
      if (m_displayState == SYS_INFO) {
        char uptimeStr[8];
        *uptimeStr = LCDController::NULL_TERMINATOR;
        appendDurationString(uptimeStr, stats->m_uptime, false, false, true);
        m_pLCD->edit(0, 8, uptimeStr);
      }
      break;
    case Stats::FREE_MEM:
      if (m_displayState == SYS_INFO) {
        char memStr[5];
        itoa(stats->m_freeMem, memStr, 10);
        appendSpaces(memStr, 5, memStr);
        m_pLCD->edit(1, 10, memStr);
      }
      break;
    case Stats::TEMP_MIN:
      if (m_displayState == TEMP_MAX_MIN) {
        char tempStr[8];
        *tempStr = LCDController::NULL_TERMINATOR;
        appendTempStringMaxMin(tempStr, stats->m_tempMin);
        m_pLCD->edit(1, 15, tempStr);
      }
      break;
    case Stats::TEMP_MAX:
      if (m_displayState == TEMP_MAX_MIN) {
        char tempStr[8];
        *tempStr = LCDController::NULL_TERMINATOR;
        appendTempStringMaxMin(tempStr, stats->m_tempMax);
        m_pLCD->edit(0, 15, tempStr);      
      }
      break;
  }
}

//TOGGLE BUTTON
void SyrupManager::update(ToggleButton *toggleButton)
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
    else {
      m_toggleButtonPressCount = 0;
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
void SyrupManager::update(TempValveManager *tempValveManager)
{
  if (m_displayState == MAIN) {
    char thresStr[9];
    *thresStr = LCDController::NULL_TERMINATOR;
    appendThresholdString(thresStr);
    m_pLCD->edit(0, 16, thresStr);
  }
}

//OVERRIDE MANAGER
void SyrupManager::update(OverrideManager *overrideManager)
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


void SyrupManager::prime()
{
  registerObservers();
  setState(WELCOME);
}

void SyrupManager::tick()
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
          if (m_pTHRESEditor) {
            if (m_pTHRESEditor->save()) {
              setState(SAVED);
              setState(THRES);
              cancelThresEditMode();
            }
          }
          else {
            m_pTHRESEditor = new THRESEditor(m_pSettingsManager, m_pEncoder, m_pToggleButton);
            m_pTHRESEditor->attach(this);
            m_pTHRESEditor->enterEditMode();
            m_editModeBlinkTime = (long)millis();
            m_toggleButtonPressCount = 0;
          }        
          break;
        case LAST_DUR:
        case AVERAGE_DUR:
        case COUNT:
        case TEMP_MAX_MIN:
          m_pStats->reset();
          shouldDraw();
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
void SyrupManager::shouldDraw()
{
  m_currentDrawDelay = (long)millis() + DELAY_BETWEEN_POSSIBLE_SHOULD_DRAWS;
  m_shouldDraw = true;
}

void SyrupManager::draw()
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
      drawDuration(false);
      break;
    case AVERAGE_DUR:
      drawDuration(true);
      break;
    case COUNT:
      drawCount();
      break;
    case TEMP_MAX_MIN:
      drawMaxMin();
      break;
    case SYS_INFO:
      drawSysInfo();
      break;
    #ifdef DEBUG
      default:
        m_pLCD->clear();
        m_pLCD->write(1, s_welcomeLineOne);    
        Serial.print("NYI STATE: ");
        Serial.println(m_displayState);
        break;
    #endif
  }
}

void SyrupManager::drawMaxMin()
{
  char tempStr[24];
  strcpy(tempStr, s_seenMax);
  strcat(tempStr, s_space);
  strcat(tempStr, s_tempCaps);
  strcat(tempStr, s_space);
  strcat(tempStr, s_seenObserved);
  strcat(tempStr, s_colon);
  strcat(tempStr, s_space);
  appendTempStringMaxMin(tempStr, m_pStats->m_tempMax);
  m_pLCD->write(0, tempStr);
  
  strcpy(tempStr, s_seenMin);
  strcat(tempStr, s_space);
  strcat(tempStr, s_tempCaps);
  strcat(tempStr, s_space);
  strcat(tempStr, s_seenObserved);
  strcat(tempStr, s_colon);
  strcat(tempStr, s_space);
  appendTempStringMaxMin(tempStr, m_pStats->m_tempMin);
  m_pLCD->write(1, tempStr);  
}

void SyrupManager::drawSysInfo()
{
  char infoStr[16];
  char infoIntStr[5];
  strcpy(infoStr, s_systemUptime);
  strcat(infoStr, s_colon);
  strcat(infoStr, s_space);
  appendDurationString(infoStr, m_pStats->m_uptime, false, false, true);
  m_pLCD->write(0, infoStr);
  
  strcpy(infoStr, s_systemFreeMem);
  strcat(infoStr, s_colon);
  strcat(infoStr, s_space);
  itoa(m_pStats->m_freeMem, infoIntStr, 10);
  strcat(infoStr, infoIntStr);
  m_pLCD->write(1, infoStr);
}

void SyrupManager::drawCount()
{
  char countStr[15];
  char countIntStr[6];
  strcpy(countStr, s_count);
  strcat(countStr, s_space);
  strcat(countStr, s_valveStateOpen);
  strcat(countStr, s_colon);
  strcat(countStr, s_space);
  itoa(m_pStats->m_countOpen, countIntStr, 10);
  strcat(countStr, countIntStr);
  m_pLCD->write(0, countStr);
  
  strcpy(countStr, s_count);
  strcat(countStr, s_space);
  strcat(countStr, s_valveStateClosed);
  strcat(countStr, s_colon);
  strcat(countStr, s_space);
  itoa(m_pStats->m_countClosed, countIntStr, 10);
  strcat(countStr, countIntStr);
  m_pLCD->write(1, countStr);  
}

void SyrupManager::drawDuration(bool useAverage)
{
  char durationString[25];  
  int offsetLine1;
  int offsetLine2;
  unsigned long durOpen;
  unsigned long durClosed;
  if (useAverage) {
    offsetLine1 = 14;
    offsetLine2 = 16;
    durOpen = m_pStats->m_averageDurationOpen;
    durClosed = m_pStats->m_averageDurationClosed;
  }
  else {
    offsetLine1 = 15;
    offsetLine2 = 17;
    durOpen = m_pStats->m_lastDurationOpen;
    durClosed = m_pStats->m_lastDurationClosed;  
  }
  
  if (useAverage) {
    strcpy(durationString, s_average);
  }
  else {
    strcpy(durationString, s_last);
  }
  strcat(durationString, s_space);
  strcat(durationString, s_duration);
  strcat(durationString, s_space);
  strcat(durationString, s_valveStateOpen);
  strcat(durationString, s_colon);
  strcat(durationString, s_space);
  if (durOpen != 0) {
    appendDurationString(durationString, durOpen, false, false);
  }
  else {
    strcat(durationString, s_calculating);
  }
  m_pLCD->write(0, durationString);
  
  if (useAverage) {
    strcpy(durationString, s_average);
  }
  else {
    strcpy(durationString, s_last);
  }
  strcat(durationString, s_space);
  strcat(durationString, s_duration);
  strcat(durationString, s_space);
  strcat(durationString, s_valveStateClosed);
  strcat(durationString, s_colon);
  strcat(durationString, s_space);
  if (durClosed != 0) {
    appendDurationString(durationString, durClosed, false, false);
  }
  else {
    strcat(durationString, s_calculating);
  }
  m_pLCD->write(1, durationString);
}

void SyrupManager::drawSettingSaved()
{
  m_pLCD->clear();
  m_pLCD->edit(0, 8, s_savedLineOne);
  m_pLCD->edit(1, 9, s_savedLineTwo);
}

void SyrupManager::drawSettingCanceled()
{
  m_pLCD->clear();
  m_pLCD->edit(0, 7, s_savedLineOne);
  m_pLCD->edit(1, 8, s_canceledLineTwo);
}

void SyrupManager::drawThres()
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

void SyrupManager::drawValveOverride()
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

void SyrupManager::drawWelcome()
{
  m_pLCD->write(0, s_welcomeLineOne);
  m_pLCD->write(1, s_welcomeLineTwo);
}

void SyrupManager::drawMain()
{
  char row[18];
  strcpy(row, s_mainTemp);
  strcat(row, s_colon);
  strcat(row, s_space);
  appendTempStringMain(row);
  m_pLCD->write(0, row);
  
  *row = LCDController::NULL_TERMINATOR;
  appendThresholdString(row);
  m_pLCD->edit(0, 16, row);
  
  strcpy(row, s_mainValve);
  strcat(row, s_colon);
  strcat(row, s_space);
  appendValveStateString(row);
  m_pLCD->write(1, row);
  
  *row = LCDController::NULL_TERMINATOR;
  appendDurationString(row, m_pStats->m_currentDuration, true, true);
  m_pLCD->edit(1, 17, row);
}

void SyrupManager::editModeBlinkDraw(int row, int offset)
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



/*
#################
### APPENDING ###
#################
*/
void SyrupManager::appendThresholdString(char *string)
{
  char thresStr[9];
  switch (m_pTempValveManager->m_thresholdRegion)
  {
    case TempValveManager::OVER:
      strcpy(thresStr, s_over);
      appendSpaces(string, 8, thresStr);
      strcat(string, thresStr);
      break;
    case TempValveManager::UPPER:
      strcpy(thresStr, s_upper);
      appendSpaces(string, 8, thresStr);
      strcat(string, thresStr);
      break;
    case TempValveManager::MEDIAN_ASCENDING:
      strcpy(thresStr, s_raising);
      appendSpaces(string, 8, thresStr);
      strcat(string, thresStr);
      break;
    case TempValveManager::MEDIAN_DESCENDING:
      strcpy(thresStr, s_dropping);
      appendSpaces(string, 8, thresStr);
      strcat(string, thresStr);
      break;
    case TempValveManager::LOWER:
      strcpy(thresStr, s_lower);
      appendSpaces(string, 8, thresStr);
      strcat(string, thresStr);
      break;
    case TempValveManager::BELOW:
      strcpy(thresStr, s_under);
      appendSpaces(string, 8, thresStr);
      strcat(string, thresStr);
      break;
    case TempValveManager::UNDEF:
      strcpy(thresStr, s_calculating);
      appendSpaces(string, 8, thresStr);
      strcat(string, thresStr);
      break;
      
  }
}

void SyrupManager::appendTempString(char *string, float temp)
{
  int charLength = 6;
  char tempStr[7];
  dtostrf(temp, 4, 2, tempStr);
  strcat(string, tempStr);
  appendSpaces(string, charLength, tempStr);
}

void SyrupManager::appendTempStringMaxMin(char *string, int tempDigital)
{
  char tempStr[8];
  float tempVal;
  switch(m_pSettingsManager->m_settings.m_tempScale)
  {
    case TempProbe::FAHRENHEIT:
      tempVal = m_pTempProbe->convertReadingToF(tempDigital);
      dtostrf(tempVal, 4, 2, tempStr);
      strcat(tempStr, s_tempDegreeF);
      break;
    case TempProbe::CELCIUS:
      tempVal = m_pTempProbe->convertReadingToC(tempDigital);
      dtostrf(tempVal, 4, 2, tempStr);
      strcat(tempStr, s_tempDegreeC);
      break;
  }
  strcat(string, tempStr);
  appendSpaces(string, 7, tempStr);
}

void SyrupManager::appendTempStringMain(char *string)
{
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
  }
  strcat(string, tempStr);
  appendSpaces(string, 7, tempStr);
}

void SyrupManager::appendTempScaleSymbol(char* string, TempProbe::e_scale scale)
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

void SyrupManager::appendValveStateString(char* string)
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

void SyrupManager::appendDurationString(char* string, unsigned long time,
  bool rightOriented, bool animate)
{
  appendDurationString(string, time, rightOriented, animate, false);
}
void SyrupManager::appendDurationString(char* string, unsigned long time,
  bool rightOriented, bool animate, bool isMinutes)
{
  int charLength = 7;
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
  
  long dayTime;
  int hourTime;
  int minuteTime;
  if (!isMinutes) {
    dayTime = 86400;
    hourTime = 3600;
    minuteTime = 60;
  }
  else {
    dayTime = 1440;
    hourTime = 60;
    minuteTime = 1;
    ignoreSeconds = true;
  }
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
        if (animate) {
          if (seconds % 2 > 0) {
            strcat(durationStr, s_colon);
          }
          else {
            strcat(durationStr, s_space);
          }
        }
        else {
          strcat(durationStr, s_hoursChar);
          strcat(durationStr, s_space);
        }
      }
      ignoreSeconds = true;
    }
    if (!ignoreMinutes && (minutes > 0 || hours > 0 || isMinutes)) {
      if (minutes < 10 && ignoreSeconds && !isMinutes) {
        strcat(durationStr, s_zeroChar);
      }
      itoa(minutes, timePart, integerBase);
      strcat(durationStr, timePart);
      if (!ignoreSeconds || isMinutes) {
        strcat(durationStr, s_minutesChar);
        strcat(durationStr, s_space);
      }
    }
    if (!ignoreSeconds) {
      itoa(seconds, timePart, integerBase);
      strcat(durationStr, timePart);
      strcat(durationStr, s_secondsChar);
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

void SyrupManager::appendSpaces(char* string, int length, char* baseString)
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
void SyrupManager::registerObservers()
{
  m_pTempProbe->attach(this);
  m_pValve->attach(this);
  m_pStats->attach(this);
  m_pOverrideManager->attach(this);
  m_pToggleButton->attach(this);
  m_pTempValveManager->attach(this);
}

void SyrupManager::unregisterObservers()
{
  m_pTempProbe->detach(this);
  m_pValve->detach(this);
  m_pStats->detach(this);
  m_pOverrideManager->detach(this);
  m_pToggleButton->detach(this);
  m_pTempValveManager->detach(this);
}

void SyrupManager::cancelThresEditMode()
{
  if (m_pTHRESEditor != NULL) {
    m_pTHRESEditor->leaveEditMode();
    m_pTHRESEditor->detach(this);
    delete(m_pTHRESEditor);
    m_pTHRESEditor = NULL;
  }
}

