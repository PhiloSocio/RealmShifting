#pragma once

class ProjectileHook	// credits to https://github.com/ersh1
{
public:
	static void Hook()
	{
		spdlog::info("ProjectileHook hooking...");
		REL::Relocation<std::uintptr_t> ProjectileVtbl{ RE::VTABLE_Projectile[0] };				    // 167C888
		REL::Relocation<std::uintptr_t> ArrowProjectileVtbl{ RE::VTABLE_ArrowProjectile[0] };	    // 1676318
		REL::Relocation<std::uintptr_t> MissileProjectileVtbl{ RE::VTABLE_MissileProjectile[0] };	// 1676318
		_GetLinearVelocityProjectile= ProjectileVtbl.write_vfunc(0x86, GetLinearVelocityProjectile);
		_GetLinearVelocityArrow 	= ArrowProjectileVtbl.write_vfunc(0x86, GetLinearVelocityArrow);
		_GetLinearVelocityMissile 	= MissileProjectileVtbl.write_vfunc(0x86, GetLinearVelocityMissile);
		_CollisionProjectile 		= ProjectileVtbl.write_vfunc(0xBE, CollisionProjectile);
		_CollisionArrow 			= ArrowProjectileVtbl.write_vfunc(0xBE, CollisionArrow);
		_CollisionMissile 			= MissileProjectileVtbl.write_vfunc(0xBE, CollisionMissile);

		spdlog::info("ProjectileHook done, long live Ersh!");
	}

private:
	static inline RE::FormID lastProjID;
	static inline bool projPassed = false;

	static void GetLinearVelocityProjectile	(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity);
	static void GetLinearVelocityArrow		(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity);
	static void GetLinearVelocityMissile	(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity);
	static void CollisionProjectile	(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);
	static void CollisionArrow		(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);
	static void CollisionMissile	(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);

	static void directionCheck(const RE::Projectile* a_this);
	static bool hitCheck(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);

	static inline REL::Relocation<decltype(GetLinearVelocityProjectile)> _GetLinearVelocityProjectile;
	static inline REL::Relocation<decltype(GetLinearVelocityArrow)> _GetLinearVelocityArrow;
	static inline REL::Relocation<decltype(GetLinearVelocityMissile)> _GetLinearVelocityMissile;
	static inline REL::Relocation<decltype(CollisionProjectile)> _CollisionProjectile;
	static inline REL::Relocation<decltype(CollisionArrow)> _CollisionArrow;
	static inline REL::Relocation<decltype(CollisionMissile)> _CollisionMissile;
};
