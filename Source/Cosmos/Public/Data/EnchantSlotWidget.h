#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/EnchantData.h"
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
	UFUNCTION(BlueprintCallable, Category = "Enchant")
	void Setup(UEnchantData* InEnchant, bool bInIsEquipped, class UEnchantInventoryWidget* InOwner);
	UFUNCTION(BlueprintCallable, Category = "Enchant")
	void OnSlotClicked();
	UFUNCTION(BlueprintImplementableEvent, Category = "Enchant")
	void OnEnchantSet(); // 블루프린트에서 구현
	UFUNCTION(BlueprintPure, Category = "Enchant")
	FString GetStatText() const;

private:
	UPROPERTY()
	TObjectPtr<UEnchantInventoryWidget> OwnerInventory;
};
	