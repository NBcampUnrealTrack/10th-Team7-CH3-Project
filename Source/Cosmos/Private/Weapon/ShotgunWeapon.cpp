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

	const FVector StartLocation = GetActorLocation(); //트레이스 시작 위치
	const FVector EndLocation = StartLocation + (GetActorForwardVector() * AttackRange); // 끝나는 위치(시작 + 사거리)

	FHitResult HitResult;
	FCollisionQueryParams QueryParams; // 트레이스 설정
	QueryParams.AddIgnoredActor(this); // 무기 자체는 맞지 않도록
	QueryParams.AddIgnoredActor(GetOwner()); // 플레이어가 맞지 않도록

	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams); // 라인 트레이스

	if (bHit)
	{
		ApplyDamage(HitResult.GetActor(), HitResult);
	}

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