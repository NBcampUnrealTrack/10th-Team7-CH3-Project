#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CosGameMode.generated.h"

class AWaveSpawner;
class AEnemyBase;
class ATutorialDeer;
class APlayerStart;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnForgeRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOver);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameClear);
// Day가 시작될 때 방송합니다. 대사 등 연출은 BP/UI가 구독해서 붙입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDayStarted, int32, Day);
// 보스 스테이지가 시작될 때 OnDayStarted 다음에 방송합니다. 보스 등장 연출·UI는 BP가 구독해서 붙입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossStageStarted);

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

	// Day 번호. Day 1 = 사슴(스테이지 0), Day 2~6 = 스테이지 1~5, Day 7 = 보스(스테이지 6)
	UFUNCTION(BlueprintPure, Category = "Day")
	int32 GetCurrentDay() const { return CurrentWaveIndex + 1; }

	UFUNCTION(BlueprintPure, Category = "Day")
	bool IsInDay1() const { return bInDay1; }

	// 지금 스테이지가 보스 스테이지(기본 6)인지. 보스 스테이지 번호는 WaveSpawner가 가지고 있습니다.
	UFUNCTION(BlueprintPure, Category = "Day")
	bool IsBossStage() const { return IsBossStageIndex(CurrentWaveIndex); }

	UPROPERTY(BlueprintAssignable)
	FOnForgeRequested OnForgeRequested;

	UPROPERTY(BlueprintAssignable)
	FOnGameOver OnGameOver;

	UPROPERTY(BlueprintAssignable)

	FOnGameClear OnGameClear;

	UPROPERTY(BlueprintAssignable, Category = "Day")
	FOnDayStarted OnDayStarted;

	UPROPERTY(BlueprintAssignable, Category = "Day")
	FOnBossStageStarted OnBossStageStarted;


protected:
	virtual void BeginPlay() override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// 사슴이 죽은 뒤 나타나는 구울 수
	UPROPERTY(EditDefaultsOnly, Category = "Day|Day1", meta = (ClampMin = "0"))
	int32 Day1GhoulCount = 3;

	// 사슴 사망 후 구울 스폰을 시작하기까지의 시간(초)
	UPROPERTY(EditDefaultsOnly, Category = "Day|Day1", meta = (ClampMin = "0.0"))
	float Day1GhoulDelay = 2.0f;

	// Day 1 종료 시 지급하는 기본 재화. 첫 대장간에서 강화를 한 번 돌릴 수 있는 양으로 맞춥니다.
	UPROPERTY(EditDefaultsOnly, Category = "Day|Day1", meta = (ClampMin = "0"))
	int32 Day1SoulReward = 300;

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

	void PushEnemyCountUI();

	bool IsBossStageIndex(int32 StageIndex) const;

	// Player Start Tag 또는 Actor Tags에 "Day{N}"이 있는 PlayerStart를 찾습니다.
	APlayerStart* FindDayStart(int32 Day) const;

	// Day 시작 시 플레이어를 시작 지점으로 옮깁니다. 지점이 N개면 Day1부터 1..N을 반복합니다.
	void MovePlayerToDayStart(int32 Day);

	// 레벨에 사슴이 있으면 Day 1로 시작합니다. 없으면 기존처럼 바로 Stage 1 전투입니다.
	void StartDay1(ATutorialDeer* Deer);

	UFUNCTION()
	void HandleDeerKilled(ATutorialDeer* Deer);

	void SpawnDay1Ghouls();

	bool bInDay1 = false;

	FTimerHandle Day1GhoulTimer;

	// 같은 순간에 사망 + 시간 초과가 같이 발생했을 때 GameOver가 두 번 실행되는 것을 막기 위함
	bool bGameOver = false;

	int32 CycleStartSoul = 0;

	int32 TotalEnemiesThisWave = 0;     // 추가. 이번 웨이브에 스폰된 총 적 수

	int32 RemainingEnemiesThisWave = 0; // 추가. 이번 웨이브에 아직 살아있는 적 수

	EPostResultAction PendingAction = EPostResultAction::NextWave;//결과창 닫은 후 실행할 행동
	
};
