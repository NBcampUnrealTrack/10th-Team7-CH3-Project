#pragma once


#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CosGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSoulChanged, int32, NewSoul);

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
	bool TrySpendSoul(int32 Amount);
	UPROPERTY(BlueprintAssignable, Category = "Soul")
	FOnSoulChanged OnSoulChanged;

private:
	UPROPERTY(VisibleAnywhere, Category = "Soul")
	int32 Soul = 0;
};
