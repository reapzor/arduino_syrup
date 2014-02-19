//Chux
#include "THRESEditor.h"

THRESEditor::THRESEditor(SyrupSettingsManager *settingsManager, Encoder *encoder,
  ToggleButton *toggleButton) :
    m_pSettingsManager(settingsManager), m_pEncoder(encoder), m_pToggleButton(toggleButton),
    Subject<THRESEditor>(), Observer<Encoder>(), Observer<ToggleButton>()
{
  m_editItem = UNDEF;
  m_inEditMode = false;
  m_upperThreshold = 0;
  m_lowerThreshold = 0;
  m_tempScale = TempProbe::FAHRENHEIT;
  m_prepForTransition = false;
}

THRESEditor::~THRESEditor()
{
  unregisterObservers();
}

void THRESEditor::enterEditMode()
{
  if (!m_inEditMode) {
    setEditMode(true);
    m_upperThreshold = m_pSettingsManager->m_settings.m_upperThreshold;
    m_lowerThreshold = m_pSettingsManager->m_settings.m_lowerThreshold;
    m_tempScale = m_pSettingsManager->m_settings.m_tempScale;
    setNextEditItem();
    registerObservers();
  }
}

void THRESEditor::leaveEditMode()
{
  if (m_inEditMode) {
    setEditMode(false);
    unregisterObservers();
  }
}

bool THRESEditor::isInEditMode()
{
  return m_inEditMode;
}

void THRESEditor::save()
{
  m_pSettingsManager->m_settings.m_upperThreshold = m_upperThreshold;
  m_pSettingsManager->m_settings.m_lowerThreshold = m_lowerThreshold;
  m_pSettingsManager->m_settings.m_tempScale = m_tempScale;
  m_pSettingsManager->save();
}

void THRESEditor::setEditMode(bool editMode)
{
  if (m_inEditMode == editMode) {
    return;
  }
  m_editItem = EDIT_MODE;
  m_inEditMode = editMode;
  notify();
  m_editItem = UNDEF;
}

void THRESEditor::registerObservers()
{
  m_pToggleButton->attach(this);
  m_pEncoder->attach(this);
}

void THRESEditor::unregisterObservers()
{
  m_pToggleButton->detach(this);
  m_pEncoder->detach(this);
}

void THRESEditor::update(Encoder *encoder)
{
  #ifdef DEBUG_OBSERVERS
    Serial.print(F("ENCODER: "));
    Serial.println(encoder->m_direction);
  #endif
  switch(encoder->m_direction)
  {
    case Encoder::FORWARD:
      alterCurrentEditItemValue(0.5);
      break;
    case Encoder::BACKWARD:
      alterCurrentEditItemValue(-0.5);
      break;
  }
}

void THRESEditor::update(ToggleButton *toggleButton)
{
  #ifdef DEBUG_OBSERVERS
    Serial.print(F("TOGGLE: "));
    Serial.println(toggleButton->m_buttonState);
  #endif
  if (toggleButton->m_buttonState == ToggleButton::ON) {
    m_prepForTransition = true;
  }
  else {
    if (m_prepForTransition) {
      setNextEditItem();
      m_prepForTransition = false;
    }
  }
}

void THRESEditor::alterCurrentEditItemValue(float addition)
{
  switch(m_editItem)
  {
    case UPPER_THRES:
      m_upperThreshold += addition;
      if (m_upperThreshold >= 999.00) {
        m_upperThreshold = 999.00;
      }
      #ifdef DEBUG
        Serial.print(F("UPPER THRES: "));
        Serial.println(m_upperThreshold);
      #endif
      break;
    case LOWER_THRES:
      m_lowerThreshold += addition;
      if (m_lowerThreshold <= -99.00) {
        m_lowerThreshold = -99.00;
      }
      #ifdef DEBUG
        Serial.print(F("LOWER THRES: "));
        Serial.println(m_lowerThreshold);
      #endif
      break;
    case TEMP_SCALE:
      if (m_tempScale == TempProbe::FAHRENHEIT) {
        m_tempScale = TempProbe::CELCIUS;
      }
      else {
        m_tempScale = TempProbe::FAHRENHEIT;
      }
      #ifdef DEBUG
        Serial.print(F("TEMP SCALE: "));
        Serial.println(m_tempScale);
      #endif
      break;
    default:
      return;
  }
  notify();
}

void THRESEditor::setNextEditItem()
{
  if (m_editItem < UPPER_THRES || m_editItem == TEMP_SCALE) {
    m_editItem = UPPER_THRES;
  }
  else {
    m_editItem = (e_editItem) (m_editItem+1);
  }
  notify();
}