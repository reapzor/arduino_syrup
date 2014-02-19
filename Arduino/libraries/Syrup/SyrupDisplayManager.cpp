//Chux
#include "SyrupDisplayManager.h"

SyrupDisplayManager::SyrupDisplayManager(LCDController *lcd,
  TempProbe *tempProbe, ValveController *valve, Stats *stats,
  OverrideManager *overrideManager, ToggleButton *toggleButton,
  SyrupSettingsManager *settingsManager, Encoder *encoder) :
    m_pLCD(lcd), m_pTempProbe(tempProbe), m_pValve(valve), m_pStats(stats),
    m_pOverrideManager(overrideManager), m_pToggleButton(toggleButton),
    m_pSettingsManager(settingsManager), m_pEncoder(encoder),
    Observer<TempProbe>(), Observer<ValveController>(), Observer<Stats>(),
    Observer<ToggleButton>(), Observer<OverrideManager>(), Observer<THRESEditor>()
{
  m_transitioning = false;
  m_nextTransition = UNDEF;
  m_currentTransitionDelay = 0;
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
char* SyrupDisplayManager::s_secondsChar = "s";
char* SyrupDisplayManager::s_zeroChar = "0";

char* SyrupDisplayManager::s_space = " ";

char* SyrupDisplayManager::s_average = "AVG";

char* SyrupDisplayManager::s_count = "COUNT";

char* SyrupDisplayManager::s_valveCaps = "VALVE";
char* SyrupDisplayManager::s_overrideValveOverride = "OVERRIDE";

char* SyrupDisplayManager::s_savedLineOne = "SETTING";
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
      Serial.println(F("TRANSITIONING!"));
    #endif
    m_nextTransition = state;
    return;
  }
  if (m_displayState == state) {
    #ifdef DEBUG
      Serial.print(F("ALREADY SET DISPLAY STATE: "));
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
  if (m_pTHRESEditor != NULL) {
    m_pTHRESEditor->leaveEditMode();
    m_pTHRESEditor->detach(this);
    delete(m_pTHRESEditor);
    m_pTHRESEditor = NULL;
  }
  #ifdef DEBUG
    Serial.print(F("LCD STATE: "));
    Serial.println(state);
  #endif
  m_displayState = state;
  draw();
}

void SyrupDisplayManager::update(THRESEditor *thresEditor)
{
  switch(thresEditor->m_editItem)
  {
    case THRESEditor::UPPER_THRES:
      char upperTemp[7];
      *upperTemp = LCDController::NULL_TERMINATOR;
      appendTempString(upperTemp, thresEditor->m_upperThreshold);
      m_pLCD->edit(0, 6, upperTemp);
      break;
    case THRESEditor::LOWER_THRES:
      char lowerTemp[7];
      *lowerTemp = LCDController::NULL_TERMINATOR;
      appendTempString(lowerTemp, thresEditor->m_lowerThreshold);
      m_pLCD->edit(1, 8, lowerTemp);
      break;
    case THRESEditor::TEMP_SCALE:
      char tempScale[2];
      *tempScale = LCDController::NULL_TERMINATOR;
      appendTempScaleSymbol(tempScale, thresEditor->m_tempScale);
      m_pLCD->edit(1, 23, tempScale);
      break;
  }
}

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
    m_toggleButtonHoldDelay = (long)millis()+TOGGLE_BUTTON_ON_DURATION;
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
  //m_currentDrawDelay = (long)millis();
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

void SyrupDisplayManager::tick()
{
  if (m_transitioning && (long)millis()-m_currentTransitionDelay >= 0) {
    m_currentTransitionDelay = 0;
    m_transitioning = false;
    e_displayState displayState = m_nextTransition;
    m_nextTransition = UNDEF;
    setState(displayState);
  }
  if (m_prepForTransition && m_displayState == THRES) {
    if ((long)millis()-m_toggleButtonHoldDelay >= 0) {
      m_prepForTransition = false;
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
  /*if (m_shouldDraw && (long)millis()-m_currentDrawDelay >= 0) {
    m_currentDrawDelay += DELAY_BETWEEN_POSSIBLE_SHOULD_DRAWS;
    m_shouldDraw = false;
    draw();
  }*/
}

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
    default:
      m_pLCD->clear();
      m_pLCD->write(1, s_welcomeLineOne);      
      break;
  }
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
  char row1[13];
  //strcpy(row1, s_seenMax);
  //strcat(row1, s_space);
  //strcat(row1, s_mainTemp);
  strcpy(row1, s_valveStateOpen);
  strcat(row1, s_colon);
  strcat(row1, s_space);
  appendTempString(row1, m_pSettingsManager->m_settings.m_upperThreshold);
  
  char row2[15];
  //strcpy(row2, s_seenMin);
  //strcat(row2, s_space);
  //strcat(row2, s_mainTemp);
  strcpy(row2, s_valveStateClosed);
  strcat(row2, s_colon);
  strcat(row2, s_space);      
  appendTempString(row2, m_pSettingsManager->m_settings.m_lowerThreshold);
  
  int scaleStrLength = 8;
  char scaleStr[scaleStrLength+1];
  strcpy(scaleStr, s_scale);
  strcat(scaleStr, s_colon);
  strcat(scaleStr, s_space);
  appendTempScaleSymbol(scaleStr, m_pSettingsManager->m_settings.m_tempScale);
  
  char settingsStr[9];
  strcpy(settingsStr, s_savedLineOne);
  strncat(settingsStr, s_savedLineOne, 1);
  
  m_pLCD->write(0, row1);
  m_pLCD->write(1, row2);
  m_pLCD->edit(1, 24-scaleStrLength, scaleStr);
  m_pLCD->edit(0, 24-8, settingsStr);
}

void SyrupDisplayManager::drawValveOverride()
{
  char row1[15];
  strcpy(row1, s_valveCaps);
  strcat(row1, s_space);
  strcat(row1, s_overrideValveOverride);
  
  char row2[14];
  strcpy(row2, s_mainValve);
  strcat(row2, s_colon);
  strcat(row2, s_space);
  appendValveStateString(row2);
  
  m_pLCD->clear();
  
  m_pLCD->edit(0, 5, row1);
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
  //sprintf(row1, "%s%s%s", s_mainTemp, s_colon, s_space);
  strcpy(row1, s_mainTemp);
  strcat(row1, s_colon);
  strcat(row1, s_space);
  appendTempStringMain(row1);

  char row2[14];
  strcpy(row2, s_mainValve);
  strcat(row2, s_colon);
  strcat(row2, s_space);
  appendValveStateString(row2);
  char row2Time[8];
  *row2Time = LCDController::NULL_TERMINATOR;
  appendDurationStringRightOriented(row2Time, m_pStats->m_currentDuration);
  
  m_pLCD->write(0, row1);
  m_pLCD->edit(0, 19, s_mainTime);
  m_pLCD->write(1, row2);
  m_pLCD->edit(1, 17, row2Time);
}

void SyrupDisplayManager::appendTempString(char *string, float temp)
{
  char tempStr[7];
  dtostrf(temp, 4, 2, tempStr);
  strcat(string, tempStr);
  for (int x = strlen(tempStr); x < 6; x++) {
    strcat(string, s_space);
  }
}

void SyrupDisplayManager::appendTempStringMain(char *string)
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
    switch(m_pSettingsManager->m_settings.m_tempScale)
    {
      case TempProbe::FAHRENHEIT:
        char tempNumStrF[7];
        dtostrf(m_pTempProbe->m_tempF, 4, 2, tempNumStrF);
        strcat(tempStr, tempNumStrF);
        strcat(tempStr, s_tempDegreeF);
        break;
      case TempProbe::CELCIUS:
        char tempNumStrC[7];
        dtostrf(m_pTempProbe->m_tempC, 4, 2, tempNumStrC);
        strcat(tempStr, tempNumStrC);
        strcat(tempStr, s_tempDegreeC);
        break;
    }
  }
  strcat(string, tempStr);
  for (int x = strlen(tempStr); x < 7; x++) {
    strcat(string, s_space);
  }
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


