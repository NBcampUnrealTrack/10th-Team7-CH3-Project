#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CosGameMode.generated.h"

class AWaveSpawner;
class AEnemyBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnForgeRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOver);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameClear);

UENUM()
enum class EPostResultAction : uint8
{
	NextWave,
	Forge,
	GameClear
};

UCLASS()
class COSMOS_API ACosGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACosGameMode();

	UFUNCTION(BlueprintCallable)
	void OnResultConfirmed();

	UFUNCTION(BlueprintCallable)
	void StartGame();//게임 시작 시 전투 맵으로 이동

	UFUNCTION(BlueprintCallable)
	void StartNextWave(bool bResetTimer = false);

	void HandleWaveCleared(int32 WaveIndex);//클리어 신호를 받는 함수(웨이브, 대장간, 보스 여부 판단)


	UFUNCTION(BlueprintCallable)
	void HandlePlayerDeath();

	void HandleEnemySpawned(AEnemyBase* Enemy);//새로 스폰된 적의 사망 델리게이트를 구독함


	void HandleEnemyKilled(AEnemyBase* DeadEnemy);	// 적 사망 신호를 받으면 GameState의 킬 수를 증가시킴

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
	
	float CurrentWaveStartTime = 0.0f;// 현재 웨이브 구간 시작 시간 추가

	EPostResultAction PendingAction = EPostResultAction::NextWave;// 결과창 닫은 후 실행할 행동 추가
	
};
