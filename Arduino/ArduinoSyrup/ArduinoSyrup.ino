//Chux  
#include <MemoryFree.h>
#include <StandardCplusplus.h>
#include <LiquidCrystal.h>
#include <EEPROMex.h>

#include <Observer.h>
#include <Subject.h>
#include <Command.h>
#include <AnalogHelper.h>

#include <Encoder.h>
#include <Button.h>
#include <ValveController.h>
#include <TempProbe.h>
#include <LCDController.h>
#include "ToggleButton.h"
#include "OverrideSwitch.h"
#include <SyrupSettingsManager.h>
#include "Stats.h"
#include "SyrupDisplayManager.h"
#include "OverrideManager.h"
#include "TempValveManager.h"
#include "THRESEditor.h"

 

 LCDController lcd(12, 11, 5, 4, 3, 2);

 Encoder encoder(8,9);

 ToggleButton toggleButton(6);

 OverrideSwitch overrideSwitch(7);
 
 TempProbe tempProbe(1); 
 
 ValveController valveController(13);
 
 SyrupSettingsManager settingsManager;
 
 Stats stats(&valveController, &tempProbe);
 
 OverrideManager overrideManager(&toggleButton, &overrideSwitch, &valveController);

 TempValveManager tempValveManager(&tempProbe, &valveController, &settingsManager);
 
 SyrupDisplayManager displayManager(&lcd, &tempProbe, &valveController,
     &stats, &overrideManager, &toggleButton, &settingsManager, &encoder);    
     
     
     
 void setup() { 
    Serial.begin (57600);
    
    tempValveManager.registerObservers();
    overrideManager.registerObservers();
    stats.registerObservers();
    displayManager.registerObservers();
    
    settingsManager.prime();
    encoder.prime();
    toggleButton.prime();
    overrideSwitch.prime();
    tempProbe.prime();
    displayManager.prime();
    stats.prime();
 } 



 void loop() { 
   encoder.tick();
   toggleButton.tick();
   overrideSwitch.tick();
   tempProbe.tick();
   valveController.tick();
   stats.tick();
   displayManager.tick();
 } 
