#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/EnchantSlotWidget.h"
#include "Components/UniformGridPanel.h"
#include "EnchantEquipWidget.generated.h"

UCLASS()
class COSMOS_API UEnchantEquipWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	UPROPERTY(EditAnywhere, Category = "Enchant")
	TSubclassOf<UEnchantSlotWidget> SlotWidgetClass;

	UPROPERTY(meta = (BindWidget))
	UUniformGridPanel* SlotContainer;

	UFUNCTION()
	void RefreshEquipped();
};
