//Chux
//Note: For v2, make this only dirty sections that are dirty instead of all 3 and copy cause thats just naughty, also write one extra for the hell of it when saving even though we only need to verify 3 successes to pass.
#include "SyrupSettingsManager.h"

SyrupSettingsManager::SyrupSettingsManager()
{
  for (int x = 0; x < 3; x++) {
    m_storageAddresses[x] = 0;
  }
  m_isLoaded = false;
  m_isUseless = false;
}

SyrupSettingsManager::~SyrupSettingsManager()
{
}

void SyrupSettingsManager::save()
{
  if (m_isUseless) {
    return;
  }
  #ifdef DEBUG
    Serial.println(F("Attempting to save settings."));
  #endif
  for (int x = 0; x < 3; x++) {
    EEPROM.writeBlock(m_storageAddresses[x], m_settings);
  }
  if (!verifyIntegrity(false)) {
    taintAndRePrime();
    save();
  }
}

bool SyrupSettingsManager::verifyIntegrity(bool loadSettings)
{
  SyrupSettings settings1;
  EEPROM.readBlock(m_storageAddresses[0], settings1);
  SyrupSettings settings2;
  EEPROM.readBlock(m_storageAddresses[1], settings2);
  SyrupSettings settings3;
  EEPROM.readBlock(m_storageAddresses[2], settings3);
  if (settings1.m_tainted || settings2.m_tainted || settings3.m_tainted) {
    #ifdef DEBUG
      Serial.println(F("Detected taint flag. Integrity check failure."));
    #endif
    return false;
  }
  if (settings1 == settings2 && settings2 == settings3) {
    if (loadSettings) {
      m_settings = settings3;
      m_isLoaded = true;
    }
    #ifdef DEBUG
      Serial.println(F("Verified Settings Integrity"));
    #endif
    return true;
  }
  if (settings2 == settings1 || settings2 == settings3) {
    #ifdef DEBUG
      Serial.println(F("Verified Settings-2 Integrity for recovery."));
    #endif
    if (loadSettings) {
      m_settings = settings2;
      m_isLoaded = true;
    }
  }
  if (settings1 == settings3) {
    #ifdef DEBUG
      Serial.println(F("Verified Settings-1 Integrity for recovery."));
    #endif
    if (loadSettings) {
      m_settings = settings1;
      #ifdef DEV
        Serial.println(F("SETTINGS:"));
        Serial.println(m_settings.m_upperThreshold);
        Serial.println(m_settings.m_lowerThreshold);
        Serial.println(m_settings.m_tempScale);
      #endif
      m_isLoaded = true;
    }
  }
  #ifdef DEBUG
    Serial.println(F("Integrity Check Failed!"));
  #endif
  return false;
}

void SyrupSettingsManager::resetClear()
{
  EEPROM.setMemPool(MEMORY_START, EEPROMSizeUno);
  findStorageAddresses();
  m_settings.reset();
  m_isLoaded = false;
  m_isUseless = false;
  save();
}

void SyrupSettingsManager::taintAndRePrime()
{
  SyrupSettings syrupSettings;
  syrupSettings.m_tainted = true;
  for (int x = 0; x < 3; x++) {
    SyrupSettings settingsRead;
    EEPROM.readBlock(m_storageAddresses[x], settingsRead);
    if (!settingsRead.m_tainted) {
      EEPROM.writeBlock(m_storageAddresses[x], syrupSettings);
    }
  }
  findStorageAddresses();
}

void SyrupSettingsManager::prime()
{
  EEPROM.setMemPool(MEMORY_START, EEPROMSizeUno);
  findStorageAddresses();
  m_isLoaded = false;
  m_isUseless = false;
  load();
}

void SyrupSettingsManager::findStorageAddresses()
{
  if (m_isUseless) {
    return;
  }
  for (int x = 0; x < 3; x++) {
    m_storageAddresses[x] = EEPROM.getAddress(sizeof(SyrupSettings));
    if (m_storageAddresses[x] > 1000) {
      #ifdef DEBUG
        Serial.println(F("EEPROM Exhausted!"));
      #endif
      m_isUseless = true;
      m_isLoaded = true;
      m_settings.reset();
      return;
    }
  }
  #ifdef DEBUG
    Serial.print(F("Storage ADDRS: "));
    Serial.print(m_storageAddresses[0]);
    Serial.print(F(" : "));
    Serial.print(m_storageAddresses[1]);
    Serial.print(F(" : "));
    Serial.println(m_storageAddresses[2]);
  #endif
}

void SyrupSettingsManager::load()
{
  if (m_isLoaded) {
    return;
  }
  #ifdef DEBUG
    Serial.println(F("Attempting to load settings."));
  #endif
  if (!verifyIntegrity(true)) {
    if (m_isLoaded) {
      taintAndRePrime();
      save();
    }
    else {
      findStorageAddresses();
      load();
    }
  }
}

