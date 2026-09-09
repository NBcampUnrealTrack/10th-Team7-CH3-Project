#include "Weapon/WeaponBase.h"

AWeaponBase::AWeaponBase()
{
	BaseDamage = 50.f;
	AttackInterval = 0.6f;
	LastAttackTime = -100.f;
}

void AWeaponBase::TryAttack()
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < AttackInterval)
	{
		return;
	}

	LastAttackTime = CurrentTime;
	PerformAttack();
}

void AWeaponBase::ApplyDamage(AActor* HitActor, const FHitResult& HitResult)
{
	if (!HitActor) return;

	if (HitActor->Implements<UDamageable>())
	{
		IDamageable::Execute_TakeDamage(HitActor, GetCurrentDamage(), HitResult);
	}
}
