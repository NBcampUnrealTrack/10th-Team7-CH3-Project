#include "UI/CosPlayerController.h"
#include "Character/CosCharacter.h"
#include "Character/CosGameMode.h"
#include "Character/CosGameState.h"
#include "Character/HealthComponent.h"
#include "Weapon/ShotgunWeapon.h"
#include "Weapon/CombatComponent.h"   
#include "Data/CosGameInstance.h" 
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h" 
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "GameFramework/CharacterMovementComponent.h" 

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

	// 현재 레벨 이름에 따라 분기b
	const FString CurrentLevelName = GetWorld()->GetMapName();

	if (CurrentLevelName.Contains(TEXT("MenuLevel")))
	{
		// 메뉴 레벨이면 타이틀 화면만 표시
		ShowTitleWidget();
	}
	else
	{
		// 그 외(전투) 레벨이면 전투 HUD + 델리게이트 바인딩
		ShowCombatHUD();
		SetupCharacterBindings();

		if (ACosGameState* GS = GetWorld()->GetGameState<ACosGameState>())
		{
			GS->OnStateChanged.RemoveDynamic(this, &ACosPlayerController::HandleGameStateChanged);
			GS->OnStateChanged.AddDynamic(this, &ACosPlayerController::HandleGameStateChanged);

			// 처음 HUD가 켜졌을 때 초기 웨이브 UI 즉시 갱신
			UpdateWaveUI(GS->GetWaveIndex());
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("현재 레벨: %s"), *CurrentLevelName);

	if (ACosGameMode* GM = Cast<ACosGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->OnForgeRequested.AddDynamic(this, &ACosPlayerController::HandleForgeRequested);
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

			// 초기 체력값 UI 즉시 반영 함수
			UpdateHP(HealthComp->GetCurrentHealth(), HealthComp->GetMaxHealth());
		}
	}

	
	// 2. ShotgunWeapon 델리게이트 바인딩
	if (UCombatComponent* CombatComp = ControlledPawn->FindComponentByClass<UCombatComponent>())
	{
		if (AShotgunWeapon* Weapon = CombatComp->GetShotgunWeapon())
		{
			UE_LOG(LogTemp, Warning, TEXT("무기 바인딩 성공"));

			Weapon->OnAmmoChanged.RemoveDynamic(this, &ACosPlayerController::UpdateAmmoUI);
			Weapon->OnAmmoChanged.AddDynamic(this, &ACosPlayerController::UpdateAmmoUI);
			CachedMaxAmmo = Weapon->GetCurrentMaxAmmo();
			UpdateAmmoUI(Weapon->GetCurrentAmmo());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ShotgunWeapon이 아직 CombatComponent에 없음 - 0.1초 후 재시도"));
			GetWorld()->GetTimerManager().SetTimer(
				WeaponBindRetryTimer,
				this,
				&ACosPlayerController::SetupCharacterBindings,
				0.1f,
				false
			);
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


void ACosPlayerController::UpdateAmmoUI(int32 CurrentAmmo)
{
	UE_LOG(LogTemp, Warning, TEXT("UpdateAmmoUI 호출됨: %d"), CurrentAmmo);

	if (IsValid(CombatHUDInstance.Get()))
	{
		FString Cmd = FString::Printf(TEXT("SetAmmoText %d %d"), CurrentAmmo, CachedMaxAmmo);
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
		ShowGameOver(0);

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

			// 게임 입력모드 전환
			SetInputMode(FInputModeGameOnly());
			bShowMouseCursor = false;


		}
	}
}


void ACosPlayerController::CloseCombatHUD()
{
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
	// Class 유효성 검사, Instance가 이미 생성되었는지 확인
	if (IsValid(TitleWidgetClass) && !IsValid(TitleWidgetInstance))
	{
		//일시 정
		if (SetPause(false))
		{
			SetPause(true);
		}

		TitleWidgetInstance = CreateWidget<UUserWidget>(this, TitleWidgetClass); //위젯 인스턴스 생성(메모리상에 객체만 둠. 아직 화면에는 X)

		if (IsValid(TitleWidgetInstance.Get()))
		{
			TitleWidgetInstance->AddToViewport();

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
	OpenForgeWidget();
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

	SetUIInputMode(false);
	ShowCombatHUD();

	// 다음 웨이브 호출
	if (ACosGameMode* GM = GetWorld()->GetAuthGameMode<ACosGameMode>())
	{
		GM->StartNextWave(true);
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
			TEXT("SetResult %d %d %d %d"),
			ResultData.WaveNumber,
			ResultData.Score,
			Minutes,
			Seconds
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
	HideTitleWidget();
	ShowCombatHUD();
}