#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/EnchantData.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "EnchantSlotWidget.generated.h"

class UEnchantInventoryWidget;

UCLASS()
class COSMOS_API UEnchantSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(BlueprintReadOnly, Category = "Enchant")
	TObjectPtr<UEnchantData> MyEnchant;
	UPROPERTY(BlueprintReadOnly, Category = "Enchant")
	bool bIsEquipped = false;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* SlotIcon;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* SlotStatText;

	UFUNCTION(BlueprintCallable, Category = "Enchant")
	void Setup(UEnchantData* InEnchant, bool bInIsEquipped);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Enchant")
	void OnEnchantSet(); // 블루프린트에서 구현
	UFUNCTION(BlueprintPure, Category = "Enchant")
	FString GetStatText() const;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UEnchantInventoryWidget> OwnerInventory;

private:

	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
};
	