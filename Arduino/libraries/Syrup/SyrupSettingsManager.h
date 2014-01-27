//Chux
#ifndef SyrupSettingsManager_h
#define SyrupSettingsManager_h

#include "Arduino.h"
#include "EEPROMex.h"
#include "TempProbe.h"
#include "StandardCPlusPlus.h"

//#define DEBUG_HW
#define DEBUG

#define MEMORY_START 100
#define CHECK_BYTE 0x63

class SyrupSettingsManager
{
  public:
    SyrupSettingsManager();
    ~SyrupSettingsManager();
    struct SyrupSettings
    {
      bool operator==(const SyrupSettings& s)
      {
        return ((s.m_upperThreshold == m_upperThreshold)
            && (s.m_lowerThreshold == m_lowerThreshold)
            && (s.m_tempScale == m_tempScale)
            && (s.m_version == m_version));
      }
      void reset()
      {
        m_tainted = false;
        m_upperThreshold = 300.0;
        m_lowerThreshold = 0.0;
        m_version = 1;
      }
      bool m_tainted;
      float m_upperThreshold;
      float m_lowerThreshold;
      TempProbe::e_scale m_tempScale;
      int m_version;
      SyrupSettings()
      {
        reset();
      }
    } m_settings;
    SyrupSettings* load();
    void save(SyrupSettings &syrupSettings);
    void prime();
    void resetClear();
  private:
    void findStorageAddresses();
    int m_storageAddresses[3];
    bool m_isLoaded;
    bool m_isUseless;
    bool verifyIntegrity(bool loadSettings);
    void taintAndRePrime();
};

#endif