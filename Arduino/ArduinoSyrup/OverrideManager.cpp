//Chux
#include "OverrideManager.h"

OverrideManager::OverrideManager(Button *toggleButton, Button *overrideSwitch,
  ValveController *valveController) :
    m_pToggleButton(toggleButton), m_pOverrideSwitch(overrideSwitch),
    m_pValveController(valveController), m_toggleButtonObserver(*this),
    m_overrideSwitchObserver(*this), Subject<OverrideManager>()
{
}
OverrideManager::~OverrideManager()
{
}

OverrideManager::ToggleButtonObserver::ToggleButtonObserver(
  OverrideManager &overrideManager) :
    m_overrideManager(overrideManager), Observer<Button>()
{
}
OverrideManager::ToggleButtonObserver::~ToggleButtonObserver()
{
}

OverrideManager::OverrideSwitchObserver::OverrideSwitchObserver(
  OverrideManager &overrideManager) :
    m_overrideManager(overrideManager), Observer<Button>()
{
}
OverrideManager::OverrideSwitchObserver::~OverrideSwitchObserver()
{
}

void OverrideManager::registerObservers()
{
  m_pOverrideSwitch->attach(&m_overrideSwitchObserver);
}

void OverrideManager::unregisterObservers()
{
  m_pOverrideSwitch->detach(&m_overrideSwitchObserver);
  disable();
}

void OverrideManager::disable()
{
  if (m_valveOverrideEnabled) {
    m_valveOverrideEnabled = false;
    m_pToggleButton->detach(&m_toggleButtonObserver);
    m_pValveController->releaseForcedState();
    notify();
  }
}

void OverrideManager::enable()
{
  m_valveOverrideEnabled = true;
  m_pValveController->forceSwapValveState();
  m_pToggleButton->attach(&m_toggleButtonObserver);
  notify();
}

bool OverrideManager::isValveOverrideEnabled()
{
  return m_valveOverrideEnabled;
}

void OverrideManager::OverrideSwitchObserver::update(Button *button)
{
  if (button->buttonState == Button::ON) {
    m_overrideManager.enable();
  }
  else {
    m_overrideManager.disable();
  }
}

void OverrideManager::ToggleButtonObserver::update(Button *button)
{
  if (button->buttonState == Button::ON) {
    m_overrideManager.m_pValveController->forceSwapValveState();
  }
}