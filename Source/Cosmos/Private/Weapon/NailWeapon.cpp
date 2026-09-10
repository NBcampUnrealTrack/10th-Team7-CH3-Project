#include "Weapon/NailWeapon.h"

ANailWeapon::ANailWeapon()
{
	BaseDamage = 50.f;
	AttackInterval = 0.6f;
	AttackRange = 200.f;
	MaxTargetPerSwing = 2;
}

void ANailWeapon::PerformAttack()
{
	const FVector StartLocation = GetActorLocation();
	const FVector EndLocation = StartLocation + (GetActorForwardVector() * AttackRange);

	TArray<FHitResult> HitResults; //맞은 대상을 담을 배열
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner()); // 샷건 코드와 동일 (무기,캐릭터 제외)

	const bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(SwingRadius),
		QueryParams
	);

	if (!bHit)
	{
		return;
	}

	TSet<AActor*> AlreadyHit;
	int32 HitCount = 0;

	for (const FHitResult& Hit : HitResults)
	{
		if (HitCount >= MaxTargetPerSwing)
		{
			break;
		}

		AActor* HitActor = Hit.GetActor();
		
		if (!HitActor || AlreadyHit.Contains(HitActor))
		{
			continue;
		}

		AlreadyHit.Add(HitActor);// 한번 본 액터 중복처리

		if (ApplyDamage(HitActor, Hit)) // 데미지를 준 경우에만 카운트
		{
			HitCount++;
		}
	}
}

void ANailWeapon::ApplyEnchant()
{
	//인챈트 로직
}

float ANailWeapon::GetCurrentDamage() const
{
	return BaseDamage; 
}