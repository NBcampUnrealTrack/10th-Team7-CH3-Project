#include "Data/CosGameInstance.h"
#include "Data/EnchantData.h"
#include "Weapon/NailWeapon.h"
#include "Weapon/ShotgunWeapon.h"

int32 UCosGameInstance::GetSoul() const
{
	return Soul;
}

// 재화를 얻는 로직
// 몬스터 쪽에서 사망 처리 로직에 AddSoul 호출
// if (UCosGameInstance* GameInstance = Cast<UCosGameInstance>(GetWorld()->GetGameInstance()))
// { GameInstance->AddSoul(EnemyData.SoulDrop); }
void UCosGameInstance::AddSoul(int32 Amount)
{
	if (Amount <= 0) return;
	Soul += Amount;
	OnCurrencyChanged.Broadcast(Soul);
}

// 재화를 사용할 때 상태 업데이트 로직
// if (UCosGameInstance* GameInstance = Cast<UCosGameInstance>(GetWorld()->GetGameInstance()))
// { 
//		if (GameInstance->TrySpendSoul(Amount)
//		{
//			성공: 실제로 구매/업그레이드 적용
//		}
//		else
//		{	
//			실패: 재화 부족 처리 (UI 메시지 등)
//		}
// }
bool UCosGameInstance::SpendSoul(int32 Amount)
{
	if (Amount <= 0 || Soul < Amount) return false;
	Soul -= Amount;
	OnCurrencyChanged.Broadcast(Soul);
	return true;
}

// 인챈트 드랍
// 몬스터 클래스에서 몬스터가 죽을 때 이 코드를 넣으면 인챈트가 드랍
// GetWorld()->SpawnActor<AEnchantPickup>(EnchantPickupClass, GetActorLocation(), FRotator::ZeroRotator);
bool UCosGameInstance::CollectEnchant(UEnchantData* Enchant)
{
	if (!Enchant) return false;

	EnchantMaxCollectedCount = 16;

	if (CollectedEnchant.Num() >= EnchantMaxCollectedCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("인첸트를 더 이상 획득할 수 없습니다."));
		return false;
	}
	CollectedEnchant.Add(Enchant);
	OnLoadoutChange.Broadcast();

	return true;
}


// 인챈트 장착
bool UCosGameInstance::EquipEnchant(UEnchantData* Enchant)
{
	UE_LOG(LogTemp, Warning, TEXT("인챈트 장착"));
	if (!Enchant || !CollectedEnchant.Contains(Enchant)) return false;

	if (EquippedEnchant.Num() >= EnchantMaxEqippedCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("인첸트를 더 이상 장착할 수 없습니다."));
		return false;
	}

	EquippedEnchant.Add(Enchant);
	OnLoadoutChange.Broadcast();

	return true;
}

void UCosGameInstance::UnEquipEnchant(UEnchantData* Enchant)
{
	if (!Enchant || !EquippedEnchant.Contains(Enchant)) return;

	EquippedEnchant.Remove(Enchant);
	OnLoadoutChange.Broadcast();
}

// GameMode에서 사용할 Recalc() 함수
// 
// void CosGameMode::Recalc()
// {
//		UCosGameInstance* GI = Cast<UCosGameInstance>(GetWorld()->GetGameInstance());
//		if (!GI) return;
//		
//		// All
//		EnchantAllAddValue = GI->GetTotalStat(EEnchantStat::AllDamageAdd);
//		EnchantAllMultiValue = GI->GetTotalStat(EEnchantStat::AllDamageMulti);
//		
//		// Range
//		EnchantRangeAddValue = GI->GetTotalStat(EEnchantStat::RangeDamageAdd);
//		EnchantRangeMultiValue = GI->GetTotalStat(EEnchantStat::RangeDamageMulti);
//		EnchantRangeFiringRateValue = GI->GetTotalStat(EEnchantStat::RangeFiringRate);
//		EnchantRangeMaxAmmoValue = GI->GetTotalStat(EEnchantStat::RangeMaxAmmo);
//		EnchantRangeReloadSpeedValue = GI->GetTotalStat(EEnchantStat::RangeReloadSpeed);
// 
//		// Melee
//		EnchantMeleeAddValue = GI->GetTotalStat(EEnchantStat::MeleeDamageAdd);
//		EnchantMeleeMultiValue = GI->GetTotalStat(EEnchantStat::MeleeDamageMulti);
//		EnchantMeleeSpeedValue = GI->GetTotalStat(EEnchantStat::MeleeSpeed);
//		EnchantMeleeMaxTargetValue = GI->GetTotalStat(EEnchantStat::MeleeMaxTarget);
//		EnchantMeleeRangeValue = GI->GetTotalStat(EEnchantStat::MeleeRange);
// 
//		// Misc
//		EnchantMaxHealthValue = GI->GetTotalStat(EEnchantStat::MaxHealth);
//		EndhantMovementSpeedValue = GI->GetTotalStat(EEnchantStat::MovementSpeed);
//		EnchantSprintSpeedMultiValue = GI->GetTotalStat(EEnchantStat::SprintSpeedMulti);
//		EnchantMaxPotionAddValue = GI->GetTotalStat(EEnchantStat::MaxPotionAdd);
//		EnchantIncreasePotionValue = GI->GetTotalStat(EEnchantStat::IncreasePotionValue);
//		EnchantDecreaseSkillCooldownValue = GI->GetTotalStat(EEnchantStat::DecreaseSkillCooldown);
//		EnchantIncreaseSoulValue = GI->GetTotalStat(EEnchantStat::IncreaseSoulValue);
//		EnchantGainHealValue = GI->GetTotalStat(EEnchantStat::GainHeal);
// }

float UCosGameInstance::GetTotalStat(EEnchantStat Stat) const
{
	float Sum = 0.0f;

	for (const UEnchantData* Enchant : EquippedEnchant)
	{
		for (const FEnchantRolledStat& RolledStat : Enchant->RolledStats)
		{
			if (RolledStat.StatType == Stat)
			{
				Sum += RolledStat.RolledValue;
			}
		}
	}
	
	return Sum;
}

const FUpgradeData* UCosGameInstance::FindUpgradeData(EShotgunModuleType Type, int32 Level) const
{
	if (!UpgradePool) return nullptr;

	const FUpgradeData* FoundRow = nullptr;
	UpgradePool->ForeachRow<FUpgradeData>(TEXT("FindUpgradeData"), [&](const FName& RowName, const FUpgradeData& Row)
		{
			if (Row.Type == Type && Row.Level == Level)
			{
				FoundRow = &Row;
			}
		});

	return FoundRow;
}

bool UCosGameInstance::UpgradeShotgun(EShotgunModuleType Type)
{
	// 업그레이드 할 샷건 모듈의 레벨을 저장
	int32 Level = ModuleLevels.FindOrAdd(Type);
	// 업그레이드 할 샷건 모듈의 데이터 테이블 행을 찾아서 "행"을 저장
	const FUpgradeData* UpgradeModule = FindUpgradeData(Type, Level + 1);

	if (!UpgradeModule)
	{
		// 이미 업그레이드가 만렙인 상태
		return false;
	}

	// 강화 실행
	if (SpendSoul(UpgradeModule->Cost))
	{
		ModuleLevels.Add(Type, UpgradeModule->Level);
		OnLoadoutChange.Broadcast();
		return true;
	}
	else
	{
		// 돈이 부족하다는 알림
		return false;
	}
}

float UCosGameInstance::GetShotgunStat(EShotgunModuleType Type) const
{
	// 현재 레벨 조회 (FindRef)

	// 레벨이 0이면 0.0f 반환

	// FindUpgradeData(Type, 현재레벨)로 행 조회

	// 행 있으면 EffectValue 반환, 없으면 0.0f

	return 0.0f;
}