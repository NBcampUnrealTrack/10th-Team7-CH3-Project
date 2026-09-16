#include "Weapon/SkillComponent.h"
#include "Data/CosGameInstance.h"
#include "Data/EnchantData.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "GameFramework/Pawn.h"

namespace  // static 함수의 효과
{
	EEnchantStat SkillTypeToStat(EEnchantSkillType SkillType) // 스킬 종류를 스탯 종류로 변환
	{
		switch (SkillType) // if else보다 효율적
		{
		case EEnchantSkillType::IncreaseAllSpeed:          return EEnchantStat::AllSpeed;
		case EEnchantSkillType::IncreaseAllDamage:         return EEnchantStat::AllDamageMulti;
		case EEnchantSkillType::IncreaseShotgunDamage:     return EEnchantStat::RangeDamageMulti;
		case EEnchantSkillType::IncreaseShotgunFiringRate: return EEnchantStat::RangeFiringRate;
		case EEnchantSkillType::IncreaseRangeReloadSpeed:  return EEnchantStat::RangeReloadSpeed;
		case EEnchantSkillType::IncreaseMeleeDamage:       return EEnchantStat::MeleeDamageMulti;
		case EEnchantSkillType::IncreaseMeleeAttackSpeed:  return EEnchantStat::MeleeSpeed;
		case EEnchantSkillType::IncreaseMeleeMaxTarget:    return EEnchantStat::MeleeMaxTarget;
		case EEnchantSkillType::IncreaseMovementSpeed:     return EEnchantStat::MovementSpeed;
		default:                                           return EEnchantStat::None;
		}
	}
}

USkillComponent::USkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void USkillComponent::BeginPlay() // 입력 바인딩
{
	Super::BeginPlay();

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!IsValid(OwnerPawn))
	{
		return;
	}

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(OwnerPawn->InputComponent))
	{
		if (IsValid(UseSkillAction))
		{
			EIC->BindAction(UseSkillAction, ETriggerEvent::Started, this, &USkillComponent::OnUseSkill);
		}
	}
}

void USkillComponent::OnUseSkill() // 입력 연결
{
	ActivateSkills();
}

void USkillComponent::ActivateSkills() // 지금 쓸 수 있는 스킬 찾아서 종료시간 기록해두는 함수
{
	UWorld* World = GetWorld(); // 월드 얻기
	if (!IsValid(World))
	{
		return;
	}

	UCosGameInstance* GameInstance = Cast<UCosGameInstance>(World->GetGameInstance()); //인스턴스 얻기
	if (!IsValid(GameInstance))
	{
		return;
	}

	const float CurrentTime = World->GetTimeSeconds(); // 현재 시간

	ActiveSkills.RemoveAll([CurrentTime](const FActiveSkill& Skill) // 만료된 기록 제거
		{
			return Skill.EndTime <= CurrentTime;
		});

	bool bActivatedAny = false; // 이번에 스킬이 하나라도 발동 됐는지

	for (const UEnchantData* Enchant : GameInstance->EquippedEnchant) // 인챈트 돌면서 검사
	{
		if (!IsValid(Enchant) || !Enchant->bHasSkill) // 스킬 없으면 건너뜀
		{
			continue;
		}

		const float* ReadyTime = SkillReadyTime.Find(Enchant->SkillType); // Enchant->SkillType 인챈트에 달린 스킬 종류 
		if (ReadyTime && CurrentTime < *ReadyTime) // 쿨타임 안됐으면 건너뜀
		{
			continue;
		}

		FActiveSkill NewSkill; // 스킬 발동기록 담을 곳.
		NewSkill.SkillType = Enchant->SkillType; // 어떤 스킬인지
		NewSkill.Value = Enchant->SkillValue; // 어떤 효과인지
		NewSkill.EndTime = CurrentTime + Enchant->SkillDuration; // 지속 시간
		ActiveSkills.Add(NewSkill); //배열에 넣기

		SkillReadyTime.FindOrAdd(Enchant->SkillType) = CurrentTime + Enchant->SkillCooldown; // 게임 10초 시점에 8초짜리 스킬을 쓰면 18.0 이 저장됨. 
		bActivatedAny = true;
	}

	if (bActivatedAny) // 하나라도 발동됐으면 방송
	{
		OnSkillStateChanged.Broadcast();
	}
}

float USkillComponent::GetActiveSkillBonus(EEnchantStat Stat) const
{
	if (Stat == EEnchantStat::None) // 등록 되어있는지
	{
		return 0.f;
	}

	UWorld* World = GetWorld(); // 월드 얻기
	if (!IsValid(World)) 
	{
		return 0.f;
	}

	const float CurrentTime = World->GetTimeSeconds(); // 현재 시간 얻기
	float Sum = 0.f;

	for (const FActiveSkill& Skill : ActiveSkills) // 현재 켜져있는 스킬
	{
		if (Skill.EndTime <= CurrentTime) // 지속시간 끝났으면 넘기기
		{
			continue;
		}

		if (SkillTypeToStat(Skill.SkillType) == Stat) // switch표로 변환후 비교
		{
			Sum += Skill.Value;
		}
	}

	return Sum; // 스킬로 인한 스탯 상승 반환
}

bool USkillComponent::IsSkillActive(EEnchantSkillType SkillType) const // 스킬이 지금 켜져있는지
{
	UWorld* World = GetWorld(); // 월드 얻기
	if (!IsValid(World))
	{
		return false;
	}

	const float CurrentTime = World->GetTimeSeconds(); // 현재 시간 얻기

	for (const FActiveSkill& Skill : ActiveSkills)
	{
		if (Skill.SkillType == SkillType && Skill.EndTime > CurrentTime) // 일치하는지, 아직 안끝났는지
		{
			return true;
		}
	}

	return false;
}