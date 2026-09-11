#pragma once


#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CosGameInstance.generated.h"

class UEnchantData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrencyChanged, int32, NewSoul);

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
	UFUNCTION(BlueprintPure, Category = "Soul")
	int32 GetSoul() const { return Soul; }
	UFUNCTION(BlueprintCallable, Category = "Soul")
	void AddSoul(int32 Amount);
	UFUNCTION(BlueprintCallable, Category = "Soul")
	bool SpendSoul(int32 Amount);
	UPROPERTY(BlueprintAssignable, Category = "Soul")
	FOnCurrencyChanged OnCurrencyChanged;

	UFUNCTION(BlueprintCallable, Category = "Enchant")
	void EquipEnchant(UEnchantData* Enchant);

	UFUNCTION(BlueprintCallable, Category = "Enchant")
	void UnEquipEnchant(UEnchantData* Enchant);
private:
	UPROPERTY(VisibleAnywhere, Category = "Soul")
	int32 Soul = 0;
};
