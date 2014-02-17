//Chux
#ifndef LCDController_h
#define LCDController_h

#include <Arduino.h>
#include <LiquidCrystal.h>
#define DEV
#define DEBUG
#define CURTEXT_DEBUG

/*
  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
*/

class LCDController
{
  public:
    static const int COLUMN_COUNT = 24;
    static const int ROW_COUNT = 2;
    static const char NULL_TERMINATOR = '\0';
    LiquidCrystal* m_pLCD;
    LCDController(int rs, int enable, int d4, int d5, int d6, int d7);
    ~LCDController() 
    {
      delete m_pLCD;
    }
    void write(int rowCount, char* rows[]);
    void write(int row, char *line);
    void edit(int row, int offset, char *edit);
    void clear();
    void clear(int row);
    void clear(int row, int offset);
    void clear(int row, int offset, int length);
    void test();

};

#endif