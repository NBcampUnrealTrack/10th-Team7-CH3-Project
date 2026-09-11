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
	OnAmmoChanged.Broadcast(CurrentAmmo); // 총알 줄어든것 방송

	const FVector StartLocation = GetActorLocation(); //트레이스 시작 위치
	const FVector EndLocation = StartLocation + (GetActorForwardVector() * AttackRange); // 끝나는 위치(시작 + 사거리)

	FHitResult HitResult; // 트레이스 결과를 담을 곳
	FCollisionQueryParams QueryParams; // 트레이스 설정
	QueryParams.AddIgnoredActor(this); // 무기 자체는 맞지 않도록
	QueryParams.AddIgnoredActor(GetOwner()); // 플레이어가 맞지 않도록

	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams); // 라인 트레이스, ECC_Visibility : 시각적 오브젝트 기준 충돌 

	if (bHit) // 무언가에 맞았는가
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

int32 AShotgunWeapon::GetCurrentAmmo() const
{
	return CurrentAmmo;
}

void AShotgunWeapon::Reload()
{
	if (bIsReloading || CurrentAmmo == MaxAmmo) 
	{
		return;
	}

	bIsReloading = true; // 재장전중

	GetWorld()->GetTimerManager().SetTimer( // 재장전하는데 시간이 들도록 함
		ReloadTimerHandle,
		this,
		&AShotgunWeapon::FinishReload, // 시간이 되면 호출할 함수
		ReloadTime, // 드는 시간
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
	OnAmmoChanged.Broadcast(CurrentAmmo); //총알 장전된것 방송
}

EWeaponType AShotgunWeapon::GetWeaponType() const
{
	return EWeaponType::Shotgun;
}