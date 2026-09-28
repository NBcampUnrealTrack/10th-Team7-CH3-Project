#include "UI/CosPlayerController.h"
#include "Character/CosCharacter.h"
#include "Character/CosGameMode.h"
#include "Character/CosGameState.h"
#include "Character/HealthComponent.h"
#include "Weapon/WeaponBase.h"
#include "Weapon/ShotgunWeapon.h"
#include "Weapon/CombatComponent.h"
#include "Weapon/CosLegacyCameraShake.h"
#include "Data/CosGameInstance.h" 
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h" 
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "UI/CosLoadingWidget.h"
#include "UI/CosDialogueWidget.h"
#include "UI/CosIntroVideoWidget.h"
#include "UI/CosBossHealthWidget.h"
#include "MediaPlayer.h"
#include "FileMediaSource.h"
#include "Misc/Paths.h"
#include "UObject/ConstructorHelpers.h"

/*
// 테스트용 임시 체력 변수
static float TestCurrentHealth = 100.f;
static float TestMaxHealth = 100.f;
static int32 TestCurrentAmmo = 4;
static int32 TestMaxAmmo = 4;
*/

ACosPlayerController::ACosPlayerController()
	: InputMappingContext(nullptr),
	TitleWidgetClass(nullptr),
	TitleWidgetInstance(nullptr),
	CombatHUDClass(nullptr),
	CombatHUDInstance(nullptr),
	ForgeWidgetClass(nullptr),
	ForgeWidgetInstance(nullptr),
	ResultWidgetClass(nullptr),
	ResultWidgetInstance(nullptr),
	ESCWidgetClass(nullptr),
	ESCWidgetInstance(nullptr)
{
	CosLegacyCameraShakeClass = UCosLegacyCameraShake::StaticClass();
	LoadingWidgetClass = UCosLoadingWidget::StaticClass();
	DialogueWidgetClass = UCosDialogueWidget::StaticClass();
	IntroVideoWidgetClass = UCosIntroVideoWidget::StaticClass();

	// 대장장이 대사 기본값. BP 디테일 패널(UI|Dialogue)에서 고치면 그 값이 우선합니다.
	auto AddVisit = [this](std::initializer_list<const TCHAR*> InLines)
	{
		FForgeVisitDialogue& Visit = ForgeVisitDialogues.AddDefaulted_GetRef();
		for (const TCHAR* Line : InLines)
		{
			Visit.Lines.Add(FText::FromString(Line));
		}
	};
	AddVisit({ // 방문 1
		TEXT("깼네. 숲 한가운데 쓰러져 있길래 끌고 왔어. 무겁더라."),
		TEXT("손에 묻은 거, 네 피는 아니야. 염소 피야. 걱정 마."),
		TEXT("빈손으로 밤을 넘기긴 어려울 거야. 이거 받아. 뭐든 제자리에 붙들어 두는 물건이야.") });
	AddVisit({ // 방문 2
		TEXT("살아 돌아왔네. 난 네가 돌아올 줄 알았어."),
		TEXT("무덤에서 일어난 것들은 다시 눕혀줘야 해. 못은 그러라고 있는 거야.") });
	AddVisit({ // 방문 3
		TEXT("오늘은 안개가 좀 들어왔네. 신경 쓰지 마, 문을 오래 열어둬서 그래."),
		TEXT("못은 박힐 때보다, 빠지지 않을 때 진짜 일을 하는 거야.") });
	AddVisit({ // 방문 4
		TEXT("아까 그 제단, 처음 보는 것 같았어? 숲은 원래 다 비슷하게 생겼어."),
		TEXT("오늘이 몇 번째 밤이더라. 난 가끔 세는 걸 잊어.") });
	AddVisit({ // 방문 5
		TEXT("그 염소 봤구나. 아직 거기 있지? 여기선 아무것도 썩지 않아. 아무것도 떠나지 않고."),
		TEXT("눈이 좀 시려서. 바닷바람을 너무 오래 맞았나 봐.") });
	AddVisit({ // 방문 6
		TEXT("이제 네 무기엔 내 손이 안 닿은 곳이 없어."),
		TEXT("못은 한쪽 끝만 날카로워. 다른 쪽 끝은 언제나 누군가 쥐고 있지."),
		TEXT("내일 밤이면 끝나. 네가 원하던 대로.") });

	// WBP_Forge가 배경 영상을 재생하는 미디어 플레이어입니다.
	static ConstructorHelpers::FObjectFinder<UMediaPlayer> ForgeMediaPlayerAsset(TEXT("/Game/Movies/NewMediaPlayer"));
	if (ForgeMediaPlayerAsset.Succeeded())
	{
		ForgeMediaPlayer = ForgeMediaPlayerAsset.Object;
	}
}
	

void ACosPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (IsValid(InputMappingContext.Get()))
			{
				Subsystem->AddMappingContext(InputMappingContext.Get(), 0);
				UE_LOG(LogTemp, Warning, TEXT("IMC_Character"));
			}
		}
	}

	// 현재 레벨 이름
	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);

	if (CurrentLevelName == TEXT("L_MenuLevel"))
	{
		ShowTitleWidget();
	}
	else
	{
		// Intro playback belongs to the menu. Wait for GameMode's Day 1 setup
		// before binding HUD values and selecting gameplay music.
		bIntroChecked = true;
		UCosGameInstance* GI = GetGameInstance<UCosGameInstance>();
		if (GI && GI->bPendingGameplayReveal && PlayerCameraManager)
		{
			GI->bPendingGameplayReveal = false;
			PlayerCameraManager->SetManualCameraFade(1.0f, FLinearColor::Black, false);
			SetInputMode(FInputModeUIOnly());
			bShowMouseCursor = false;
			SetIgnoreMoveInput(true);
			SetIgnoreLookInput(true);
			GetWorldTimerManager().SetTimer(GameplayEntryTimer, this,
				&ACosPlayerController::FinishGameplayEntry, FMath::Max(GameplayWarmupSeconds, 0.01f), false);
		}
		else
		{
			GetWorldTimerManager().SetTimerForNextTick(this, &ACosPlayerController::ShowGameHUD);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("현재 레벨: %s"), *CurrentLevelName);

	if (ACosGameMode* GM = Cast<ACosGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->OnForgeRequested.AddDynamic(
			this,
			&ACosPlayerController::HandleForgeRequested
		);

		GM->OnGameOver.AddDynamic(
			this,
			&ACosPlayerController::HandleGameOverRequested
		);
	}

		

}



void ACosPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (IsValid(InputMappingContext.Get()))
			{
				Subsystem->RemoveMappingContext(InputMappingContext.Get());
				Subsystem->AddMappingContext(InputMappingContext.Get(), 0);
			}
		}
	}

	// 레벨 재시작이나 폰 재스폰 시 바인딩을 다시 안전하게 연결
	SetupCharacterBindings();
}

// 캐릭터 및 컴포넌트 델리게이트 바인딩
void ACosPlayerController::SetupCharacterBindings()
{
	APawn* ControlledPawn = GetPawn();
	if (!IsValid(ControlledPawn)) return;

	// 1. HealthComponent 델리게이트 바인딩
	if (UHealthComponent* HealthComp = ControlledPawn->FindComponentByClass<UHealthComponent>())
	{
		if (IsValid(HealthComp))
		{
			// 체력 변경 이벤트
			HealthComp->OnHealthChanged.RemoveDynamic(this, &ACosPlayerController::UpdateHP);
			HealthComp->OnHealthChanged.AddDynamic(this, &ACosPlayerController::UpdateHP);

			// 사망 이벤트
			HealthComp->OnDeath.RemoveDynamic(this, &ACosPlayerController::OnCharacterDeath);
			HealthComp->OnDeath.AddDynamic(this, &ACosPlayerController::OnCharacterDeath);

			// 피격 이벤트. 카메라 흔들림
			HealthComp->OnDamaged.RemoveDynamic(this, &ACosPlayerController::HandlePlayerDamaged);
			HealthComp->OnDamaged.AddDynamic(this, &ACosPlayerController::HandlePlayerDamaged);

			// 초기 체력값 UI 즉시 반영 함수
			UpdateHP(HealthComp->GetCurrentHealth(), HealthComp->GetMaxHealth());
		}
	}

	
	// 2. ShotgunWeapon 델리게이트 바인딩
	if (UCombatComponent* CombatComp = ControlledPawn->FindComponentByClass<UCombatComponent>())
	{
		if (!CombatComp->IsCombatReady()) // 컴포넌트 BeginPlay 전이면 준비 완료 시 다시 호출
		{
			CombatComp->OnCombatReady.RemoveDynamic(this, &ACosPlayerController::SetupCharacterBindings);
			CombatComp->OnCombatReady.AddDynamic(this, &ACosPlayerController::SetupCharacterBindings);
		}
		else
		{
			if (AShotgunWeapon* Weapon = CombatComp->GetShotgunWeapon())
			{
				Weapon->OnAmmoChanged.RemoveDynamic(this, &ACosPlayerController::UpdateAmmoUI);
				Weapon->OnAmmoChanged.AddDynamic(this, &ACosPlayerController::UpdateAmmoUI);
				UpdateAmmoUI(Weapon->GetCurrentAmmo(), Weapon->GetCurrentMaxAmmo());
			}

			CombatComp->OnPotionCountChanged.RemoveDynamic(this, &ACosPlayerController::UpdatePotionUI);
			CombatComp->OnPotionCountChanged.AddDynamic(this, &ACosPlayerController::UpdatePotionUI);
			UpdatePotionUI(CombatComp->GetPotionCount());

			TArray<AActor*> AttachedActors;
			ControlledPawn->GetAttachedActors(AttachedActors, true, true); // 하위 부착까지 포함
			for (AActor* Attached : AttachedActors)
			{
				if (AWeaponBase* Weapon = Cast<AWeaponBase>(Attached))
				{
					Weapon->OnKillConfirmed.RemoveDynamic(this, &ACosPlayerController::HandleKillConfirmed);
					Weapon->OnKillConfirmed.AddDynamic(this, &ACosPlayerController::HandleKillConfirmed);
				}
				if (AShotgunWeapon* Shotgun = Cast<AShotgunWeapon>(Attached))
				{
					Shotgun->OnHitConfirmed.RemoveDynamic(this, &ACosPlayerController::HandleHitConfirmed);
					Shotgun->OnHitConfirmed.AddDynamic(this, &ACosPlayerController::HandleHitConfirmed);
				}
			}
		}
	}

	// 3. GameInstance의 소울(재화) 변경 이벤트 바인딩
	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		GI->OnCurrencyChanged.RemoveDynamic(this, &ACosPlayerController::UpdateSoulUI);
		GI->OnCurrencyChanged.AddDynamic(this, &ACosPlayerController::UpdateSoulUI);

		// 초기값 UI 즉시 반영
		UpdateSoulUI(GI->GetSoul());
	}



}

// ESC 키 입력을 ToggleESCMenu에 바인딩해줌
void ACosPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (IsValid(ESCAction))
		{
			EnhancedInput->BindAction(
				ESCAction,
				ETriggerEvent::Started,
				this,
				&ACosPlayerController::ToggleESCMenu
			);
		}
	}
}

// OnStateChanged가 실행되면 자동 호출
void ACosPlayerController::HandleGameStateChanged()
{
	if (ACosGameState* GS = GetWorld()->GetGameState<ACosGameState>())
	{
		// GameState에서 WaveIndex를 가져와 UI 업데이트
		UpdateWaveUI(GS->GetWaveIndex());
		UpdateKillUI(GS->GetKillCount());
	}
}

// UI 호출 함수
void ACosPlayerController::UpdateWaveUI(int32 CurrentWave)
{
	if (IsValid(CombatHUDInstance.Get()))
	{
		FString Cmd = FString::Printf(TEXT("SetWaveText %d"), CurrentWave);
		CombatHUDInstance->CallFunctionByNameWithArguments(*Cmd, *GLog, nullptr, true);
	}
}

void ACosPlayerController::UpdateHP(float CurrentHealth, float MaxHealth)
{

	UE_LOG(LogTemp, Warning, TEXT("UpdateHP 호출됨: %f / %f"), CurrentHealth, MaxHealth);
	// UI 위젯이 살아있는지 IsValid로 검사
	if (IsValid(CombatHUDInstance.Get()))
	{
		FString Cmd = FString::Printf(TEXT("UpdateHP %f %f"), CurrentHealth, MaxHealth);
		CombatHUDInstance->CallFunctionByNameWithArguments(*Cmd, *GLog, nullptr, true);
	}
}


void ACosPlayerController::UpdateAmmoUI(int32 CurrentAmmo, int32 MaxAmmo)
{
	UE_LOG(LogTemp, Warning, TEXT("UpdateAmmoUI 호출됨: %d / %d"), CurrentAmmo, MaxAmmo);

	if (IsValid(CombatHUDInstance.Get()))
	{
		FString Cmd = FString::Printf(TEXT("SetAmmoText %d %d"), CurrentAmmo, MaxAmmo);
		CombatHUDInstance->CallFunctionByNameWithArguments(*Cmd, *GLog, nullptr, true);
	}
}

// 소울이 변경될 때마다 호출
void ACosPlayerController::UpdateSoulUI(int32 CurrentSoul)
{
	UE_LOG(LogTemp, Warning, TEXT("UpdateSoulUI 호출됨: %d"), CurrentSoul);

	if (IsValid(CombatHUDInstance.Get()))
	{

		FString Cmd = FString::Printf(TEXT("SetSoulText %d"), CurrentSoul);
		UE_LOG(LogTemp, Warning, TEXT("호출할 커맨드: %s"), *Cmd);  

		CombatHUDInstance->CallFunctionByNameWithArguments(*Cmd, *GLog, nullptr, true);
	}
}

void ACosPlayerController::UpdateKillUI(int32 KillCount)
{
	if (IsValid(CombatHUDInstance.Get()))
	{
		FString Cmd = FString::Printf(TEXT("SetKillText %d"), KillCount);
		CombatHUDInstance->CallFunctionByNameWithArguments(*Cmd, *GLog, nullptr, true);
	}
}

void ACosPlayerController::HandleStageTimeChanged(float RemainingSeconds)
{
	UpdateStageTimeUI(RemainingSeconds);
}

void ACosPlayerController::UpdateStageTimeUI(float RemainingSeconds)
{
	const ACosGameMode* GM = GetWorld()->GetAuthGameMode<ACosGameMode>();
	const bool bBossStage = GM && GM->IsBossStage();
	if (CombatHUDInstance)
	{
		if (UWidget* TimerText = CombatHUDInstance->GetWidgetFromName(TEXT("StageTime")))
		{
			TimerText->SetVisibility(bBossStage ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
		}
	}
	if (bBossStage) return;

	// 분/초 단위 계산
	const int32 TotalSeconds = FMath::Max(0, FMath::FloorToInt(RemainingSeconds));
	const int32 Minutes = TotalSeconds / 60;
	const int32 Seconds = TotalSeconds % 60;

	// 2. 전투 HUD에 남은 시간 전달
	if (IsValid(CombatHUDInstance.Get()))
	{
		// 블프 SetStageTimeText(Minutes, Seconds) 함수 호출
		FString Cmd = FString::Printf(TEXT("SetStageTimeText %d %d"), Minutes, Seconds);
		CombatHUDInstance->CallFunctionByNameWithArguments(*Cmd, *GLog, nullptr, true);
	}

	// 시간이 0이 됐을 때 게임오버인지(보스 스테이지) 클리어인지(생존 스테이지)는 게임모드가 정합니다.
	// 게임오버면 게임모드의 OnGameOver -> HandleGameOverRequested로 게임오버 화면이 뜹니다.
}

void ACosPlayerController::UpdateEnemyCountUI(int32 RemainingEnemies, int32 TotalEnemies)
{
	if (IsValid(CombatHUDInstance.Get()))
	{
		// 블루프린트 커스텀 이벤트 호출
		FString Cmd = FString::Printf(TEXT("SetEnemyCountText %d %d"), RemainingEnemies, TotalEnemies);
		CombatHUDInstance->CallFunctionByNameWithArguments(*Cmd, *GLog, nullptr, true);
	}
}

void ACosPlayerController::OnCharacterDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("플레이어 캐릭터 사망"));

	if (APawn* ControlledPawn = GetPawn())
	{
		// 입력 비활성화
		ControlledPawn->DisableInput(this);

		// 이동 강제 정지
		if (ACharacter* PossessedCharacter = Cast<ACharacter>(ControlledPawn))
		{
			if (UCharacterMovementComponent* MoveComp = PossessedCharacter->GetCharacterMovement())
			{
				MoveComp->StopMovementImmediately();
			}
		}
	}

	if (ACosGameMode* GM = GetWorld()->GetAuthGameMode<ACosGameMode>())
	{
		GM->HandlePlayerDeath();
	}

}

void ACosPlayerController::RefreshCombatHUD()
{
	if (APawn* ControlledPawn = GetPawn())
	{
		if (UHealthComponent* HealthComp = ControlledPawn->FindComponentByClass<UHealthComponent>())
		{
			UpdateHP(HealthComp->GetCurrentHealth(), HealthComp->GetMaxHealth());
		}

		if (UCombatComponent* CombatComp = ControlledPawn->FindComponentByClass<UCombatComponent>())
		{
			if (AShotgunWeapon* Weapon = CombatComp->GetShotgunWeapon())
			{
				UpdateAmmoUI(Weapon->GetCurrentAmmo(), Weapon->GetCurrentMaxAmmo()); 
			}

			UpdatePotionUI(CombatComp->GetPotionCount());
		}
	}

	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		UpdateSoulUI(GI->GetSoul());
	}
}

void ACosPlayerController::ShowCombatHUD()
{
	// Class 유효성 및 Instance가 이미 생성되었는지 IsValid로 검사
	if (IsValid(CombatHUDClass) && !IsValid(CombatHUDInstance))
	{
		CombatHUDInstance = CreateWidget<UUserWidget>(this, CombatHUDClass);

		if (IsValid(CombatHUDInstance.Get()))
		{
			CombatHUDInstance->AddToViewport();
			BossHealthHUDInstance = CreateWidget<UCosBossHealthWidget>(this);
			if (BossHealthHUDInstance) BossHealthHUDInstance->AddToViewport(1);

			// 게임 입력모드 전환
			SetInputMode(FInputModeGameOnly());
			bShowMouseCursor = false;

			RefreshCombatHUD();
			if (const ACosGameState* GS = GetWorld()->GetGameState<ACosGameState>())
			{
				UpdateStageTimeUI(GS->GetStageRemainingTime());
			}
		}
	}
}


void ACosPlayerController::CloseCombatHUD()
{
	if (BossHealthHUDInstance)
	{
		BossHealthHUDInstance->RemoveFromParent();
		BossHealthHUDInstance = nullptr;
	}
	if (IsValid(CombatHUDInstance.Get()))
	{
		CombatHUDInstance->RemoveFromParent();
		CombatHUDInstance = nullptr;
	}
}

UUserWidget* ACosPlayerController::GetHUDWidget() const
{
	return CombatHUDInstance.Get();
}

void ACosPlayerController::ShowTitleWidget()
{
	if (UGameplayStatics::GetCurrentLevelName(this, true) != TEXT("L_MenuLevel"))
	{
		return;
	}

	// Class 유효성 검사, Instance가 이미 생성되었는지 확인
	if (IsValid(TitleWidgetClass) && !IsValid(TitleWidgetInstance))
	{
		// No gameplay runs in the menu; keep widget fades and latent delays ticking.
		SetPause(false);

		TitleWidgetInstance = CreateWidget<UUserWidget>(this, TitleWidgetClass); //위젯 인스턴스 생성(메모리상에 객체만 둠. 아직 화면에는 X)

		if (IsValid(TitleWidgetInstance.Get()))
		{
			TitleWidgetInstance->AddToViewport();

			// 타이틀 BGM 시작
			OnTitleBGMRequested();

			// 메뉴 화면이므로 입력 모드를 UI 전용으로 전환
			bShowMouseCursor = true;
			SetInputMode(FInputModeUIOnly());
		}
	}
}

void ACosPlayerController::HideTitleWidget()
{
	if (IsValid(TitleWidgetInstance.Get()))
	{
		TitleWidgetInstance->RemoveFromParent(); // TitleWidgetInstance를 부모(뷰포트)로부터 분리
		TitleWidgetInstance = nullptr; // nullptr로 리셋해서 인스턴스가 없음을 명확히 함 
	}

	// 메뉴를 닫으면 다시 게임 입력 모드로 전환
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}

/*
// 테스트용
void ACosPlayerController::TestDecreaseHP() { UpdateHP(70.0f, 100.0f); }

void ACosPlayerController::TestHealHP() { UpdateHP(100.0f, 100.0f); }

void ACosPlayerController::TestDecreaseAmmo() { UpdateAmmoUI(7, 30); }

void ACosPlayerController::TestReloadAmmo() { UpdateAmmoUI(30, 30); }
*/


void ACosPlayerController::HandleForgeRequested()
{
	// 방금 끝난 Day 번호가 곧 대장간 방문 번호입니다(Day 1 종료 → 방문 1). 로딩이 걷힌 뒤 대사를 띄웁니다.
	const ACosGameMode* GM = GetWorld()->GetAuthGameMode<ACosGameMode>();
	PendingForgeVisit = GM ? GM->GetCurrentDay() : 0;

	// Day 1에서 쓰러졌다면 대장간에서 깨어납니다.
	RecoverFromCollapse();

	// 대장간 배경 영상은 비동기로 열려 처음 몇 프레임이 비어 보일 수 있으므로,
	// 로딩 화면을 대장간 위에 덮어 두고 영상이 재생되기 시작하면 걷습니다.
	if (IsValid(LoadingWidgetClass) && !IsValid(LoadingWidgetInstance))
	{
		LoadingWidgetInstance = CreateWidget<UUserWidget>(this, LoadingWidgetClass);
	}
	if (IsValid(LoadingWidgetInstance.Get()) && !LoadingWidgetInstance->IsInViewport())
	{
		LoadingWidgetInstance->AddToViewport(100); // 대장간 위젯보다 위에 그립니다.
	}

	OpenForgeWidget();

	if (IsValid(LoadingWidgetInstance.Get()))
	{
		ForgeLoadingStartTime = GetWorld()->GetRealTimeSeconds();
		GetWorldTimerManager().SetTimer(ForgeLoadingTimer, this, &ACosPlayerController::CheckForgeLoading, 0.1f, true);
	}
	else
	{
		ShowForgeDialogue(); // 로딩 화면이 없으면 바로 띄웁니다.
	}
}

void ACosPlayerController::ShowForgeDialogue()
{
	const int32 VisitIndex = PendingForgeVisit - 1;
	PendingForgeVisit = 0;

	if (!ForgeVisitDialogues.IsValidIndex(VisitIndex) || ForgeVisitDialogues[VisitIndex].Lines.IsEmpty()
		|| !IsValid(DialogueWidgetClass))
	{
		return;
	}

	ForgeDialogueInstance = CreateWidget<UCosDialogueWidget>(this, DialogueWidgetClass);
	if (IsValid(ForgeDialogueInstance.Get()))
	{
		ForgeDialogueInstance->AddToViewport(50); // 대장간 위, 로딩 화면 아래
		ForgeDialogueInstance->ShowLines(ForgeSpeaker, ForgeVisitDialogues[VisitIndex].Lines);
	}
}

void ACosPlayerController::CheckForgeLoading()
{
	const float Elapsed = GetWorld()->GetRealTimeSeconds() - ForgeLoadingStartTime;
	const bool bVideoReady = !ForgeMediaPlayer || ForgeMediaPlayer->IsPlaying();

	if ((Elapsed >= ForgeLoadingMinSeconds && bVideoReady) || Elapsed >= ForgeLoadingMaxSeconds)
	{
		HideLoadingWidget();
	}
}

void ACosPlayerController::HideLoadingWidget()
{
	GetWorldTimerManager().ClearTimer(ForgeLoadingTimer);
	if (IsValid(LoadingWidgetInstance.Get()))
	{
		LoadingWidgetInstance->RemoveFromParent();
	}
	ShowForgeDialogue();
}

void ACosPlayerController::OpenForgeWidget()
{
	CloseCombatHUD();

	if (IsValid(ForgeWidgetClass) && !IsValid(ForgeWidgetInstance))
	{
		ForgeWidgetInstance = CreateWidget<UUserWidget>(this, ForgeWidgetClass);
	}

	if (IsValid(ForgeWidgetInstance.Get()))
	{
		ForgeWidgetInstance->AddToViewport();
		SetUIInputMode(true);
	}
}

void ACosPlayerController::CloseForgeWidget()
{
	if (IsValid(ForgeWidgetInstance.Get()))
	{
		ForgeWidgetInstance->RemoveFromParent();
		ForgeWidgetInstance = nullptr;
	}

	// 대사를 다 넘기기 전에 대장간을 나가도 대사창이 전투 화면에 남지 않게 닫습니다.
	PendingForgeVisit = 0;
	if (IsValid(ForgeDialogueInstance.Get()))
	{
		ForgeDialogueInstance->RemoveFromParent();
		ForgeDialogueInstance = nullptr;
	}

	//포션 충전
	if (APawn* ControlledPawn = GetPawn())
	{
		if (UCombatComponent* CombatComp = ControlledPawn->FindComponentByClass<UCombatComponent>())
		{
			CombatComp->RefillPotions();
			if (AShotgunWeapon* Shotgun = CombatComp->GetShotgunWeapon()) // 탄약 최대로
			{
				Shotgun->RefillAmmo();
			}
		}
		if (UHealthComponent* HealthComp = ControlledPawn->FindComponentByClass<UHealthComponent>()) // 체력 최대로
		{
			HealthComp->Heal(HealthComp->GetMaxHealth()); 
		}
	}

	SetUIInputMode(false);
	ShowCombatHUD();

	// 다음 웨이브 호출
	if (ACosGameMode* GM = GetWorld()->GetAuthGameMode<ACosGameMode>())
	{
		GM->StartNextWave();
	}
}

void ACosPlayerController::ShowResult(const FWaveResultData& ResultData)
{
	CloseCombatHUD();

	if (IsValid(ResultWidgetClass) && !IsValid(ResultWidgetInstance))
	{
		ResultWidgetInstance = CreateWidget<UUserWidget>(this, ResultWidgetClass);
	}

	if (IsValid(ResultWidgetInstance.Get()))
	{
		ResultWidgetInstance->AddToViewport();

		const int32 TotalSeconds = FMath::FloorToInt(ResultData.ElapsedSeconds);
		const int32 Minutes = TotalSeconds / 60;
		const int32 Seconds = TotalSeconds % 60;

		FString Cmd = FString::Printf(
			TEXT("SetResult %d %d %d %d %d"), // 인자 5개
			ResultData.WaveNumber,
			Minutes,
			Seconds,
			ResultData.SoulEarned,
			ResultData.TotalSoul
	);
		ResultWidgetInstance->CallFunctionByNameWithArguments(*Cmd, *GLog, nullptr, true);

		bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
}

bool ACosPlayerController::InputKey(const FInputKeyEventArgs& EventArgs)
{
	if (HandleDebugKey(EventArgs)) return true;
	// 쓰러지는 동안은 ESC 메뉴를 포함한 모든 입력을 무시합니다.
	if (bIsCollapsing)
	{
		return true;
	}

	if (IsValid(ResultWidgetInstance.Get()) && EventArgs.Event == IE_Pressed)
	{
		HideResult();
		return true; 
	}

	return Super::InputKey(EventArgs);
}

void ACosPlayerController::HideResult()
{
	if (IsValid(ResultWidgetInstance.Get()))
	{
		ResultWidgetInstance->RemoveFromParent();
		ResultWidgetInstance = nullptr;
	}

	SetUIInputMode(false); // 상황에 맞게 게임 입력모드 복귀

	if (ACosGameMode* GM = GetWorld()->GetAuthGameMode<ACosGameMode>())
	{
		GM->OnResultConfirmed(); // Forge/NextWave/GameClear 중 하나로 이어짐
	}
}

void ACosPlayerController::PlayCollapseThenShowResult(const FWaveResultData& ResultData)
{
	APawn* ControlledPawn = GetPawn();
	UCameraComponent* Camera = ControlledPawn ? ControlledPawn->FindComponentByClass<UCameraComponent>() : nullptr;
	if (bIsCollapsing || !IsValid(Camera))
	{
		ShowResult(ResultData); // 연출을 못 하면 결과창만이라도 띄웁니다.
		return;
	}

	PendingCollapseResult = ResultData;
	bIsCollapsing = true;
	bIsCollapsed = true;
	bCollapseImpactPlayed = false;

	// 입력 비활성화, 이동 강제 정지
	ControlledPawn->DisableInput(this);
	if (ACharacter* PossessedCharacter = Cast<ACharacter>(ControlledPawn))
	{
		if (UCharacterMovementComponent* MoveComp = PossessedCharacter->GetCharacterMovement())
		{
			MoveComp->StopMovementImmediately();
		}
	}
	CloseCombatHUD();
	CollapseCamera = Camera;

	// 먼저 심박만 들리다가 HeartbeatDuration 뒤에 쓰러집니다.
	if (HeartbeatSound)
	{
		HeartbeatAudio = UGameplayStatics::SpawnSound2D(this, HeartbeatSound);
	}

	// 심박에 맞춰 화면이 붉게 번쩍입니다.
	HeartbeatFlashesLeft = HeartbeatFlashCount;
	if (HeartbeatFlashesLeft > 0)
	{
		GetWorldTimerManager().SetTimer(HeartbeatFlashTimer, this, &ACosPlayerController::PlayHeartbeatFlash,
			HeartbeatFlashInterval, true, FMath::Max(HeartbeatFlashFirstDelay, KINDA_SMALL_NUMBER));
	}

	if (HeartbeatDuration > 0.0f)
	{
		GetWorldTimerManager().SetTimer(CollapseFallTimer, this, &ACosPlayerController::BeginCollapseFall, HeartbeatDuration, false);
	}
	else
	{
		BeginCollapseFall();
	}
}

void ACosPlayerController::PlayHeartbeatFlash()
{
	if (HeartbeatFlashesLeft <= 0)
	{
		GetWorldTimerManager().ClearTimer(HeartbeatFlashTimer);
		return;
	}
	--HeartbeatFlashesLeft;

	if (PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(
			HeartbeatFlashAlpha, 0.0f, HeartbeatFlashDuration, HeartbeatFlashColor, false, false);
	}

	if (HeartbeatFlashesLeft <= 0)
	{
		GetWorldTimerManager().ClearTimer(HeartbeatFlashTimer);
	}
}

void ACosPlayerController::BeginCollapseFall()
{
	// 쓰러지기 시작하면 남은 번쩍임은 취소합니다. 뒤이은 검은 페이드와 겹치지 않게 합니다.
	GetWorldTimerManager().ClearTimer(HeartbeatFlashTimer);
	HeartbeatFlashesLeft = 0;

	UCameraComponent* Camera = CollapseCamera.Get();
	if (!Camera)
	{
		FinishCollapse();
		return;
	}

	// 컨트롤 회전 대신 카메라를 직접 기울이기 위해 시작 시선을 기억해 둡니다.
	CollapseCameraStartLocation = Camera->GetRelativeLocation();
	CollapseStartPitch = GetControlRotation().GetNormalized().Pitch;
	Camera->bUsePawnControlRotation = false;
	CollapseStartTime = GetWorld()->GetTimeSeconds();

	UpdateCollapseCamera();
	GetWorldTimerManager().SetTimer(CollapseCameraTimer, this, &ACosPlayerController::UpdateCollapseCamera, 0.01f, true);
	GetWorldTimerManager().SetTimer(CollapseFadeTimer, this, &ACosPlayerController::StartCollapseFade,
		FMath::Max(CollapseFadeDelay, KINDA_SMALL_NUMBER), false);
	GetWorldTimerManager().SetTimer(CollapseResultTimer, this, &ACosPlayerController::FinishCollapse,
		FMath::Max(CollapseResultDelay, KINDA_SMALL_NUMBER), false);
}

void ACosPlayerController::UpdateCollapseCamera()
{
	UCameraComponent* Camera = CollapseCamera.Get();
	if (!Camera)
	{
		GetWorldTimerManager().ClearTimer(CollapseCameraTimer);
		return;
	}

	// 무게감 있는 쓰러짐: 무릎이 꺾이고(Buckle) → 잠깐 버티다 무게에 끌려 넘어가고(Topple)
	// → 바닥에 부딪혀 살짝 튕긴 뒤 가라앉습니다(Settle).
	constexpr float BuckleRatio = 0.4f;      // 쓰러지는 시간 중 무릎이 꺾이는 구간 비율
	constexpr float BuckleDropRatio = 0.3f;  // 무릎이 꺾이며 내려가는 높이 비율
	constexpr float BuckleTiltRatio = 0.1f;  // 무릎이 꺾이며 기우는 각도 비율
	constexpr float SettleDuration = 0.45f;  // 바닥에 닿은 뒤 가라앉는 시간(초)
	constexpr float BounceRatio = 0.05f;     // 바닥에서 튕겨 오르는 높이 비율
	constexpr float ImpactNodPitch = 5.0f;   // 부딪힐 때 고개가 툭 숙여지는 각도

	const float FallDuration = FMath::Max(CollapseFallDuration, 0.01f);
	const float BuckleTime = FallDuration * BuckleRatio;
	const float Elapsed = GetWorld()->GetTimeSeconds() - CollapseStartTime;

	float DropAlpha = 1.0f;
	float TiltAlpha = 1.0f;
	float ImpactOffset = 0.0f; // 0 → 튕김 최고점 → 0

	if (Elapsed < BuckleTime)
	{
		// 힘이 빠지듯 확 내려앉았다가 느려지며 버팁니다.
		const float T = Elapsed / BuckleTime;
		DropAlpha = BuckleDropRatio * FMath::InterpEaseOut(0.0f, 1.0f, T, 3.0f);
		TiltAlpha = BuckleTiltRatio * FMath::InterpEaseInOut(0.0f, 1.0f, T, 2.0f);
	}
	else if (Elapsed < FallDuration)
	{
		// 천천히 기울기 시작해 중력처럼 가속하며 바닥으로 떨어집니다.
		const float T = (Elapsed - BuckleTime) / (FallDuration - BuckleTime);
		DropAlpha = FMath::Lerp(BuckleDropRatio, 1.0f, FMath::InterpEaseIn(0.0f, 1.0f, T, 2.5f));
		TiltAlpha = FMath::Lerp(BuckleTiltRatio, 1.0f, FMath::InterpEaseIn(0.0f, 1.0f, T, 2.0f));
	}
	else
	{
		if (!bCollapseImpactPlayed)
		{
			bCollapseImpactPlayed = true;
			if (CollapseSound)
			{
				UGameplayStatics::PlaySound2D(this, CollapseSound); // 털썩
			}
		}

		// 짧게 튕겼다가 감쇠하며 가라앉습니다. 떨리는 흔들림 대신 한 번의 묵직한 반동만 줍니다.
		const float S = FMath::Clamp((Elapsed - FallDuration) / SettleDuration, 0.0f, 1.0f);
		ImpactOffset = FMath::Sin(PI * S) * (1.0f - S);
		if (S >= 1.0f)
		{
			GetWorldTimerManager().ClearTimer(CollapseCameraTimer);
		}
	}

	const float Drop = CollapseCameraDrop * (DropAlpha - BounceRatio * ImpactOffset);
	Camera->SetRelativeLocation(CollapseCameraStartLocation - FVector(0.0f, 0.0f, Drop));
	Camera->SetRelativeRotation(FRotator(
		FMath::Lerp(CollapseStartPitch, CollapseEndPitch, TiltAlpha) - ImpactNodPitch * ImpactOffset,
		0.0f,
		CollapseRoll * TiltAlpha));
}

void ACosPlayerController::StartCollapseFade()
{
	if (PlayerCameraManager)
	{
		// 끝난 뒤에도 검은 화면을 유지해 결과창 뒤가 비치지 않게 합니다.
		PlayerCameraManager->StartCameraFade(0.0f, 1.0f, CollapseFadeDuration, FLinearColor::Black, false, true);
	}

	// 의식이 멀어지듯 심박도 화면과 함께 잦아듭니다.
	if (UAudioComponent* Heartbeat = HeartbeatAudio.Get())
	{
		Heartbeat->FadeOut(FMath::Max(CollapseFadeDuration, 0.01f), 0.0f);
	}
}

void ACosPlayerController::FinishCollapse()
{
	bIsCollapsing = false;
	ShowResult(PendingCollapseResult);
}

void ACosPlayerController::RecoverFromCollapse()
{
	if (!bIsCollapsed)
	{
		return;
	}
	bIsCollapsed = false;
	bIsCollapsing = false;

	GetWorldTimerManager().ClearTimer(CollapseFallTimer);
	GetWorldTimerManager().ClearTimer(HeartbeatFlashTimer);
	HeartbeatFlashesLeft = 0;
	GetWorldTimerManager().ClearTimer(CollapseCameraTimer);
	GetWorldTimerManager().ClearTimer(CollapseFadeTimer);
	GetWorldTimerManager().ClearTimer(CollapseResultTimer);

	if (UAudioComponent* Heartbeat = HeartbeatAudio.Get())
	{
		Heartbeat->Stop();
	}
	HeartbeatAudio.Reset();

	if (UCameraComponent* Camera = CollapseCamera.Get())
	{
		Camera->SetRelativeLocation(CollapseCameraStartLocation);
		Camera->SetRelativeRotation(FRotator::ZeroRotator);
		Camera->bUsePawnControlRotation = true;
	}
	CollapseCamera.Reset();

	FRotator Upright = GetControlRotation();
	Upright.Pitch = 0.0f;
	Upright.Roll = 0.0f;
	SetControlRotation(Upright);

	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->EnableInput(this);
	}

	if (PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(1.0f, 0.0f, WakeUpFadeDuration, FLinearColor::Black, false, false);
	}
}

void ACosPlayerController::ShowGameOver(int32 Score)
{
	CloseCombatHUD();

	// 타이틀 위젯이 아직 안 떠있으면 새로 생성
	if (IsValid(TitleWidgetClass) && !IsValid(TitleWidgetInstance))
	{
		TitleWidgetInstance = CreateWidget<UUserWidget>(this, TitleWidgetClass);

		if (IsValid(TitleWidgetInstance.Get()))
		{
			TitleWidgetInstance->AddToViewport();
		}
	}

	if (IsValid(TitleWidgetInstance.Get()))
	{
		// UI 입력 모드로 전환
		bShowMouseCursor = true;
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TitleWidgetInstance->TakeWidget());
		SetInputMode(InputMode);

		// 게임오버 그룹 보이기
		if (UWidget* GameOverGroup = TitleWidgetInstance->GetWidgetFromName(TEXT("GameOverGroup")))
		{
			GameOverGroup->SetVisibility(ESlateVisibility::Visible);
		}

		// 시작 그룹 숨기기
		if (UWidget* StartGroup = TitleWidgetInstance->GetWidgetFromName(TEXT("StartGroup")))
		{
			StartGroup->SetVisibility(ESlateVisibility::Collapsed);
		}

		// 점수 표시
		if (UTextBlock* ScoreText = Cast<UTextBlock>(TitleWidgetInstance->GetWidgetFromName(TEXT("ScoreText"))))
		{
			ScoreText->SetText(FText::FromString(FString::Printf(TEXT("		Soul: %d"), Score)));
		}
	}

}

void ACosPlayerController::HideGameOver()
{
	if (IsValid(TitleWidgetInstance.Get()))
	{
		TitleWidgetInstance->RemoveFromParent();
		TitleWidgetInstance = nullptr;
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}

void ACosPlayerController::ToggleESCMenu()
{
	// 결과창 또는 게임오버 화면이 떠 있을 경우 ESC 창을 열지 않음
	if (IsValid(ResultWidgetInstance.Get()) || IsValid(TitleWidgetInstance.Get()))
	{
		return;
	}

	if (IsValid(ESCWidgetInstance.Get()))
	{
		// 이미 열려 있으면 닫음
		HideESCMenu();
		return;
	}

	// Class가 유효한지 확인 후 새로 생성
	if (!IsValid(ESCWidgetClass))
	{
		return;
	}

	ESCWidgetInstance = CreateWidget<UUserWidget>(this, ESCWidgetClass);

	if (IsValid(ESCWidgetInstance.Get()))
	{
		ESCWidgetInstance->AddToViewport();

		// Refresh the snapshot on every open; gameplay values stop changing while paused.
		auto SetPauseText = [this](FName Name, const FText& Text)
		{
			if (UTextBlock* Label = Cast<UTextBlock>(ESCWidgetInstance->GetWidgetFromName(Name)))
			{
				Label->SetText(Text);
			}
		};
		if (const ACosGameState* GS = GetWorld()->GetGameState<ACosGameState>())
		{
			SetPauseText(TEXT("PauseWaveText"), FText::Format(
				NSLOCTEXT("PauseMenu", "Wave", "Wave: {0}"), FText::AsNumber(GS->GetWaveIndex())));
			SetPauseText(TEXT("PauseKillText"), FText::Format(
				NSLOCTEXT("PauseMenu", "Kills", "Kills: {0}"), FText::AsNumber(GS->GetKillCount())));
			const int32 Seconds = FMath::Max(0, FMath::FloorToInt(GS->GetStageRemainingTime()));
			SetPauseText(TEXT("PauseTimeText"), FText::Format(
				NSLOCTEXT("PauseMenu", "Time", "Time: {0}"),
				FText::FromString(FString::Printf(TEXT("%02d:%02d"), Seconds / 60, Seconds % 60))));
		}
		// The HUD retains the existing enemy-count event binding, even when hidden.
		if (IsValid(CombatHUDInstance.Get()))
		{
			if (const UTextBlock* EnemyCount = Cast<UTextBlock>(CombatHUDInstance->GetWidgetFromName(TEXT("EnemyCountText"))))
			{
				SetPauseText(TEXT("PauseEnemyText"), EnemyCount->GetText());
			}
		}

		// UI 입력 모드로 전환
		SetUIInputMode(true);

		// 게임 일시정지
		SetPause(true);
	}
}

void ACosPlayerController::HideESCMenu()
{
	if (IsValid(ESCWidgetInstance.Get()))
	{
		ESCWidgetInstance->RemoveFromParent();
		ESCWidgetInstance = nullptr;
	}

	// 게임 입력 모드
	SetUIInputMode(false);

	// 일시정지 해제
	SetPause(false);
}

void ACosPlayerController::SetUIInputMode(bool bUIMode)
{

	if (bUIMode)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;

		UE_LOG(LogTemp, Warning, TEXT("설정 직후 bShowMouseCursor = %s"), bShowMouseCursor ? TEXT("true") : TEXT("false"));
	}
	else
	{
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = false;
	}
}

void ACosPlayerController::ShowGameHUD()
{
	if (bGameplayTravelRequested)
	{
		return;
	}

	// 시작을 누르면 오프닝 영상을 먼저 보여주고, 영상이 끝나면 이 함수가 다시 불려 게임을 시작합니다.
	if (TryPlayIntro())
	{
		return;
	}

	if (UGameplayStatics::GetCurrentLevelName(this, true) == TEXT("L_MenuLevel"))
	{
		bGameplayTravelRequested = true;
		if (UCosGameInstance* GI = GetGameInstance<UCosGameInstance>())
		{
			GI->bPendingGameplayReveal = true;
		}
		OnStopBGMRequested();
		UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/Cosmos/Maps/L_BlockoutAstra")));
		return;
	}

	// 타이틀에서 걸어둔 일시정지 해제
	SetPause(false);

	// 타이틀 닫고 전투 HUD 표시
	HideTitleWidget();
	ShowCombatHUD();

	// 타이틀 BGM을 끕니다. Day 1은 BGM 없이 진행하고, 그 외 스테이지는 전투 BGM으로 바꿉니다.
	OnStopBGMRequested();
	if (ACosGameMode* GM = GetWorld()->GetAuthGameMode<ACosGameMode>())
	{
		if (GM->IsBossStage())
		{
			OnBossBGMRequested();
		}
		else if (!GM->IsInDay1())
		{
			OnCombatBGMRequested();
		}
	}

	// 캐릭터 관련 UI 연결
	SetupCharacterBindings();

	// GameState 관련 UI 연결
	if (ACosGameState* GS = GetWorld()->GetGameState<ACosGameState>())
	{
		UpdateKillUI(GS->GetKillCount());
		UpdateStageTimeUI(GS->GetStageRemainingTime());

		GS->OnStateChanged.RemoveDynamic(
			this, &ACosPlayerController::HandleGameStateChanged);
		GS->OnStateChanged.AddDynamic(
			this, &ACosPlayerController::HandleGameStateChanged);

		GS->OnStageTimeChanged.RemoveDynamic(
			this, &ACosPlayerController::HandleStageTimeChanged);
		GS->OnStageTimeChanged.AddDynamic(
			this, &ACosPlayerController::HandleStageTimeChanged);

		UpdateWaveUI(GS->GetWaveIndex());
	}
}

void ACosPlayerController::FinishGameplayEntry()
{
	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);
	ShowGameHUD();
	if (PlayerCameraManager)
	{
		PlayerCameraManager->StartCameraFade(1.0f, 0.0f, GameplayFadeInSeconds,
			FLinearColor::Black, false, false);
	}
}

bool ACosPlayerController::TryPlayIntro()
{
	if (IsValid(IntroVideoWidgetInstance.Get()))
	{
		return true; // 재생 중에 또 불리면 무시
	}
	if (bIntroChecked)
	{
		return false;
	}
	bIntroChecked = true;

	UCosGameInstance* GI = GetGameInstance<UCosGameInstance>();
	if (bPlayIntroOnlyOnce && GI && GI->bHasPlayedIntro)
	{
		return false;
	}

	UMediaSource* Source = ResolveIntroMediaSource();
	if (!Source || !IsValid(IntroVideoWidgetClass))
	{
		return false;
	}

	IntroVideoWidgetInstance = CreateWidget<UCosIntroVideoWidget>(this, IntroVideoWidgetClass);
	if (!IsValid(IntroVideoWidgetInstance.Get()))
	{
		return false;
	}

	// 타이틀을 닫고 BGM을 끈 뒤 영상을 띄웁니다. 영상이 끝날 때까지 타이틀의 일시정지는 유지합니다.
	HideTitleWidget();
	OnStopBGMRequested();

	IntroVideoWidgetInstance->AddToViewport(200);
	if (!IntroVideoWidgetInstance->PlayIntro(Source))
	{
		IntroVideoWidgetInstance->RemoveFromParent();
		IntroVideoWidgetInstance = nullptr;
		return false;
	}
	IntroVideoWidgetInstance->OnFinished.AddUObject(this, &ACosPlayerController::HandleIntroFinished);

	if (GI)
	{
		GI->bHasPlayedIntro = true;
	}

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(IntroVideoWidgetInstance->TakeWidget());
	SetInputMode(InputMode);
	bShowMouseCursor = false;
	return true;
}

UMediaSource* ACosPlayerController::ResolveIntroMediaSource()
{
	if (IsValid(IntroMediaSource.Get()))
	{
		return IntroMediaSource.Get();
	}

	if (IntroMoviePath.IsEmpty())
	{
		return nullptr;
	}

	// Content/Movies 폴더의 파일은 패키징할 때 그대로 함께 복사됩니다.
	const FString FullPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / IntroMoviePath);
	if (!FPaths::FileExists(FullPath))
	{
		UE_LOG(LogTemp, Log, TEXT("[Intro] 인트로 영상 파일이 없어 바로 시작합니다: %s"), *FullPath);
		return nullptr;
	}

	UFileMediaSource* FileSource = NewObject<UFileMediaSource>(this);
	FileSource->SetFilePath(FullPath);
	return FileSource;
}

void ACosPlayerController::HandleIntroFinished()
{
	if (IsValid(IntroVideoWidgetInstance.Get()))
	{
		IntroVideoWidgetInstance->OnFinished.RemoveAll(this);
		IntroVideoWidgetInstance->RemoveFromParent();
		IntroVideoWidgetInstance = nullptr;
	}

	ShowGameHUD();
}

void ACosPlayerController::UpdatePotionUI(int32 CurrentPotion)
{
	if (IsValid(CombatHUDInstance.Get()))
	{
		FString Cmd = FString::Printf(TEXT("SetPotionText %d"), CurrentPotion);
		CombatHUDInstance->CallFunctionByNameWithArguments(*Cmd, *GLog, nullptr, true);
	}
}

void ACosPlayerController::HandleKillConfirmed(AActor* Victim)
{
	if (IsValid(CombatHUDInstance.Get()))
	{
		CombatHUDInstance->CallFunctionByNameWithArguments(TEXT("PlayKillMarker"), *GLog, nullptr, true);
	}
}

void ACosPlayerController::HandleHitConfirmed(AActor* Target)
{
	if (IsValid(CombatHUDInstance.Get()))
	{
		CombatHUDInstance->CallFunctionByNameWithArguments(TEXT("PlayHitMarker"), *GLog, nullptr, true);
	}
}

void ACosPlayerController::OnBossBGMRequested_Implementation()
{
	// BP가 보스 BGM(BGM_Combat_Boss)을 연결하기 전까지는 일반 전투 BGM으로 대신합니다.
	OnCombatBGMRequested();
}

void ACosPlayerController::HandleGameOverRequested()
{
	int32 CurrentSoul = 0;

	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		CurrentSoul = GI->GetSoul();
	}

	ShowGameOver(CurrentSoul);
}

void ACosPlayerController::HandlePlayerDamaged(float Damage)
{
	if (IsValid(CosLegacyCameraShakeClass))
	{
		ClientStartCameraShake(CosLegacyCameraShakeClass);
	}
}
