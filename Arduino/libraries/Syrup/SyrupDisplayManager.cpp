//Chux
#include "SyrupDisplayManager.h"

SyrupDisplayManager::SyrupDisplayManager(LCDController *lcd,
  TempProbe *tempProbe, ValveController *valve, Stats *stats) :
    m_pLCD(lcd), m_pTempProbe(tempProbe), m_pValve(valve), m_pStats(stats),
    Observer<TempProbe>(), Observer<ValveController>(), Observer<Stats>()
{
  m_transitioning = false;
  m_nextTransition = UNDEF;
  m_currentTransitionDelay = 0;
  m_currentDrawDelay = 0;
  m_shouldDraw = false;
  m_displayState = UNDEF;
}

SyrupDisplayManager::~SyrupDisplayManager()
{

}

char* SyrupDisplayManager::s_welcomeLineOne = "THE SYRUP-O-MATIC  V1.0";
char* SyrupDisplayManager::s_welcomeLineTwo = "CREATED BY CHUCK BENSON";

char* SyrupDisplayManager::s_mainTemp = "TEMP:  ";
char* SyrupDisplayManager::s_mainValve = "VALVE: ";
char* SyrupDisplayManager::s_mainTime = "TIME";

char* SyrupDisplayManager::s_tempDegree = "f";
char* SyrupDisplayManager::s_valveStateOpen = "OPEN";
char* SyrupDisplayManager::s_valveStateClosed = "CLOSED";

char* SyrupDisplayManager::s_thresTempOpen = "OPEN TEMP:  ";
char* SyrupDisplayManager::s_thresTempClose = "CLOSE TEMP: ";

char* SyrupDisplayManager::s_seenTempMax = "MAX TEMP SEEN: ";
char* SyrupDisplayManager::s_seenTempMin = "MIN TEMP SEEN: ";

char* SyrupDisplayManager::s_lastDurClose = "LAST CLOSE DUR: ";
char* SyrupDisplayManager::s_lastDurOpen = "LAST OPEN DUR:  ";

char* SyrupDisplayManager::s_secondsChar = "s";
char* SyrupDisplayManager::s_minutesChar = "m";
char* SyrupDisplayManager::s_hoursChar = "h";

char* SyrupDisplayManager::s_averageDurClose = "AVG CLOSE DUR: ";
char* SyrupDisplayManager::s_averageDurOpen = "AVG OPEN DUR:  ";

char* SyrupDisplayManager::s_countOpen = "OPEN COUNT:  ";
char* SyrupDisplayManager::s_countClose = "CLOSE COUNT: ";

char* SyrupDisplayManager::s_overrideValveLineOne = "    VALVE OVERRIDE";
char* SyrupDisplayManager::s_overrideValveLineTwo = "      VALVE OPEN";

char* SyrupDisplayManager::s_savedLineOne = "        SETTING";
char* SyrupDisplayManager::s_savedLineTwo = "         SAVED";

char* SyrupDisplayManager::s_systemUptime = "UPTIME:   ";
char* SyrupDisplayManager::s_systemFreeMem = "FREE MEM: ";

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
  m_displayState = state;
  draw();
}

void SyrupDisplayManager::update(TempProbe *tempProbe)
{
  //Serial.println("KFJS");
}

void SyrupDisplayManager::update(ValveController *valve)
{
//Serial.println("KFJS");
}

void SyrupDisplayManager::update(Stats *stats)
{
//Serial.println("KFJS");
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
    m_currentDrawDelay = (long)millis() + DELAY_BETWEEN_POSSIBLE_SHOULD_DRAWS;
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
  }
}

void SyrupDisplayManager::drawWelcome()
{
  m_pLCD->write(0, s_welcomeLineOne);
  m_pLCD->write(1, s_welcomeLineTwo);
}

void SyrupDisplayManager::drawMain()
{
  char temp[5];
  temp[5] = '\0';
  //sprintf(temp, "%i", 20);
  dtostrf(m_pTempProbe->m_tempF, 4, 2, temp);
  //Serial.println(temp);
  Serial.println(temp);
  m_pLCD->write(0, temp);
  //m_pLCD->edit(0, strlen(s_mainTemp), );
  //m_pLCD->edit(0, strlen(s_mainTemp)+strlen(
}

/*
long eventTimeout=(long)millis()+1000;
if((long)millis()-eventTimeout>=0) {
eventTimeout=(long)millis()+1000;
}
*/


