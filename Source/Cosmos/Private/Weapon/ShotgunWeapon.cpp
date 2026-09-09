#include "Weapon/ShotgunWeapon.h"

AShotgunWeapon::AShotgunWeapon()
{
	BaseDamage = 30.f;
	AttackInterval = 1.f;
	AttackRange = 2000.f;
	MaxAmmo = 4;
	CurrentAmmo = MaxAmmo;
	ReloadTime = 2.f;
}

void AShotgunWeapon::PerformAttack()
{
	if (bIsReloading || CurrentAmmo <= 0)
	{
		return;
	}

	CurrentAmmo--;

}
void AShotgunWeapon::ApplyEnchant()
{
	//인챈트 로직
}

void AShotgunWeapon::ApplyUpgrade()
{
	//업그레이드 로직
}

float AShotgunWeapon::GetCurrentDamage() const
{
	return BaseDamage;
}

void AShotgunWeapon::Reload()
{
	if (bIsReloading || CurrentAmmo == MaxAmmo)
	{
		return;
	}

	bIsReloading = true;

	GetWorld()->GetTimerManager().SetTimer(
		ReloadTimerHandle,
		this,
		&AShotgunWeapon::FinishReload,
		ReloadTime,
		false
	);
}

void AShotgunWeapon::FinishReload()
{
	if (!bIsReloading)
	{
		return;
	}
	CurrentAmmo = MaxAmmo;
	bIsReloading = false;
}