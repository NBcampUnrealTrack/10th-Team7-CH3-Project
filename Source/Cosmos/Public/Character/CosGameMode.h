#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CosGameMode.generated.h"

class AWaveSpawner;
class AEnemyBase;
class ATutorialDeer;
class APlayerStart;
class USoundBase;
class AEnchantPickup;
class UEnchantData;

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
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// 사슴이 죽은 뒤 나타나는 구울 수
	UPROPERTY(EditDefaultsOnly, Category = "Day|Day1", meta = (ClampMin = "0"))
	int32 Day1GhoulCount = 10;

	// 사슴 사망 후 구울 스폰을 시작하기까지의 시간(초)
	UPROPERTY(EditDefaultsOnly, Category = "Day|Day1", meta = (ClampMin = "0.0"))
	float Day1GhoulDelay = 2.0f;

	// Day 1 종료 시 지급하는 기본 재화. 첫 대장간에서 강화를 한 번 돌릴 수 있는 양으로 맞춥니다.
	UPROPERTY(EditDefaultsOnly, Category = "Day|Day1", meta = (ClampMin = "0"))
	int32 Day1SoulReward = 300;

	// 사슴(염소)이 죽는 순간 그 자리에서 재생할 비명. 비워두면 재생하지 않습니다.
	UPROPERTY(EditDefaultsOnly, Category = "Day|Day1|Sound")
	TObjectPtr<USoundBase> Day1DeerDeathSound;

	// 사망 후 저주가 걸리는 소리(화면 전체에 들림)
	UPROPERTY(EditDefaultsOnly, Category = "Day|Day1|Sound")
	TObjectPtr<USoundBase> Day1CurseSound;

	// 사망 후 몇 초 뒤에 저주 소리를 낼지
	UPROPERTY(EditDefaultsOnly, Category = "Day|Day1|Sound", meta = (ClampMin = "0.0"))
	float Day1CurseSoundDelay = 0.5f;

	// 귀신들이 울부짖는 소리(화면 전체에 들림). 안개가 짙어지는 동안 겹쳐 들리게 합니다.
	UPROPERTY(EditDefaultsOnly, Category = "Day|Day1|Sound")
	TObjectPtr<USoundBase> Day1GhostWailSound;

	// 사망 후 몇 초 뒤에 울부짖는 소리를 낼지. 구울 등장(Day1GhoulDelay)보다 조금 앞이 자연스럽습니다.
	UPROPERTY(EditDefaultsOnly, Category = "Day|Day1|Sound", meta = (ClampMin = "0.0"))
	float Day1GhostWailSoundDelay = 1.5f;

	// Day 1에 인챈트가 하나도 안 떨어졌을 때 마지막 구울 자리에 떨굴 인챈트. 비워두면 보장하지 않습니다.
	UPROPERTY(EditDefaultsOnly, Category = "Day|Day1|Enchant")
	TSubclassOf<AEnchantPickup> Day1EnchantPickupClass;

	// 사슴이 죽는 순간 화면이 번쩍이는 색
	UPROPERTY(EditDefaultsOnly, Category = "Day|Day1|Flash")
	FLinearColor Day1FlashColor = FLinearColor(0.6f, 0.0f, 0.0f);

	// 번쩍이는 순간의 진하기(0~1). 1이면 화면이 완전히 그 색으로 덮입니다.
	UPROPERTY(EditDefaultsOnly, Category = "Day|Day1|Flash", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Day1FlashAlpha = 0.6f;

	// 번쩍인 뒤 원래 화면으로 돌아오는 시간(초)
	UPROPERTY(EditDefaultsOnly, Category = "Day|Day1|Flash", meta = (ClampMin = "0.0"))
	float Day1FlashDuration = 0.8f;

private:
	void PlayDay1Sound(USoundBase* Sound);

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

	// Day 1 인챈트 보장: 떨어진 수를 세고, 하나라도 먹어야 Day 1을 끝냅니다.
	void HandleDay1ActorSpawned(AActor* SpawnedActor);
	UFUNCTION()
	void HandleDay1EnchantCollected(UEnchantData* CollectedEnchant);
	void StopTrackingDay1Enchant();

	FDelegateHandle Day1ActorSpawnedHandle;
	int32 Day1EnchantDropCount = 0;
	bool bDay1EnchantCollected = false;
	bool bDay1ClearWaitingForEnchant = false;
	FVector LastDay1KillLocation = FVector::ZeroVector;

	FTimerHandle Day1GhoulTimer;
	FTimerHandle Day1CurseSoundTimer;
	FTimerHandle Day1GhostWailSoundTimer;

	// 같은 순간에 사망 + 시간 초과가 같이 발생했을 때 GameOver가 두 번 실행되는 것을 막기 위함
	bool bGameOver = false;

	int32 CycleStartSoul = 0;

	int32 TotalEnemiesThisWave = 0;     // 추가. 이번 웨이브에 스폰된 총 적 수

	int32 RemainingEnemiesThisWave = 0; // 추가. 이번 웨이브에 아직 살아있는 적 수

	EPostResultAction PendingAction = EPostResultAction::NextWave;//결과창 닫은 후 실행할 행동
	
};
