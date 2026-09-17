#include "Spawn/WaveSpawner.h"
#include "Spawn/EnemySpawnPoint.h"
#include "Enemy/EnemyBase.h" // Cast<AEnemyBase>를 하려면 전방선언만으로는 부족하고 전체 정의가 필요합니다.
#include "Data/CosDataTable.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NavigationSystem.h" // Build.cs에 "NavigationSystem" 모듈이 있어야 합니다.
#include "NavigationPath.h"


AWaveSpawner::AWaveSpawner()
{
	// 스폰은 전부 타이머로 처리하므로 Tick이 필요 없습니다.
	// AActor 기본값이 true이므로 명시적으로 꺼줘야 합니다.
	PrimaryActorTick.bCanEverTick = false;
}

void AWaveSpawner::BeginPlay()
{
	Super::BeginPlay();

	CollectSpawnPoints();
}


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

	// 이전 스테이지의 타이머가 남아 있을 수 있으므로 먼저 끕니다.
	GetWorldTimerManager().ClearTimer(SpawnTimer);
	GetWorldTimerManager().ClearTimer(WaveTimer);

	// 스테이지 상태 초기화. 누적 스폰이므로 초기화는 스테이지 시작 때 딱 한 번만 합니다.
	// 웨이브가 바뀔 때는 비우지 않고 순서표 뒤에 이어 붙입니다.
	CurrentStage = WaveIndex; // 시그니처는 WaveIndex지만 의미는 스테이지 번호입니다.
	CurrentWave = 0;
	SpawnQueue.Empty();
	SpawnedCount = 0;
	AliveEnemies.Empty();
	bIsStageActive = true;
	bIsSpawning = false;
	bAllWavesQueued = false;
	StageStartTime = GetGameTimeSinceCreation(); // UI 5분 카운트다운의 기준 시각

	UE_LOG(LogTemp, Log, TEXT("[WaveSpawner] Stage %d 시작"), WaveIndex);

	// 웨이브 1은 기다리지 않고 즉시 추가합니다.
	AddNextWave();

	// AddNextWave 안에서 웨이브 1 행을 못 찾아 스테이지가 중단됐거나, 웨이브가 1개뿐이면 타이머를 걸지 않습니다.
	if (!bIsStageActive || bAllWavesQueued)
	{
		return;
	}

	// 웨이브 2~5는 WaveInterval(60초)마다 추가합니다.
	// 마지막 인자를 생략하면 첫 실행도 60초 뒤입니다. 웨이브 1을 위에서 이미 넣었기 때문에 이게 맞습니다.
	GetWorldTimerManager().SetTimer(
		WaveTimer, this, &AWaveSpawner::AddNextWave, WaveInterval, true);
}

void AWaveSpawner::StopWave()
{
	// 타이머 두 개를 모두 끕니다. 이미 스폰된 적을 지울지는 게임모드가 판단합니다.
	GetWorldTimerManager().ClearTimer(SpawnTimer);
	GetWorldTimerManager().ClearTimer(WaveTimer);
	bIsSpawning = false;
	bIsStageActive = false; // 중단된 스테이지는 클리어 방송을 하지 않습니다.

	UE_LOG(LogTemp, Log, TEXT("[WaveSpawner] Stage %d 종료"), CurrentStage);
}

void AWaveSpawner::AddNextWave()
{
	// 스테이지가 끝났거나 이미 모든 웨이브를 넣었으면 타이머만 정리하고 나갑니다.
	if (!bIsStageActive || bAllWavesQueued)
	{
		GetWorldTimerManager().ClearTimer(WaveTimer);
		return;
	}

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
	AddToQueue(BossClass, Row->BossCount, TEXT("Boss"));

	UE_LOG(LogTemp, Log, TEXT("[WaveSpawner] Stage %d 웨이브 %d 추가: +%d마리 / 스폰 대기 %d / 생존 %d"),
		CurrentStage, CurrentWave, SpawnQueue.Num() - QueueSizeBefore,
		SpawnQueue.Num() - SpawnedCount, AliveEnemies.Num());

	// 마지막 웨이브였으면 표시하고 웨이브 타이머를 끕니다.
	if (CurrentWave >= WavesPerStage)
	{
		bAllWavesQueued = true;
		GetWorldTimerManager().ClearTimer(WaveTimer);
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

	if (FindSpawnLocationNearPlayer(SpawnLocation))
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
		// 플레이어 근처를 못 찾았으면 예비 스폰 포인트 중 하나를 무작위로 고릅니다. 1) 각주
		const int32 Index = FMath::RandRange(0, SpawnPoints.Num() - 1);
		AEnemySpawnPoint* Point = SpawnPoints[Index];
		if (!IsValid(Point))
		{
			return; // 이번 칸은 소모하지 않고 다음 틱에 다시 시도합니다.
		}
		SpawnLocation = Point->GetActorLocation();
		SpawnRotation = Point->GetActorRotation();
		UE_LOG(LogTemp, Verbose, TEXT("[WaveSpawner] 플레이어 근처 위치 실패, 예비 SpawnPoint 사용"));
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

	for (int32 Attempt = 0; Attempt < MaxSpawnAttempts; ++Attempt)
	{
		// 1) 방향: 이번 칸 안에서 랜덤 각도. 다음 시도는 옆 칸을 씁니다.
		const float Angle = NextSliceIndex * SliceAngle + FMath::FRandRange(0.0f, SliceAngle);
		NextSliceIndex = (NextSliceIndex + 1) % SpawnDirectionSlices;
		const FVector Direction = FRotator(0.0f, Angle, 0.0f).Vector(); // Yaw만 돌린 수평 방향

		// 2) 거리: 최소~최대 사이 랜덤
		const float Distance = FMath::FRandRange(MinSpawnRadius, MaxSpawnRadius);
		const FVector Candidate = PlayerLocation + Direction * Distance;

		// 3) 바닥: 후보 점을 가장 가까운 NavMesh 위로 붙입니다. NavMesh가 없는 곳이면 다시 뽑기.
		FNavLocation NavLocation;
		if (!NavSys->ProjectPointToNavigation(Candidate, NavLocation, FVector(500.0f, 500.0f, 1000.0f)))
		{
			continue;
		}

		// 4) 경로: 스폰 점에서 플레이어까지 걸어갈 수 있는지. 끊겼거나 중간까지만 가면 버립니다.
		//    바위 틈처럼 NavMesh가 따로 떨어진 섬에 붙은 경우를 여기서 거릅니다.
		UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(
			GetWorld(), NavLocation.Location, PlayerLocation);
		if (Path == nullptr || !Path->IsValid() || Path->IsPartial())
		{
			continue;
		}

		// 5) 우회: 경로가 직선거리의 MaxPathRatio배를 넘으면 너무 빙 도는 위치라 버립니다.
		const float StraightDistance = FVector::Dist(NavLocation.Location, PlayerLocation);
		if (Path->GetPathLength() > StraightDistance * MaxPathRatio)
		{
			continue;
		}

		OutLocation = NavLocation.Location;
		return true;
	}

	return false; // 전부 실패하면 SpawnOne이 예비 SpawnPoint로 대체합니다.
}

void AWaveSpawner::TryBroadcastStageCleared()
{
	// 네 조건이 모두 맞아야 클리어입니다.
	//   bIsStageActive    : 진행 중인 스테이지여야 함 (한 번 방송하면 false가 되어 중복 방송 방지)
	//   bAllWavesQueued   : 마지막 웨이브까지 들어갔어야 함 (웨이브 사이 공백에서 조기 클리어 방지)
	//   !bIsSpawning      : 순서표를 다 꺼냈어야 함
	//   AliveEnemies == 0 : 살아있는 적이 없어야 함
	if (bIsStageActive && bAllWavesQueued && !bIsSpawning && AliveEnemies.Num() == 0)
	{
		bIsStageActive = false;
		UE_LOG(LogTemp, Log, TEXT("[WaveSpawner] Stage %d 클리어"), CurrentStage);
		OnWaveCleared.Broadcast(CurrentStage);
	}
}

void AWaveSpawner::HandleEnemyDestroyed(AActor* DestroyedActor)
{
	AliveEnemies.Remove(DestroyedActor);
	TryBroadcastStageCleared();
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