#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include "REL/Relocation.h"

#define RELOCATION_OFFSET(SE, AE) REL::VariantOffset(SE, AE, 0).offset()

using namespace std::literals;

#define perfect TimingDodge::DodgeType::kPerfectDodge
#define timing  TimingDodge::DodgeType::kTimingDodge
#define early   TimingDodge::DodgeType::kEarlyDodge
#define late    TimingDodge::DodgeType::kLateDodge
#define none    TimingDodge::DodgeType::kNone