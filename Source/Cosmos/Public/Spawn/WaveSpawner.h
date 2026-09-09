#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaveSpawner.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnWaveCleared, int32 /*waveIndex*/);

UCLASS()
class COSMOS_API AWaveSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AWaveSpawner();
	
};
