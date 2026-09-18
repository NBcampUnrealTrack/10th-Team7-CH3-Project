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
	void StartNextWave();

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
	
	float CurrentStageStartTime = 0.0f;//현재 스테이지가 시작된 시간을 저장함. 현재시간 - 이 값으로 엉ㄹ마나 지났는지 알 수 있음

	float StageDuration = 300.0f;//제한시작 : 5분

	FTimerHandle StageUpdateTimer;

	void StartStageTimer();//스테이지 제한시간 시작

	void StopStageTimer();//타이머 정지, 스테이지 클리어, 플레이어 사망 등에 사용

	void HandleStageTimeout();

	void UpdateStageTimer();//남은시간 계산, 게임스테이트 전달

	void TriggerGameOver();//타임오버/ 사망하는 경우 모아두려고 넣음

	// 같은 순간에 사망 + 시간 초과가 같이 발생했을 때 GameOver가 두 번 실행되는 것을 막기 위함
	bool bGameOver = false;

	int32 CycleStartSoul = 0;


	EPostResultAction PendingAction = EPostResultAction::NextWave;//결과창 닫은 후 실행할 행동
	
};
