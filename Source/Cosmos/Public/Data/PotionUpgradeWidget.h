#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PotionUpgradeWidget.generated.h"

class UTextBlock;
class UButton;
class USoundBase;

UCLASS()
class COSMOS_API UPotionUpgradeWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> UpgradeButton;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MaxCountText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HealAmountText;
	UPROPERTY(EditDefaultsOnly, Category = "Potion")
	TObjectPtr<USoundBase> UpgradeSuccessSound;
	UPROPERTY(EditDefaultsOnly, Category = "Potion")
	TObjectPtr<USoundBase> UpgradeFailSound;

	UFUNCTION()
	void RefreshAll();
	UFUNCTION()
	void OnClickUpgrade();
	UFUNCTION()
	void TryUpgrade();
	UFUNCTION()
	void RefreshButtonStates();

};
