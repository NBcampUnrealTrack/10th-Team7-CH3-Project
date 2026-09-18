#include "Character/CosGameMode.h"
#include "Character/CosCharacter.h"
#include "Character/CosGameState.h"//CosGameState 연결
#include "UI/CosPlayerController.h"
#include "Spawn/WaveSpawner.h"
#include "Enemy/EnemyBase.h"
#include "Data/CosGameInstance.h"
#include "TimerManager.h"
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

	WaveSpawner->OnEnemySpawned.AddUObject(
		this,
		&ACosGameMode::HandleEnemySpawned
	);

	// 첫 웨이브 시작
	StartNextWave();
}


void ACosGameMode::StartNextWave()
{//CurrentWaveIndex+1->  웨이브 스포너에 현재 웨이브 번호 전달(적 스폰)-> GameState에서도 번호 저장
	++CurrentWaveIndex; // 의미가 스테이지 번호(1~6)로 바뀜

	UE_LOG(LogTemp, Warning,
		TEXT("[GameMode] StartNextWave -> %d"),
		CurrentWaveIndex);

	StartStageTimer(); // 이제 호출될 때마다 새 스테이지이므로 조건문 삭제

	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		CycleStartSoul = GI->GetSoul();
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

void ACosGameMode::StartStageTimer()//새로운 스테이지 시작될 때 5분 타이머 시작
{
	StopStageTimer();//혹시 이전 타이머 남아있으면 제거

	CurrentStageStartTime = GetWorld()->GetTimeSeconds();

	if (ACosGameState* GS = GetGameState<ACosGameState>())//스테이트 남은 시간을 처음에 300초로 설정
	{
		GS->SetStageRemainingTime(StageDuration);
	}

	GetWorld()->GetTimerManager().SetTimer(
		StageUpdateTimer,
		this,
		&ACosGameMode::UpdateStageTimer,
		0.1f,
		true
	);

	UE_LOG(LogTemp, Log,
		TEXT("[GameMode] Stage Timer 시작 / %.0f초"),
		StageDuration);
}

void ACosGameMode::UpdateStageTimer()// 현재 스테이지의 남은 시간을 계산해서 GameState에 전달
{
	if (!GetWorld())
	{
		return;
	}


	// 스테이지가 시작된 뒤 몇 초가 지났는지 계산
	const float ElapsedTime =
		GetWorld()->GetTimeSeconds() - CurrentStageStartTime;


	// 300초 - 경과시간 = 남은시간
	// FMath::Max를 사용해서 음수가 되지 않도록 함
	const float RemainingTime =
		FMath::Max(0.0f, StageDuration - ElapsedTime);





	if (ACosGameState* GS = GetGameState<ACosGameState>())	// GameState에 남은 시간 저장. 여기서 OnStageTimeChanged도 Broadcast 됨
	{
		GS->SetStageRemainingTime(RemainingTime);
	}

	if (RemainingTime <= 0.0f)	// 시간이 0이 되면 제한시간 초과
	{
		HandleStageTimeout();
	}
}

void ACosGameMode::StopStageTimer()// 스테이지가 끝났거나 GameOver가 됐을 때 타이머를 정지
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(StageUpdateTimer);
	}
}

void ACosGameMode::HandleStageTimeout()// 스테이지 제한시간 5분을 모두 사용했을 때 호출
{
	UE_LOG(LogTemp, Warning,
		TEXT("[GameMode] Stage Timeout -> Game Over"));

	TriggerGameOver();
}

void ACosGameMode::TriggerGameOver()// GameOver가 발생하는 경우의 공통으로 처리
{
	// 이미 GameOver가 처리된 상태라면 다시 실행하지 않음
	if (bGameOver)
	{
		return;
	}

	bGameOver = true;

	StopStageTimer();

	if (WaveSpawner)
	{
		WaveSpawner->StopWave();
	}

	UE_LOG(LogTemp, Log,
		TEXT("[GameMode] Game Over"));

	// UI 등에게 GameOver 알림
	OnGameOver.Broadcast();
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

	StopStageTimer();

	if (WaveIndex == 6) // 보스 스테이지
	{
		PendingAction = EPostResultAction::GameClear;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Forge 분기 진입"));

		PendingAction = EPostResultAction::Forge;
	}
	// NextWave 분기는 스포너가 1분 박자를 내부에서 처리하므로 삭제

	if (PendingAction == EPostResultAction::NextWave)
	{
		StartNextWave();// 그 외는 다음 웨이브로 넘어가게
		return;
	}

	// [추가] 결과창에 넘길 데이터 구성
	FWaveResultData ResultData;

	ResultData.WaveNumber = WaveIndex;

	ResultData.ElapsedSeconds =
		GetWorld()->GetTimeSeconds() - CurrentStageStartTime;

	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		ResultData.SoulEarned = GI->GetSoul() - CycleStartSoul;

		ResultData.TotalSoul = GI->GetSoul();
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
	TriggerGameOver();
}

void ACosGameMode::HandleEnemyKilled(AEnemyBase* DeadEnemy)
{
	if (ACosGameState* GS = GetGameState<ACosGameState>())
	{
		GS->AddKill();

		UE_LOG(LogTemp, Log,
			TEXT("[GameMode] KillCount 증가 -> %d"),
			GS->GetKillCount());
	}
}

void ACosGameMode::HandleEnemySpawned(AEnemyBase* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	// 새로 생성된 적이 죽었을 때
	// GameMode의 HandleEnemyKilled가 호출되도록 연결
	Enemy->OnEnemyKilled.AddUObject(
		this,
		&ACosGameMode::HandleEnemyKilled
	);
}