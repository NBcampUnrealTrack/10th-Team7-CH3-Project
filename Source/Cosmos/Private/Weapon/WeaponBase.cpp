#include "Weapon/WeaponBase.h"
#include "Weapon/Damageable.h"
#include "Data/CosGameInstance.h"
#include "Weapon/SkillComponent.h"
#include "Character/HealthComponent.h"

AWeaponBase::AWeaponBase()
{
	WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot")); // 무기 메쉬
	SetRootComponent(WeaponRoot);
	
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(WeaponRoot);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); //캐릭터와 무기의 물리적 충돌로 인한 버그 예방

	LastAttackTime = -100.f;
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* ParentActor = GetAttachParentActor()) // 캐릭터 얻기
	{
		SkillComponent = ParentActor->FindComponentByClass<USkillComponent>(); // 무기가 붙어있는 캐릭터의 스킬 컴포넌트를 저장해두기
	}
}

void AWeaponBase::TryAttack() //공격 쿨타임이 지났으면 공격
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
	OnAttackPlayed(); // 공격 모션 재생하기 위해
}

//HitResult안에 HitActor도 포함
bool AWeaponBase::TryApplyDamage(AActor* HitActor) // 데미지를 준 경우에만 true를 리턴해서 대못의 적 공격 수를 카운트함 , 데미저블인지 판단함
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

	UHealthComponent* TargetHealth = HitActor->FindComponentByClass<UHealthComponent>(); // 킬 판정용
	const bool bWasAlive = TargetHealth && !TargetHealth->IsDead(); // 때리기 전 생존 여부
	Damageable->TakeHit(GetCurrentDamage(), GetWeaponType());
	
	if (bWasAlive && TargetHealth->IsDead()) // 이번 공격으로 죽었으면 킬
	{
		OnKillConfirmed.Broadcast(HitActor);
	}
	else
	{
		OnHitConfirmed.Broadcast(HitActor);  // 안 죽었으면 히트마커만
	}
	return true; // 데미지를 준 경우
}

bool AWeaponBase::GetTraceStartAndDirection(FVector& OutStart, FVector& OutDirection) const // 트레이스 방향 정해주는 함수. out 파라미터 형식. 시점 바꾸는 용도
{
	APawn* OwnerPawn = Cast<APawn>(GetAttachParentActor()); // 무기가 붙어있는 부모 액터를 폰으로 캐스팅함
	if (!IsValid(OwnerPawn))                                // GetOwner()가 아닌 이유는 Child Actor Component로 스폰된 무기는 소유 관계가 안 맺어져서
	{
		return false;
	}

	AController* Controller = OwnerPawn->GetController(); // 이 폰을 조종중인 컨트롤러를 얻음. 시점 정보를 갖고 있는게 컨트롤러이기 때문에.
	if (!IsValid(Controller))
	{
		return false;
	}

	FRotator ViewRotation;
	Controller->GetPlayerViewPoint(OutStart, ViewRotation); // out파라미터 패턴, 시점의 위치와 시점의 회전을 돌려줌.
	OutDirection = ViewRotation.Vector(); // FRotator를 FVector로 변환. 회전값을, 어느쪽을 향하고 있나. 로 바꿔줌

	return true;
}

float AWeaponBase::GetAnimationPlayRate() const
{
	const float CurrentInterval = GetCurrentAttackInterval();
	if (CurrentInterval <= 0.f)
	{
		return 1.f;
	}

	return AttackInterval / CurrentInterval;
}

float AWeaponBase::GetEnchantStat(EEnchantStat Stat) const // 스탯 하나를 받아서 그 스탯의 보너스를 리턴
{
	if (Stat == EEnchantStat::None) // 따로 지정해두지 않았다면 0.f 리턴
	{
		return 0.f;
	}

	float Total = 0.f; // 총 보너스 담을 변수

	if (UWorld* World = GetWorld()) // this->GetWorld(); 웨폰베이스가 속한 월드 얻기. 인스턴스를 알기 위해
	{
		if (UCosGameInstance* GameInstance = Cast<UCosGameInstance>(World->GetGameInstance())) //다운캐스트로 CosGameInstance 얻기
		{
			Total += GameInstance->GetTotalStat(Stat);// 스탯부분 보너스 
		}
	}

	if (IsValid(SkillComponent))
	{
		Total += SkillComponent->GetActiveSkillBonus(Stat); // 스킬에서 오는 보너스 추가
	}

	Total += GetUpgradeBonus(Stat); // 강화 보너스 추가

	return Total; // 스탯 + 스킬 보너스 리턴
}

float AWeaponBase::GetCurrentAttackInterval() const // 최종 공격 속도
{
	const float SpeedBonus =
		GetEnchantStat(EEnchantStat::AllSpeed) + // 무기 공통 속도
		GetEnchantStat(GetSpeedStatType()); // 무기 타입 속도

	return AttackInterval / (1.f + SpeedBonus * 0.01f); // 공속은 나눠야지 빨라짐
}

float AWeaponBase::GetCurrentDamage() const // 최종 데미지
{
	const float AddBonus =
		GetEnchantStat(EEnchantStat::AllDamageAdd) +   // 공격 속도 구하는 것과 동일
		GetEnchantStat(GetDamageAddStatType());

	const float MultiBonus =
		GetEnchantStat(EEnchantStat::AllDamageMulti) +
		GetEnchantStat(GetDamageMultiStatType());

	return (BaseDamage + AddBonus) * (1.f + MultiBonus * 0.01f); // 깡뎀 , 퍼센트가 따로 있음
}
