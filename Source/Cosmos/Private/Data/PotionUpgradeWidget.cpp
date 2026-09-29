#include "Data/PotionUpgradeWidget.h"
#include "Data/CosGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Weapon/CombatComponent.h"

void UPotionUpgradeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UpgradeButton)
	{
		UpgradeButton->OnClicked.AddDynamic(this, &UPotionUpgradeWidget::OnClickUpgrade);
	}

	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		GI->OnLoadoutChange.AddDynamic(this, &UPotionUpgradeWidget::RefreshAll);
	}

	RefreshAll();
}

void UPotionUpgradeWidget::OnClickUpgrade()
{
	TryUpgrade();
}

void UPotionUpgradeWidget::TryUpgrade()
{
	UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance());
	if (!GI) return;

	if (GI->UpgradePotion())
	{
		UGameplayStatics::PlaySound2D(this, UpgradeSuccessSound);
		if (APawn* Pawn = GetOwningPlayerPawn())
		{
			if (UCombatComponent* Combat = Pawn->FindComponentByClass<UCombatComponent>())
			{
				Combat->RefillPotions();
			}
		}
	}
	else
	{
		UGameplayStatics::PlaySound2D(this, UpgradeFailSound);
	}
}

void UPotionUpgradeWidget::NativeDestruct()
{
	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		GI->OnLoadoutChange.RemoveDynamic(this, &UPotionUpgradeWidget::RefreshAll);
	}

	Super::NativeDestruct();
}

void UPotionUpgradeWidget::RefreshAll()
{
	UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance());
	if (!GI) return;

	MaxCountText->SetText(FText::FromString(FString::Printf(TEXT("Max Potion Count: %d"), GI->GetPotionMaxCount())));
	HealAmountText->SetText(FText::FromString(FString::Printf(TEXT("Heal Amount: %d"), GI->GetPotionHealAmount())));

	RefreshButtonStates();
}

void UPotionUpgradeWidget::RefreshButtonStates()
{
	UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance());
	if (!GI) return;

	if (UpgradeButton)
	{
		UpgradeButton->SetIsEnabled(GI->PotionLevel < GI->GetMaxPotionLevel());
	}
}