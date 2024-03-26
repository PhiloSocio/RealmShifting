#include "util.h"
#include "RealmShifting.h"

using namespace Util;

static std::jthread dodgeThread;
static std::jthread buffThread; 
static std::jthread timingDodgeThread;

void TimingDodge::SetIsDodging(const float a_tdDurationSec, const float a_pdDurationSec)
{
    isDodging = true;
    isPerfectDodge = true;
        spdlog::info("dodge start");

    dodgeThread = std::jthread([=]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(a_pdDurationSec * 1000)));
        isPerfectDodge = false;
            spdlog::debug("perf dodge end");
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(a_tdDurationSec * 1000)));
        isDodging = false;
            spdlog::info("dodge end");
    });
    dodgeThread.detach();
}
void TimingDodge::SetDodgeState(const TimingDodge::DodgeType a_NewDodgeType)
{
    dodgeType = a_NewDodgeType;
    if (dodgeType == DodgeType::kPerfectDodge)  {spdlog::info("dodge state is perfect dodge."); return;}
    if (dodgeType == DodgeType::kTimingDodge) {spdlog::info("dodge state is timing dodge.");  return;}
    if (dodgeType == DodgeType::kEarlyDodge) {spdlog::info("dodge state is early dodge.");  return;}
    if (dodgeType == DodgeType::kLateDodge) {spdlog::info("dodge state is late dodge.");  return;}
    if (dodgeType == DodgeType::kNone)     {spdlog::info("dodge state is none.");       return;}
}
void TimingDodge::ResetDodgeState(TimingDodge::DodgeType a_CurDodgeType, TimingDodge::DodgeType a_NewDodgeType, float a_delaySeconds)
{
        spdlog::debug("resetting dodge state...");
    timingDodgeThread = std::jthread([=]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(a_delaySeconds * 1000)));
        if (dodgeType == a_CurDodgeType) {
            SetDodgeState(a_NewDodgeType);
            spdlog::debug("resetted dodge state.");
        }
    });
    timingDodgeThread.detach();
}

PRECISION_API::PreHitCallbackReturn TimingDodge::PreHitCallback(const PRECISION_API::PrecisionHitData& a_PrecHitData)
{
    PRECISION_API::PreHitCallbackReturn ret;
    RE::Actor* aggressor = a_PrecHitData.attacker;
    RE::TESObjectREFR* victim = a_PrecHitData.target;
    if (aggressor && !aggressor->IsPlayerRef()) {
        if (victim && victim->IsPlayerRef()) {
            if (isDodging) { ret.bIgnoreHit = true;
                if (isPerfectDodge) {
                    TimingDodgeBuff(true, true);
                    return ret;
                    }
                TimingDodgeBuff(false, true);
                return ret;
            }

        } else  spdlog::debug("Victim is not player");
    } else      spdlog::debug("Attacker is player");
                return ret;
}
void TimingDodge::PostHit(const PRECISION_API::PrecisionHitData& a_PrecHitData, const RE::HitData& a_HitData)
{
    auto aggressor = a_PrecHitData.attacker;
    auto victim = a_PrecHitData.target;
    if (aggressor && !aggressor->IsPlayerRef()) {
        if (victim && victim->IsPlayerRef()) {
            spdlog::info("Timing dodge win closed");
            if (dodgeType != late) {
                SetDodgeState(late);
                ResetDodgeState(dodgeType, DodgeType::kNone, 1.f);
            }
        }
    }
}

void TimingDodge::TimingDodgeBuff(const bool a_isPerfectDodge, const bool a_useThread)
{
    auto AnArchos = RE::PlayerCharacter::GetSingleton();
    if (!AnArchos->HasPerk(RShiftingPerk))
        { spdlog::info("is a timing dodge but you are not have Realm Shifting skill"); return; }
    if (RShiftEffect && AnArchos->AsMagicTarget()->HasMagicEffect(RShiftEffect))
        { spdlog::info("is a timing dodge but you already in buff effect"); return; }
    if (a_isPerfectDodge) 
            spdlog::info("is a perfect dodge!!");
    else    spdlog::info("is a timing dodge!");

    static auto& RShiftEffItem = RShiftingSpell->effects[0]->effectItem;
    const auto buffFlags = RShiftEffect->data.flags;
    if (buffFlags & RE::EffectSetting::EffectSettingData::Flag::kNoDuration) {
        auto rate = RShiftEffItem.magnitude;
        if (a_isPerfectDodge) {
            rate *= 1.5f;
            AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(RShiftingSpell, false, AnArchos, 1.f, false, rate, AnArchos);
            return;
        } else {
            AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(RShiftingSpell, false, AnArchos, 1.f, false, rate, AnArchos);
        }
    }

    static auto& durationRef    = RShiftEffItem.duration;
    const auto defDuration      = durationRef;
    durationRef                 = Config::timeSlowDuration;

    auto rate = Config::timeSlowRate;

    if (!a_useThread) {
        if (a_isPerfectDodge) {
        //  ManipulateUtil::Time::setTimeMult(Config::timeSlowRate, Config::timeSlowDuration * 1.5);
            durationRef *= 1.5f;
            AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(RShiftingSpell, false, AnArchos, 1.f, false, rate, AnArchos);
            //  other buffs
            return;
        } else {
        //  ManipulateUtil::Time::setTimeMult(Config::timeSlowRate, Config::timeSlowDuration);
            AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(RShiftingSpell, false, AnArchos, 1.f, false, rate, AnArchos);
            //  other buffs
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(durationRef * 1000)));
        durationRef     = defDuration;
        return;
    }

    buffThread = std::jthread([=]() {
        if (a_isPerfectDodge) {
        //  ManipulateUtil::Time::setTimeMult(Config::timeSlowRate, Config::timeSlowDuration * 1.5);
            durationRef *= 1.5f;
            AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(RShiftingSpell, false, AnArchos, 1.f, false, rate, AnArchos);
            //  other buffs
            return;
        } else {
        //  ManipulateUtil::Time::setTimeMult(Config::timeSlowRate, Config::timeSlowDuration);
            AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(RShiftingSpell, false, AnArchos, 1.f, false, rate, AnArchos);
            //  other buffs
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(durationRef * 1000)));
        durationRef     = defDuration;
    });
    buffThread.detach();
}
void TimingDodge::Register()
{
    auto playerCharacter = PlayerCharacter::GetSingleton();
    if (!playerCharacter) {spdlog::info("Failed to register {}, because the player is not loaded yet", typeid(BSAnimationGraphEvent).name()); return;}
    bool bSuccess = playerCharacter->AddAnimationGraphEventSink(TimingDodge::GetSingleton());
    if (bSuccess) {
        spdlog::info("Registered {}", typeid(BSAnimationGraphEvent).name());
    } else {
        BSAnimationGraphManagerPtr graphManager;
        playerCharacter->GetAnimationGraphManager(graphManager);
        bool bSinked = false;
        if (graphManager) {         
            for (auto& animationGraph : graphManager->graphs) {
                if (bSinked) {
                    break;
                }
                auto eventSource = animationGraph->GetEventSource<BSAnimationGraphEvent>();
                for (auto& sink : eventSource->sinks) {
                    if (sink == TimingDodge::GetSingleton()) {
                        bSinked = true;
                        break;
                    }
                }
            }
        }
        
        if (!bSinked) {
            spdlog::info("Failed to register {}", typeid(BSAnimationGraphEvent).name());
        }
    }
}
RE::BSEventNotifyControl TimingDodge::ProcessEvent(const BSAnimationGraphEvent* a_event, BSTEventSource<BSAnimationGraphEvent>*)
{
    if (a_event) 
    {
        std::string eventTag = a_event->tag.data();

        switch (hash(eventTag.data(), eventTag.size())) {
        case "MCO_DisableSecondDodge"_h:    // for DMCO 0.9.x
        case "MCO_DodgeInitiate"_h:         // for DMCO 2+
        case "TKDR_DodgeStart"_h:           // for TK Dodge (all versions)
        case "RollTrigger"_h:               // for TUDM (all versions)
        case "SidestepTrigger"_h:           // for TUDM (all versions)
            TimingDodge::SetIsDodging(Config::TdMinusPd, Config::PerfectDodgeWin);

            switch (dodgeType) {
            case none:
                spdlog::info("is an ordinary dodge");
                break;
            case perfect:
                TimingDodgeBuff(true, true);
                break;
            case timing:
                TimingDodgeBuff(false, true);
                break;
            case early:
                spdlog::info("is an early dodge");
                break;
            case late:
                spdlog::info("is a late dodge");
                break;
            }
            break;
        }
    }

        return  RE::BSEventNotifyControl::kContinue;
}
