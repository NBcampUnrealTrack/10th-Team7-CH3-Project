#include "Weapon/WeaponBase.h"
#include "Weapon/Damageable.h"

AWeaponBase::AWeaponBase()
{
	BaseDamage = 50.f;
	AttackInterval = 0.6f;
	LastAttackTime = -100.f;
	AttackRange = 200.f;
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

//HitResult안에 HitActor도 포함
void AWeaponBase::ApplyDamage(AActor* HitActor, const FHitResult& HitResult)
{
	if (!HitActor)
	{
		return;
	}

	IDamageable* Damageable = Cast<IDamageable>(HitActor);
	if (Damageable)
	{
		Damageable->TakeDamage(GetCurrentDamage(), HitResult);
	}
}
