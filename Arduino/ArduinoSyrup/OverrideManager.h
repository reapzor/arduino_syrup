//Chux
#ifndef OverrideManager_h
#define OverrideManager_h

#include "Arduino.h"

#include "Observer.h"
#include "Subject.h"
#include "ValveController.h"
#include "Button.h"

#define DEBUG_OBSERVERS
#define DEBUG

class OverrideManager : public Subject<OverrideManager>
{
  public:
    OverrideManager(Button *toggleButton, Button *overrideSwitch,
      ValveController *valveController);
    ~OverrideManager();
    void registerObservers();
    void unregisterObservers();
    bool isValveOverrideEnabled();
  private:
    Button *m_pToggleButton;
    Button *m_pOverrideSwitch;
    ValveController *m_pValveController;
    bool m_valveOverrideEnabled;
    void enable();
    void disable();
    class ToggleButtonObserver : public Observer<Button>
    {
      public:
        OverrideManager &m_overrideManager;
        ToggleButtonObserver(OverrideManager &overrideManager);
        ~ToggleButtonObserver();
        void update(Button *button);
    } m_toggleButtonObserver;
    class OverrideSwitchObserver : public Observer<Button>
    {
      public:
        OverrideManager &m_overrideManager;
        OverrideSwitchObserver(OverrideManager &overrideManager);
        ~OverrideSwitchObserver();
        void update(Button *button);
    } m_overrideSwitchObserver;
};

#endif