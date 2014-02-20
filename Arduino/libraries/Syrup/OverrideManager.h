//Chux
#ifndef OverrideManager_h
#define OverrideManager_h

#include "Arduino.h"

#include "Observer.h"
#include "Subject.h"
#include "ValveController.h"
#include "ToggleButton.h"
#include "OverrideSwitch.h"

//#define DEBUG_OBSERVERS
//#define DEBUG

class OverrideManager : public Subject<OverrideManager>, Observer<ToggleButton>,
  Observer<OverrideSwitch>
{
  public:
    OverrideManager(ToggleButton *toggleButton, OverrideSwitch *overrideSwitch,
      ValveController *valveController);
    ~OverrideManager();
    void registerObservers();
    void unregisterObservers();
    bool isValveOverrideEnabled();
    
  private:
    ToggleButton *m_pToggleButton;
    OverrideSwitch *m_pOverrideSwitch;
    ValveController *m_pValveController;
    bool m_valveOverrideEnabled;
    void enable();
    void disable();
    void update(ToggleButton *toggleButton);
    void update(OverrideSwitch *overrideSwitch);
    
};

#endif