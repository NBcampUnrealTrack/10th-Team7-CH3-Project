#include "UI/CosPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"




ACosPlayerController::ACosPlayerController()
{
	bShowMouseCursor = false;
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

void ACosPlayerController::HideCombatHUD()
{

}

void ACosPlayerController::ShowForgeWidget()
{

}

void ACosPlayerController::HideForgeWidget()
{

}

void ACosPlayerController::ShowResultWidget(bool bWin, int32 Score)
{

}

void ACosPlayerController::HideResultWidget()
{

}

void ACosPlayerController::ToggleESCMenu()
{

}

void ACosPlayerController::HideESCMenu()
{

}