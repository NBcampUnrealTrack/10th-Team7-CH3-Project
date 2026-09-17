#include "Data/EnchantSlotWidget.h"
#include "Data/EnchantInventoryWidget.h"
#include "Data/CosGameInstance.h"

void UEnchantSlotWidget::Setup(UEnchantData* InEnchant, bool InIsEquipped)
{
	MyEnchant = InEnchant;
	bIsEquipped = InIsEquipped;

	if (!MyEnchant)
	{
		SetToolTipText(FText::GetEmpty());
		OnEnchantSet();
		return;
	}

	SetToolTipText(FText::FromString(GetStatText()));
	OnEnchantSet();
}


FString UEnchantSlotWidget::GetStatText() const
{
	if (!MyEnchant) return FString();

	FString Result;

	if (MyEnchant->bHasSkill)
	{
		const UEnum* SkillEnumPtr = StaticEnum<EEnchantSkillType>();
		const FString SkillName = SkillEnumPtr ? SkillEnumPtr->GetDisplayNameTextByValue((int64)MyEnchant->SkillType).ToString() : TEXT("Unknown");
		Result += FString::Printf(TEXT("[Skill] %s\n"), *SkillName);
	}

	for (const FEnchantRolledStat& Stat : MyEnchant->RolledStats)
	{
		const UEnum* StatEnumPtr = StaticEnum<EEnchantStat>();
		const FString StatName = StatEnumPtr ? StatEnumPtr->GetDisplayNameTextByValue((int64)Stat.StatType).ToString() : TEXT("Unknown");

		FString ValueText;

		if (Stat.ValueType == EEnchantValueType::Percent)
		{
			ValueText = FString::Printf(TEXT("+%.1f%%"), Stat.RolledValue);
		}
		else if (Stat.ValueType == EEnchantValueType::Integer)
		{
			ValueText = FString::Printf(TEXT("+%d"), FMath::RoundToInt(Stat.RolledValue));
		}
		else // Flat
		{
			ValueText = FString::Printf(TEXT("+%.1f"), Stat.RolledValue);
		}

		Result += FString::Printf(TEXT("%s %s\n"), *StatName, *ValueText);
	}

	return Result;
}

FReply UEnchantSlotWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
	
	if (MyEnchant)
	{
		if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
		{
			if (bIsEquipped)
			{
				GI->UnEquipEnchant(MyEnchant);
			}
			else
			{
				GI->EquipEnchant(MyEnchant);
			}
		}
	}

	return FReply::Handled();
}