#include "UI/CosPlayerController.h"
#include "Character/CosCharacter.h"
#include "Character/HealthComponent.h"
#include "Weapon/ShotgunWeapon.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

// 테스트용 임시 체력 변수
static float TestCurrentHealth = 100.f;
static float TestMaxHealth = 100.f;
static int32 TestCurrentAmmo = 4;
static int32 TestMaxAmmo = 4;


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

	
	// Enhanced Input Context 등록
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			// IsValid로 체크
			if (IsValid(InputMappingContext.Get()))
			{
				Subsystem->AddMappingContext(InputMappingContext.Get(), 0);
			}
		}
	}

	// 1. 기본 전투 HUD
	ShowCombatHUD();
	// 2. 델리게이트 바인딩
	SetupCharacterBindings();
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

			// 초기 체력값 UI 즉시 반영 함수 여야되는데 일단 Getter 받기 전까지 임시 함수
			UpdateHP(TestCurrentHealth, TestMaxHealth);
		}
	}

	/*
	// 2. ShotgunWeapon 델리게이트 바인딩
	if (ACosCharacter* CosChar = Cast<ACosCharacter>(ControlledPawn))
	{
		if (AShotgunWeapon* Weapon = CosChar->GetEquippedWeapon())
		{
			if (IsValid(Weapon))
			{
				Weapon->OnAmmoChanged.RemoveDynamic(this, &ACosPlayerController::UpdateAmmoUI);
				Weapon->OnAmmoChanged.AddDynamic(this, &ACosPlayerController::UpdateAmmoUI);

				// 초기 탄약수 UI 즉시 반영
				UpdateAmmoUI(Weapon->GetCurrentAmmo(), Weapon->GetMaxAmmo());
			}
		}
	}
	*/
}

void ACosPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
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
	if (IsValid(CombatHUDInstance.Get()))
	{
		FString Cmd = FString::Printf(TEXT("SetAmmoText %d %d"), CurrentAmmo, MaxAmmo);
		CombatHUDInstance->CallFunctionByNameWithArguments(*Cmd, *GLog, nullptr, true);
	}
}

void ACosPlayerController::OnCharacterDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("플레이어 캐릭터 사망"));

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

}

void ACosPlayerController::HideTitleWidget()
{

}


// 테스트용
void ACosPlayerController::TestDecreaseHP() { UpdateHP(70.0f, 100.0f); }

void ACosPlayerController::TestHealHP() { UpdateHP(100.0f, 100.0f); }

void ACosPlayerController::TestDecreaseAmmo() { UpdateAmmoUI(7, 30); }

void ACosPlayerController::TestReloadAmmo() { UpdateAmmoUI(30, 30); }




void ACosPlayerController::OpenForgeWidget()
{

}

void ACosPlayerController::CloseForgeWidget()
{

}

void ACosPlayerController::ShowResult(bool bWin, int32 Score)
{

}

void ACosPlayerController::HideResult()
{

}

void ACosPlayerController::ToggleESCMenu()
{

}

void ACosPlayerController::HideESCMenu()
{

}

void ACosPlayerController::SetUIInputMode(bool bUIMode)
{

}

void ACosPlayerController::ShowGameHUD() {
}