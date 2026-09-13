// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTreeTypes.h"  //need for selector
#include "BTS_RangeCheck.generated.h"

/**
 *
 */
UCLASS()
class COSMOS_API UBTS_RangeCheck : public UBTService
{
	GENERATED_BODY()

public:
	UBTS_RangeCheck();

	//need for set value as object/bool, Key name -> blackboard ID
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Blacboard")
	FBlackboardKeySelector TargetActorKey;
	UPROPERTY(EditAnywhere, Category = "Blacboard")
	FBlackboardKeySelector InAttackRangeKey;
	//for extra range
	UPROPERTY(EditAnywhere, Category = "Blacboard")
	float ExtraRange = 1.15f;
};
