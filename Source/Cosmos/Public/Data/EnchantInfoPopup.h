#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/EnchantData.h"
#include "EnchantInfoPopup.generated.h"

class UTextBlock;
class UButton;
UCLASS()
class COSMOS_API UEnchantInfoPopup : public UUserWidget
{
	GENERATED_BODY()
	

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionButtonLabel;
	UPROPERTY(meta = (BindWidget))
	UButton* ActionButton;
	
	UFUNCTION(BlueprintCallable)
	void Setup(UEnchantData* Enchant, bool bIsEquipped);
};
