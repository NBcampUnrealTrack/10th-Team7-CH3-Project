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
	const FVector EndLocation = StartLocation + (GetActorForwardVector() * AttackRange); // 샷건과 동일. 시작점, 끝점  

	TArray<FHitResult> HitResults; //맞은 대상을 담을 배열
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner()); // 샷건 코드와 동일 (무기,캐릭터 제외)

	const bool bHit = GetWorld()->SweepMultiByChannel( //구체를 보내서 여러명이 맞도록 하는 트레이스
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,// FQuat: 회전을 표현하는 방식 , Identity : 회전이 없는 상태. 구체는 회전이 의미 없기 때문에
		ECC_Visibility,
		FCollisionShape::MakeSphere(SwingRadius), // 구체 반지름
		QueryParams // 나머진 샷건과 동일
	);

	if (!bHit) // 안맞은 경우
	{
		return;
	}

	TSet<AActor*> AlreadyHit; // 맞은 적 기록. 중복데미지 방지
	int32 HitCount = 0; // 데미지를 입은 대상 카운트

	for (const FHitResult& Hit : HitResults) //공격에 맞은 대상을 순회
	{
		if (HitCount >= MaxTargetPerSwing)
		{
			break;
		}

		AActor* HitActor = Hit.GetActor(); // 이번 공격에 맞은 액터
		
		if (!HitActor || AlreadyHit.Contains(HitActor)) // 이번에 꺼낸 액터가 처리된 액터면 건너뜀
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