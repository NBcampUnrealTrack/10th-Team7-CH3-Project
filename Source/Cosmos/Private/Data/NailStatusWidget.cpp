#include "Data/NailStatusWidget.h"
#include "Data/CosGameInstance.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Weapon/CombatComponent.h"
#include "Weapon/NailWeapon.h"

void UNailStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		GI->OnLoadoutChange.AddDynamic(this, &UNailStatusWidget::RefreshAll);
	}

	RefreshAll();
}

void UNailStatusWidget::NativeDestruct()
{
	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		GI->OnLoadoutChange.RemoveDynamic(this, &UNailStatusWidget::RefreshAll);
	}

	Super::NativeDestruct();
}

void UNailStatusWidget::RefreshAll()
{
	APawn* Pawn = GetOwningPlayerPawn();
	if (!Pawn) return;

	UCombatComponent* Combat = Pawn->FindComponentByClass<UCombatComponent>();
	if (!Combat) return;

	ANailWeapon* Weapon = Combat->GetNailWeapon();
	if (!Weapon) return;

	RefreshOne(Weapon->GetCurrentDamage(), DamageValueText);
	RefreshOne(Weapon->GetCurrentAttackInterval(), AttackSpeedValueText);
}

void UNailStatusWidget::RefreshOne(float Value, UTextBlock* ValueText) const
{
	if (ValueText)
	{
		ValueText->SetText(FormatNailStatText(Value));
	}
}

FText UNailStatusWidget::FormatNailStatText(float Value) const
{
	return FText::FromString(FString::Printf(TEXT("%.1f"), Value));
}