#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/EnchantData.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "EnchantSlotWidget.generated.h"


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
	UButton* EquipButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* SlotStatText;

	UFUNCTION(BlueprintCallable, Category = "Enchant")
	void Setup(UEnchantData* InEnchant, bool bInIsEquipped, UEnchantInventoryWidget* InOwner = nullptr);
	UFUNCTION(BlueprintCallable, Category = "Enchant")
	void OnSlotClicked();
	UFUNCTION(BlueprintImplementableEvent, Category = "Enchant")
	void OnEnchantSet(); // 블루프린트에서 구현
	UFUNCTION(BlueprintPure, Category = "Enchant")
	FString GetStatText() const;

private:

	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	TObjectPtr<UEnchantInventoryWidget> OwnerInventory;
};
	