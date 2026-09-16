#include "Weapon/NailWeapon.h"
#include "DrawDebugHelpers.h" // 트레이스 시각화

ANailWeapon::ANailWeapon()
{
	BaseDamage = 50.f;
	AttackInterval = 0.3f;
	AttackRange = 200.f;
	MaxTargetPerSwing = 2;
	SwingRadius = 75.f;

	ComboWindowBonus = 0.3f;
	bSwingLeftToRight = false;
	LastSwingTime = -100.f;
}

bool ANailWeapon::PerformAttack()
{
	UpdateSwingDirection();

	FVector StartLocation; 
	FVector Direction;
	if (!GetTraceStartAndDirection(StartLocation, Direction)) //샷건과 동일
	{
		return false;
	}
	const FVector EndLocation = StartLocation + (Direction * GetCurrentRange());

	TArray<FHitResult> HitResults; //맞은 대상을 담을 배열
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetAttachParentActor()); // 샷건 코드와 동일 (무기,캐릭터 제외)

	const bool bHit = GetWorld()->SweepMultiByChannel( //구체를 보내서 여러명이 맞도록 하는 트레이스
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,// FQuat: 회전을 표현하는 방식 , Identity : 회전이 없는 상태. 구체는 회전이 의미 없기 때문에
		ECC_Weapon,
		FCollisionShape::MakeSphere(SwingRadius), // 구체 반지름
		QueryParams // 나머진 샷건과 동일
	);

	DrawDebugLine( // 구체 트레이스 이동경로
		GetWorld(), // 샷건과 동일
		StartLocation,
		EndLocation,
		FColor::Blue,
		false,
		2.f,
		0,
		2.f
	);

	DrawDebugSphere( // 근접 광역 공격
		GetWorld(),
		StartLocation, // 구체 중심좌표 (플레이어 위치)
		SwingRadius, // 반지름
		12, // 구체를 몇각형으로 근사해서 나타낼지
		FColor::Cyan, // 색
		false, // 영구적이지 않음
		2.f // 2초간 표시
	);

	DrawDebugSphere( // 위와 동일
		GetWorld(),
		EndLocation, // 구체 중심좌표 , 이번에는 구체가 이동한 끝지점
		SwingRadius,
		12,
		FColor::Cyan,
		false,
		2.f
	);

	if (!bHit) // 안맞은 경우
	{
		return true; // 안맞아도 모션은 나가야하니 트루
	}

	TSet<AActor*> AlreadyHit; // 맞은 적 기록. 중복데미지 방지
	int32 HitCount = 0; // 데미지를 입은 대상 카운트

	const int32 MaxTarget = GetCurrentMaxTarget();
	for (const FHitResult& Hit : HitResults) //공격에 맞은 대상을 순회
	{
		if (HitCount >= MaxTarget) // 이미 최대치를 맞췄다면 중지
		{
			break;
		}

		AActor* HitActor = Hit.GetActor(); // 이번 공격에 맞은 액터
		
		if (!HitActor || AlreadyHit.Contains(HitActor)) // 이번에 꺼낸 액터가 처리된 액터면 건너뜀
		{
			continue;
		}

		AlreadyHit.Add(HitActor);// 한번 본 액터 중복처리

		if (TryApplyDamage(HitActor)) // 데미지를 준 경우에만 카운트
		{
			HitCount++;
		}
	}

	return true;
}

void ANailWeapon::UpdateSwingDirection() // 좌공격모션 후 몇 초 동안은 우공격 모션이 나오도록.
{
	const float CurrentTime = GetWorld()->GetTimeSeconds(); // 공격속도 로직과 동일
	if (CurrentTime - LastSwingTime <= GetComboWindow())
	{
		bSwingLeftToRight = !bSwingLeftToRight;
	}

	else
	{
		bSwingLeftToRight = true;
	}

	LastSwingTime = CurrentTime;
}

int32 ANailWeapon::GetCurrentMaxTarget() const
{
	return MaxTargetPerSwing + FMath::RoundToInt(GetEnchantStat(EEnchantStat::MeleeMaxTarget));
}

float ANailWeapon::GetCurrentRange() const
{
	return AttackRange * (1.f + GetEnchantStat(EEnchantStat::MeleeRange) * 0.01f);
}