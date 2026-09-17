#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/CosDataTable.h"
#include "ShotgunUpgradeButtonWidget.generated.h"

class UButton;
UCLASS()
class COSMOS_API UShotgunUpgradeButtonWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	// 각 버튼은 선언할 때 TObjectPtr로 선언. UE5 표준
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DamageButton;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> FireSpeedButton;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReloadButton;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MagazineButton;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase> UpgradeSuccessSound;
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase> UpgradeFailSound;

	UFUNCTION() 
	void OnClickDamage();
	UFUNCTION()
	void OnClickFireSpeed();
	UFUNCTION()
	void OnClickReload();
	UFUNCTION()
	void OnClickMagazine();
	UFUNCTION()
	void RefreshButtonStates();

	void TryUpgrade(EShotgunModuleType Type);
};
