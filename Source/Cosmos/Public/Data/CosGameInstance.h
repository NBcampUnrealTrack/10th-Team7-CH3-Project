#pragma once


#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CosGameInstance.generated.h"

class UEnchantData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrencyChanged, int32, NewSoul);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadoutChanged); 

UENUM(BlueprintType)
enum class EShotgunModuleType : uint8
{
	Damage,
	FireSpeed,
	Reload,
	Magazine
};


UCLASS()
class COSMOS_API UCosGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintAssignable, Category = "Soul")
	FOnCurrencyChanged OnCurrencyChanged;
	UPROPERTY(BlueprintAssignable, Category = "Change")
	FOnLoadoutChanged OnLoadoutChange;
	UPROPERTY()
	TArray<UEnchantData*> CollectedEnchant; // 인챈트 인벤토리
	UPROPERTY()
	TArray<UEnchantData*> EquippedEnchant; // 장착한 인텐츠
	
	
	UFUNCTION(BlueprintPure, Category = "Soul")
	int32 GetSoul() const { return Soul; }
	UFUNCTION(BlueprintCallable, Category = "Soul")
	void AddSoul(int32 Amount);
	UFUNCTION(BlueprintCallable, Category = "Soul")
	bool SpendSoul(int32 Amount);
	UFUNCTION(BlueprintCallable, Category = "Enchant")
	bool CollectEnchant(UEnchantData* Enchant);
	UFUNCTION(BlueprintCallable, Category = "Enchant")
	bool EquipEnchant(UEnchantData* Enchant);

	UFUNCTION(BlueprintCallable, Category = "Enchant")
	void UnEquipEnchant(UEnchantData* Enchant);
private:
	UPROPERTY(VisibleAnywhere, Category = "Soul")
	int32 Soul = 0;
};
