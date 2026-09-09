#include "UI/CosPlayerController.h"
#include "Character/CosCharacter.h"
#include "Character/HealthComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"



ACosPlayerController::ACosPlayerController()
{
	Super::BeginPlay();


};
	
void ACosPlayerController::UpdateHP(float CurrentHealth, float MaxHealth)
{
}

void ACosPlayerController::BeginPlay() 
{
	Super::BeginPlay();
}

void ACosPlayerController::ShowTitleWidget()
{

}

void ACosPlayerController::HideTitleWidget()
{

}

void ACosPlayerController::ShowCombatHUD()
{

}

void ACosPlayerController::CloseCombatHUD()
{

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