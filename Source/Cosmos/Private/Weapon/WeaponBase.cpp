#include "Weapon/WeaponBase.h"
#include "Weapon/Damageable.h"

AWeaponBase::AWeaponBase()
{
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); //캐릭터와 무기의 물리적 충돌로 인한 버그 예방

	LastAttackTime = -100.f;
}

void AWeaponBase::TryAttack() //공격 속도
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < AttackInterval) // 공격 딜레이만큼 시간이 지났는지.
	{
		UE_LOG(LogTemp, Warning, TEXT("TryAttack: CoolDown"));
		return;
	}

	LastAttackTime = CurrentTime;
	UE_LOG(LogTemp, Warning, TEXT("TryAttack: PerformAttack Called"));
	PerformAttack();
}

//HitResult안에 HitActor도 포함
bool AWeaponBase::ApplyDamage(AActor* HitActor, const FHitResult& HitResult)
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
