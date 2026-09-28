#include "Weapon/ShotgunWeapon.h"
#include "Data/CosGameInstance.h"

AShotgunWeapon::AShotgunWeapon()
{
	BaseDamage = 30.f;
	AttackInterval = 1.f;
	AttackRange = 3000.f;
	MaxAmmo = 4;
	ReloadTime = 2.f;
}

void AShotgunWeapon::BeginPlay()
{
	Super::BeginPlay(); // 오버라이드한 함수에서 Super:: 부르는건 거의 항상 옳음

	if (UWorld* World = GetWorld()) // 현재 월드를 포인터로 받아오면서 널검사
	{
		if (UCosGameInstance* GameInstance = Cast<UCosGameInstance>(World->GetGameInstance())) // 만들어둔 World로 게임인스턴스 찾고 그거를 CosGameInstance로 다운캐스트
		{
			GameInstance->OnLoadoutChange.AddDynamic(this, &AShotgunWeapon::OnLoadoutChanged); //인챈트 구성이 바뀌면 this에게 탄약 바뀌었다는 함수 호출하라고 알림
		}
	}

	CurrentAmmo = GetCurrentMaxAmmo(); // 생성자 시점에는 인챈트를 알 수 없기에 여기서 초기화
	BroadcastAmmo(); // 방송
}

bool AShotgunWeapon::PerformAttack()
{

	if (bIsReloading || CurrentAmmo <= 0) 
	{
		return false;
	}
	PlayCameraShake(ShotgunShake);
	FVector StartLocation;
	FVector Direction;
	if (!GetTraceStartAndDirection(StartLocation, Direction)) // 시작점, 방향 설정
	{
		return false;
	}

	CurrentAmmo--;
	BroadcastAmmo(); // 총알 줄어든것 방송

	const FVector EndLocation = StartLocation + (Direction * AttackRange); //끝점

	FHitResult HitResult; // 트레이스 결과를 담을 곳
	FCollisionQueryParams QueryParams; // 트레이스 설정
	QueryParams.AddIgnoredActor(this); // 무기 자체는 맞지 않도록
	QueryParams.AddIgnoredActor(GetAttachParentActor()); // 플레이어가 맞지 않도록

	const bool bHit = GetWorld()->SweepSingleByChannel(// 구체 트레이스
		HitResult, // 결과 담는 곳
		StartLocation, // 시작 
		EndLocation, // 끝
		FQuat::Identity, // 구체라 회전 의미 없음
		ECC_Weapon, // 무기 판정 전용 채널
		FCollisionShape::MakeSphere(ShotRadius), // 선 굵기
		QueryParams // 트레이스 설정
	); 

	if (bHit) // 무언가에 맞았는가
	{
		TryApplyDamageAlt(HitResult.GetActor(), HitResult);
		
	}

	OnAttackPerformed.Broadcast(true);
	return true;
}

void AShotgunWeapon::Reload()
{
	if (bIsReloading || CurrentAmmo == GetCurrentMaxAmmo())
	{
		return;
	}

	bIsReloading = true; // 재장전중
	OnReloadStarted();
	OnReloadPerformed.Broadcast();

	GetWorld()->GetTimerManager().SetTimer( // 재장전하는데 시간이 들도록 함
		ReloadTimerHandle,
		this,
		&AShotgunWeapon::FinishReload, // 시간이 되면 호출할 함수
		GetCurrentReloadTime(), // 드는 시간
		false
	);
}

void AShotgunWeapon::FinishReload()
{
	if (!bIsReloading)
	{
		return;
	}
	CurrentAmmo = GetCurrentMaxAmmo();
	bIsReloading = false;
	BroadcastAmmo(); //총알 장전된것 방송
	OnReloadFinished();
	OnReloadFinishedPerformed.Broadcast();
}

int32 AShotgunWeapon::GetCurrentAmmo() const
{
	return CurrentAmmo;
}

int32 AShotgunWeapon::GetCurrentMaxAmmo() const
{
	return MaxAmmo + FMath::RoundToInt(GetEnchantStat(EEnchantStat::RangeMaxAmmo)); // 인챈트로 최대 탄약이 늘었을 수도 있으니 더해서 알려줌
}                                                                                   // 인챈트값이 float이라 인트로 변환

float AShotgunWeapon::GetCurrentReloadTime() const // 공격 속도랑 같은 방식
{
	const float SpeedBonus = GetEnchantStat(EEnchantStat::RangeReloadSpeed);
	return ReloadTime / (1.f + SpeedBonus * 0.01f);
}

float AShotgunWeapon::GetUpgradeBonus(EEnchantStat Stat) const
{
	const UWorld* World = GetWorld();
	const UCosGameInstance* GI = World ? Cast<UCosGameInstance>(World->GetGameInstance()) : nullptr;
	if (!GI) return 0.f;

	switch (Stat)
	{
	case EEnchantStat::RangeDamageAdd:   return GI->GetShotgunStat(EShotgunModuleType::Damage);    // 깡뎀
	case EEnchantStat::RangeFiringRate:  return GI->GetShotgunStat(EShotgunModuleType::FireSpeed); // 연사 %
	case EEnchantStat::RangeReloadSpeed: return GI->GetShotgunStat(EShotgunModuleType::Reload);    // 재장전 %
	case EEnchantStat::RangeMaxAmmo:     return GI->GetShotgunStat(EShotgunModuleType::Magazine);  // 탄창 +N
	default:                             return 0.f;
	}
}

void AShotgunWeapon::OnLoadoutChanged() // 인챈트 끼거나 뺄때 호출돼서 현재 탄약수 설정
{
	CurrentAmmo = FMath::Min(CurrentAmmo, GetCurrentMaxAmmo());
	BroadcastAmmo();
}

void AShotgunWeapon::BroadcastAmmo()
{
	OnAmmoChanged.Broadcast(CurrentAmmo, GetCurrentMaxAmmo());
}

void AShotgunWeapon::RefillAmmo() // 대장간 종료 시 탄약 최대로
{
	if (bIsReloading) // 재장전 중에 대장간에 들어갔던 경우 재장전 취소
	{
		GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);
		bIsReloading = false;
	}

	CurrentAmmo = GetCurrentMaxAmmo();
	BroadcastAmmo();
}