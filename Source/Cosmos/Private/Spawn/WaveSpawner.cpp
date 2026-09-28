#include "Spawn/WaveSpawner.h"
#include "Spawn/EnemySpawnPoint.h"
#include "Enemy/EnemyBase.h" // Cast<AEnemyBase>를 하려면 전방선언만으로는 부족하고 전체 정의가 필요합니다.
#include "Enemy/Boss.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Data/CosDataTable.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NavigationSystem.h" // Build.cs에 "NavigationSystem" 모듈이 있어야 합니다.
#include "NavigationPath.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#if WITH_EDITOR
#include "UObject/ObjectSaveContext.h"
#endif


AWaveSpawner::AWaveSpawner()
{
	// 스폰은 전부 타이머로 처리하므로 Tick이 필요 없습니다.
	// AActor 기본값이 true이므로 명시적으로 꺼줘야 합니다.
	PrimaryActorTick.bCanEverTick = false;

	// 시각 검증 전의 시작용 프리셋입니다. 실제 맵에서 조정한 뒤 기능을 켭니다.
	for (int32 Stage = 1; Stage <= 6; ++Stage)
	{
		FStageEnvironmentSettings Settings;
		const float Progress = static_cast<float>(FMath::Min(Stage, 5) - 1);
		Settings.MoonlightMultiplier = 1.0f - 0.1f * Progress;
		Settings.SkylightMultiplier = 1.0f - 0.1f * Progress;
		Settings.FogDensityMultiplier = 1.0f + 0.15f * Progress;
		// 볼류메트릭 안개도 FogDensity의 영향을 받으므로 소멸 배율은 기본 1을 유지합니다.
		StageEnvironments.Add(Stage, Settings);
	}
}

void AWaveSpawner::BeginPlay()
{
	Super::BeginPlay();

	CollectSpawnPoints();
}

void AWaveSpawner::Destroyed()
{
	RestoreFogPreview();
	Super::Destroyed();
}

void AWaveSpawner::PreviewFog()
{
#if WITH_EDITOR
	UWorld* World = GetWorld();
	if (IsTemplate() || !World || World->WorldType != EWorldType::Editor)
	{
		return;
	}
	// Simulate/PIE 중에도 원본 에디터 액터에 버튼을 누를 수 있으므로 전체 컨텍스트를 확인합니다.
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE)
			{
				UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] 플레이를 종료한 뒤 안개를 미리 보세요"));
				return;
			}
		}
	}
	const FStageEnvironmentSettings* Settings = StageEnvironments.Find(PreviewStage);
	if (!Settings || !IsValid(EnvironmentFog) || EnvironmentFog->GetWorld() != World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] Preview Stage 설정과 Environment Fog 연결을 확인하세요"));
		return;
	}
	UExponentialHeightFogComponent* Source = EnvironmentFog->GetComponent();
	if (!IsValid(Source))
	{
		return;
	}

	RestoreFogPreview();
	FogPreviewSource = EnvironmentFog;
	bFogPreviewSourceWasHidden = EnvironmentFog->IsTemporarilyHiddenInEditor();
	// 원본을 템플릿으로 색/높이/볼류메트릭 설정을 그대로 복사합니다.
	FogPreviewComponent = NewObject<UExponentialHeightFogComponent>(
		this, NAME_None, RF_Transient | RF_DuplicateTransient, Source);
	FogPreviewComponent->SetIsVisualizationComponent(true);
	FogPreviewComponent->SetWorldTransform(Source->GetComponentTransform());
	FogPreviewComponent->SetFogDensity(Source->FogDensity * FMath::Max(0.0f, Settings->FogDensityMultiplier));
	FogPreviewComponent->SetVolumetricFogExtinctionScale(
		Source->VolumetricFogExtinctionScale * FMath::Max(0.0f, Settings->VolumetricExtinctionMultiplier));
	// 에디터 임시 숨김은 맵에 저장되지 않으며 원본의 게임 내 표시 상태도 바꾸지 않습니다.
	EnvironmentFog->SetIsTemporarilyHiddenInEditor(true);
	FogPreviewComponent->RegisterComponentWithWorld(World);
	UE_LOG(LogTemp, Log, TEXT("[WaveSpawner] 안개 미리보기 Stage %d / 배율 %.2f / 밀도 %.3f (원본 %.3f)"),
		PreviewStage, Settings->FogDensityMultiplier, FogPreviewComponent->FogDensity, Source->FogDensity);
#endif
}

void AWaveSpawner::RestoreFogPreview()
{
#if WITH_EDITOR
	if (IsValid(FogPreviewComponent))
	{
		FogPreviewComponent->DestroyComponent();
	}
	FogPreviewComponent = nullptr;
	if (FogPreviewSource.IsValid())
	{
		FogPreviewSource->SetIsTemporarilyHiddenInEditor(bFogPreviewSourceWasHidden);
	}
	FogPreviewSource.Reset();
	bFogPreviewSourceWasHidden = false;
#endif
}

#if WITH_EDITOR
void AWaveSpawner::PreSave(FObjectPreSaveContext SaveContext)
{
	RestoreFogPreview();
	Super::PreSave(SaveContext);
}

void AWaveSpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// 단계 선택/대상 교체/설정 변경 시 이전 미리보기를 정리합니다.
	RestoreFogPreview();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWaveFogPreviewTest, "Cosmos.Wave.FogPreview",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWaveFogPreviewTest::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Editor, false);
	if (!TestNotNull(TEXT("Test world"), TestWorld))
	{
		return false;
	}
	AWaveSpawner* Spawner = TestWorld->SpawnActor<AWaveSpawner>();
	AExponentialHeightFog* FogActor = TestWorld->SpawnActor<AExponentialHeightFog>();
	if (!Spawner || !FogActor)
	{
		AddError(TEXT("Could not create preview test actors"));
		TestWorld->DestroyWorld(false);
		return false;
	}
	UExponentialHeightFogComponent* Source = FogActor->GetComponent();
	Source->SetFogDensity(2.3f);
	Source->SetVolumetricFogExtinctionScale(0.35f);
	FogActor->SetActorLocation(FVector(10.0f, 20.0f, 150.0f));
	Spawner->EnvironmentFog = FogActor;
	Spawner->StageEnvironments[1].FogDensityMultiplier = 0.6f;
	Spawner->StageEnvironments[3].FogDensityMultiplier = 1.0f;
	Spawner->StageEnvironments[5].FogDensityMultiplier = 1.3f;
	for (int32 Stage : {1, 5, 3, 1})
	{
		Spawner->PreviewStage = Stage;
		Spawner->PreviewFog();
		if (TestNotNull(TEXT("Preview component"), Spawner->FogPreviewComponent.Get()))
		{
			TestTrue(TEXT("Always uses original density"), FMath::IsNearlyEqual(
				Spawner->FogPreviewComponent->FogDensity, 2.3f * Spawner->StageEnvironments[Stage].FogDensityMultiplier));
			TestTrue(TEXT("Preserves source transform"), Spawner->FogPreviewComponent->GetComponentTransform().Equals(Source->GetComponentTransform()));
			TestTrue(TEXT("Preview cannot be serialized or duplicated"), Spawner->FogPreviewComponent->HasAllFlags(RF_Transient | RF_DuplicateTransient));
			TestTrue(TEXT("Preview is editor-only"), Spawner->FogPreviewComponent->IsEditorOnly());
		}
		TestEqual(TEXT("Source density untouched"), Source->FogDensity, 2.3f);
		TestEqual(TEXT("Source extinction untouched"), Source->VolumetricFogExtinctionScale, 0.35f);
		TestTrue(TEXT("Source temporarily hidden"), FogActor->IsTemporarilyHiddenInEditor());
		TestEqual(TEXT("No wave started"), Spawner->CurrentStage, 0);
	}
	// PIE duplication must not carry the preview reference or dynamically registered fog.
	FObjectDuplicationParameters DuplicateParams(Spawner, TestWorld->PersistentLevel);
	DuplicateParams.DuplicateMode = EDuplicateMode::PIE;
	AWaveSpawner* Duplicate = CastChecked<AWaveSpawner>(StaticDuplicateObjectEx(DuplicateParams));
	TestNull(TEXT("PIE has no preview reference"), Duplicate->FogPreviewComponent.Get());
	TArray<UExponentialHeightFogComponent*> DuplicateFogs;
	Duplicate->GetComponents(DuplicateFogs);
	TestEqual(TEXT("PIE has no preview fog components"), DuplicateFogs.Num(), 0);
	Duplicate->Destroy();
	Spawner->RestoreFogPreview();
	Spawner->RestoreFogPreview();
	TestFalse(TEXT("Restore shows original"), FogActor->IsTemporarilyHiddenInEditor());
	TestNull(TEXT("Restore removes preview"), Spawner->FogPreviewComponent.Get());
	FogActor->SetIsTemporarilyHiddenInEditor(true);
	Spawner->PreviewFog();
	Spawner->RestoreFogPreview();
	TestTrue(TEXT("Preserves initially hidden source"), FogActor->IsTemporarilyHiddenInEditor());
	FogActor->SetIsTemporarilyHiddenInEditor(false);
	Spawner->PreviewFog();
	Spawner->Destroy();
	TestFalse(TEXT("Deleting spawner restores original"), FogActor->IsTemporarilyHiddenInEditor());
	TestWorld->DestroyWorld(false);
	return true;
}
#endif


void AWaveSpawner::StartWave(int32 WaveIndex)
{
	// 만들 대상이 지정되지 않았으면 중단합니다. 에디터 Details에서 설정합니다.
	if (!IsValid(WaveDataTable))
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] WaveDataTable 안끼워넣음 바보바보"));
		return;
	}

	// 액터 BeginPlay 순서는 보장되지 않아, 패키징 빌드에서는
	// 게임모드가 이 함수를 BeginPlay보다 먼저 부를 수 있습니다.
	// 그래서 여기서 수집을 한 번 더 보장합니다.
	if (SpawnPoints.Num() == 0)
	{
		CollectSpawnPoints();
	}

	// 이제 스폰 포인트는 예비용이라 없어도 중단하지 않고 경고만 남깁니다.
	if (SpawnPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] 예비 SpawnPoint 없음. 플레이어 근처 위치를 못 찾으면 스폰이 밀립니다"));
	}

	ResetStageState(WaveIndex); // 시그니처는 WaveIndex지만 의미는 스테이지 번호입니다.
	RestoreTutorialFog(); // Day 1에서 짙게 만든 안개를 원래 밀도로 돌려야 스테이지 배율 기준이 맞습니다.
	SetFogVisible(true); // Day 1에서 숨긴 안개를 전투 스테이지부터 되돌립니다.
	ApplyStageEnvironment(CurrentStage);

	// 웨이브 1은 기다리지 않고 즉시 추가합니다.
	AddNextWave();

}

void AWaveSpawner::StartTutorialStage(int32 GhoulCount)
{
	if (SpawnPoints.Num() == 0)
	{
		CollectSpawnPoints();
	}

	ResetStageState(0);

	if (GhoulCount > 0 && !GhoulClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] Day 1: Ghoul Class 미설정, %d마리 건너뜀"), GhoulCount);
	}
	else
	{
		for (int32 i = 0; i < GhoulCount; ++i)
		{
			SpawnQueue.Add(GhoulClass);
		}
	}

	// 웨이브는 하나뿐이라 바로 "마지막 웨이브까지 넣음" 상태가 됩니다.
	CurrentWave = 1;
	bAllWavesQueued = true;

	if (SpawnQueue.Num() > 0)
	{
		bIsSpawning = true;
		GetWorldTimerManager().SetTimer(
			SpawnTimer, this, &AWaveSpawner::SpawnOne, SpawnInterval, true, 0.f);
	}

	// Day 1은 구울이 끼여서 안 오면 진행이 막히므로 주기적으로 끼인 구울을 옮겨 줍니다.
	if (TutorialStuckSeconds > 0.0f)
	{
		GetWorldTimerManager().SetTimer(StuckCheckTimer, this, &AWaveSpawner::CheckStuckEnemies, 1.0f, true);
	}

	// 스폰할 게 없으면 여기서 바로 클리어를 방송합니다.
	TryBroadcastStageCleared();
} 

void AWaveSpawner::ResetStageState(int32 StageIndex)
{
	// 이전 스테이지의 타이머가 남아 있을 수 있으므로 먼저 끕니다.
	GetWorldTimerManager().ClearTimer(SpawnTimer);
	GetWorldTimerManager().ClearTimer(WaveTimer);
	GetWorldTimerManager().ClearTimer(ProgressTimer);
	GetWorldTimerManager().ClearTimer(StuckCheckTimer);
	StuckTracking.Reset();

	// 스테이지 상태 초기화. 누적 스폰이므로 초기화는 스테이지 시작 때 딱 한 번만 합니다.
	// 웨이브가 바뀔 때는 비우지 않고 순서표 뒤에 이어 붙입니다.
	CurrentStage = StageIndex;
	CurrentWave = 0;
	SpawnQueue.Empty();
	SpawnedCount = 0;
	AliveEnemies.Empty();
	bIsStageActive = true;
	bIsSpawning = false;
	bAllWavesQueued = false;
	StageStartTime = GetGameTimeSinceCreation(); // UI 5분 카운트다운의 기준 시각

	UE_LOG(LogTemp, Log, TEXT("[WaveSpawner] Stage %d 시작"), StageIndex);
}

void AWaveSpawner::CheckStuckEnemies()
{
	if (CurrentStage != 0 || !bIsStageActive)
	{
		GetWorldTimerManager().ClearTimer(StuckCheckTimer);
		StuckTracking.Reset();
		return;
	}

	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!IsValid(Player))
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	for (AActor* Enemy : AliveEnemies)
	{
		if (!IsValid(Enemy))
		{
			continue;
		}

		const FVector Location = Enemy->GetActorLocation();
		TPair<FVector, float>& Info = StuckTracking.FindOrAdd(Enemy, TPair<FVector, float>(Location, Now));

		// 1m 이상 움직였거나 플레이어 곁(5m 안)에서 싸우는 중이면 정상입니다.
		const bool bMoved = FVector::DistSquared2D(Location, Info.Key) > FMath::Square(100.0f);
		const bool bNearPlayer = FVector::DistSquared2D(Location, Player->GetActorLocation()) < FMath::Square(500.0f);
		if (bMoved || bNearPlayer)
		{
			Info = TPair<FVector, float>(Location, Now);
			continue;
		}
		if (Now - Info.Value < TutorialStuckSeconds)
		{
			continue;
		}

		// 끼였습니다. 스폰할 때와 같은 방식으로 플레이어 근처 걸어올 수 있는 자리를 찾아 옮깁니다.
		FVector NewLocation;
		if (FindSpawnLocationNearPlayer(NewLocation))
		{
			NewLocation.Z += SpawnHeightOffset;
			FVector ToPlayer = Player->GetActorLocation() - NewLocation;
			ToPlayer.Z = 0.0f;
			Enemy->TeleportTo(NewLocation, ToPlayer.Rotation());

			// 옛 경로를 버리고 새 자리에서 다시 길을 찾게 합니다.
			if (const APawn* EnemyPawn = Cast<APawn>(Enemy))
			{
				if (AAIController* AI = Cast<AAIController>(EnemyPawn->GetController()))
				{
					AI->StopMovement();
				}
			}
			UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] Day 1 끼인 구울 %s를 %.0fm 지점으로 옮김"),
				*Enemy->GetName(), FVector::Dist(NewLocation, Player->GetActorLocation()) / 100.0f);
		}
		Info = TPair<FVector, float>(Enemy->GetActorLocation(), Now);
	}
}

void AWaveSpawner::StopWave()
{
	GetWorldTimerManager().ClearTimer(ProgressTimer);
	GetWorldTimerManager().ClearTimer(StuckCheckTimer);
	StuckTracking.Reset();
	// 스폰과 웨이브 타이머도 끕니다. 이미 스폰된 적을 지울지는 게임모드가 판단합니다.
	GetWorldTimerManager().ClearTimer(SpawnTimer);
	GetWorldTimerManager().ClearTimer(WaveTimer);
	bIsSpawning = false;
	bIsStageActive = false; // 중단된 스테이지는 클리어 방송을 하지 않습니다.

	UE_LOG(LogTemp, Log, TEXT("[WaveSpawner] Stage %d 종료"), CurrentStage);
}

void AWaveSpawner::ApplyStageEnvironment(int32 StageIndex)
{
	if (!bEnableStageEnvironment || !GetWorld() || !GetWorld()->IsGameWorld())
	{
		return;
	}

	const FStageEnvironmentSettings* Settings = StageEnvironments.Find(StageIndex);
	if (!Settings)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] Stage %d 환경 설정 없음. 현재 환경 유지"), StageIndex);
		return;
	}

	if (IsValid(EnvironmentMoonlight))
	{
		ULightComponent* Light = EnvironmentMoonlight->GetLightComponent();
		if (Light && Light->Mobility != EComponentMobility::Static)
		{
			if (CachedEnvironmentMoonlight.Get() != EnvironmentMoonlight.Get())
			{
				CachedEnvironmentMoonlight = EnvironmentMoonlight.Get();
				BaseMoonlightIntensity = Light->Intensity;
			}
			Light->SetIntensity(BaseMoonlightIntensity * FMath::Max(0.0f, Settings->MoonlightMultiplier));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] 환경 달빛은 Stationary 또는 Movable이어야 합니다"));
		}
	}

	if (IsValid(EnvironmentSkylight))
	{
		USkyLightComponent* Light = EnvironmentSkylight->GetLightComponent();
		if (Light && Light->Mobility != EComponentMobility::Static)
		{
			if (CachedEnvironmentSkylight.Get() != EnvironmentSkylight.Get())
			{
				CachedEnvironmentSkylight = EnvironmentSkylight.Get();
				BaseSkylightIntensity = Light->Intensity;
			}
			Light->SetIntensity(BaseSkylightIntensity * FMath::Max(0.0f, Settings->SkylightMultiplier));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] 환경 Sky Light는 Stationary 또는 Movable이어야 합니다"));
		}
	}

	if (IsValid(EnvironmentFog))
	{
		UExponentialHeightFogComponent* Fog = EnvironmentFog->GetComponent();
		if (Fog)
		{
			if (CachedEnvironmentFog.Get() != EnvironmentFog.Get())
			{
				CachedEnvironmentFog = EnvironmentFog.Get();
				BaseFogDensity = Fog->FogDensity;
				BaseVolumetricExtinction = Fog->VolumetricFogExtinctionScale;
			}
			Fog->SetFogDensity(BaseFogDensity * FMath::Max(0.0f, Settings->FogDensityMultiplier));
			Fog->SetVolumetricFogExtinctionScale(
				BaseVolumetricExtinction * FMath::Max(0.0f, Settings->VolumetricExtinctionMultiplier));
		}
	}

	if (!IsValid(EnvironmentMoonlight) && !IsValid(EnvironmentSkylight) && !IsValid(EnvironmentFog))
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] 환경 기능은 켜졌지만 대상 액터가 지정되지 않았습니다"));
	}
}

void AWaveSpawner::SetFogVisible(bool bVisible)
{
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
	{
		if (UExponentialHeightFogComponent* Fog = It->GetComponent())
		{
			Fog->SetVisibility(bVisible);
		}
	}
}

void AWaveSpawner::StartTutorialFogFadeIn()
{
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	// 두 번 불려도 짙어진 값이 아니라 원래 밀도를 기준으로 다시 시작합니다.
	RestoreTutorialFog();

	for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
	{
		if (UExponentialHeightFogComponent* Fog = It->GetComponent())
		{
			TutorialFogBaseDensities.Emplace(Fog, Fog->FogDensity);
			Fog->SetFogDensity(0.0f);
			Fog->SetVisibility(true);
		}
	}

	TutorialFogStartTime = GetGameTimeSinceCreation();
	UpdateTutorialFog();
	if (TutorialFogFadeDuration > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			TutorialFogTimer, this, &AWaveSpawner::UpdateTutorialFog, 0.05f, true);
	}
}

void AWaveSpawner::UpdateTutorialFog()
{
	const float Alpha = TutorialFogFadeDuration > 0.0f
		? FMath::Clamp((GetGameTimeSinceCreation() - TutorialFogStartTime) / TutorialFogFadeDuration, 0.0f, 1.0f)
		: 1.0f;
	// 처음엔 서서히, 뒤로 갈수록 빠르게 짙어집니다.
	const float Eased = FMath::InterpEaseIn(0.0f, 1.0f, Alpha, 2.0f);

	for (const TPair<TWeakObjectPtr<UExponentialHeightFogComponent>, float>& Entry : TutorialFogBaseDensities)
	{
		if (UExponentialHeightFogComponent* Fog = Entry.Key.Get())
		{
			Fog->SetFogDensity(Entry.Value * FMath::Max(0.0f, TutorialFogDensityMultiplier) * Eased);
		}
	}

	if (Alpha >= 1.0f)
	{
		GetWorldTimerManager().ClearTimer(TutorialFogTimer);
	}
}

void AWaveSpawner::RestoreTutorialFog()
{
	GetWorldTimerManager().ClearTimer(TutorialFogTimer);
	for (const TPair<TWeakObjectPtr<UExponentialHeightFogComponent>, float>& Entry : TutorialFogBaseDensities)
	{
		if (UExponentialHeightFogComponent* Fog = Entry.Key.Get())
		{
			Fog->SetFogDensity(Entry.Value);
		}
	}
	TutorialFogBaseDensities.Reset();
}

void AWaveSpawner::AddNextWave()
{
	// 스테이지가 끝났거나 이미 모든 웨이브를 넣었으면 타이머만 정리하고 나갑니다.
	if (!bIsStageActive || bAllWavesQueued)
	{
		GetWorldTimerManager().ClearTimer(WaveTimer);
		return;
	}

	GetWorldTimerManager().ClearTimer(WaveTimer);
	++CurrentWave;

	// 스테이지 + 웨이브 번호로 DT 행을 찾습니다. 2) 각주
	// 세 번째 인자 false: 행이 없을 때 엔진 경고를 끄고, 아래에서 직접 로그를 남깁니다.
	const FName RowName = MakeRowName(CurrentStage, CurrentWave);
	const FWaveData* Row = WaveDataTable->FindRow<FWaveData>(RowName, TEXT("AddNextWave"), false);

	if (Row == nullptr)
	{
		// 웨이브 1부터 없으면 DT 설정 실수입니다. 클리어로 넘기지 않고 중단합니다.
		if (CurrentWave == 1)
		{
			UE_LOG(LogTemp, Error, TEXT("[WaveSpawner] Row %s 없음. 스테이지 중단"), *RowName.ToString());
			StopWave();
			return;
		}

		// 웨이브 2 이후가 없으면 "이 스테이지는 웨이브가 여기까지"로 봅니다. (예: 보스 스테이지는 1행만)
		UE_LOG(LogTemp, Log, TEXT("[WaveSpawner] Row %s 없음. Stage %d는 웨이브 %d까지로 처리"),
			*RowName.ToString(), CurrentStage, CurrentWave - 1);
		bAllWavesQueued = true;
		GetWorldTimerManager().ClearTimer(WaveTimer);
		TryBroadcastStageCleared();
		return;
	}

	const int32 QueueSizeBefore = SpawnQueue.Num();

	// 종류 하나를 Count만큼 순서표 뒤에 넣는 작은 함수(람다)입니다. 3) 각주
	// BP 칸이 비어 있는데 Count가 있으면 경고만 찍고 건너뜁니다.
	auto AddToQueue = [this, &RowName](TSubclassOf<AActor> Class, int32 Count, const TCHAR* Name)
		{
			if (Count <= 0)
			{
				return;
			}
			if (!Class)
			{
				UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] %s: %s Class 미설정, %d마리 건너뜀"),
					*RowName.ToString(), Name, Count);
				return;
			}
			for (int32 i = 0; i < Count; ++i)
			{
				SpawnQueue.Add(Class);
			}
		};

	AddToQueue(GhoulClass, Row->GhoulCount, TEXT("Ghoul"));
	AddToQueue(EnhancedGhoulClass, Row->EnhancedGhoulCount, TEXT("EnhancedGhoul"));
	AddToQueue(GargoyleClass, Row->GargoyleCount, TEXT("Gargoyle"));
	AddToQueue(CrowClass, Row->CrowCount, TEXT("Crow"));

	// 보스 스테이지 첫 웨이브는 DT 값과 관계없이 보스가 최소 1마리 나오게 합니다.
	int32 BossCount = Row->BossCount;
	if (CurrentStage == BossStageIndex && CurrentWave == 1 && BossCount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] %s의 BossCount가 0이라 보스 1마리를 추가합니다"), *RowName.ToString());
		BossCount = 1;
	}
	if (BossCount > 0 && !BossClass)
	{
		// 보스가 빠지면 보스 스테이지가 바로 클리어되어 게임 클리어로 넘어가므로 에러로 남깁니다.
		UE_LOG(LogTemp, Error, TEXT("[WaveSpawner] %s: Boss Class 미설정. 레벨의 WaveSpawner Details에서 BP_Boss를 지정하세요"),
			*RowName.ToString());
	}
	AddToQueue(BossClass, BossCount, TEXT("Boss"));

	UE_LOG(LogTemp, Log, TEXT("[WaveSpawner] Stage %d 웨이브 %d 추가: +%d마리 / 스폰 대기 %d / 생존 %d"),
		CurrentStage, CurrentWave, SpawnQueue.Num() - QueueSizeBefore,
		SpawnQueue.Num() - SpawnedCount, AliveEnemies.Num());

	// 마지막 웨이브였으면 표시하고 웨이브 타이머를 끕니다.
	if (CurrentWave >= WavesPerStage)
	{
		bAllWavesQueued = true;
		GetWorldTimerManager().ClearTimer(WaveTimer);
	}

	else
	{
		// 조기 시작한 웨이브도 시작 시점부터 다시 60초를 셉니다.
		GetWorldTimerManager().SetTimer(
			WaveTimer, this, &AWaveSpawner::AddNextWave, WaveInterval, false);
	}

	// 꺼낼 칸이 남았는데 SpawnTimer가 멈춰 있으면 다시 켭니다.
	// 이전 웨이브를 다 꺼내서 타이머가 꺼진 상태일 수 있기 때문입니다.
	if (!bIsSpawning && SpawnedCount < SpawnQueue.Num())
	{
		bIsSpawning = true;
		// 마지막 두 인자: true = 반복, 0.f = 첫 실행까지의 지연(즉시 시작)
		GetWorldTimerManager().SetTimer(
			SpawnTimer, this, &AWaveSpawner::SpawnOne, SpawnInterval, true, 0.f);
	}

	// 마지막 웨이브 행이 0마리였고 적도 이미 전멸이면, 사망 이벤트가 안 오므로 여기서 검사합니다.
	TryBroadcastStageCleared();
}

void AWaveSpawner::SpawnOne()
{
	// 꺼낼 칸이 없으면 스폰을 멈춥니다.
	if (!SpawnQueue.IsValidIndex(SpawnedCount))
	{
		GetWorldTimerManager().ClearTimer(SpawnTimer);
		bIsSpawning = false;
		TryBroadcastStageCleared();
		return;
	}

	// 순서표에서 이번 차례의 적 종류를 꺼냅니다. SpawnedCount가 곧 순서 번호입니다.
	const TSubclassOf<AActor> ClassToSpawn = SpawnQueue[SpawnedCount];

	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;

	if (ClassToSpawn && ClassToSpawn->IsChildOf(ABoss::StaticClass())
		&& FindBossSpawnLocation(ClassToSpawn, SpawnLocation, SpawnRotation))
	{
		UE_LOG(LogTemp, Log, TEXT("[WaveSpawner] 보스 등장 위치 %s"), *SpawnLocation.ToString());
	}
	else if (FindSpawnLocationNearPlayer(SpawnLocation))
	{
		// NavMesh 점은 바닥 표면이라 캡슐 중심 높이만큼 올립니다.
		SpawnLocation.Z += SpawnHeightOffset;

		// 스폰되자마자 플레이어를 바라보게 합니다. 위아래 기울기는 빼고 수평 방향만 씁니다.
		if (const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			FVector ToPlayer = Player->GetActorLocation() - SpawnLocation; // 스폰 점 → 플레이어
			ToPlayer.Z = 0.0f;
			SpawnRotation = ToPlayer.Rotation();
		}
	}
	else if (SpawnPoints.Num() > 0)
	{
		// 플레이어 근처를 못 찾았으면 예비 스폰 포인트 중 플레이어와 가장 가까운 것을 고릅니다. 1) 각주
		// 무작위로 고르면 맵 반대편 포인트가 뽑혀 한참 걸어오거나 길을 못 찾는 경우가 생깁니다.
		const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
		AEnemySpawnPoint* Point = nullptr;
		float BestDistSq = TNumericLimits<float>::Max();
		for (AEnemySpawnPoint* Candidate : SpawnPoints)
		{
			if (!IsValid(Candidate))
			{
				continue;
			}
			const float DistSq = Player
				? FVector::DistSquared(Candidate->GetActorLocation(), Player->GetActorLocation())
				: 0.0f;
			if (!Point || DistSq < BestDistSq)
			{
				Point = Candidate;
				BestDistSq = DistSq;
			}
		}
		if (!Point)
		{
			return; // 이번 칸은 소모하지 않고 다음 틱에 다시 시도합니다.
		}
		SpawnLocation = Point->GetActorLocation();
		SpawnRotation = Point->GetActorRotation();

		// 포인트가 NavMesh 밖(땅속, 공중)에 놓여 있으면 적이 태어나자마자 못 움직이므로 NavMesh 위로 붙여 줍니다.
		if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
		{
			FNavLocation NavLocation;
			if (NavSys->ProjectPointToNavigation(SpawnLocation, NavLocation, FVector(500.0f, 500.0f, 1000.0f)))
			{
				SpawnLocation = NavLocation.Location + FVector(0.0f, 0.0f, SpawnHeightOffset);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] 예비 SpawnPoint(%s) 주변에 NavMesh가 없습니다. 적이 움직이지 못할 수 있습니다"),
					*Point->GetName());
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] 플레이어 근처 위치 실패, 가장 가까운 예비 SpawnPoint(%s, %.0fm) 사용"),
			*Point->GetName(), FMath::Sqrt(BestDistSq) / 100.0f);
	}
	else
	{
		// 둘 다 없으면 이번 칸은 소모하지 않고 다음 틱(SpawnInterval 뒤)에 다시 시도합니다.
		UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] 스폰 위치를 못 찾음. 다음 틱에 재시도"));
		return;
	}

	if (ClassToSpawn)
	{
		FActorSpawnParameters Params;
		// 스폰 지점에 뭔가 겹쳐 있어도 위치를 보정해서 반드시 생성합니다.
		// 기본값이면 겹칠 때 조용히 실패해서 "왜 적이 안나옴?"하게 됩니다.
		Params.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AActor* Enemy = GetWorld()->SpawnActor<AActor>(ClassToSpawn, SpawnLocation, SpawnRotation, Params);
		if (Enemy)
		{
			AliveEnemies.Add(Enemy);

			// 이 적이 파괴되면 HandleEnemyDestroyed를 불러달라고 등록합니다.
			// AddDynamic은 UFUNCTION()이 붙은 함수만 받습니다.
			Enemy->OnDestroyed.AddDynamic(this, &AWaveSpawner::HandleEnemyDestroyed);

			// 스폰된 적이 AEnemyBase 계열이면 게임모드에 방송합니다. 4) 각주
			if (AEnemyBase* SpawnedEnemy = Cast<AEnemyBase>(Enemy))
			{
				OnEnemySpawned.Broadcast(SpawnedEnemy);
				SpawnedEnemy->OnEnemyKilled.AddUObject(this, &AWaveSpawner::HandleEnemyKilled);
			}
		}
	}

	// 스폰에 실패했어도 칸은 소모합니다. 안 그러면 같은 칸에서 영원히 멈춥니다.
	// ++SpawnedCount는 먼저 1 올린 뒤 비교합니다.
	if (++SpawnedCount >= SpawnQueue.Num())
	{
		GetWorldTimerManager().ClearTimer(SpawnTimer);
		bIsSpawning = false;
		TryBroadcastStageCleared(); // 스폰 실패로 생존 0인 채 끝났을 수도 있어서 검사합니다.
	}
}

bool AWaveSpawner::FindSpawnLocationNearPlayer(FVector& OutLocation)
{
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!IsValid(Player))
	{
		return false;
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());	if (NavSys == nullptr)
	{
		return false;
	}

	const FVector PlayerLocation = Player->GetActorLocation();
	const float SliceAngle = 360.0f / SpawnDirectionSlices; // 8칸이면 한 칸 45도

	// 2) 거리: Day 1(Stage 0)은 튜토리얼 전용 거리를 씁니다.
	const bool bTutorial = CurrentStage == 0;
	const float MinRadius = bTutorial ? TutorialMinSpawnRadius : MinSpawnRadius;
	const float MaxRadius = bTutorial ? TutorialMaxSpawnRadius : MaxSpawnRadius;

	// 실패 원인을 세어 두었다가 전부 실패하면 로그로 알려줍니다.
	int32 NoNavCount = 0, NoPathCount = 0, DetourCount = 0;

	auto TryFind = [&](int32 Attempts, bool bCheckDetour) -> bool
	{
		for (int32 Attempt = 0; Attempt < Attempts; ++Attempt)
		{
			// 1) 방향: 이번 칸 안에서 랜덤 각도. 다음 시도는 옆 칸을 씁니다.
			const float Angle = NextSliceIndex * SliceAngle + FMath::FRandRange(0.0f, SliceAngle);
			NextSliceIndex = (NextSliceIndex + 1) % SpawnDirectionSlices;
			const FVector Direction = FRotator(0.0f, Angle, 0.0f).Vector(); // Yaw만 돌린 수평 방향
			const FVector Candidate = PlayerLocation + Direction * FMath::FRandRange(MinRadius, MaxRadius);

			// 3) 바닥: 후보 점을 가장 가까운 NavMesh 위로 붙입니다. NavMesh가 없는 곳이면 다시 뽑기.
			FNavLocation NavLocation;
			if (!NavSys->ProjectPointToNavigation(Candidate, NavLocation, FVector(500.0f, 500.0f, 1000.0f)))
			{
				++NoNavCount;
				continue;
			}

			// 4) 경로: 스폰 점에서 플레이어까지 걸어갈 수 있는지. 끊겼거나 중간까지만 가면 버립니다.
			//    바위 틈처럼 NavMesh가 따로 떨어진 섬에 붙은 경우를 여기서 거릅니다.
			UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(
				GetWorld(), NavLocation.Location, PlayerLocation);
			if (Path == nullptr || !Path->IsValid() || Path->IsPartial())
			{
				++NoPathCount;
				continue;
			}

			// 5) 우회: 경로가 직선거리의 MaxPathRatio배를 넘으면 너무 빙 도는 위치라 버립니다.
			const float StraightDistance = FVector::Dist(NavLocation.Location, PlayerLocation);
			if (bCheckDetour && Path->GetPathLength() > StraightDistance * MaxPathRatio)
			{
				++DetourCount;
				continue;
			}

			OutLocation = NavLocation.Location;
			return true;
		}
		return false;
	};

	if (TryFind(MaxSpawnAttempts, true))
	{
		return true;
	}

	// 2차: 우회 검사를 빼고 두 배로 더 찾아봅니다. 멀리 떨어진 예비 포인트보다 조금 돌아오는 편이 낫습니다.
	if (TryFind(MaxSpawnAttempts * 2, false))
	{
		return true;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("[WaveSpawner] 플레이어 근처 스폰 실패 (NavMesh 없음 %d / 경로 없음 %d / 너무 돌아감 %d) - 플레이어 주변 NavMesh를 확인하세요"),
		NoNavCount, NoPathCount, DetourCount);
	return false; // 전부 실패하면 SpawnOne이 예비 SpawnPoint로 대체합니다.
}

bool AWaveSpawner::FindBossSpawnLocation(TSubclassOf<AActor> BossToSpawn, FVector& OutLocation, FRotator& OutRotation) const
{
	TArray<AActor*> Points;
	UGameplayStatics::GetAllActorsWithTag(this, BossSpawnPointTag, Points);
	if (Points.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] '%s' 태그 BossPoint 없음. 보스를 플레이어 근처에 스폰합니다 (텔레포트 불가)"),
			*BossSpawnPointTag.ToString());
		return false;
	}

	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	const FVector PlayerLocation = IsValid(Player) ? Player->GetActorLocation() : GetActorLocation();

	// 플레이어와 가장 가까운 지점에서 등장시켜 바로 전투가 시작되게 합니다.
	const AActor* BestPoint = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	for (const AActor* Point : Points)
	{
		if (!IsValid(Point))
		{
			continue;
		}
		const float DistSq = FVector::DistSquared(PlayerLocation, Point->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestPoint = Point;
		}
	}
	if (!BestPoint)
	{
		return false;
	}

	// ABoss::PickBossPointLocation과 같이 캡슐 절반 높이만큼 올려 바닥에 박히지 않게 합니다.
	OutLocation = BestPoint->GetActorLocation();
	if (const ACharacter* BossCDO = Cast<ACharacter>(BossToSpawn->GetDefaultObject()))
	{
		if (const UCapsuleComponent* Capsule = BossCDO->GetCapsuleComponent())
		{
			OutLocation.Z += Capsule->GetScaledCapsuleHalfHeight();
		}
	}

	FVector ToPlayer = PlayerLocation - OutLocation;
	ToPlayer.Z = 0.0f;
	OutRotation = ToPlayer.IsNearlyZero() ? BestPoint->GetActorRotation() : ToPlayer.Rotation();
	return true;
}

void AWaveSpawner::TryBroadcastStageCleared()
{
	// 스폰 대기와 생존 적이 모두 없어야 조기 진행합니다.
	if (bIsStageActive && !bIsSpawning && SpawnedCount >= SpawnQueue.Num() && AliveEnemies.Num() == 0)
	{
		GetWorldTimerManager().ClearTimer(WaveTimer);
		if (!bAllWavesQueued)
		{
			// 빈 웨이브가 연속되어도 재귀 호출하지 않습니다.
			WaveTimer = GetWorldTimerManager().SetTimerForNextTick(this, &AWaveSpawner::AddNextWave);
			return;
		}
		GetWorldTimerManager().ClearTimer(SpawnTimer);
		GetWorldTimerManager().ClearTimer(ProgressTimer);
		bIsStageActive = false;
		UE_LOG(LogTemp, Log, TEXT("[WaveSpawner] Stage %d 클리어"), CurrentStage);
		OnWaveCleared.Broadcast(CurrentStage);
	}
}

void AWaveSpawner::HandleEnemyKilled(AEnemyBase* KilledEnemy)
{
	HandleEnemyDestroyed(KilledEnemy);
}

void AWaveSpawner::HandleEnemyDestroyed(AActor* DestroyedActor)
{
	// 처치 후 시체가 파괴될 때는 중복 처리하지 않습니다.
	if (AliveEnemies.Remove(DestroyedActor) > 0 && bIsStageActive)
	{
		// 보상 등 모든 사망 콜백이 끝난 다음 진행합니다.
		GetWorldTimerManager().ClearTimer(ProgressTimer);
		ProgressTimer = GetWorldTimerManager().SetTimerForNextTick(this, &AWaveSpawner::TryBroadcastStageCleared);
	}
}

void AWaveSpawner::CollectSpawnPoints()
{
	// StartWave에서 먼저 불렸다가 BeginPlay에서 다시 불릴 수 있으므로
	// 중복 적재를 막기 위해 비우고 시작합니다.
	SpawnPoints.Empty();

	// 레벨에 배치된 스폰 포인트를 전부 찾아서 배열에 담습니다.
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemySpawnPoint::StaticClass(), Found);

	for (AActor* A : Found)
	{
		// Cast가 실패하면 nullptr이 나오므로 if 안에서 바로 검사합니다.
		if (AEnemySpawnPoint* Point = Cast<AEnemySpawnPoint>(A))
		{
			SpawnPoints.Add(Point);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("[WaveSpawner] 예비 SpawnPoint %d개 수집"), SpawnPoints.Num());
}

// 스테이지 번호 + 웨이브 번호를 Row Name으로 바꿉니다.
// 예: Stage 2, Wave 3 -> "Stage2_Wave3"
FName AWaveSpawner::MakeRowName(int32 StageIndex, int32 WaveIndex) const
{
	return FName(*FString::Printf(TEXT("Stage%d_Wave%d"), StageIndex, WaveIndex));
}


// ─────────────────────────────────────────────
// 각주
// ─────────────────────────────────────────────
// 1) FMath::RandRange(최소, 최대)는 최소~최대 사이 정수를 랜덤하게 하나 뽑아주는 언리얼 함수입니다. 양끝도 포함됩니다.
//    배열 번호는 0부터 시작하므로, 스폰 포인트가 3개면 번호는 0, 1, 2입니다.
//    그래서 -1을 해줍니다. 빼지 않으면 없는 3번이 뽑혀 크래시가 납니다.
// 
// 2) FindRow는 DT의 몇 번째 줄이 아니라 맨 왼쪽 Row Name으로 줄을 찾습니다.
//    Row Name은 "Stage2_Wave3" = 스테이지 2(5분)의 웨이브 3(1분) 형식입니다.
//    이름이 DT와 한 글자라도 다르면 nullptr이 나옵니다.
//
// 3) SpawnQueue는 스테이지 전체의 스폰 순서표입니다. 웨이브마다 뒤에 이어 붙습니다.
//    웨이브1 구울 3 → [구, 구, 구]
//    웨이브2 구울 1, 가고일 1 → [구, 구, 구, 구, 가]
//    SpawnOne은 SpawnedCount번째 칸을 꺼내므로, 못 꺼낸 앞 웨이브 적 뒤에 새 웨이브 적이 줄을 섭니다.
//
// 4) Cast<AEnemyBase>는 Enemy가 AEnemyBase이거나 그 자식이면 포인터를, 아니면 nullptr을 돌려줍니다.
//    스포너는 적을 AActor로 만들기 때문에, AEnemyBase 전용 기능(OnEnemyKilled)을 쓰려면 이 변환이 필요합니다.
//
// 5) 타이머 두 개의 역할
//    WaveTimer  (60초) : 순서표에 다음 웨이브를 추가만 함. 적을 만들지 않음
//    SpawnTimer (1초)  : 순서표에서 한 칸씩 꺼내 실제로 만듦. 꺼낼 게 없으면 멈추고, 웨이브가 추가되면 다시 켜짐
//
// 6) 플레이어 근처 스폰 순서
//    방향(8칸 순환) → 거리(최소~최대) → NavMesh에 붙이기 → 경로 있는지 → 너무 돌아오는지
//    하나라도 실패하면 다시 뽑고, MaxSpawnAttempts번 모두 실패하면 예비 SpawnPoint를 씁니다.
