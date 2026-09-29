#include "Weapon/NailWeapon.h"
#include "TimerManager.h"

ANailWeapon::ANailWeapon()
{
	BaseDamage = 50.f;
	AttackInterval = 0.6f;
	AttackRange = 250.f;
	MaxTargetPerSwing = 2;
	SwingRadius = 100.f;

	ComboWindowBonus = 0.3f;
	bSwingLeftToRight = false;
	LastSwingTime = -100.f;

	//히트 스탑
	HitStopDuration = 0.08f;
	HitStopTimeDilation = 0.05f;
	bIsHitStopIncludeOwner = false;
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
	QueryParams.AddIgnoredActor(GetOwner()); // 혹시 몰라서 제외

	FCollisionObjectQueryParams ObjectParams; // 어떤 종류의 오브젝트를 찾을지 지정
	ObjectParams.AddObjectTypesToQuery(ECC_Enemies);
	ObjectParams.AddObjectTypesToQuery(ECC_EnemyHitbox);  // 적 지정

	const bool bHit = GetWorld()->SweepMultiByObjectType( //구체를 보내서 여러명이 맞도록 하는 트레이스
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,// FQuat: 회전을 표현하는 방식 , Identity : 회전이 없는 상태. 구체는 회전이 의미 없기 때문에
		ObjectParams,
		FCollisionShape::MakeSphere(SwingRadius), // 구체 반지름
		QueryParams // 나머진 샷건과 동일
	);

	if (!bHit) // 안맞은 경우
	{
		OnAttackPerformed.Broadcast(bSwingLeftToRight);
		return true; // 안맞아도 모션은 나가야하니 트루
	}

	TSet<AActor*> AlreadyHit; // 맞은 적 기록. 중복데미지 방지
	int32 HitCount = 0; // 데미지를 입은 대상 카운트
	TArray<AActor*> DamagedActors; // [히트스탑] 실제로 데미지를 준 대상

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

		if (TryApplyDamageAlt(HitActor, Hit)) // 데미지를 준 경우에만 카운트
		{
			HitCount++;
			DamagedActors.Add(HitActor); // 히트스탑
		}
	}

	OnAttackPerformed.Broadcast(bSwingLeftToRight);
	if (HitCount > 0)
	{
		const float Scale = FMath::Min(1.f + (HitCount - 1) * 0.2f, 1.6f);
		PlayCameraShake(HitShake, Scale);
		StartHitStop(DamagedActors);
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

void ANailWeapon::StartHitStop(const TArray<AActor*>& Targets)
{
	EndHitStop(); // 이전 히트스탑이 남아있으면 먼저 원래대로 복구

	for (AActor* Target : Targets)
	{
		if (!IsValid(Target))
		{
			continue;
		}
		HitStoppedActors.Emplace(Target, Target->CustomTimeDilation); // 원래 값 저장
		Target->CustomTimeDilation = HitStopTimeDilation;
	}

	if (bIsHitStopIncludeOwner)
	{
		AActor* OwnerActor = GetAttachParentActor(); // 무기를 들고 있는 캐릭터
		if (IsValid(OwnerActor))
		{
			HitStoppedActors.Emplace(OwnerActor, OwnerActor->CustomTimeDilation);
			OwnerActor->CustomTimeDilation = HitStopTimeDilation;
		}
	}

	if (HitStoppedActors.Num() > 0)
	{
		// 월드 시간은 느려지지 않으므로 일반 타이머로 복구 가능
		GetWorldTimerManager().SetTimer(HitStopTimerHandle, this, &ANailWeapon::EndHitStop, HitStopDuration, false);
	}
}

void ANailWeapon::EndHitStop()
{
	GetWorldTimerManager().ClearTimer(HitStopTimerHandle);

	for (const TPair<TWeakObjectPtr<AActor>, float>& Pair : HitStoppedActors)
	{
		if (Pair.Key.IsValid()) // 그 사이 파괴된 적은 건너뜀
		{
			Pair.Key->CustomTimeDilation = Pair.Value;
		}
	}
	HitStoppedActors.Empty();
}

void ANailWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	EndHitStop(); // 무기가 사라져도 적이 멈춘 채로 남지 않게
	Super::EndPlay(EndPlayReason);
}