#include "logger.h"
#include "settings.h"
#include "RealmShifting.h"

void RequestAPIs()
{
    auto precision = reinterpret_cast<PRECISION_API::IVPrecision1*>(PRECISION_API::RequestPluginAPI(PRECISION_API::InterfaceVersion::V1));
        if (precision) {
            spdlog::info("Requesting Precision API ok");
            auto resultPreHit 	= precision->AddPreHitCallback(SKSE::GetPluginHandle(), TimingDodge::PreHitCallback);
            auto resultPostHit 	= precision->AddPostHitCallback(SKSE::GetPluginHandle(), TimingDodge::PostHit);
            Config::isPrecisionInstalled = true;

            if (resultPreHit == PRECISION_API::APIResult::OK)
                    spdlog::info("Precision API: PreHitCallback registered");
            else if (resultPreHit == PRECISION_API::APIResult::AlreadyRegistered)
                    spdlog::info("Precision API: PreHitCallback already registered");
            else 	spdlog::info("Precision API: PreHitCallback not registered");

            if (resultPostHit == PRECISION_API::APIResult::OK)
                    spdlog::info("Precision API: PostHitCallback registered");
            else if (resultPostHit == PRECISION_API::APIResult::AlreadyRegistered)
                    spdlog::info("Precision API: PostHitCallback already registered");
            else 	spdlog::info("Precision API: PostHitCallback not registered");
        } else {
            Config::isPrecisionInstalled = false;
                    spdlog::info("Requesting Precision API not ok");
        }
}

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
    switch (a_msg->type) {
    case SKSE::MessagingInterface::kDataLoaded:
        Config::CheckConfig();
        if (!Config::GetObjectsFromData()) {spdlog::error("RealmShifting installation stopped"); return;}
        ProjectileHook::Hook();
        break;
    case SKSE::MessagingInterface::kPostLoad:
        RequestAPIs();
        break;
    case SKSE::MessagingInterface::kPreLoadGame:
        break;
    case SKSE::MessagingInterface::kPostLoadGame:
    case SKSE::MessagingInterface::kNewGame:
        TimingDodge::GetSingleton()->Register();
        break;
    }
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {

    SetupLog();

    auto plugin  = SKSE::PluginDeclaration::GetSingleton();
    spdlog::info("{} v{} is loading...", plugin->GetName(), plugin->GetVersion());

    SKSE::Init(skse);

    auto messaging = SKSE::GetMessagingInterface();
    if (!messaging->RegisterListener("SKSE", MessageHandler)) {
        return false;
    }

    spdlog::info("{} by {} has finished loading. Support for more mods! {}", plugin->GetName(), plugin->GetAuthor(), plugin->GetSupportEmail());

    return true;
}
/**/
SKSEPluginInfo(
    .Version = REL::Version{ 1, 1, 3, 0 },
    .Name = "RealmShifting"sv,
    .Author = "AnArchos"sv,
    .SupportEmail = "patreon.com/AnArchos"sv,
    .StructCompatibility = SKSE::StructCompatibility::Independent,
    .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary,
    .MinimumSKSEVersion = REL::Version{ 2, 0, 0, 2 }
)

