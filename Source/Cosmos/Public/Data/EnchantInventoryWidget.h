#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/EnchantData.h"
#include "Data/EnchantSlotWidget.h"
#include "EnchantInventoryWidget.generated.h"

UCLASS()
class COSMOS_API UEnchantInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Enchant")
	void RefreshList();
	UFUNCTION()
	void SelectEnchant(UEnchantData* InEnchant) { SelectedEnchant = InEnchant; }
	UFUNCTION(BlueprintCallable, Category = "Enchant")
	void OnEquipButtonClicked();
	UFUNCTION(BlueprintCallable, Category = "Enchant")
	void OnUnEquipButtonClicked();
	
	UPROPERTY(BlueprintReadOnly, Category = "Enchant")
	TObjectPtr<UEnchantData> SelectedEnchant;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enchant")
	TSubclassOf<UEnchantSlotWidget> SlotWidgetClass; // 에디터에서 WBP_EnchantSlot으로 지정해둠
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UVerticalBox> SlotContainer; // UMG 디자이너에서 이름을 SlotContainer로 지정해둠

protected:
	virtual void NativeConstruct() override;
};
