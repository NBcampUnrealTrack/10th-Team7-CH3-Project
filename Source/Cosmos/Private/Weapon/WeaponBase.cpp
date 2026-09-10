#include "Weapon/WeaponBase.h"
#include "Weapon/Damageable.h"

AWeaponBase::AWeaponBase()
{
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	LastAttackTime = -100.f;
}

void AWeaponBase::TryAttack()
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < AttackInterval) // 공격 딜레이만큼 시간이 지났는지.
	{
		return;
	}

	LastAttackTime = CurrentTime;
	PerformAttack();
}

//HitResult안에 HitActor도 포함
bool AWeaponBase::ApplyDamage(AActor* HitActor, const FHitResult& HitResult)
{
	if (!IsValid(HitActor))
	{
		return false;
	}

	IDamageable* Damageable = Cast<IDamageable>(HitActor); // HitActor가 IDamageable을 포함한다면 데미저블 포인터를 반환
	if (!Damageable) // 데미지를 받을 수 없다면
	{
		return false;
	}

	Damageable->TakeDamage(GetCurrentDamage(), HitResult);
	return true; // 데미지를 준 경우
}
