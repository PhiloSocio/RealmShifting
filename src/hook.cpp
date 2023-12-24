#include "hook.h"
#include "util.h"
#include "settings.h"
#include "RealmShifting.h"

static std::mutex 	projMutex;
static std::jthread projThread;

inline void _ResetDodgeState(TimingDodge::DodgeType a_CurDodgeType, TimingDodge::DodgeType a_NewDodgeType, float a_delaySeconds)
{
			spdlog::debug("resetting dodge state...");
	projThread = std::jthread([=]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(a_delaySeconds * 1000)));
		if (TimingDodge::dodgeType == a_CurDodgeType) {
			TimingDodge::SetDodgeState(a_NewDodgeType);
			spdlog::debug("resetted dodge state.");
		}
	});
	projThread.detach();
}

void ProjectileHook::directionCheck(const RE::Projectile* a_this)
{
	auto projID = a_this->GetFormID();
	if (lastProjID && projID == lastProjID && projPassed)	{ spdlog::debug("proj passed and going..."); return; }
	lastProjID = projID;
	projPassed = false;

	auto projectileNode = a_this->Get3D2();
	auto AnArchos = RE::PlayerCharacter::GetSingleton();
	if (!projectileNode)
		return; //{ spdlog::debug("incoming projectile not 3D loaded"); return; }
    if (TimingDodge::RShiftingPerk && !AnArchos->HasPerk(TimingDodge::RShiftingPerk))
		return; //{ spdlog::debug("projectile coming but you are not have Realm Shifting skill"); return; }
    if (TimingDodge::RShiftEffect && AnArchos->AsMagicTarget()->HasMagicEffect(TimingDodge::RShiftEffect)) 
		return; //{ spdlog::debug("projectile coming but you already in buff effect"); return; }
	if (AnArchos->IsInRagdollState())
		return; //{ spdlog::debug("projectile coming but you are in ragdoll state"); return; }

	auto& runtimeData = a_this->GetProjectileRuntimeData();
	auto& shooter = runtimeData.shooter;
	auto& desiredTarget = runtimeData.desiredTarget;

	if (shooter.native_handle() != 0x100000 
	&& (shooter.get().get()->As<RE::Actor>()->IsHostileToActor(AnArchos) || desiredTarget.native_handle() == 0x100000)) // except player and friends, 0x100000 = player
	{
		float livingTime = runtimeData.livingTime;
		if (livingTime > 36.f) return;

		auto velocity = runtimeData.linearVelocity;
		float speed = velocity.Length();
		if (speed < Config::minComingSpeed) return;

		auto& projPos = a_this->data.location;
		auto playerBone = AnArchos->GetNodeByName("NPC Neck [Neck]");	// alt: NPC Pelvis [Pelv] , NPC Spine [Spn0] , NPC Spine1 [Spn1] , NPC Spine2 [Spn2]
		auto playerPos = playerBone->world.translate;
		auto playerDir = (playerPos - projPos);
	//		spdlog::debug("incoming projectile offset: {}, {}, {}", offsetVec.x, offsetVec.y, offsetVec.z);
		float offset = playerDir.Length();
		float offsetAngle = acos(velocity.Dot(playerDir) / speed / offset);			//	cos(offsetAngle) = (velocity . playerDir) / (speed * offset); 	/|	/: offset, |: speed
		if (static_cast<int>(livingTime * 10) % 2 < 1)	spdlog::debug("incoming projectile offset angle: {}", offsetAngle / 0.0175f);

		if (offsetAngle >= Config::projDirectionOffset) {
			if (TimingDodge::dodgeType != none) TimingDodge::SetDodgeState(none);
			return;
		}

		float distance = playerPos.GetDistance(projPos);
		float timeToHit = distance / speed;
		if (distance <= 60 || timeToHit < 0.02f) {projPassed = true; return;}		// return before the hit.
		if (timeToHit <= Config::TimingDodgeWin) {
			if (timeToHit <= Config::PerfectDodgeWin) {
				if (TimingDodge::dodgeType != perfect) {
					TimingDodge::SetDodgeState(perfect);
					_ResetDodgeState(perfect, none, Config::PerfectDodgeWin);
				}
					return;
			}
			if (TimingDodge::dodgeType != timing) {
				TimingDodge::SetDodgeState(timing);
				_ResetDodgeState(timing, none, Config::TimingDodgeWin);
			}
				return;
		}
		if (TimingDodge::dodgeType == early || TimingDodge::dodgeType == none) return;
		TimingDodge::SetDodgeState(early);
		_ResetDodgeState(early, none, 1.f);
	}
}

void ProjectileHook::GetLinearVelocityProjectile(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity)
{
	_GetLinearVelocityProjectile(a_this, a_outVelocity);

	if (projMutex.try_lock()) { directionCheck(a_this); projMutex.unlock(); }
			
}
void ProjectileHook::GetLinearVelocityArrow(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity)
{
	_GetLinearVelocityArrow(a_this, a_outVelocity);

	if (projMutex.try_lock()) { directionCheck(a_this); projMutex.unlock(); }
			
}
void ProjectileHook::GetLinearVelocityMissile(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity)
{
	_GetLinearVelocityMissile(a_this, a_outVelocity);

	if (projMutex.try_lock()) { directionCheck(a_this); projMutex.unlock(); }
			
}

/*collision*/
void ProjectileHook::CollisionProjectile(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
	if (hitCheck(a_this, a_AllCdPointCollector)) return;

	_CollisionProjectile(a_this, a_AllCdPointCollector);
}
void ProjectileHook::CollisionArrow(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
	if (hitCheck(a_this, a_AllCdPointCollector)) return;

	_CollisionArrow(a_this, a_AllCdPointCollector);
}
void ProjectileHook::CollisionMissile(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
	if (hitCheck(a_this, a_AllCdPointCollector)) return;

	_CollisionMissile(a_this, a_AllCdPointCollector);
}

bool ProjectileHook::hitCheck(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
{
	if (!a_AllCdPointCollector) 
			return false;

	for (auto& point : a_AllCdPointCollector->hits) {
		auto collidableA = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableA);
		auto collidableB = RE::TESHavokUtilities::FindCollidableRef(*point.rootCollidableB);

		if (collidableA && collidableA->IsPlayerRef())
			return TimingDodge::RShiftEffect && RE::PlayerCharacter::GetSingleton()->AsMagicTarget()->HasMagicEffect(TimingDodge::RShiftEffect);

		if (collidableB && collidableB->IsPlayerRef())
			return TimingDodge::RShiftEffect && RE::PlayerCharacter::GetSingleton()->AsMagicTarget()->HasMagicEffect(TimingDodge::RShiftEffect);
	} 		return false;
}