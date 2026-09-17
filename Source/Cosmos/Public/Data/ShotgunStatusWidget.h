#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/CosDataTable.h"
#include "ShotgunStatusWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class COSMOS_API UShotgunStatusWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> DamageProgressBar;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> FireSpeedProgressBar;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ReloadProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DamageValueText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> FireSpeedValueText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ReloadValueText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MagazineValueText;

	UFUNCTION()
	void RefreshAll();
	void RefreshOne(EShotgunModuleType Type, UProgressBar* Slider, UTextBlock* ValueText);
	FText FormatShotgunStatText(EShotgunModuleType Type, float Value, int32 Level) const;
};
