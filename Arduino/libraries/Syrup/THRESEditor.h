//Chux
#ifndef THRESEditor_h
#define THRESEditor_h

#include <Arduino.h>
#include <Subject.h>
#include <Observer.h>
#include <Encoder.h>
#include "SyrupSettingsManager.h"
#include "ToggleButton.h"

#define DEBUG
//#define DEBUG_OBSERVERS

class THRESEditor : public Subject<THRESEditor>, public Observer<Encoder>, 
  public Observer<ToggleButton>
{
  public:
    enum e_editItem
    {
      UNDEF,
      EDIT_MODE,
      UPPER_THRES,
      LOWER_THRES,
      TEMP_SCALE
    };
    e_editItem m_editItem;
    
    THRESEditor(SyrupSettingsManager *settingsManager, Encoder *encoder,
      ToggleButton *toggleButton);
    ~THRESEditor();
    
    bool isInEditMode();
    
    float m_upperThreshold;
    float m_lowerThreshold;
    TempProbe::e_scale m_tempScale;
    
    void registerObservers();
    void unregisterObservers();
    
    void enterEditMode();
    void leaveEditMode();
    void save();
    
    void update(Encoder *encoder);
    void update(ToggleButton *toggleButton);
    
  private:
    SyrupSettingsManager *m_pSettingsManager;
    Encoder *m_pEncoder;
    ToggleButton *m_pToggleButton;
    void setEditMode(bool editMode);
    bool m_inEditMode;
    bool m_prepForTransition;
    void setNextEditItem();
    void alterCurrentEditItemValue(float addition);
};

#endif