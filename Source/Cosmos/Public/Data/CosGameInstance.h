#pragma once


#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CosDataTable.h"
#include "CosGameInstance.generated.h"

class UEnchantData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrencyChanged, int32, NewSoul);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadoutChanged); 





UCLASS()
class COSMOS_API UCosGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
	
public:

	UPROPERTY(BlueprintAssignable, Category = "Soul")
	FOnCurrencyChanged OnCurrencyChanged;
	UPROPERTY(BlueprintAssignable, Category = "Change")
	FOnLoadoutChanged OnLoadoutChange;
	UPROPERTY(BlueprintReadOnly, Category = "Enchant")
	TArray<UEnchantData*> CollectedEnchant; // 인챈트 인벤토리
	UPROPERTY(BlueprintReadOnly, Category = "Enchant")
	TArray<UEnchantData*> EquippedEnchant; // 장착한 인챈트
	UPROPERTY(BlueprintReadOnly, Category = "Enchant")
	int32 EnchantMaxEquippedCount = 3;
	UPROPERTY(BlueprintReadOnly, Category = "Enchant")
	int32 EnchantMaxCollectedCount = 16;

	UPROPERTY(BlueprintReadOnly, Category = "Upgrade")
	TMap<EShotgunModuleType, int32> ModuleLevels;
	UPROPERTY(EditDefaultsOnly, Category = "Upgrade")
	UDataTable* UpgradePool;
	
	UFUNCTION(BlueprintPure, Category = "Soul")
	int32 GetSoul() const;
	UFUNCTION(BlueprintCallable, Category = "Soul")
	void AddSoul(int32 Amount);
	UFUNCTION(BlueprintCallable, Category = "Soul")
	bool SpendSoul(int32 Amount);
	UFUNCTION(BlueprintCallable, Category = "Enchant")
	bool CollectEnchant(UEnchantData* Enchant);
	UFUNCTION(BlueprintCallable, Category = "Enchant")
	bool EquipEnchant(UEnchantData* Enchant);

	// 샷건 변수
	// constexpr -> 컴파일할 때 이미 값을 알 수 있어야 할 때 사용함.
	// 샷건 MaxLevel은 컴파일 할 때 이미 값을 알고 있어야 강화쪽에서 에러가 안남
	static constexpr int32 ShotgunMaxUpgradeLevel = 6;
	
	// 인챈트 함수

	UFUNCTION(BlueprintCallable, Category = "Enchant")
	void UnEquipEnchant(UEnchantData* Enchant);
	UFUNCTION(BlueprintPure, Category = "Enchant")
	float GetTotalStat(EEnchantStat Stat) const;
	UFUNCTION(BlueprintCallable, Category = "Enchant")
	void IncreaseMaxEquippedEnchant(int32 Amount);

	// 샷건 함수

	UFUNCTION(BlueprintCallable, Category = "Shotgun")
	bool UpgradeShotgun(EShotgunModuleType Type);
	const FUpgradeData* FindUpgradeData(EShotgunModuleType Type, int32 Level) const;
	UFUNCTION(BlueprintPure, Category = "Upgrade")
	float GetShotgunStat(EShotgunModuleType Type) const;
	UFUNCTION(BlueprintPure, Category = "Shotgun")
	int32 GetModuleLevel(EShotgunModuleType Type) const { return ModuleLevels.FindRef(Type); }
	UFUNCTION(BlueprintPure, Category = "Shotgun")
	int32 GetMaxUpgradeLevel(EShotgunModuleType Type) const { return ShotgunMaxUpgradeLevel; }

	// 테스트
	UFUNCTION(Exec)
	void Debug_AddSoul(int32 Amount);
private:
	UPROPERTY(VisibleAnywhere, Category = "Soul")
	int32 Soul = 999;
};
