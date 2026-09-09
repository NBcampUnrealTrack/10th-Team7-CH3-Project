#include "Weapon/ShotgunWeapon.h"

AShotgunWeapon::AShotgunWeapon()
{
	BaseDamage = 30.f;
	AttackInterval = 1.f;
	MaxAmmo = 4;
	CurrentAmmo = MaxAmmo;
	ReloadTime = 2.f;
}

void AShotgunWeapon::PerformAttack()
{
	//원거리 트레이스 로직
}

float AShotgunWeapon::GetCurrentDamage() const
{
	return BaseDamage;
}

void AShotgunWeapon::Reload()
{
	//재장전 로직
}