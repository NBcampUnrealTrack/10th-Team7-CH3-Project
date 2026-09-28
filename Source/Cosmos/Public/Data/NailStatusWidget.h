#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/CosDataTable.h"
#include "NailStatusWidget.generated.h"

class UTextBlock;

UCLASS()
class COSMOS_API UNailStatusWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;



	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DamageValueText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AttackSpeedValueText;

	UFUNCTION()
	void RefreshAll();
	void RefreshOne(float Value, UTextBlock* ValueText) const;
	FText FormatNailStatText(float value) const;
};
