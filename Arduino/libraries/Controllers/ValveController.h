//Chux
#ifndef ValveController_h
#define ValveController_h

#include "Arduino.h"
#include "Subject.h"
#define DEBUG
#define DEV

class ValveController : public Subject<ValveController>
{
  public:
    enum e_valveState
    {
      UNDEF,
      OPEN,
      CLOSED,
    };
    int m_valveSignalPin;
    e_valveState m_valveState;
    bool m_stateIsForced;
    void openValve();
    void forceOpenValve();
    void closeValve();
    void swapValveState();
    void forceSwapValveState();
    void forceCloseValve();
    void releaseForcedState();
    void tick();
    ValveController(int valvePin);
    ~ValveController() {}
  private:
    e_valveState m_suggestedState;
    long m_delayTime;
    static const int STATE_CHANGE_DELAY = 1000;
    bool delayExpired();
    void resetDelay();
    void forceExpireDelay();
    void swapValveState(bool force);
    void changeValveState(bool ignoreDelay, e_valveState state);
    void changeValveToSuggestedState();
};

#endif