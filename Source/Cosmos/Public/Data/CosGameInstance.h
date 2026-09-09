#pragma once


#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CosGameInstance.generated.h"

UENUM(BlueprintType)
enum class EShotgunModuleType : uint8
{
	Muzzle,
	Magazine,
	Stock,
	Sight,
	Trigger
};


UCLASS()
class COSMOS_API UCosGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
};
