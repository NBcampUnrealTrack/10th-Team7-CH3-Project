#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaveSpawner.generated.h"


class AEnemySpawnPoint; // 플레이어 근처 위치를 못 찾았을 때 대신 쓸 예비 스폰 위치입니다.
class UDataTable; // 스테이지·웨이브 데이터 테이블 전방선언
class AEnemyBase; // 스폰된 적을 델리게이트로 넘기기 위해 전방선언합니다. 포인터로만 쓰므로 include 없이 충분합니다.
class ADirectionalLight;
class ASkyLight;
class AExponentialHeightFog;
class UExponentialHeightFogComponent;

// 맵에 설정된 원래 값에 곱합니다. 1이면 유지, 밝기는 작을수록 어둡고 안개는 클수록 짙습니다.
USTRUCT(BlueprintType)
struct FStageEnvironmentSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0"))
	float MoonlightMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0"))
	float SkylightMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0"))
	float FogDensityMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0"))
	float VolumetricExtinctionMultiplier = 1.0f;
};


// 용어
//   Stage : 게임모드가 시작시키는 단위. 끝나면 대장간으로 갑니다. UI에 남은 시간이 노출됩니다.
//           Day 1(Stage 0)과 보스 스테이지는 전멸하면 클리어, 그 사이 스테이지는 스테이지 타이머를 버티면 클리어(생존)입니다.
//   Wave (1분 간격) : 전멸하거나 1분이 지나면 다음 적을 추가하는 내부 단위. UI에 노출하지 않습니다.
//           생존 스테이지는 DT의 웨이브를 다 쓰면 마지막 웨이브를 반복합니다.


// 델리게이트 선언. 모든 웨이브가 순서표에 들어갔고 + 전부 스폰됐고 + 생존 적 0 -> 스테이지당 한 번 방송됨. 게임 모드가 구독합니다.
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

	// 스테이지 하나를 시작합니다. 게임모드가 부르는 유일한 진입점.
	// 함수 시그니처는 팀 합의대로 StartWave를 유지합니다. 넘기는 번호는 스테이지 번호입니다.
	// 게임모드는 스테이지 단위(1, 2, 3...)로 한 번만 부릅니다. 1분 웨이브 단위로 부르지 않습니다.
	// 내부에서 웨이브 1을 즉시 추가하고, 이후 전멸하거나 WaveInterval(60초)이 지나면 WavesPerStage(5)개까지 추가합니다.
	UFUNCTION(BlueprintCallable, Category = "Wave") // BlueprintCallable은 실행핀을 뽑을 수 있는 뭔가를 바꿀 수 있는 함수입니다.
		void StartWave(int32 WaveIndex); // WaveIndex = 스테이지 번호

	// Day 1 전용. DT 없이 구울만 GhoulCount마리 넣고, 전멸하면 Stage 0 클리어를 방송합니다.
	// 웨이브 추가·환경 변경은 하지 않습니다.
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StartTutorialStage(int32 GhoulCount);

	// 진행 중인 스폰과 웨이브 추가를 중단합니다. 이미 스폰된 적을 지우지는 않습니다.
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StopWave();

	// 살아 있는 적과 날아가던 적 투사체를 소울 없이 치웁니다. 생존 스테이지가 끝날 때 게임모드가 부릅니다.
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void ClearRemainingEnemies();

	// 지금 스테이지가 생존 조건인지. Day 1(Stage 0)과 보스 스테이지가 아니면 생존입니다.
	UFUNCTION(BlueprintPure, Category = "Wave")
	bool IsSurvivalStage() const;

	// 게임 중 환경만 적용합니다. 웨이브/적/타이머는 진행시키지 않습니다.
	UFUNCTION(BlueprintCallable, Category = "Wave|Environment")
	void ApplyStageEnvironment(int32 StageIndex);

	// 맵의 Height Fog를 전부 숨기거나 다시 보이게 합니다. Day 1(안개 없음)용.
	// 밀도 값은 건드리지 않으므로 스테이지 환경 배율의 기준값이 바뀌지 않습니다.
	UFUNCTION(BlueprintCallable, Category = "Wave|Environment")
	void SetFogVisible(bool bVisible);

	// Day 1 사슴 사망 연출. 숨겨둔 안개를 밀도 0에서 시작해 TutorialFogFadeDuration초 동안
	// 맵 원래 밀도 × TutorialFogDensityMultiplier까지 점점 짙게 만듭니다. StartWave에서 원래 밀도로 되돌립니다.
	UFUNCTION(BlueprintCallable, Category = "Wave|Environment")
	void StartTutorialFogFadeIn();

	// 에디터에서만 안개를 비교합니다. 조명, 전투와 원본 안개 값은 변경하지 않습니다.
	UFUNCTION(CallInEditor, Category = "Wave|Fog Preview", meta = (DisplayName = "Preview Fog"))
	void PreviewFog();

	UFUNCTION(CallInEditor, Category = "Wave|Fog Preview", meta = (DisplayName = "Restore Fog Preview"))
	void RestoreFogPreview();

	// 살아있는 적 수. AliveEnemies 배열에 지금 몇 마리 들어있는지 알려줍니다.
	UFUNCTION(BlueprintPure, Category = "Wave") // BlueprintPure은 실행핀 없고 값만 돌려줍니다.
		int32 GetAliveCount() const // .cpp로 따로 안빼고 인라인 정의함. 
	{
		return AliveEnemies.Num();
	}

	// 이번 스테이지의 남은 시간(초). UI가 5분 카운트다운을 표시할 때 읽습니다.
	// 1분 웨이브는 내부 스폰 박자이므로 노출하지 않습니다.
	// 5분이 지나도 적이 남아 있으면 0에서 멈춥니다.
	UFUNCTION(BlueprintPure, Category = "Wave")
	float GetRemainingWaveTime() const // 헤더에서 UWorld를 몰라도 되도록 AActor 함수인 GetGameTimeSinceCreation을 씁니다.
	{
		if (!bIsStageActive) // 스테이지 진행 중이 아니면 0
		{
			return 0.0f;
		}
		const float TotalTime = WaveInterval * WavesPerStage; // 60 × 5 = 300초
		const float Elapsed = GetGameTimeSinceCreation() - StageStartTime; // 스테이지 시작 후 흐른 시간
		return FMath::Max(0.0f, TotalTime - Elapsed); // 음수로 안 내려가게
	}

	// 보스 스테이지 번호. 게임모드도 이 값을 읽어 보스 BGM·게임 클리어를 판단합니다.
	UFUNCTION(BlueprintPure, Category = "Wave|Boss")
	int32 GetBossStageIndex() const
	{
		return BossStageIndex;
	}

	FOnWaveCleared OnWaveCleared; // 스테이지(5분) 클리어 시 방송. 게임모드가 AddUObject로 구독합니다.

	FOnEnemySpawned OnEnemySpawned; // 게임모드가 AddUObject로 구독합니다.

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

#if WITH_EDITOR
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif


private: // 내부함수들이기 때문에 private임. 바깥에 쓰는 것만 public

	// WaveTimer(60초)마다 호출되어 다음 웨이브의 적을 순서표 "뒤에" 추가합니다. 5) 각주
	void AddNextWave();

	// SpawnTimer(1초)마다 호출되어 순서표에서 한 마리씩 꺼내 실제로 만듭니다.
	void SpawnOne();

	// 플레이어 주변 NavMesh 위에서 스폰 위치를 찾습니다. 찾으면 true, 실패하면 false. 6) 각주
	bool FindSpawnLocationNearPlayer(FVector& OutLocation);

	// 전멸하면 다음 웨이브를 예약하고, 마지막 웨이브면 클리어를 한 번 방송합니다.
	// 적 사망 / 스폰 종료 / 마지막 웨이브 추가 세 곳에서 부릅니다.
	void TryBroadcastStageCleared();

	void HandleEnemyKilled(AEnemyBase* KilledEnemy);

	UFUNCTION() // 적이 파괴되었을 때 언리얼이 자동 호출합니다.
		// cpp에서 Enemy->OnDestroyed.AddDynamic(this, ...)으로 등록해두기 때문입니다.
		// AddDynamic은 UFUNCTION() 매크로가 붙은 함수만 받습니다.
		void HandleEnemyDestroyed(AActor* DestroyedActor);

	// 보스는 제자리에서 BossPoint 사이를 텔레포트하므로, 플레이어와 가장 가까운 BossPoint에 스폰합니다.
	// BossPoint가 하나도 없으면 false를 돌려주고, SpawnOne이 일반 적과 같은 방식으로 위치를 찾습니다.
	bool FindBossSpawnLocation(TSubclassOf<AActor> BossToSpawn, FVector& OutLocation, FRotator& OutRotation) const;

	// 레벨의 AEnemySpawnPoint를 전부 찾아 SpawnPoints에 담아줍니다.
	// 이제는 플레이어 근처 위치를 못 찾았을 때만 쓰는 예비용입니다.
	void CollectSpawnPoints();

	// 이전 스테이지 타이머를 끄고 스테이지 상태를 처음으로 되돌립니다. StartWave / StartTutorialStage 공용.
	void ResetStageState(int32 StageIndex);

	// [DT] 스테이지 번호 + 웨이브 번호를 DT의 Row Name("Stage2_Wave3" 형식)으로 바꿔줍니다.
	FName MakeRowName(int32 StageIndex, int32 WaveIndex) const;

	// -- 에디터 설정값 

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditInstanceOnly, Transient, Category = "Wave|Fog Preview", meta = (ClampMin = "1", UIMin = "1", UIMax = "6"))
	int32 PreviewStage = 1;

	// 저장/PIE 복제에서 제외되는 비교용 컴포넌트입니다. InstanceComponents에는 넣지 않습니다.
	UPROPERTY(Transient, DuplicateTransient)
	TObjectPtr<UExponentialHeightFogComponent> FogPreviewComponent;

	UPROPERTY(Transient, DuplicateTransient)
	TWeakObjectPtr<AExponentialHeightFog> FogPreviewSource;

	UPROPERTY(Transient, DuplicateTransient)
	bool bFogPreviewSourceWasHidden = false;
#endif

	friend class FWaveFogPreviewTest;

	// 레벨에 배치한 스포너에서 대상 액터를 지정하고 켭니다.
	UPROPERTY(EditAnywhere, Category = "Wave|Environment")
	bool bEnableStageEnvironment = false;

	UPROPERTY(EditInstanceOnly, Category = "Wave|Environment")
	TObjectPtr<ADirectionalLight> EnvironmentMoonlight;

	UPROPERTY(EditInstanceOnly, Category = "Wave|Environment")
	TObjectPtr<ASkyLight> EnvironmentSkylight;

	UPROPERTY(EditInstanceOnly, Category = "Wave|Environment")
	TObjectPtr<AExponentialHeightFog> EnvironmentFog;

	// 키는 스테이지 번호입니다. 1~5는 일반 스테이지, 6은 현재 게임모드의 보스 스테이지입니다.
	// 설정이 없는 번호는 현재 환경을 유지합니다.
	UPROPERTY(EditAnywhere, Category = "Wave|Environment")
	TMap<int32, FStageEnvironmentSettings> StageEnvironments;

	// Day 1 사슴이 죽은 뒤 안개가 최종 밀도까지 짙어지는 데 걸리는 시간(초). 0이면 즉시.
	UPROPERTY(EditAnywhere, Category = "Wave|Tutorial Fog", meta = (ClampMin = "0.0"))
	float TutorialFogFadeDuration = 10.0f;

	// 맵에 설정된 원래 안개 밀도에 곱할 최종 배율. 클수록 짙습니다.
	UPROPERTY(EditAnywhere, Category = "Wave|Tutorial Fog", meta = (ClampMin = "0.0"))
	float TutorialFogDensityMultiplier = 3.0f;

	// 스테이지·웨이브별 스폰 수가 담긴 데이터 테이블. 에디터 Details에서 DT 에셋을 꽂습니다.
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
	TSubclassOf<AActor> SprinterClass;

	UPROPERTY(EditAnywhere, Category = "Wave|Enemy")
	TSubclassOf<AActor> BossClass;

	// 스테이지 5 다음 스테이지. 이 스테이지의 웨이브 1에 보스가 없으면 1마리를 넣어 보스 등장을 보장합니다.
	UPROPERTY(EditAnywhere, Category = "Wave|Boss", meta = (ClampMin = "1"))
	int32 BossStageIndex = 6;

	// 보스가 스폰·텔레포트할 지점의 액터 태그. ABoss::BossPointTag와 같은 값이어야 합니다.
	UPROPERTY(EditAnywhere, Category = "Wave|Boss")
	FName BossSpawnPointTag = TEXT("BossPoint");

	UPROPERTY(EditAnywhere, Category = "Wave", meta = (ClampMin = "0.1"))  // 0을 넣으면 무한생성됩니다. 0.1로 막아놓은 것.
		float SpawnInterval = 1.0f; // 한 마리씩 꺼내는 간격(초)

	// 웨이브 사이 간격(초). 기획서 기준 1분입니다.
	UPROPERTY(EditAnywhere, Category = "Wave", meta = (ClampMin = "1.0"))
	float WaveInterval = 60.0f;

	// 한 스테이지에 들어있는 웨이브 수. 기획서 기준 5개입니다.
	UPROPERTY(EditAnywhere, Category = "Wave", meta = (ClampMin = "1"))
	int32 WavesPerStage = 5;

	// 플레이어로부터 최소 스폰 거리(cm). 안개 시야 밖이어야 하므로 시야 거리(기준선 25m)보다 크게 둡니다.
	UPROPERTY(EditAnywhere, Category = "Wave|Spawn", meta = (ClampMin = "0.0"))
	float MinSpawnRadius = 3000.0f;

	// 플레이어로부터 최대 스폰 거리(cm).
	UPROPERTY(EditAnywhere, Category = "Wave|Spawn", meta = (ClampMin = "0.0"))
	float MaxSpawnRadius = 4000.0f;

	// Day 1(튜토리얼) 구울의 최소/최대 스폰 거리(cm). 안개가 짙어지는 연출 속이라 일반 스테이지보다 가깝게 둡니다.
	UPROPERTY(EditAnywhere, Category = "Wave|Spawn", meta = (ClampMin = "0.0"))
	float TutorialMinSpawnRadius = 1200.0f;

	UPROPERTY(EditAnywhere, Category = "Wave|Spawn", meta = (ClampMin = "0.0"))
	float TutorialMaxSpawnRadius = 2000.0f;

	// 지상 적이 이 시간(초) 동안 거의 움직이지 못하면 끼인 것으로 보고 플레이어 근처 새 위치로 옮깁니다. 0이면 끕니다.
	// 처음엔 Day 1 전용이라 이름에 Tutorial이 붙어 있지만, 지금은 모든 스테이지에 적용됩니다. (BP에 저장된 값 유지를 위해 이름은 그대로 둠)
	UPROPERTY(EditAnywhere, Category = "Wave|Spawn", meta = (ClampMin = "0.0"))
	float TutorialStuckSeconds = 4.0f;

	// 360도를 몇 칸으로 나눠 돌아가며 쓸지. 한쪽으로 몰리지 않고 사방에서 나오게 합니다.
	UPROPERTY(EditAnywhere, Category = "Wave|Spawn", meta = (ClampMin = "1"))
	int32 SpawnDirectionSlices = 8;

	// 적당한 위치를 못 찾았을 때 몇 번까지 다시 뽑을지.
	UPROPERTY(EditAnywhere, Category = "Wave|Spawn", meta = (ClampMin = "1"))
	int32 MaxSpawnAttempts = 10;

	// 경로 길이가 직선거리의 몇 배를 넘으면 버릴지. 영묘 반대편처럼 빙 돌아오는 위치를 거릅니다.
	UPROPERTY(EditAnywhere, Category = "Wave|Spawn", meta = (ClampMin = "1.0"))
	float MaxPathRatio = 1.5f;

	// NavMesh 점은 바닥 표면 높이라, 캐릭터 캡슐이 땅에 박히지 않게 올려주는 값(cm).
	UPROPERTY(EditAnywhere, Category = "Wave|Spawn", meta = (ClampMin = "0.0"))
	float SpawnHeightOffset = 100.0f;

	// ---런타임 상태--- 아래는 에디터에서 설정하는 값이 아니라 게임이 돌면서 변하는 값들입니다.
	UPROPERTY()
	TArray<TObjectPtr<AEnemySpawnPoint>> SpawnPoints;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> AliveEnemies;

	UPROPERTY()
	TArray<TSubclassOf<AActor>> SpawnQueue; // 스테이지 전체 스폰 순서표. 웨이브마다 뒤에 이어 붙습니다.

	FTimerHandle SpawnTimer; // 1초마다 한 마리 꺼내는 타이머의 이름표
	FTimerHandle WaveTimer;  // 웨이브 시작부터 60초 후 추가. 전멸하면 다음 틱으로 당깁니다.
	FTimerHandle ProgressTimer; // 사망 콜백 완료 후 진행 조건을 검사합니다.
	FTimerHandle StuckCheckTimer; // 끼인 지상 적을 찾아 옮깁니다.

	// 끼임 검사용. 적마다 마지막으로 움직인 위치와 시각을 기억합니다.
	void CheckStuckEnemies();
	TMap<TWeakObjectPtr<AActor>, TPair<FVector, float>> StuckTracking;

	int32 CurrentStage = 0;     // 지금 몇 번째 스테이지인지. 클리어 방송할 때 이 번호를 같이 넘깁니다.
	int32 CurrentWave = 0;      // 지금 몇 번째 웨이브까지 추가했는지. 0이면 스테이지 시작 전.
	int32 LastWaveRowIndex = 0; // DT에서 실제로 찾은 마지막 웨이브 번호. 생존 스테이지가 웨이브를 다 쓰면 이 행을 반복합니다.
	int32 SpawnedCount = 0;     // 순서표에서 지금까지 몇 칸 꺼냈는지. 곧 다음에 꺼낼 칸 번호입니다.
	int32 NextSliceIndex = 0;   // 다음에 쓸 방향 칸 번호 (0 ~ SpawnDirectionSlices-1)

	bool bIsStageActive = false;  // 스테이지 진행 중인지. 클리어 방송을 한 번만 하기 위한 잠금 역할도 합니다.
	bool bIsSpawning = false;     // SpawnTimer가 돌고 있는지
	bool bAllWavesQueued = false; // 마지막 웨이브까지 순서표에 들어갔는지. 클리어 조기 방송 방지용.

	float StageStartTime = 0.0f; // 스테이지가 시작된 시각. GetGameTimeSinceCreation() 기준으로 저장합니다.

	// 대상이 바뀌면 새 기준을 읽고, 같은 대상에는 항상 최초 값을 사용해 배율 누적을 막습니다.
	TWeakObjectPtr<ADirectionalLight> CachedEnvironmentMoonlight;
	TWeakObjectPtr<ASkyLight> CachedEnvironmentSkylight;
	TWeakObjectPtr<AExponentialHeightFog> CachedEnvironmentFog;
	float BaseMoonlightIntensity = 0.0f;
	float BaseSkylightIntensity = 0.0f;
	float BaseFogDensity = 0.0f;
	float BaseVolumetricExtinction = 0.0f;

	// Day 1 안개 연출. 시작할 때 각 안개의 원래 밀도를 기억해 두고, 끝나면 그 값으로 되돌립니다.
	void UpdateTutorialFog();
	void RestoreTutorialFog();

	FTimerHandle TutorialFogTimer;
	float TutorialFogStartTime = 0.0f;
	TArray<TPair<TWeakObjectPtr<UExponentialHeightFogComponent>, float>> TutorialFogBaseDensities;
};
