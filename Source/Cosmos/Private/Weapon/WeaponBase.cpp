#include "Weapon/WeaponBase.h"
#include "Weapon/Damageable.h"
#include "Data/CosGameInstance.h"

AWeaponBase::AWeaponBase()
{
	WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot"));
	SetRootComponent(WeaponRoot);
	
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(WeaponRoot);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); //캐릭터와 무기의 물리적 충돌로 인한 버그 예방

	LastAttackTime = -100.f;
}

void AWeaponBase::TryAttack() //공격 속도
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < GetCurrentAttackInterval()) // 공격 딜레이만큼 시간이 지났는지.
	{
		return;
	}

	if (!PerformAttack()) // 샷건탄약이 없어서 공격이 안되는 경우 리턴. 공격속도를 소모하지 않게 하고 공격 모션이 재생되지 않도록 
	{
		return;
	}
	LastAttackTime = CurrentTime;
	OnAttackPlayed();
}

//HitResult안에 HitActor도 포함
bool AWeaponBase::TryApplyDamage(AActor* HitActor)
{
	if (!IsValid(HitActor)) //맞은 대상이 없다면
	{
		return false;
	}

	IDamageable* Damageable = Cast<IDamageable>(HitActor); // HitActor가 IDamageable을 포함한다면 데미저블 포인터를 반환
	if (!Damageable) // 데미지를 받을 수 없다면
	{
		return false;
	}

	//Damageable->TakeDamage(GetCurrentDamage(), HitResult);
	Damageable->TakeHit(GetCurrentDamage(), GetWeaponType());

	OnHitConfirmed.Broadcast(HitActor); // 적이 맞았을때 방송

	return true; // 데미지를 준 경우
}

bool AWeaponBase::GetTraceStartAndDirection(FVector& OutStart, FVector& OutDirection) const
{
	APawn* OwnerPawn = Cast<APawn>(GetAttachParentActor());
	if (!IsValid(OwnerPawn))
	{
		return false;
	}

	AController* Controller = OwnerPawn->GetController();
	if (!IsValid(Controller))
	{
		return false;
	}

	FRotator ViewRotation;
	Controller->GetPlayerViewPoint(OutStart, ViewRotation);
	OutDirection = ViewRotation.Vector();

	return true;
}

float AWeaponBase::GetEnchantStat(EEnchantStat Stat) const
{
	if (Stat == EEnchantStat::None)
	{
		return 0.f;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return 0.f;
	}

	UCosGameInstance* GameInstance = Cast<UCosGameInstance>(World->GetGameInstance());
	if (!IsValid(GameInstance))
	{
		return 0.f;
	}

	return GameInstance->GetTotalStat(Stat);
}

float AWeaponBase::GetCurrentAttackInterval() const
{
	const float SpeedBonus =
		GetEnchantStat(EEnchantStat::AllSpeed) +
		GetEnchantStat(GetSpeedStatType());

	return AttackInterval / (1.f + SpeedBonus * 0.01f);
}

float AWeaponBase::GetCurrentDamage() const
{
	const float AddBonus =
		GetEnchantStat(EEnchantStat::AllDamageAdd) +
		GetEnchantStat(GetDamageAddStatType());

	const float MultiBonus =
		GetEnchantStat(EEnchantStat::AllDamageMulti) +
		GetEnchantStat(GetDamageMultiStatType());

	return (BaseDamage + AddBonus) * (1.f + MultiBonus * 0.01f);
}