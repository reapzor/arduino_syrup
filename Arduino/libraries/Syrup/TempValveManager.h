//Chux
#ifndef TempValveManager_h
#define TempValveManager_h

#include "Arduino.h"

#include "Observer.h"
#include "Subject.h"
#include "ValveController.h"
#include "TempProbe.h"
#include "SyrupSettingsManager.h"

//#define DEBUG_THRES
#define DEBUG_BOUNDS

class TempValveManager : public Observer<TempProbe>
{
  public:
    enum e_thresholdRegion
    {
      UNDEF,
      BELOW,
      LOWER,
      MEDIAN_ASCENDING,
      MEDIAN_DESCENDING,
      UPPER,
      OVER
    };
    TempValveManager(TempProbe *tempProbe, ValveController *valveController,
      SyrupSettingsManager *settingsManager);
    ~TempValveManager();
    e_thresholdRegion m_thresholdRegion;
    static const int BOUNDS_THRESHOLD_F = 1;
    static const float BOUNDS_THRESHOLD_C = 0.555;
    
    void update(TempProbe *tempProbe);
    
    void registerObservers();
    void unregisterObservers();
    
  private:
    TempProbe *m_pTempProbe;
    ValveController *m_pValveController;
    SyrupSettingsManager *m_pSettingsManager;
    void updateThreshold(float temp, float boundsThreshold);
    bool m_hasDoneUpperBoundsTask;
    void tryUpperBoundsTask();
    void tryLowerBoundsTask();
    void doUpperBoundsTask();
    void doLowerBoundsTask();
};

#endif