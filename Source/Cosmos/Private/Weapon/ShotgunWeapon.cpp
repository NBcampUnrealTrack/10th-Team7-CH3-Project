#include "Weapon/ShotgunWeapon.h"
#include "DrawDebugHelpers.h" // 트레이스 시각화
#include "Data/CosGameInstance.h"

AShotgunWeapon::AShotgunWeapon()
{
	BaseDamage = 30.f;
	AttackInterval = 1.f;
	AttackRange = 1000.f;
	MaxAmmo = 4;
	CurrentAmmo = MaxAmmo;
	ReloadTime = 2.f;
}

void AShotgunWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UCosGameInstance* GameInstance = Cast<UCosGameInstance>(World->GetGameInstance()))
		{
			GameInstance->OnLoadoutChange.AddDynamic(this, &AShotgunWeapon::OnLoadoutChanged);
		}
	}

	CurrentAmmo = GetCurrentMaxAmmo();
	OnAmmoChanged.Broadcast(CurrentAmmo);
}

bool AShotgunWeapon::PerformAttack()
{
	if (bIsReloading || CurrentAmmo <= 0) 
	{
		return false;
	}

	FVector StartLocation;
	FVector Direction;
	if (!GetTraceStartAndDirection(StartLocation, Direction)) // 시작점, 방향 설정
	{
		return false;
	}

	CurrentAmmo--;
	OnAmmoChanged.Broadcast(CurrentAmmo); // 총알 줄어든것 방송

	const FVector EndLocation = StartLocation + (Direction * AttackRange); //끝점

	FHitResult HitResult; // 트레이스 결과를 담을 곳
	FCollisionQueryParams QueryParams; // 트레이스 설정
	QueryParams.AddIgnoredActor(this); // 무기 자체는 맞지 않도록
	QueryParams.AddIgnoredActor(GetAttachParentActor()); // 플레이어가 맞지 않도록

	const bool bHit = GetWorld()->LineTraceSingleByChannel(// 라인 트레이스
		HitResult, // 결과 담는 곳
		StartLocation, // 시작 
		EndLocation, // 끝
		ECC_Weapon, //시각적 오브젝트 기준 충돌
		QueryParams // 트레이스 설정
	); 

	DrawDebugLine( // 라인트레이스 시각화
		GetWorld(), //현재 월드
		StartLocation, //시작
		EndLocation, //끝
		bHit ? FColor::Green : FColor::Red, // 라인 색 (맞으면 초록, 안맞으면 빨강)
		false, // 계속 남아있는지
		2.f, // 몇 초간 남아있나
		0, // 그리기 우선순위, 0이면 벽은 뚫지 못함
		2.f // 선 두께
	);

	if (bHit) // 무언가에 맞았는가
	{
		TryApplyDamage(HitResult.GetActor());
	}

	return true;
}

int32 AShotgunWeapon::GetCurrentAmmo() const
{
	return CurrentAmmo;
}

void AShotgunWeapon::Reload()
{
	if (bIsReloading || CurrentAmmo == GetCurrentMaxAmmo())
	{
		return;
	}

	bIsReloading = true; // 재장전중
	OnReloadStarted();

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
	CurrentAmmo = GetCurrentMaxAmmo();
	bIsReloading = false;
	OnAmmoChanged.Broadcast(CurrentAmmo); //총알 장전된것 방송
	OnReloadFinished();
}

EWeaponType AShotgunWeapon::GetWeaponType() const
{
	return EWeaponType::Shotgun;
}

void AShotgunWeapon::OnLoadoutChanged()
{
	CurrentAmmo = FMath::Min(CurrentAmmo, GetCurrentMaxAmmo());
	OnAmmoChanged.Broadcast(CurrentAmmo);
}

int32 AShotgunWeapon::GetCurrentMaxAmmo() const
{
	return MaxAmmo + FMath::RoundToInt(GetEnchantStat(EEnchantStat::RangeMaxAmmo));
}

float AShotgunWeapon::GetCurrentReloadTime() const
{
	const float SpeedBonus = GetEnchantStat(EEnchantStat::RangeReloadSpeed);
	return ReloadTime / (1.f + SpeedBonus * 0.01f);
}
