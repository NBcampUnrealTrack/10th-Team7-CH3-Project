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
	if (CurrentTime - LastAttackTime < AttackInterval) // 공격 딜레이만큼 시간이 지났는지.
	{
		return;
	}

	LastAttackTime = CurrentTime;
	PerformAttack();
}

//HitResult안에 HitActor도 포함
void AWeaponBase::ApplyDamage(AActor* HitActor, const FHitResult& HitResult)
{
	if (!IsValid(HitActor))
	{
		return;
	}

	IDamageable* Damageable = Cast<IDamageable>(HitActor);
	if (Damageable)
	{
		Damageable->TakeDamage(GetCurrentDamage(), HitResult);
	}
}
