//Chux
#include "OverrideManager.h"

OverrideManager::OverrideManager(ToggleButton *toggleButton, 
  OverrideSwitch *overrideSwitch, ValveController *valveController) :
    m_pToggleButton(toggleButton), m_pOverrideSwitch(overrideSwitch),
    m_pValveController(valveController), Subject<OverrideManager>()
{
}
OverrideManager::~OverrideManager()
{
  //unregisterObservers();
  //Since it never dies anyway
}


void OverrideManager::registerObservers()
{
  m_pOverrideSwitch->attach(this);
}

void OverrideManager::unregisterObservers()
{
  m_pOverrideSwitch->detach(this);
  disable();
}

void OverrideManager::disable()
{
  if (m_valveOverrideEnabled) {
    m_valveOverrideEnabled = false;
    m_pToggleButton->detach(this);
    m_pValveController->releaseForcedState();
    notify();
  }
}

void OverrideManager::enable()
{
  m_valveOverrideEnabled = true;
  m_pValveController->forceSwapValveState();
  m_pToggleButton->attach(this);
  notify();
}

bool OverrideManager::isValveOverrideEnabled()
{
  return m_valveOverrideEnabled;
}

void OverrideManager::update(ToggleButton *toggleButton)
{
  if (toggleButton->m_buttonState == ToggleButton::ON) {
    m_pValveController->forceSwapValveState();
  }
}

void OverrideManager::update(OverrideSwitch *overrideSwitch)
{
  if (overrideSwitch->m_buttonState == OverrideSwitch::ON) {
    enable();
  }
  else {
    disable();
  }
}