#include "Spawn/WaveSpawner.h"
#include "Spawn/EnemySpawnPoint.h"
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
	UE_LOG(LogTemp, Log, TEXT("[WaveSpawner] SpawnPoint %개 수집"), SpawnPoints.Num());
	return;
}


void AWaveSpawner::StartWave(int32 WaveIndex)
{
	// 스폰 포인트가 없으면 어디에 만들지 알 수 없으므로 중단합니다.
	if (SpawnPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] SpawnPoint 없음"));
		return;
	}

	// 만들 대상이 지정되지 않았으면 중단합니다. 에디터 Details에서 설정합니다.
	if (!EnemyClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WaveSpawner] EnemyClass 미설정"));
		return;
	}

	// 웨이브 상태 초기화
	CurrentWave = WaveIndex;
	SpawnedCount = 0;
	bIsSpawning = true;
	AliveEnemies.Empty();

	UE_LOG(LogTemp, Log, TEXT("[WaveSpawner] Wave %d 시작", WaveIndex));

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
	// Num()은 개수이므로 마지막 인덱스는 Num() -1입니다.
	const int32 Index = FMath::RandRange(0, SpawnPoints.Num() - 1);
	AEnemySpawnPoint* Point = SpawnPoints[Index];

	if (IsValid(Point))
	{
		FActorSpawnParameters Params;
		// 스폰 지점에 뭔가 겹쳐 있어도 위치를 보정해서 반드시 생성합니다.
		// 기본값이면 겹칠 때 조용히 실패해서 "왜 적이 안나옴?"하게 됩니다.
		Params.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AActor* Enemy = GetWorld()->SpawnActor<AActor>(
			EnemyClass, Point->GetActorLocation(), Point->GetActorRotation(), Params);
		//------------------------------------------------------------------------
	}
}