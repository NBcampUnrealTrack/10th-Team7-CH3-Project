#include "Weapon/NailWeapon.h"

ANailWeapon::ANailWeapon()
{
	BaseDamage = 50.f;
	AttackInterval = 0.6f;
}

void ANailWeapon::PerformAttack()
{
	// 근접 트레이스 로직
}

void ANailWeapon::ApplyEnchant()
{
	//인챈트 로직
}

float ANailWeapon::GetCurrentDamage() const
{
	return BaseDamage; 
}