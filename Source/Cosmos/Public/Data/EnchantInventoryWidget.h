#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/EnchantData.h"
#include "Data/EnchantSlotWidget.h"
#include "Components/UniformGridPanel.h"
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
	
	UFUNCTION(BlueprintCallable, Category = "Enchant")
	void ToggleEquip(UEnchantData* Enchant, bool bCurrentlyEquipped);
	UPROPERTY(BlueprintReadOnly, Category = "Enchant")
	TObjectPtr<UEnchantData> SelectedEnchant;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enchant")
	TSubclassOf<UEnchantSlotWidget> SlotWidgetClass; // 에디터에서 WBP_EnchantSlot으로 지정해둠
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UUniformGridPanel> SlotContainer; // UMG 디자이너에서 이름을 SlotContainer로 지정해둠


	// 인챈트 인벤토리 Grid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enchant")
	int32 ColumsPerRow = 4;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enchant")
	int32 SlotsPerPage = 8;
	int32 CurrentPage = 0;

	UFUNCTION(BlueprintCallable, Category = "Enchant")
	void NextPage();
	UFUNCTION(BlueprintCallable, Category = "Enchant")
	void PrevPage();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UEnchantInfoPopup> InfoPopup;
	bool bSelectedIsEquipped = false;

	UFUNCTION(BlueprintCallable)
	void ShowEnchantInfo(UEnchantData* Enchant, bool bIsEquipped);
	UFUNCTION(BlueprintCallable)
	void OnPopupActionClicked();

protected:
	virtual void NativeConstruct() override;
};
