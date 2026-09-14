#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CosGameMode.generated.h"

class AWaveSpawner;

UCLASS()
class COSMOS_API ACosGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACosGameMode();

	UFUNCTION(BlueprintCallable)
	void StartGame();

	UFUNCTION(BlueprintCallable)
	void StartNextWave();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<AWaveSpawner> WaveSpawner;

	int32 CrrentWaveIndex = 0;//몇번째 웨이브인지

	
};
