//Chux
#include <Arduino.h>
#include <LiquidCrystal.h>
#include "LCDController.h"

LCDController::LCDController(int rs, int enable, int d4, int d5, int d6, int d7) 
{
  m_pLCD = new LiquidCrystal(rs, enable, d4, d5, d6, d7);
  m_pLCD->begin(COLUMN_COUNT, ROW_COUNT);
}

void LCDController::write(int rowCount, char *rows[])
{ 
  #if defined(DEV) || defined(DEBUG)
    if (rowCount > ROW_COUNT) {
      #ifdef DEBUG
        Serial.print(F("Too Many Rows: "));
        Serial.println(rowCount);
        Serial.println();
      #endif
      rowCount = ROW_COUNT;
      rows[0] = "E:RS\0";
    }
  #endif
  for (int x = 0; x < rowCount; x++) {
    write(x, rows[x]);
  }
}

void LCDController::write(int row, char *line)
{
  int lineLength = strlen(line);
  char lineStr[COLUMN_COUNT+1];
  *lineStr = NULL_TERMINATOR;
  #if defined(DEV) || defined(DEBUG)
    if (row > ROW_COUNT) {
      #ifdef DEBUG
        Serial.print(F("Row Out Of Bounds: "));
        Serial.println(row);
        Serial.println(line);
        Serial.println();
      #endif
      row = 0;
      line = "E:RW";
    }
    if (lineLength > COLUMN_COUNT) {
      #ifdef DEBUG
        Serial.print(F("Row too long: "));
        Serial.print(row);
        Serial.print(F(" Length: "));
        Serial.println(lineLength);
        Serial.println(line);
      #endif
      //If youre hitting this and see weird chars after your string
      // chances are there is no teminator at the end. This is important
      // to determine the end of a string: '\0' char Str[3] = {'a' 'b' '\0'}
      row = 0;
      line = "E:CL";
    }
  #endif
  strcpy(lineStr, line);
  for (int x = lineLength; x < COLUMN_COUNT; x++) {
    strcat(lineStr, " ");
  }
  #ifdef DEBUG
    Serial.print(F("Setting LCD Row: "));
    Serial.print(row);
    Serial.print(F(" To: "));
    Serial.println(line);
  #endif
  m_pLCD->setCursor(0, row);
  m_pLCD->print(lineStr);
}

void LCDController::edit(int row, int offset, char *edit)
{
  int editLength = strlen(edit);
  #if defined(DEV) || defined(DEBUG)
    int lineLength = offset + editLength;
    if ((row > ROW_COUNT) || (lineLength > COLUMN_COUNT)) {
      #ifdef DEBUG
        Serial.print(F("Edit Too Big. Row: "));
        Serial.print(row);
        Serial.print(F(" Position: "));
        Serial.print(offset);
        Serial.print(F(" Length: "));
        Serial.println(lineLength);
        Serial.println(edit);
      #endif
      //If youre hitting this and see weird chars after your string
      // chances are there is no teminator at the end. This is important
      // to determine the end of a string: '\0' char Str[3] = {'a' 'b' '\0'}
      offset = 0;
      edit = "E:ED";
    }
  #endif
  #ifdef DEBUG
    Serial.print(F("Editing LCD. Row: "));
    Serial.print(row);
    Serial.print(F(" Position: "));
    Serial.print(offset);
    Serial.print(F(" Length: "));
    Serial.print(editLength);
    Serial.print(F(" Text: "));
    Serial.println(edit);
  #endif
  m_pLCD->setCursor(offset, row);
  m_pLCD->print(edit);
}

void LCDController::clear(int row)
{
  clear(row, 0);
}

void LCDController::clear(int row, int offset)
{
  clear(row, offset, COLUMN_COUNT-offset);
}

void LCDController::clear(int row, int offset, int length)
{
  #if defined(DEV) || defined(DEBUG)
    if (row > ROW_COUNT) {
      #ifdef DEBUG
        Serial.print(F("Row Out Of Bounds (CLEAR): "));
        Serial.println(row);
        Serial.println();
      #endif
      write(0, "E:CR");
      return;
    }
  #endif
  #ifdef DEBUG
    Serial.print(F("Clearing LCD Row: "));
    Serial.print(row);
    Serial.print(F(" Offset Start: "));
    Serial.print(offset);
    Serial.print(F(" Length: "));
    Serial.println(length);
  #endif
  char clearString[length+1];
  //Build a string of space chars, which 'clears' the LCD.
  //Length is determined by offset start position up to
  // the final length of the LCD (COLUMN_LENGTH)
  for (int x = 0; x < length; x++) {
    clearString[x] = 0x20;
  }
  clearString[length] = NULL_TERMINATOR;
  m_pLCD->setCursor(offset, row);
  m_pLCD->print(clearString);
}

void LCDController::clear()
{
  #ifdef DEBUG
    Serial.println(F("Clearing LCD"));
  #endif
  m_pLCD->clear();
}

void LCDController::test()
{
  //REVIEW: Why?
  //Must be a number higher than 1 otherwise compiler 'optimizes'
  // the for loop below which breaks things.
  int testRepeatCount = 2;
  int testDelay = 500;
  #if defined(DEV) || defined(DEBUG)
    #ifdef DEBUG
      Serial.println(F("Testing LCD Screen"));
    #endif
    testRepeatCount = 2;
    testDelay = 1000;
  #endif
  //Build char array of test string.
  char testString[COLUMN_COUNT+1];
  for (int x = 0; x < COLUMN_COUNT; x++) {
    //0xff fills up an entire cell
    testString[x] = 0xff;
  }
  testString[COLUMN_COUNT] = NULL_TERMINATOR;
  //Build a row array consisting of one test string per row.
  char *lcdString[ROW_COUNT];
  for (int x = 0; x < ROW_COUNT; x++) {
    lcdString[x] = testString;
  }
  //Write the row array to LCD, clear,
  // then repeat testRepeatCount times.
  //Delay testDelay ms between populated and cleared LCD.
  for (int x = 0; x < testRepeatCount; x++) {
    write(2, lcdString);
    delay(testDelay);
    clear();
    delay(testDelay);
  }
}
