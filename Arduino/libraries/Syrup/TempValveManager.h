//Chux
#ifndef TempValveManager_h
#define TempValveManager_h

#include "Arduino.h"

#include "Observer.h"
#include "Subject.h"
#include "ValveController.h"
#include "TempProbe.h"

#define DEBUG_THRES
#define DEBUG_BOUNDS
#define DEBUG

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
    TempValveManager(TempProbe *tempProbe, ValveController *valveController);
    ~TempValveManager();
    e_thresholdRegion m_thresholdRegion;
    static const int BOUNDS_THRESHOLD_F = 1;
    static const float BOUNDS_THRESHOLD_C = 0.555;
    //In anticipation of this becoming an observable, 
    //  use sets instead of direct access
    void setUpperThreshold(float upperThreshold);
    void setLowerThreshold(float lowerThreshold);
    float getUpperThreshold();
    float getLowerThreshold();
    
    void update(TempProbe *tempProbe);
    
    void registerObservers();
    void unregisterObservers();
    
  private:
    float m_upperThreshold;
    float m_lowerThreshold;
    TempProbe *m_pTempProbe;
    ValveController *m_pValveController;
    void updateThreshold(float temp, float boundsThreshold);
    bool m_hasDoneUpperBoundsTask;
    void tryUpperBoundsTask();
    void tryLowerBoundsTask();
    void doUpperBoundsTask();
    void doLowerBoundsTask();
};

#endif