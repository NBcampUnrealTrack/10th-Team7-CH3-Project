#include "UI/CosPlayerController.h"
#include "Character/CosCharacter.h"
#include "Character/HealthComponent.h"
#include "EnhancedInputSubsystems.h"
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
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}

	// 1. 기본 전투 HUD
	ShowCombatHUD();
}


void ACosPlayerController::ShowTitleWidget()
{

}

void ACosPlayerController::HideTitleWidget()
{

}

void ACosPlayerController::ShowCombatHUD()
{
	// 에디터에서 CombatHUDClass(WBP_CombatHUD)를 지정했고 아직 생성된 인스턴스가 없을때
	if (CombatHUDClass && !CombatHUDInstance)
	{
		// 위젯 인스턴스 생성
		CombatHUDInstance = CreateWidget<UUserWidget>(this, CombatHUDClass);

		// 화면에 띄우기
		if (CombatHUDInstance)
		{
			CombatHUDInstance->AddToViewport();
		}
	}
}

void ACosPlayerController::CloseCombatHUD()
{
	if (CombatHUDInstance)
	{
		CombatHUDInstance->RemoveFromParent();
		CombatHUDInstance = nullptr;
	}
}

UUserWidget* ACosPlayerController::GetHUDWidget() const
{
	return CombatHUDInstance;
}


void ACosPlayerController::SetupInputComponent()
{


	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

}

void ACosPlayerController::UpdateHP(float CurrentHealth, float MaxHealth)
{
	if (CombatHUDInstance)
	{
		//  함수/이벤트 호출
		FString Cmd = FString::Printf(TEXT("UpdateHP %f %f"), CurrentHealth, MaxHealth);
		CombatHUDInstance->CallFunctionByNameWithArguments(*Cmd, *GLog, nullptr, true);

	}
}

void ACosPlayerController::UpdateAmmoUI(int32 CurrentAmmo, int32 MaxAmmo)
{
	if (CombatHUDInstance)
	{
		// 블루프린트에 작성한 SetAmmoText 이벤트를 이름으로 호출
		FString Cmd = FString::Printf(TEXT("SetAmmoText %d %d"), CurrentAmmo, MaxAmmo);
		CombatHUDInstance->CallFunctionByNameWithArguments(*Cmd, *GLog, nullptr, true);
	}
}



// 테스트용
void ACosPlayerController::TestDecreaseHP()
{
	UpdateHP(70.0f, 100.0f);
}

void ACosPlayerController::TestHealHP()
{
	UpdateHP(100.0f, 100.0f);
}

void ACosPlayerController::TestDecreaseAmmo()
{
	UpdateAmmoUI(7, 30);
}

void ACosPlayerController::TestReloadAmmo()
{
	UpdateAmmoUI(30, 30);
}




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