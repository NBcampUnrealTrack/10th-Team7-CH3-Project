// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTT_EnemyAttack.h"
#include "Enemy/EnemyBase.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTT_EnemyAttack::UBTT_EnemyAttack()
{
	NodeName = TEXT("Enemy Attack");

	// Need for get TickTask
	bNotifyTick = true;
}

uint16 UBTT_EnemyAttack::GetInstanceMemorySize() const
{
	return sizeof(FBTPerformAttackMemory);
}

EBTNodeResult::Type UBTT_EnemyAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AEnemyBase* Enemy = Cast<AEnemyBase>(AIController->GetPawn());
	if (!Enemy)
	{
		return EBTNodeResult::Failed;
	}
	//aviod Slide while attack
	AIController->StopMovement();

	const float AttackDuration = Enemy->EnemyAttack();

	FBTPerformAttackMemory* Memory = reinterpret_cast<FBTPerformAttackMemory*>(NodeMemory);
	Memory->RemainingTime = FMath::Max(AttackDuration, MinDuration);

	//End it after TicklTask
	return EBTNodeResult::InProgress;
}

void UBTT_EnemyAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTPerformAttackMemory* Memory = reinterpret_cast<FBTPerformAttackMemory*>(NodeMemory);

	Memory->RemainingTime -= DeltaSeconds;

	if (Memory->RemainingTime <= 0.f)
	{

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}