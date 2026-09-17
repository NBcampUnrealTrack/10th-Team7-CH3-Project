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

	//웨이브 스포너에서 웨이브 클리어 델리게이트 구독
	//웨이브 끝나면 HandleWaveCleared가 자동 호출
	if (WaveSpawner)
	{
		WaveSpawner->OnWaveCleared.AddUObject(
			this,
			&ACosGameMode::HandleWaveCleared
		);
	}

	//indPlayerDeath();
	StartNextWave(true);
}

void ACosGameMode::StartNextWave(bool bResetTimer)
{//CurrentWaveIndex+1->  웨이브 스포너에 현재 웨이브 번호 전달(적 스폰)-> GameState에서도 번호 저장
	++CurrentWaveIndex;

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
	if (WaveIndex == 26)//보스 웨이브 클리어
	{
		PendingAction = EPostResultAction::GameClear;
	}
	else if (WaveIndex % 5 == 0)
	{
		PendingAction = EPostResultAction::Forge;

		LastForgeWave = WaveIndex;

		UE_LOG(LogTemp, Log, TEXT("[GameMode] 체크포인트 저장 : Wave %d"), LastForgeWave);

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

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	if (!PlayerController)
	{
		return;
	}

	// 죽은 캐릭터 제거
	if (APawn* OldPawn = PlayerController->GetPawn())
	{
		PlayerController->UnPossess();
		OldPawn->Destroy();
	}

	// 새 캐릭터 리스폰
	RestartPlayer(PlayerController);

	if (LastForgeWave == 0)//대장간 도달도 전에 죽으면 그냥 처음부터 시작하게
	{
		CurrentWaveIndex = 0;

		PendingAction = EPostResultAction::NextWave;

		StartNextWave(true);
		return;

	}

	CurrentWaveIndex = LastForgeWave;

	if (ACosGameState* GS = GetGameState<ACosGameState>())
	{
		GS->SetWaveIndex(LastForgeWave);
	}

	UE_LOG(LogTemp, Log, TEXT("[GameMode] 마지막 대장간으로 복귀: Wave %d"), LastForgeWave);

	PendingAction = EPostResultAction::Forge;

	//웨이브 다시 시작이 아니라 대장간 화면부터 다시 띄움
	OnForgeRequested.Broadcast();
}

