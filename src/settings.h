#pragma once
#include "SimpleIni.h"
#include "API/PrecisionAPI.h"

class Config
{
public:
    static inline bool isPrecisionInstalled		= false;

    static inline std::string hitEvent			= "HitFrame";
    static inline std::string preHitEvent		= "preHitFrame";

    static inline float timeSlowRate			= 0.08f;	//	1
    static inline float timeSlowDuration		= 4.f;		//	s
    static inline float TimingDodgeWin			= 0.25f;	//	s
    static inline float PerfectDodgeWin			= 0.1f;	 	//	s
    static inline float projDirectionOffset		= 10.f;		//	degrees
    static inline float minComingSpeed			= 1000.f;	//	cm / s

    static inline float TdMinusPd;	 //	s	= TimingDodgeWin - PerfectDodgeWin

    static inline std::string RingOf9RealmsEID	= "RealmShiftingRing";
    static inline std::string RShiftingPerkEID	= "RealmShiftingSkill";
    static inline std::string RShiftingSpellEID = "RealmShiftSpell";

    static inline std::string RealmShiftingESP	= "RealmShifting.esp";
    static inline std::string MagicModESP		= "Slow Time Edit.esp";
    static inline uint32_t BuffSpellFormID		= 0x800;

    static void CheckConfig();
    static bool GetObjectsFromData();

private:
    static void ReadStringSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, std::string& a_setting);
    static void ReadFloatSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, float& a_setting);
    static void ReadIntSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, uint32_t& a_setting);
//	static void ReadBoolSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, bool& a_setting);
};
