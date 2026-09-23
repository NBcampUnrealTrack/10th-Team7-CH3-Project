#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/CosDataTable.h"
#include "NailStatusWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class COSMOS_API UNailStatusWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> DamageProgressBar;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> AttackSpeedProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DamageValueText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AttackSpeedValueText;

	UFUNCTION()
	void RefreshAll();
	void RefreshOne(float Value, UProgressBar* Slider, UTextBlock* ValueText) const;
	FText FormatNailStatText(float value) const;
};
