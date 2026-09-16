#include "Spawn/WaveSpawner.h"
#include "Spawn/EnemySpawnPoint.h"
#include "Data/CosDataTable.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"


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

	// 액터 BeginPlay 순서는 보장되지 않아, 패키징 빌드에서는
	// 게임모드가 이 함수를 BeginPlay보다 먼저 부를 수 있습니다.
	// 그래서 여기서 수집을 한 번 더 보장합니다.
	if (SpawnPoints.Num() == 0)
	{
		CollectSpawnPoints();
	}


	// 스폰 포인트가 없으면 어디에 만들지 알 수 없으므로 중단합니다.
	if (SpawnPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] SpawnPoint 없음 바보바보"));
		return;
	}

	// 만들 대상이 지정되지 않았으면 중단합니다. 에디터 Details에서 설정합니다.
	if (!IsValid(WaveDataTable))
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] WaveDataTable 안끼워넣음 바보바보"));
		return;
	}

	// 번호를 Row Name으로 바꿔서 해당 줄을 찾습니다. 2) 각주
	const FName RowName = MakeRowName(WaveIndex);
	const FWaveData* Row = WaveDataTable->FindRow<FWaveData>(RowName, TEXT("StartWave"));

	// 줄을 못 찾으면 nullptr입니다. 그대로 쓰면 크래시이므로 중단합니다.
	if (Row == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[WaveSpawner] Row %s 없음"), *RowName.ToString());
		return;
	}

	// 스폰 순서표를 새로 만듭니다. 3) 각주
	SpawnQueue.Empty();

	// 종류 하나를 Count만큼 순서표에 넣는 작은 함수(람다)입니다.
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

	// 순서표 길이가 이번웨이브의 총 마리 수라고 합니다.
	TargetSpawnCount = SpawnQueue.Num();

	// 0이면 스폰할 게 없어서 클리어 신호가 영원히 안 옵니다. 중단하고 알립니다.
	if (TargetSpawnCount <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[WaveSpawner] Row %s 스폰할 적 없음"), *RowName.ToString());
		return;
	}

	// 웨이브 상태 초기화
	CurrentWave = WaveIndex;
	SpawnedCount = 0;
	bIsSpawning = true;
	AliveEnemies.Empty();

	UE_LOG(LogTemp, Log, TEXT("[WaveSpawner] Wave %d 시작"), WaveIndex);

	// 마지막 두 인자: true = 반복, 0.f = 첫 실행까지의 지연(즉시 시작)
	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimer, this, &AWaveSpawner::SpawnOne, SpawnInterval, true, 0.f);

}

void AWaveSpawner::StopWave()
{
	// 타이머만 끕니다. 이미 스폰된 적을 지울지는 게임모드가 판단합니다.
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
	bIsSpawning = false;

	UE_LOG(LogTemp, Log, TEXT("[WaveSpawner] Wave %d 종료"), CurrentWave);
}

void AWaveSpawner::SpawnOne()
{
	// 스폰 포인트 중 하나를 무작위로 고릅니다.
	const int32 Index = FMath::RandRange(0, SpawnPoints.Num() - 1); // 1) 각주
	AEnemySpawnPoint* Point = SpawnPoints[Index]; // 랜덤으로 뽑은 번호의 스폰 포인트 하나를 Point로 가리킵니다.

	// 순서표에서 이번 차례의 적 종류를 꺼냅니다. SpawnedCount가 곧 순서 번호입니다.
	const TSubclassOf<AActor> ClassToSpawn =
		SpawnQueue.IsValidIndex(SpawnedCount) ? SpawnQueue[SpawnedCount] : nullptr;

	if (IsValid(Point) && ClassToSpawn) // 스폰 포인트가 살아있고, 만들 종류도 있어야 적을 만듭니다.
		//IsValid는 언리얼 함수이고, nullptr 체크 + "삭제 예정인가"까지 같이 봅니다. 그래서 Point != nullptr보다 안전합니다. valid는 "유효한" 이라는 뜻을 가진 형용사입니다.
	{
		FActorSpawnParameters Params;
		// 스폰 지점에 뭔가 겹쳐 있어도 위치를 보정해서 반드시 생성합니다.
		// 기본값이면 겹칠 때 조용히 실패해서 "왜 적이 안나옴?"하게 됩니다.
		Params.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AActor* Enemy = GetWorld()->SpawnActor<AActor>(
			ClassToSpawn, Point->GetActorLocation(), Point->GetActorRotation(), Params);
		// 월드에, ClassToSpawn 종류의 적을, 스폰 포인트 위치에, 스폰 포인트 방향으로, 아까 만든 옵션으로 만들고, 만든 적을 Enemy로 가리켜라는 말입니다.
		if (Enemy)
		{
			AliveEnemies.Add(Enemy);

			// 이 적이 파괴되면 HandleEnemyDestroyed를 불러달라고 등록합니다.
			// AddDynamic은 UFUNCTION()이 붙은 함수만 받습니다.
			Enemy->OnDestroyed.AddDynamic(this, &AWaveSpawner::HandleEnemyDestroyed);

		}
	}
	// ++SpawnedCount는 먼저 1 올린 뒤 비교합니다.
	if (++SpawnedCount >= TargetSpawnCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
		bIsSpawning = false;
	}
}

void AWaveSpawner::HandleEnemyDestroyed(AActor* DestroyedActor)
{
	AliveEnemies.Remove(DestroyedActor);

	// bIsSpawning 검사가 없으면, 첫 적을 바로 죽였을 때
	// 아직 더 나올 예정인데도 클리어로 오판합니다.
	if (!bIsSpawning && AliveEnemies.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[WaveSpawner] Wave %d 클리어"), CurrentWave);
		OnWaveCleared.Broadcast(CurrentWave);
	}
}

void AWaveSpawner::CollectSpawnPoints()
{
	// StartWave에서 먼저 불렸다가 BeginPlay에서 다시 불릴 수 있으므로
	// 중복 적재를 막기 위해 비우고 시작합니다.
	SpawnPoints.Empty();

	// 레벨에 배치된 스폰 포인트를 전부 찾아서 배열에 담습니다.
	// 위치를 코드에 하드코딩하지 않고 에디터에서 드래그로 조정하기 위함입니다.
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
	UE_LOG(LogTemp, Log, TEXT("[WaveSpawner] SpawnPoint %d개 수집"), SpawnPoints.Num());
}

// 번호(1~26)를 Row Name으로 바꿉니다. 한 스테이지에 웨이브 5개 기준입니다.
FName AWaveSpawner::MakeRowName(int32 WaveIndex) const
{
	const int32 Stage = (WaveIndex - 1) / 5 + 1; // 1~5 -> 1, 6~10 -> 2, 26 -> 6
	const int32 Wave = (WaveIndex - 1) % 5 + 1; // 1, 2, 3, 4 , 5 반복. 26 -> 1
	return FName(*FString::Printf(TEXT("Stage%d_Wave%d"), Stage, Wave));
}


// ─────────────────────────────────────────────
// 각주
// ─────────────────────────────────────────────
// 1) FMath::RandRange(최소, 최대)는 최소~최대 사이 정수를 랜덤하게 하나 뽑아주는 언리얼 함수입니다. 양끝도 포함됩니다.
//    예시: int32 A = FMath::RandRange(1, 6); // 주사위처럼 1~6 사이 하나
//    SpawnPoints는 헤더에 선언한 배열입니다. .Num()은 배열에 몇 개가 들어있는지 알려주는 함수입니다. vector.size()와 같습니다.
//    배열 번호는 0부터 시작하므로, 스폰 포인트가 3개면 번호는 0, 1, 2입니다.
//    그래서 -1을 해줍니다. 빼지 않으면 없는 3번이 뽑혀 크래시가 납니다.
// 
// 2) FindRow는 DT의 몇 번째 줄이 아니라 맨 왼쪽 Row Name으로 줄을 찾습니다.
//    게임모드는 1~26 번호를 넘기므로 MakeRowName이 이를 "Stage1_Wave1" 형식으로 바꿔줍니다.
//    이름이 DT와 한 글자라도 다르면 nullptr이 나옵니다.
//
// 3) SpawnQueue는 이번 웨이브의 스폰 순서표입니다.
//    DT가 구울 3, 가고일 2라면 [구울, 구울, 구울, 가고일, 가고일]이 됩니다.
//    SpawnOne은 SpawnedCount번째 칸을 꺼내 만들기 때문에 0번부터 차례대로 나옵니다.