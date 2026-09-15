#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CosGameMode.generated.h"

class AWaveSpawner;
class UHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnForgeRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOver);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameClear);

UCLASS()
class COSMOS_API ACosGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACosGameMode();

	UFUNCTION(BlueprintCallable)
	void StartGame();//게임 시작 시 전투 맵으로 이동

	UFUNCTION(BlueprintCallable)
	void StartNextWave();//다음 웨이브 시작, 웨이브 번호 증가(GmaeState에 전달)

	void HandleWaveCleared(int32 WaveIndex);//클리어 신호를 받는 함수(웨이브, 대장간, 보스 여부 판단)

	void HandlePlayerDeath();//플레이어 사망 시

	void ResPawnPlayer();// 플레이어 리스폰

	UPROPERTY(BlueprintAssignable)
	FOnForgeRequested OnForgeRequested;

	UPROPERTY(BlueprintAssignable)
	FOnGameOver OnGameOver;

	UPROPERTY(BlueprintAssignable)

	FOnGameClear OnGameClear;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<AWaveSpawner> WaveSpawner;//WaveSpawner 참조, 게임모드에서 언제 시작할지 결정

	int32 CurrentWaveIndex = 0;//몇번째 웨이브인지

	FTimerHandle RespawnTimer;

	void BindPlayerDeath();

	UPROPERTY()
	TObjectPtr<AWaveSpawner> WaveSpawner;

	int32 CurrentWaveIndex = 0;

	FTimerHandle RespawnTimer;

	
};
