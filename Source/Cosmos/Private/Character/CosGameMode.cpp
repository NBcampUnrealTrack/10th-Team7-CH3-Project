#include "Character/CosGameMode.h"
#include "Character/CosCharacter.h"
#include "Character/CosGameState.h"//CosGameState 연결
#include "UI/CosPlayerController.h"
#include "Spawn/WaveSpawner.h"
#include "Enemy/EnemyBase.h"
#include "Tutorial/TutorialDeer.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"
#include "Data/CosGameInstance.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Camera/PlayerCameraManager.h"
#include "Data/EnchantPickup.h"
#include "UObject/ConstructorHelpers.h"

ACosGameMode::ACosGameMode()
{
	DefaultPawnClass = ACosCharacter::StaticClass();
	GameStateClass = ACosGameState::StaticClass();

	// 적들이 떨구는 것과 같은 인챈트를 기본값으로 씁니다. BP에서 바꿀 수 있습니다.
	static ConstructorHelpers::FClassFinder<AEnchantPickup> EnchantPickupBP(
		TEXT("/Game/Cosmos/BluePrints/BP_EnchantPickup"));
	if (EnchantPickupBP.Succeeded())
	{
		Day1EnchantPickupClass = EnchantPickupBP.Class;
	}
}

void ACosGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// The menu reuses the UI controller, but must not spawn the combat character.
	if (UGameplayStatics::GetCurrentLevelName(this, true) == TEXT("L_MenuLevel"))
	{
		DefaultPawnClass = nullptr;
	}
}

void ACosGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UGameplayStatics::GetCurrentLevelName(this, true) == TEXT("L_MenuLevel"))
	{
		return;
	}

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

#if !UE_BUILD_SHIPPING
	// Debug travel opens a fresh world: no tutorial timers, drops or death flow carry over.
	const int32 DebugDay = UGameplayStatics::GetIntOption(OptionsString, TEXT("CosDebugDay"), 0);
	if (DebugDay >= 2 && DebugDay <= 7)
	{
		for (TActorIterator<ATutorialDeer> It(GetWorld()); It; ++It)
		{
			It->Destroy();
		}
		CurrentWaveIndex = DebugDay - 2;
		StartNextWave();
		return;
	}
#endif

	// 레벨에 사슴이 있으면 Day 1부터, 없으면 기존처럼 첫 전투부터 시작
	ATutorialDeer* Deer = Cast<ATutorialDeer>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialDeer::StaticClass()));
	if (Deer && !Deer->IsDead())
	{
		StartDay1(Deer);
	}
	else
	{
		StartNextWave();
	}
}

AActor* ACosGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
#if !UE_BUILD_SHIPPING
	const int32 DebugDay = UGameplayStatics::GetIntOption(OptionsString, TEXT("CosDebugDay"), 0);
	if (DebugDay >= 2 && DebugDay <= 7)
	{
		if (APlayerStart* DebugStart = FindDayStart(DebugDay)) return DebugStart;
		int32 NumStarts = 0;
		while (FindDayStart(NumStarts + 1)) ++NumStarts;
		if (NumStarts > 0)
		{
			if (APlayerStart* CyclicStart = FindDayStart((DebugDay - 1) % NumStarts + 1)) return CyclicStart;
		}
	}
#endif
	// 처음 태어나는 곳은 Day 1 지점. 태그가 없는 맵(타이틀 등)은 엔진 기본 선택을 따릅니다.
	if (APlayerStart* Day1Start = FindDayStart(1))
	{
		return Day1Start;
	}
	return Super::ChoosePlayerStart_Implementation(Player);
}

APlayerStart* ACosGameMode::FindDayStart(int32 Day) const
{
	const FName Tag(*FString::Printf(TEXT("Day%d"), Day));
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		// Player Start Tag 칸과 Actor > Tags 배열 어느 쪽에 적어도 인식합니다.
		if (It->PlayerStartTag == Tag || It->ActorHasTag(Tag))
		{
			return *It;
		}
	}
	return nullptr;
}

void ACosGameMode::MovePlayerToDayStart(int32 Day)
{
	// Day1, Day2 ... 로 이어진 시작 지점 개수를 셉니다. 중간 번호가 빠지면 거기까지만 씁니다.
	int32 NumStarts = 0;
	while (FindDayStart(NumStarts + 1))
	{
		++NumStarts;
	}
	if (NumStarts == 0)
	{
		return; // Day 태그를 하나도 안 쓰는 맵이면 제자리에서 진행
	}

	// 지점을 순서대로 돌려 씁니다. 3개면 Day1~7 -> 1,2,3,1,2,3,1
	const int32 Slot = (Day - 1) % NumStarts + 1;
	APlayerStart* Start = FindDayStart(Day);
	if (!Start)
	{
		Start = FindDayStart(Slot);
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (!Pawn)
	{
		return;
	}

	const FRotator StartRotation(0.0f, Start->GetActorRotation().Yaw, 0.0f);
	if (ACharacter* Character = Cast<ACharacter>(Pawn))
	{
		Character->GetCharacterMovement()->StopMovementImmediately();
	}
	Pawn->SetActorLocationAndRotation(
		Start->GetActorLocation(), StartRotation, false, nullptr, ETeleportType::TeleportPhysics);
	PC->SetControlRotation(StartRotation); // 카메라가 컨트롤 회전을 따르므로 같이 돌려줍니다.

	UE_LOG(LogTemp, Log, TEXT("[GameMode] Day%d -> 시작 지점 %s로 이동"), Day, *Start->GetName());
}

void ACosGameMode::StartDay1(ATutorialDeer* Deer)
{
	bInDay1 = true;
	CurrentWaveIndex = 0;
	TotalEnemiesThisWave = 0;
	RemainingEnemiesThisWave = 0;

	// Day 1은 제한시간이 없습니다. 결과창 경과 시간 계산용으로 시작 시각만 기록합니다.
	CurrentStageStartTime = GetWorld()->GetTimeSeconds();

	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		CycleStartSoul = GI->GetSoul();
	}

	if (ACosGameState* GS = GetGameState<ACosGameState>())
	{
		GS->SetWaveIndex(GetCurrentDay());
	}

	// Day 1은 안개 없이 진행합니다. Day 2의 StartWave에서 다시 켜집니다.
	WaveSpawner->SetFogVisible(false);

	Deer->OnDeerKilled.AddDynamic(this, &ACosGameMode::HandleDeerKilled);

	// Day 1 동안 떨어진 인챈트 수와 획득 여부를 추적합니다.
	Day1EnchantDropCount = 0;
	bDay1EnchantCollected = false;
	bDay1ClearWaitingForEnchant = false;
	Day1ActorSpawnedHandle = GetWorld()->AddOnActorSpawnedHandler(
		FOnActorSpawned::FDelegate::CreateUObject(this, &ACosGameMode::HandleDay1ActorSpawned));
	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		GI->OnEnchantCollected.AddDynamic(this, &ACosGameMode::HandleDay1EnchantCollected);
	}

	UE_LOG(LogTemp, Warning, TEXT("[GameMode] Day 1 시작 - 사슴 대기"));
	OnDayStarted.Broadcast(GetCurrentDay());
}

void ACosGameMode::HandleDeerKilled(ATutorialDeer* Deer)
{
	if (!bInDay1 || bGameOver)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[GameMode] Day 1 사슴 사망 -> %.1f초 후 구울 %d마리"),
		Day1GhoulDelay, Day1GhoulCount);

	// 사슴이 죽는 순간부터 안개가 점점 짙어집니다.
	if (WaveSpawner)
	{
		WaveSpawner->StartTutorialFogFadeIn();
	}

	// 저주가 걸리는 순간 화면이 붉게 한 번 번쩍였다가 사라집니다.
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (PC->PlayerCameraManager && Day1FlashAlpha > 0.0f)
		{
			PC->PlayerCameraManager->StartCameraFade(
				Day1FlashAlpha, 0.0f, Day1FlashDuration, Day1FlashColor, false, false);
		}
	}

	// 비명은 사슴 자리에서, 저주·울부짖음은 조금씩 뒤따라 화면 전체에 들립니다.
	if (Day1DeerDeathSound && Deer)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Day1DeerDeathSound, Deer->GetActorLocation());
	}
	GetWorldTimerManager().SetTimer(Day1CurseSoundTimer,
		FTimerDelegate::CreateUObject(this, &ACosGameMode::PlayDay1Sound, Day1CurseSound.Get()),
		FMath::Max(Day1CurseSoundDelay, KINDA_SMALL_NUMBER), false);
	GetWorldTimerManager().SetTimer(Day1GhostWailSoundTimer,
		FTimerDelegate::CreateUObject(this, &ACosGameMode::PlayDay1Sound, Day1GhostWailSound.Get()),
		FMath::Max(Day1GhostWailSoundDelay, KINDA_SMALL_NUMBER), false);

	if (Day1GhoulDelay > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			Day1GhoulTimer, this, &ACosGameMode::SpawnDay1Ghouls, Day1GhoulDelay, false);
	}
	else
	{
		SpawnDay1Ghouls();
	}
}

void ACosGameMode::HandleDay1ActorSpawned(AActor* SpawnedActor)
{
	if (bInDay1 && SpawnedActor && SpawnedActor->IsA<AEnchantPickup>())
	{
		++Day1EnchantDropCount;
	}
}

void ACosGameMode::HandleDay1EnchantCollected(UEnchantData* CollectedEnchant)
{
	if (!bInDay1 || bDay1EnchantCollected)
	{
		return;
	}
	bDay1EnchantCollected = true;

	// 구울을 다 잡고 인챈트를 기다리던 중이면 이제 Day 1을 끝냅니다.
	if (bDay1ClearWaitingForEnchant && !bGameOver)
	{
		bDay1ClearWaitingForEnchant = false;
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Day 1 인챈트 획득 - Day 1 종료"));
		HandleWaveCleared(0);
	}
}

void ACosGameMode::StopTrackingDay1Enchant()
{
	if (Day1ActorSpawnedHandle.IsValid())
	{
		GetWorld()->RemoveOnActorSpawnedHandler(Day1ActorSpawnedHandle);
		Day1ActorSpawnedHandle.Reset();
	}
	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		GI->OnEnchantCollected.RemoveDynamic(this, &ACosGameMode::HandleDay1EnchantCollected);
	}
}

void ACosGameMode::PlayDay1Sound(USoundBase* Sound)
{
	if (Sound && !bGameOver)
	{
		UGameplayStatics::PlaySound2D(this, Sound);
	}
}

void ACosGameMode::SpawnDay1Ghouls()
{
	if (!bInDay1 || bGameOver || !WaveSpawner)
	{
		return;
	}

	// Day 1은 전투 BGM 없이 안개와 효과음만으로 진행합니다.

	// 구울이 전멸하면 스포너가 Stage 0 클리어를 방송하고 HandleWaveCleared로 이어집니다.
	WaveSpawner->StartTutorialStage(Day1GhoulCount);
}


void ACosGameMode::StartNextWave()
{//CurrentWaveIndex+1->  웨이브 스포너에 현재 웨이브 번호 전달(적 스폰)-> GameState에서도 번호 저장
	// 보스 스테이지가 마지막이므로 그 다음 스테이지는 시작하지 않습니다.
	if (bGameOver || IsBossStageIndex(CurrentWaveIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Stage %d 이후 스테이지 없음. StartNextWave 무시"), CurrentWaveIndex);
		return;
	}

	++CurrentWaveIndex; // 의미가 스테이지 번호(1~6)로 바뀜
	const bool bBossStage = IsBossStageIndex(CurrentWaveIndex);

	UE_LOG(LogTemp, Warning,
		TEXT("[GameMode] StartNextWave -> %d"),
		CurrentWaveIndex);

	// 스포너가 플레이어 위치 기준으로 스폰하므로 StartWave 전에 옮깁니다.
	MovePlayerToDayStart(GetCurrentDay());

	TotalEnemiesThisWave = 0;
	RemainingEnemiesThisWave = 0;

	StartStageTimer(); // 이제 호출될 때마다 새 스테이지이므로 조건문 삭제

	//전투 BGM 추가. 보스 스테이지는 보스 BGM
	if (ACosPlayerController* CosPC =
		Cast<ACosPlayerController>(
			UGameplayStatics::GetPlayerController(this, 0)))
	{
		if (bBossStage)
		{
			CosPC->OnBossBGMRequested();
		}
		else
		{
			CosPC->OnCombatBGMRequested();
		}
	}

	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		CycleStartSoul = GI->GetSoul();
	}

	if (WaveSpawner)//웨이브 스포너에 현재 웨이브 번호 전달, 적 스폰 시작함
	{
		WaveSpawner->StartWave(CurrentWaveIndex);
	}

	//게임스테이트에는 UI 표시용 Day 번호 전달 (Stage n = Day n+1)
	if (ACosGameState* GS = GetGameState<ACosGameState>())
	{
		GS->SetWaveIndex(GetCurrentDay());
	}

	OnDayStarted.Broadcast(GetCurrentDay());

	if (bBossStage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Stage %d 보스 등장"), CurrentWaveIndex);
		OnBossStageStarted.Broadcast();
	}
}

bool ACosGameMode::IsBossStageIndex(int32 StageIndex) const
{
	const int32 BossStageIndex = WaveSpawner ? WaveSpawner->GetBossStageIndex() : 6;
	return StageIndex == BossStageIndex;
}

void ACosGameMode::StartStageTimer()//새로운 스테이지 시작될 때 StageDuration(3분) 타이머 시작
{
	StopStageTimer();//혹시 이전 타이머 남아있으면 제거

	CurrentStageStartTime = GetWorld()->GetTimeSeconds();

	// 보스전은 처치로만 끝납니다. 남은 시간 변경을 방송해 HUD도 숨깁니다.
	if (IsBossStage())
	{
		if (ACosGameState* GS = GetGameState<ACosGameState>())
		{
			GS->SetStageRemainingTime(0.0f);
		}
		return;
	}

	if (ACosGameState* GS = GetGameState<ACosGameState>())//스테이트 남은 시간을 처음에 StageDuration으로 설정
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


	// StageDuration - 경과시간 = 남은시간
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

void ACosGameMode::HandleStageTimeout()// 스테이지 제한시간을 모두 사용했을 때 호출
{
	if (bGameOver || IsBossStage())
	{
		return;
	}

	if (!WaveSpawner)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[GameMode] Stage Timeout -> Game Over"));

		TriggerGameOver();
		return;
	}

	// 생존 스테이지는 시간을 버티면 클리어입니다. 남은 적은 소울 없이 치웁니다.
	UE_LOG(LogTemp, Warning,
		TEXT("[GameMode] Stage Timeout -> 생존 성공, Stage %d 클리어"), CurrentWaveIndex);

	StopStageTimer();
	WaveSpawner->StopWave();
	WaveSpawner->ClearRemainingEnemies();

	RemainingEnemiesThisWave = 0;
	PushEnemyCountUI();

	HandleWaveCleared(CurrentWaveIndex);
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
	GetWorldTimerManager().ClearTimer(Day1GhoulTimer);
	GetWorldTimerManager().ClearTimer(Day1CurseSoundTimer);
	GetWorldTimerManager().ClearTimer(Day1GhostWailSoundTimer);

	if (WaveSpawner)
	{
		WaveSpawner->StopWave();
	}

	UE_LOG(LogTemp, Log,
		TEXT("[GameMode] Game Over"));

	if (ACosPlayerController* CosPC =
		Cast<ACosPlayerController>(
			UGameplayStatics::GetPlayerController(this, 0)))
	{
		CosPC->OnGameOverBGMRequested();
	}

	// UI 등에게 GameOver 알림
	OnGameOver.Broadcast();
}

void ACosGameMode::StartGame()//타이들에서 게임 시작 누르면 전투(임시) 맵으로 이동하도록함
{
	if (UGameplayStatics::GetCurrentLevelName(this, true) == TEXT("L_MenuLevel"))
	{
		if (ACosPlayerController* PC = Cast<ACosPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
		{
			PC->ShowGameHUD(); // Keep the same intro/travel path as the title button.
			return;
		}
	}
	UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/Cosmos/Maps/L_BlockoutAstra")));
}

void ACosGameMode::HandleWaveCleared(int32 WaveIndex)
{
	UE_LOG(LogTemp, Warning,
		TEXT("[GameMode] HandleWaveCleared 호출 / WaveIndex = %d"),
		WaveIndex);

	// Day 1은 인챈트를 하나라도 먹어야 끝납니다. 먹는 순간 HandleDay1EnchantCollected가 여기로 다시 부릅니다.
	if (bInDay1 && WaveIndex == 0 && !bDay1EnchantCollected)
	{
		if (!bDay1ClearWaitingForEnchant)
		{
			bDay1ClearWaitingForEnchant = true;

			// 확률 드롭이 하나도 안 나왔으면 마지막 구울 자리에 하나 떨굽니다.
			if (Day1EnchantDropCount == 0)
			{
				AEnchantPickup* Pickup = nullptr;
				if (Day1EnchantPickupClass)
				{
					FActorSpawnParameters Params;
					Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					Pickup = GetWorld()->SpawnActor<AEnchantPickup>(
						Day1EnchantPickupClass, LastDay1KillLocation, FRotator::ZeroRotator, Params);
				}
				if (!Pickup)
				{
					// 떨굴 수 없으면 영원히 못 끝나므로 기다리지 않고 진행합니다.
					UE_LOG(LogTemp, Warning, TEXT("[GameMode] Day 1 인챈트 드롭 실패(Day1EnchantPickupClass 확인) - 인챈트 없이 진행"));
					bDay1EnchantCollected = true;
				}
			}

			if (!bDay1EnchantCollected)
			{
				UE_LOG(LogTemp, Warning, TEXT("[GameMode] Day 1 구울 전멸 - 인챈트를 획득할 때까지 대기"));
				return;
			}
		}
		else
		{
			return;
		}
	}

	if (bInDay1 && WaveIndex == 0)
	{
		StopTrackingDay1Enchant();
	}

	StopStageTimer();

	// 전투 종료 → BGM 정지
	if (ACosPlayerController* CosPC =
		Cast<ACosPlayerController>(
			UGameplayStatics::GetPlayerController(this, 0)))
	{
		CosPC->OnStopBGMRequested();
	}

	const bool bDay1Cleared = bInDay1 && WaveIndex == 0;

	if (bDay1Cleared) // Day 1 종료: 기본 재화 지급 후 첫 대장간
	{
		bInDay1 = false;

		if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
		{
			GI->AddSoul(Day1SoulReward);
		}

		PendingAction = EPostResultAction::Forge;
	}
	else if (IsBossStageIndex(WaveIndex)) // 보스 스테이지
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

	ResultData.WaveNumber = GetCurrentDay();

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
			if (bDay1Cleared)
			{
				// 주인공이 기절한 뒤 결과창 → 대장간에서 깨어남
				CosPC->PlayCollapseThenShowResult(ResultData);
			}
			else
			{
				CosPC->ShowResult(ResultData);
			}
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

		if (ACosPlayerController* CosPC =
			Cast<ACosPlayerController>(
				UGameplayStatics::GetPlayerController(this, 0)))
		{
			CosPC->OnForgeBGMRequested();
		}

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
	if (bInDay1 && DeadEnemy)
	{
		LastDay1KillLocation = DeadEnemy->GetActorLocation(); // 인챈트 보장 드롭 위치
	}

	if (ACosGameState* GS = GetGameState<ACosGameState>())
	{
		GS->AddKill();

		UE_LOG(LogTemp, Log,
			TEXT("[GameMode] KillCount 증가 -> %d"),
			GS->GetKillCount());
	}

	RemainingEnemiesThisWave = FMath::Max(0, RemainingEnemiesThisWave - 1);
	PushEnemyCountUI();

}

void ACosGameMode::HandleEnemySpawned(AEnemyBase* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	++TotalEnemiesThisWave;
	++RemainingEnemiesThisWave;
	PushEnemyCountUI();

	// 새로 생성된 적이 죽었을 때
	// GameMode의 HandleEnemyKilled가 호출되도록 연결
	Enemy->OnEnemyKilled.AddUObject(
		this,
		&ACosGameMode::HandleEnemyKilled
	);
}

void ACosGameMode::PushEnemyCountUI()
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (ACosPlayerController* CosPC = Cast<ACosPlayerController>(PC))
		{
			CosPC->UpdateEnemyCountUI(RemainingEnemiesThisWave, TotalEnemiesThisWave);
		}
	}
}
