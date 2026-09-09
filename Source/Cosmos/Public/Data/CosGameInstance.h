#pragma once


#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CosGameInstance.generated.h"

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
	
};
