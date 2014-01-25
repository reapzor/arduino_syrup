
 /* Read Quadrature Encoder
  * Connect Encoder to Pins encoder0PinA, encoder0PinB, and +5V.
  *
  * Sketch by max wolf / www.meso.net
  * v. 0.1 - very basic functions - mw 20061220
  *
  */  
  
//#include <avr/pgmspace.h>
#include <MemoryFree.h>
#include <StandardCplusplus.h>
#include <LiquidCrystal.h>

#include <Observer.h>
#include <Subject.h>
#include <Command.h>
#include <AnalogHelper.h>

#include <Encoder.h>
#include <Button.h>
#include <ValveController.h>
#include <TempProbe.h>
#include <LCDController.h>
#include "Stats.h"
#include "SyrupDisplayManager.h"
#include "OverrideManager.h"
#include "TempValveManager.h"

/*
class Helpers {
  public:
    static int charSpacing(int integer);
    
};

int Helpers::charSpacing(int integer) {
  if (integer >= 1000 && integer < 10000) return 4;
  if (integer >= 100) return 3;
  if (integer >= 10) return 2;
  if (integer >= 0) return 1;
  if (integer > -10) return 2;
  if (integer > -100) return 3;
  if (integer > -1000) return 4;
  if (integer > -10000) return 5;
  return 0;
}
*/


 LCDController lcd(12, 11, 5, 4, 3, 2);
 Encoder encoder(8,9);
 Button toggleButton(6);
 Button overrideSwitch(7);
 TempProbe tempProbe(1); 
 ValveController valveController(13);
 Stats stats(&valveController, &tempProbe);
 OverrideManager overrideManager(&toggleButton, &overrideSwitch, &valveController);
 TempValveManager tempValveManager(&tempProbe, &valveController);
 SyrupDisplayManager displayManager(&lcd, &tempProbe, &valveController, &stats);

 
//const prog_uchar hi[] PROGMEM = "isfh\0";
//const prog_uchar *hi2[] = {hi};
//char hine[10];
  
 void setup() { 
    Serial.begin (57600);
    //EncoderObserver *obs = new EncoderObserver();
    //char buffer[300];
   // encoder.attach(*obs);
  //lcd.setState(LCDController::MAIN);
//  char *lines = "line hereline herelffffg";
//  lines[1] = "line hereline herelffff\0";
//stats.registerObservers();
  lcd.test();
//  lcd.write(1, "FUCK");
//  delay(1300);
//  lcd.edit(1, 2, "N!");
//delay(1300);
  //lcd.write(0, lines);
  lcd.write(0, SyrupDisplayManager::s_welcomeLineOne);
//  lcd.write(1, SyrupDisplayManager::s_welcomeLineOne);
  
//    Serial.println(lcd.m_currentText[1]);
  //Serial.println("HWLIUH");
  //delay(1000);
  //strcpy_P(hine, (char*)pgm_read_word(&(hi2[0])));
//  char str = (char*) pgm_read_word(&SyrupDisplayManager::s_welcomeLineOne)
 // Serial.println(SyrupDisplayManager::s_welcomeLineOne);
//    Serial.println(SyrupDisplayManager::s_welcomeLineTwo);
  //delay(5000);
//    lcd.write(0, buffer);
//    lcd.edit(0, 2, "nnnnnnnnnn");
//    lcd.clear(0, 10);
  //encoder.pause();
  // Serial.print("hi");
  // MyReceiver r;
   //Receiver *re = static_cast<Receiver *>(&r);
//   MyCommand c(r);
//   c.execute();
tempValveManager.registerObservers();
overrideManager.registerObservers();
valveController.openValve();
valveController.closeValve();

//vc.forceCloseValve();
//vc.releaseForcedState();

   /*
Temperature*       temp = new Temperature();
PanicSirene*       panic = new PanicSirene();
   Serial.println(freeMemory());
temp->attach(*panic);temp->attach(*panic);
temp->temperatureChanged();
//delay(200);
temp->detach(*panic);
//Serial.println("FS:JKBN");
   Serial.println(freeMemory());
//delay(200);
temp->attach(*panic);
delay(300);
temp->temperatureChanged();
   Serial.println(freeMemory());
   delete panic;
   delete temp;
   Serial.println(freeMemory());
//temp->temperatureChanged();   
*/
Serial.println("lol");
displayManager.setState(SyrupDisplayManager::WELCOME);
delay(500);

 } 

 void loop() { 
//   delay(1000);
//   long hi = millis();
   encoder.tick();
//   long hi2 = millis();
   tempProbe.tick();
   valveController.tick();
   toggleButton.tick();
   overrideSwitch.tick();
   stats.tick();
   displayManager.tick();
  /* long bye = millis();
   Serial.print(hi);
   Serial.print(" | ");
   Serial.println(bye);
   delay(300); */
   //Serial.print("NOP: ");

//  Serial.println(freeMemory());
 } 
