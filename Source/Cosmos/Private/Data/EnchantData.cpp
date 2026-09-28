#include "Data/EnchantData.h"

FText UEnchantData::GetStatLabel(EEnchantStat Stat)
{
	switch (Stat)
	{
	case EEnchantStat::AllDamageAdd: return FText::FromString(TEXT("All Damage"));
	case EEnchantStat::AllDamageMulti: return FText::FromString(TEXT("All Damage"));
	case EEnchantStat::AllSpeed: return FText::FromString(TEXT("All Speed"));
	case EEnchantStat::RangeDamageAdd: return FText::FromString(TEXT("Ranged Damage"));
	case EEnchantStat::RangeDamageMulti: return FText::FromString(TEXT("Ranged Damage"));
	case EEnchantStat::RangeFiringRate: return FText::FromString(TEXT("Fire Rate"));
	case EEnchantStat::RangeMaxAmmo: return FText::FromString(TEXT("Magazine Size"));
	case EEnchantStat::RangeReloadSpeed: return FText::FromString(TEXT("Reload Speed"));
	case EEnchantStat::MeleeDamageAdd: return FText::FromString(TEXT("Melee Damage"));
	case EEnchantStat::MeleeDamageMulti: return FText::FromString(TEXT("Melee Damage"));
	case EEnchantStat::MeleeSpeed: return FText::FromString(TEXT("Melee Attack Speed"));
	case EEnchantStat::MeleeMaxTarget: return FText::FromString(TEXT("Melee Max Target"));
	case EEnchantStat::MeleeRange: return FText::FromString(TEXT("Melee Range"));
	case EEnchantStat::MaxHealth: return FText::FromString(TEXT("Max Health"));
	case EEnchantStat::MovementSpeed: return FText::FromString(TEXT("Move Speed"));
	case EEnchantStat::SprintSpeedMulti: return FText::FromString(TEXT("Sprint Speed"));
	case EEnchantStat::MaxPotionAdd: return FText::FromString(TEXT("Max Potions"));
	case EEnchantStat::IncreasePotionValue: return FText::FromString(TEXT("Potion Heal"));
	case EEnchantStat::DecreaseSkillCooldown: return FText::FromString(TEXT("Skill Cooldown"));
	case EEnchantStat::IncreaseSoulValue: return FText::FromString(TEXT("Soul Gain"));
	case EEnchantStat::GainHeal: return FText::FromString(TEXT("Life Steal"));
	default: return FText::GetEmpty();
	}
}

FText UEnchantData::GetSkillLabel(EEnchantSkillType Type)
{
	switch (Type)
	{
	case EEnchantSkillType::IncreaseAllSpeed: return FText::FromString(TEXT("All Speed"));
	case EEnchantSkillType::IncreaseAllDamage: return FText::FromString(TEXT("All Damage"));
	case EEnchantSkillType::IncreaseShotgunDamage: return FText::FromString(TEXT("Shotgun Damage"));
	case EEnchantSkillType::IncreaseShotgunFiringRate: return FText::FromString(TEXT("Fire Rate"));
	case EEnchantSkillType::IncreaseRangeReloadSpeed: return FText::FromString(TEXT("Reload Speed"));
	case EEnchantSkillType::InfinityAmmo: return FText::FromString(TEXT("Infinity Ammo"));
	case EEnchantSkillType::IncreaseMeleeDamage: return FText::FromString(TEXT("Melee Damage"));
	case EEnchantSkillType::IncreaseMeleeAttackSpeed: return FText::FromString(TEXT("Melee Attack Speed"));
	case EEnchantSkillType::IncreaseMeleeMaxTarget: return FText::FromString(TEXT("Melee Max Target"));
	case EEnchantSkillType::IncreaseMovementSpeed: return FText::FromString(TEXT("Move Speed"));
	case EEnchantSkillType::MadePotion: return FText::FromString(TEXT("Potion Count"));
	default:  return FText::GetEmpty();
	}
}

EEnchantValueType UEnchantData::GetSkillValueType(EEnchantSkillType Type)
{
	switch (Type)
	{
	case EEnchantSkillType::IncreaseAllSpeed:
	case EEnchantSkillType::IncreaseAllDamage:
	case EEnchantSkillType::IncreaseShotgunDamage:
	case EEnchantSkillType::IncreaseMeleeDamage:
		return EEnchantValueType::Percent;

	case EEnchantSkillType::IncreaseShotgunFiringRate:
	case EEnchantSkillType::IncreaseRangeReloadSpeed:
	case EEnchantSkillType::IncreaseMeleeAttackSpeed:
		return EEnchantValueType::Flat;

	case EEnchantSkillType::IncreaseMeleeMaxTarget:
	case EEnchantSkillType::MadePotion:
	case EEnchantSkillType::IncreaseMovementSpeed:
		return EEnchantValueType::Integer;

	case EEnchantSkillType::InfinityAmmo:
		return EEnchantValueType::None;

	default:
		return EEnchantValueType::None;
		
	}
}

FString UEnchantData::GetStatText() const
{
	FString Result;

	if (bHasSkill)
	{
		const UEnum* SkillEnumPtr = StaticEnum<EEnchantSkillType>();
		const FString SkillName = SkillEnumPtr ? SkillEnumPtr->GetDisplayNameTextByValue((int64)SkillType).ToString() : TEXT("Unknown");

		const EEnchantValueType SkillValueType = GetSkillValueType(SkillType);

		FString SkillValueText;
		if (SkillValueType == EEnchantValueType::Percent)
		{
			SkillValueText = FString::Printf(TEXT(" + %.1f%%"), SkillValue);
		}
		else if (SkillValueType == EEnchantValueType::Integer)
		{
			SkillValueText = FString::Printf(TEXT(" + %d"), FMath::RoundToInt(SkillValue));
		}
		else if (SkillValueType == EEnchantValueType::Flat)
		{
			SkillValueText = FString::Printf(TEXT(" + %.1f"), SkillValue);
		}

		Result += FString::Printf(TEXT("[Skill] %s%s\n"), *SkillName, *SkillValueText);
		Result += FString::Printf(TEXT("\n"));
	}

	for (const FEnchantRolledStat& Stat : RolledStats)
	{
		const UEnum* StatEnumPtr = StaticEnum<EEnchantStat>();
		const FString StatName = StatEnumPtr ? StatEnumPtr->GetDisplayNameTextByValue((int64)Stat.StatType).ToString() : TEXT("Unknown");

		FString ValueText;

		if (Stat.ValueType == EEnchantValueType::Percent)
		{
			ValueText = FString::Printf(TEXT("+ %.1f%%"), Stat.RolledValue);
		}
		else if (Stat.ValueType == EEnchantValueType::Integer)
		{
			ValueText = FString::Printf(TEXT("+ %d"), FMath::RoundToInt(Stat.RolledValue));
		}
		else
		{
			ValueText = FString::Printf(TEXT("+ %.1f"), Stat.RolledValue);
		}

		Result += FString::Printf(TEXT("%s %s\n"), *StatName, *ValueText);
		Result += FString::Printf(TEXT("\n"));
	}

	return Result;
}