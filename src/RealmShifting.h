#pragma once
#include "settings.h"
#include "hook.h"

class TimingDodge : public RE::BSTEventSink<RE::BSAnimationGraphEvent>
{
public:
    static TimingDodge* GetSingleton() {static TimingDodge singleton; return &singleton;};

    void Register();
    virtual RE::BSEventNotifyControl ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) override;

    enum class DodgeType : uint8_t
    {
        kNone,
        kEarlyDodge,
        kTimingDodge,
        kPerfectDodge,
        kLateDodge
    };
    static inline DodgeType dodgeType	= DodgeType::kNone;
    static inline bool isDodging 		= false;
    static inline bool isPerfectDodge 	= false;

    static inline RE::TESObjectARMO*	RingOf9Realms	= nullptr;
    static inline RE::BGSPerk*			RShiftingPerk	= nullptr;
    static inline RE::SpellItem*		RShiftingSpell	= nullptr;
    static inline RE::EffectSetting*	RShiftEffect	= nullptr;

    static void SetIsDodging	(const float a_tdDurationSec, const float a_pdDurationSec);
    static void TimingDodgeBuff (const bool a_isPerfectDodge, const bool a_useThread);
    static void SetDodgeState	(const DodgeType a_NewDodgeType);
    static void ResetDodgeState (DodgeType a_CurDodgeType, DodgeType a_NewDodgeType, float a_delaySeconds);

    static PRECISION_API::PreHitCallbackReturn PreHitCallback(const PRECISION_API::PrecisionHitData& a_PrecHitData);
    static void PostHit(const PRECISION_API::PrecisionHitData& a_hitData, const RE::HitData&);

private:
    static inline std::mutex dodgeMutex;
};