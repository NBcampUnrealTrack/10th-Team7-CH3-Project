// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/EnemyAIController.h"
#include "Enemy/EnemyBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h" 

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = false;
	// Change Rotate when it's moving. -> use in Pawn, not here
	bSetControlRotationFromPawnOrientation = false;
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackBoard"));
	//AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AI Perception"));
	//SetPerceptionComponent(*AIPerception);
	//
	//SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	//SightConfig->SightRadius = 1500.0f;
	//SightConfig->LoseSightRadius = 2000.0f;
	//SightConfig->PeripheralVisionAngleDegrees = 90.0f;
	//SightConfig->SetMaxAge(5.0f);
	//
	//SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	//SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	//SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	//
	//AIPerception->ConfigureSense(*SightConfig);
	//AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	//Spawn on field -> Run to Player instantly
	Super::OnPossess(InPawn);
	//dont cast eveery time, just one time on here
	// CachedEnemy = Cast<AEnemyBase>(InPawn);
	AEnemyBase* Enemy = Cast<AEnemyBase>(InPawn);
	if (!Enemy)
	{
		return;
	}

	if (BehaviorTreeAsset && BlackboardComp)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}
	else
	{
		return;
	}
}

void AEnemyAIController::OnUnPossess()
{
	if (BrainComponent)
	{
		BrainComponent->StopLogic(TEXT("UnPossess"));
	}
	Super::OnUnPossess();
}

