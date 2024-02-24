#include "settings.h"
#include "RealmShifting.h"
#define ConfigPath "Data\\SKSE\\Plugins\\RealmShifting.ini"

void Config::ReadStringSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, std::string& a_setting)
{
	const char* bFound = nullptr;
	bFound = a_ini.GetValue(a_sectionName, a_settingName);
	if (bFound) {
		//spdlog::info("found {} with value {}", a_settingName, bFound);
		a_setting = a_ini.GetValue(a_sectionName, a_settingName);
	}
}

void Config::ReadFloatSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, float& a_setting)
{
	const char* bFound = nullptr;
	bFound = a_ini.GetValue(a_sectionName, a_settingName);
	if (bFound) {
		//spdlog::info("found {} with value {}", a_settingName, bFound);
		a_setting = static_cast<float>(a_ini.GetDoubleValue(a_sectionName, a_settingName));
	}
}

void Config::ReadIntSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, uint32_t& a_setting)
{
	const char* bFound = nullptr;
	bFound = a_ini.GetValue(a_sectionName, a_settingName);
	if (bFound) {
		//spdlog::info("found {} with value {}", a_settingName, bFound);
		a_setting = static_cast<int>(a_ini.GetDoubleValue(a_sectionName, a_settingName));
	}
}
/*
void Config::ReadBoolSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, bool& a_setting)
{
	const char* bFound = nullptr;
	bFound = a_ini.GetValue(a_sectionName, a_settingName);
	if (bFound) {
		//spdlog::info("found {} with value {}", a_settingName, bFound);
		a_setting = a_ini.GetBoolValue(a_sectionName, a_settingName);
	}
}
*/

void Config::CheckConfig()
{
	CSimpleIniA ini;
	ini.LoadFile(ConfigPath);

	ReadFloatSetting( ini, "Main", 	"fTimeSlowRate", 		timeSlowRate);
	ReadFloatSetting( ini, "Main", 	"fTimeSlowDuration",	timeSlowDuration);
	ReadFloatSetting( ini, "Main", 	"fTimingDodgeWin", 		TimingDodgeWin);
	ReadFloatSetting( ini, "Main", 	"fPerfectDodgeWin",		PerfectDodgeWin);
	ReadFloatSetting( ini, "Main", 	"fProjDirectionOffset",	projDirectionOffset);
	ReadFloatSetting( ini, "Main", 	"fMinComingSpeed",		minComingSpeed);

	ReadStringSetting(ini, "Main", 	"sRingOf9RealmsEID", 	RingOf9RealmsEID);
	ReadStringSetting(ini, "Main", 	"sRShiftingPerkEID", 	RShiftingPerkEID);
	ReadStringSetting(ini, "Main", 	"sRShiftingSpellEID", 	RShiftingSpellEID);
	ReadStringSetting(ini, "Main",	"sMagicModESP",			MagicModESP);

	ReadIntSetting   (ini, "Main", 	"iBuffSpellFormID",		BuffSpellFormID);

	if (timeSlowRate < 0.0001f)		timeSlowRate = 0.0001f;		//	min rate is 0.01%, why? why not?
	if (timeSlowRate > 4.f)			timeSlowRate = 2.f;			//	yes i let speed up time option
	if (timeSlowDuration < 0.1f)	timeSlowDuration = 0.1f;	//	min duration is 0.1s
	if (timeSlowDuration > 360.f)	timeSlowDuration = 360.f;	//	max duration is 6 minutes
	if (projDirectionOffset < 0.5f)	projDirectionOffset = 0.5f;	//	min offset is 0.5 degrees
	if (projDirectionOffset > 180.f)projDirectionOffset = 180.f;//	min offset is 0.5 degrees
		projDirectionOffset *= 0.0174532925f;					//	convert to radians

	TdMinusPd = TimingDodgeWin - PerfectDodgeWin;		//	calculate for fast access

	if (ini.GetBoolValue("Main", "DebugModeOpen")) {
		spdlog::set_level(spdlog::level::debug);
		spdlog::debug("Debug mode enabled");
	} else spdlog::set_level(spdlog::level::info);

	spdlog::info("Kratos Combat's configurations checked.");
}

bool Config::GetObjectsFromData()
{
	auto dataHandler = RE::TESDataHandler::GetSingleton();
	TimingDodge::RingOf9Realms 	= dataHandler->LookupForm<RE::TESObjectARMO>(0x802, RealmShiftingESP);
	TimingDodge::RShiftingPerk 	= dataHandler->LookupForm<RE::BGSPerk>		(0x803, RealmShiftingESP);
//	TimingDodge::RShiftingSpell = dataHandler->LookupForm<RE::SpellItem>	(0x800, RealmShiftingESP);
	TimingDodge::RShiftingSpell = dataHandler->LookupForm<RE::SpellItem>	(BuffSpellFormID, MagicModESP);

	if (TimingDodge::RingOf9Realms)		spdlog::info("Ring of Nine Realms is {}", 	TimingDodge::RingOf9Realms->GetName());
	else {	spdlog::warn("Can't find the Ring of Nine Realms"); return false;}
	if (TimingDodge::RShiftingPerk)		spdlog::info("Realm Shifting Perk is {}", 	TimingDodge::RShiftingPerk->GetName());
	else {	spdlog::warn("Can't find the Realm Shifting Perk"); return false;}
	if (TimingDodge::RShiftingSpell)  {	spdlog::info("Realm Shifting Spell is {}", 	TimingDodge::RShiftingSpell->GetName());}
	else {
			spdlog::warn("Can't find the Realm Shifting Spell");
			spdlog::info("Changing spell to default slow time spell...");
		TimingDodge::RShiftingSpell = dataHandler->LookupForm<RE::SpellItem>	(0x48AD0, "Skyrim.esm");
		if (TimingDodge::RShiftingSpell)
			spdlog::info("Spell changed to default. Look mod page for recommended slow time mod.");
		else {
			spdlog::warn("WEIRD Can't find the default slow time spell!");
			return false;
		}
	}
	TimingDodge::RShiftEffect	= TimingDodge::RShiftingSpell->effects[0] ? TimingDodge::RShiftingSpell->effects[0]->baseEffect : nullptr;

	const auto CastingType = TimingDodge::RShiftingSpell->data.castingType;
	if (CastingType == RE::MagicSystem::CastingType::kConstantEffect || CastingType == RE::MagicSystem::CastingType::kConcentration) {
		spdlog::error("The buff spell's casting type cannot be constant effect or concentration! Please select another one.");
		return false;
	}
	return true;
}
