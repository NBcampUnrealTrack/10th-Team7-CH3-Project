#include "Character/CosGameMode.h"
#include "Character/CosCharacter.h"
#include "Character/CosGameState.h"//CosGameState 연결
#include "UI/CosPlayerController.h"
#include "Spawn/WaveSpawner.h"
#include "Kismet/GameplayStatics.h"

ACosGameMode::ACosGameMode()
{
	DefaultPawnClass = ACosCharacter::StaticClass();
	GameStateClass = ACosGameState::StaticClass();
}

void ACosGameMode::BeginPlay()
{
	Super::BeginPlay();

	WaveSpawner = Cast<AWaveSpawner>(//현재 월드에 AWaveSpawner인 액터 찾아서 WaveSpawner에 저장
		UGameplayStatics::GetActorOfClass(//나중에 WaveSpawner->StartWave(1); 이렇게 호출
			GetWorld(),
			AWaveSpawner::StaticClass()
		)
	);

	if (!WaveSpawner)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[GameMode] WaveSpawner를 찾지 못함"));
		return;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("[GameMode] WaveSpawner 찾음"));

	// 웨이브 종료 신호 구독
	WaveSpawner->OnWaveCleared.AddUObject(
		this,
		&ACosGameMode::HandleWaveCleared
	);

	// 첫 웨이브 시작
	StartNextWave(true);
}

void ACosGameMode::StartNextWave(bool bResetTimer)
{//CurrentWaveIndex+1->  웨이브 스포너에 현재 웨이브 번호 전달(적 스폰)-> GameState에서도 번호 저장
	++CurrentWaveIndex;

	UE_LOG(LogTemp, Warning,
		TEXT("[GameMode] StartNextWave -> %d"),
		CurrentWaveIndex);

	if (bResetTimer)
	{
		CurrentWaveStartTime = GetWorld()->GetTimeSeconds();
	}

	if (WaveSpawner)//웨이브 스포너에 현재 웨이브 번호 전달, 적 스폰 시작함
	{
		WaveSpawner->StartWave(CurrentWaveIndex);
	}

	//게임스테이트에서도 웨이브 번호 전달
	if (ACosGameState* GS = GetGameState<ACosGameState>())
	{
		GS->SetWaveIndex(CurrentWaveIndex);
	}
}

void ACosGameMode::StartGame()//타이들에서 게임 시작 누르면 전투(임시) 맵으로 이동하도록함
{
	UGameplayStatics::OpenLevel(this, FName("L_BlockoutTriangle"));
}

void ACosGameMode::HandleWaveCleared(int32 WaveIndex)
{
	UE_LOG(LogTemp, Warning,
		TEXT("[GameMode] HandleWaveCleared 호출 / WaveIndex = %d"),
		WaveIndex);
	if (WaveIndex == 26)//보스 웨이브 클리어
	{
		PendingAction = EPostResultAction::GameClear;
	}
	else if (WaveIndex % 5 == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Forge 분기 진입"));

		PendingAction = EPostResultAction::Forge;
	}
	else
	{
		PendingAction = EPostResultAction::NextWave;
	}

	if (PendingAction == EPostResultAction::NextWave)
	{
		StartNextWave();// 그 외는 다음 웨이브로 넘어가게
		return;
	}

	// [추가] 결과창에 넘길 데이터 구성
	FWaveResultData ResultData;

	ResultData.WaveNumber = WaveIndex;

	ResultData.ElapsedSeconds =
		GetWorld()->GetTimeSeconds() - CurrentWaveStartTime;

	if (ACosGameState* GS = GetGameState<ACosGameState>())
	{
		ResultData.Score = GS->GetScore();
	}

	// [추가] PlayerController에게 결과창 표시 요청
	if (APlayerController* PC =
		UGameplayStatics::GetPlayerController(this, 0))
	{
		if (ACosPlayerController* CosPC =
			Cast<ACosPlayerController>(PC))
		{
			CosPC->ShowResult(ResultData);
		}
	}
}
	
void ACosGameMode::OnResultConfirmed()
{
	switch (PendingAction)
	{
	case EPostResultAction::NextWave:

		StartNextWave();
		break;

	case EPostResultAction::Forge:

		// 결과창 → 대장간
		OnForgeRequested.Broadcast();
		break;

	case EPostResultAction::GameClear:

		OnGameClear.Broadcast();
		break;
	}
}

void ACosGameMode::HandlePlayerDeath()
{

	if (WaveSpawner)
	{
		WaveSpawner->StopWave();
	}
	
	UE_LOG(LogTemp, Log, TEXT("[GameMode] PLayer Death -> Game Over"));

	OnGameOver.Broadcast();
}

