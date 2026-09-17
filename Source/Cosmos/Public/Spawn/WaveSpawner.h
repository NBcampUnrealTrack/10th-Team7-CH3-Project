#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaveSpawner.generated.h"


class AEnemySpawnPoint; // 어디에 적이 생성될지 에디터 상에 찍어주는 EnemySpawnPoint를 전방선언합니다.
class UDataTable; // 웨이브 데이터 테이블 전방선언
class AEnemyBase; // 스폰된 적을 델리게이트로 넘기기 위해 전방선언합니다. 포인터로만 쓰므로 include 없이 충분합니다.


// 델리게이트 선언. 5페이즈 스폰 완료 + 생존 적 0 -> 웨이브당 한 번 방송됨. 게임 모드가 구독합니다.
DECLARE_MULTICAST_DELEGATE_OneParam(FOnWaveCleared, int32 /*WaveIndex*/); // OneParam은 인자 1개를 넘긴다, TwoParam은 인자 2개를 넘긴다. 없으면 안넘김.

// 델리게이트 선언. 적 하나가 스폰에 성공한 순간 방송됨. 게임 모드가 구독합니다.
// 게임모드는 이 방송으로 받은 적의 OnEnemyKilled를 구독해 킬 카운트를 올립니다.
// 스포너가 게임모드를 직접 부르지 않고 방송만 하므로, 스포너는 누가 듣는지 몰라도 됩니다.
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnemySpawned, AEnemyBase* /*SpawnedEnemy*/);

UCLASS()
class COSMOS_API AWaveSpawner : public AActor
{
	GENERATED_BODY()

public:
	AWaveSpawner();

	// 웨이브 하나를 시작합니다. 게임모드가 부르는 유일한 진입점.
	// 내부에서 PhaseInterval(1분)마다 페이즈를 PhasesPerWave(5)개까지 누적 추가합니다.
	UFUNCTION(BlueprintCallable, Category = "Wave") // BlueprintCallable은 실행핀을 뽑을 수 있는 뭔가를 바꿀 수 있는 함수입니다.
		void StartWave(int32 WaveIndex); // WaveIndex를 매개변수로 넣습니다.

	// 진행 중인 스폰을 중단합니다. 이미 스폰된 적을 지우지는 않습니다. 스폰만 중단합니다.
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StopWave();

	// 살아있는 적을 리턴해줍니다. AliveEnemies 라는 살아있는 적 목록을 담는 배열 안에 .Num 지금 몇 마리 들어있는지 알려줍니다. 3마리 살아있으면 3을 돌려줌.
	UFUNCTION(BlueprintPure, Category = "Wave") // BlueprintPure은 실행핀 없고 값만 돌려줍니다.
		int32 GetAliveCount() const // .cpp로 따로 안빼고 인라인 정의함. 
	{
		return AliveEnemies.Num();
	}

	// 이번 웨이브의 남은 시간(초). UI가 5분 카운트다운을 표시할 때 읽습니다.
	// 1분 페이즈는 내부 스폰 박자이므로 노출하지 않습니다.
	// 5분이 지나도 적이 남아 있으면 0에서 멈춥니다.
	UFUNCTION(BlueprintPure, Category = "Wave")
	float GetRemainingWaveTime() const // 헤더에서 UWorld를 몰라도 되도록 AActor 함수인 GetGameTimeSinceCreation을 씁니다.
	{
		if (CurrentPhase == 0) // 웨이브가 시작 전이면 0
		{
			return 0.0f;
		}
		const float TotalTime = PhaseInterval * PhasesPerWave; // 60 × 5 = 300초
		const float Elapsed = GetGameTimeSinceCreation() - WaveStartTime; // 웨이브 시작 후 흐른 시간
		return FMath::Max(0.0f, TotalTime - Elapsed); // 음수로 안 내려가게
	}

	FOnWaveCleared OnWaveCleared; // FOnWaveCleared 델리게이트 타입의 OnWaveCleared 변수를 선언.

	FOnEnemySpawned OnEnemySpawned; // FOnEnemySpawned 델리게이트 타입의 OnEnemySpawned 변수를 선언. 게임모드가 AddUObject로 구독합니다.

protected:
	virtual void BeginPlay() override;


private: // 내부함수들이기 때문에 private임. 바깥에 쓰는 것만 public
	// SpawnOne은 적하나를 실제로 만드는 함수입니다. SpawnTimer(1초)마다 순서표에서 한 마리씩 꺼냅니다.
	void SpawnOne();

	// PhaseTimer(60초)마다 호출되어 다음 페이즈의 적을 순서표 뒤에 추가합니다.
	void AddNextPhase();

	UFUNCTION() // 적이 파괴되었을 때 언리얼이 자동 호출합니다.
		// cpp에서 Enemy->OnDestroyed.AddDynamic(this, ...)으로 등록해두기 때문입니다.
		// AddDynamic은 UFUNCTION() 매크로가 붙은 함수만 받습니다.
		void HandleEnemyDestroyed(AActor* DestroyedActor);


	// 레벨의 AEnemySpawnPoint를 전부 찾아 SpawnPoints에 담아줍니다. 패키징할 때 문제가 있어서 넣어봤습니다.
	// BeginPlay와 StartWave 양쪽에서 부르므로 함수로 일단 분리했습니다만, 9월15일 알파패키징 이후로는 쓸모가 없을 것 같다고 생각중입니다.
	// PIE 할 때랑 패키징 후랑 액터의 순서차이가 날 수도 있나본데요. 아무튼 그렇습니다.
	void CollectSpawnPoints();

	// [DT] 게임모드가 넘긴 번호(1~26)를 DT의 Row Name(Stage1_Wave1의 형식)으로 바꿔줍니다.
	FName MakeRowName(int32 WaveIndex) const;

	// -- 에디터 설정값 
	// 웨이브별 스폰 수가 담긴 데이터 테이블. 에디터 Details에서 DT 에셋을 꽂습니다.

	UPROPERTY(EditAnywhere, Category = "Wave")
	TObjectPtr<UDataTable> WaveDataTable;

	UPROPERTY(EditAnywhere, Category = "Wave|Enemy")
	TSubclassOf<AActor> GhoulClass;

	UPROPERTY(EditAnywhere, Category = "Wave|Enemy")
	TSubclassOf<AActor> EnhancedGhoulClass;

	UPROPERTY(EditAnywhere, Category = "Wave|Enemy")
	TSubclassOf<AActor> GargoyleClass;

	UPROPERTY(EditAnywhere, Category = "Wave|Enemy")
	TSubclassOf<AActor> CrowClass;

	UPROPERTY(EditAnywhere, Category = "Wave|Enemy")
	TSubclassOf<AActor> BossClass;

	UPROPERTY(EditAnywhere, Category = "Wave", meta = (ClampMin = "0.1"))  // 0을 넣으면 무한생성됩니다. 0.1로 막아놓은 것.
		float SpawnInterval = 1.0f; // 스폰하는 인터벌

	// 페이즈 사이 간격(초). 기획서 기준 1분입니다.
	UPROPERTY(EditAnywhere, Category = "Wave", meta = (ClampMin = "1.0"))
	float PhaseInterval = 60.0f;

	// 한 웨이브에 들어있는 페이즈 수. 기획서 기준 5개입니다.
	UPROPERTY(EditAnywhere, Category = "Wave", meta = (ClampMin = "1"))
	int32 PhasesPerWave = 5;

	// ---런타임 상태--- 아래는 에디터에서 설정하는 값이 아니라 게임이 돌면서 변하는 값들입니다.
	UPROPERTY()
	TArray<TObjectPtr<AEnemySpawnPoint>> SpawnPoints;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> AliveEnemies;
	UPROPERTY()
	TArray<TSubclassOf<AActor>> SpawnQueue;

	FTimerHandle SpawnTimer; // 타이머의 이름표입니다. 타이머 그자체는 아니고 내가 걸어놓은 타이머를 찾기 위한 식별자입니다.
	FTimerHandle PhaseTimer; // 60초 페이즈 타이머의 이름표. SpawnTimer(1초)와 별개라 서로 안 꺼집니다.
	int32 CurrentWave = 0; // 지금이 몇 번째 웨이브인지 클리어 방송할 때 이 번호를 같이 넘깁니다.
	int32 CurrentPhase = 0; // 지금 몇 번째 페이즈까지 추가했는지 (1~5). 0이면 웨이브 시작 전.
	int32 SpawnedCount = 0; // 이 웨이브에서 지금까지 몇 마리 만들었는지.
	int32 TargetSpawnCount = 0; // 이번 웨이브에 만들 총 마리 수 . 에디터가 아니라 DT에서 채워서 작동함.
	bool bIsSpawning = false; // 아직 스폰중인지?
	bool bAllPhasesQueued = false; // 5페이즈가 전부 순서표에 들어갔는지. 클리어 조기 방송 방지용.
	float WaveStartTime = 0.0f; // 웨이브가 시작된 시각. GetGameTimeSinceCreation() 기준으로 저장합니다.
};