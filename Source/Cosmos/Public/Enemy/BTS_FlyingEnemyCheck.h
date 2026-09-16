// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTS_FlyingEnemyCheck.generated.h"

/**
 * 
 */
UCLASS()
class COSMOS_API UBTS_FlyingEnemyCheck : public UBTService
{
	GENERATED_BODY()

public:
	UBTS_FlyingEnemyCheck();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector InRangeKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector CanAttackKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector CloseToPlayerKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard|Setting")
	float RangeTolerance = 1.05f;
};

