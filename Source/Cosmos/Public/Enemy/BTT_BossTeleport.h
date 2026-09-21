// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_BossTeleport.generated.h"

/**
 * 
 */
UCLASS()
class COSMOS_API UBTT_BossTeleport : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTT_BossTeleport();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
