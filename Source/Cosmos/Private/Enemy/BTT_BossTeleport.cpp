// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTT_BossTeleport.h"
#include "Enemy/Boss.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTT_BossTeleport::UBTT_BossTeleport()
{
	NodeName = TEXT("Boss Teleport");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTT_BossTeleport::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	ABoss* Boss = AICon ? Cast<ABoss>(AICon->GetPawn()) : nullptr;

	if (!Boss || !Boss->IsAlive())
	{
		return EBTNodeResult::Failed;
	}

	Boss->BeginTeleport();
	return EBTNodeResult::InProgress;
}


void UBTT_BossTeleport::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	ABoss* Boss = AICon ? Cast<ABoss>(AICon->GetPawn()) : nullptr;
	if (!Boss || !Boss->IsAlive())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	if (!Boss->IsTeleporting())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}