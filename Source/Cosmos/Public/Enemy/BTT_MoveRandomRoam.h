// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTT_MoveRandomRoam.generated.h"

/**
 * 
 */
UCLASS()
class COSMOS_API UBTT_MoveRandomRoam : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTT_MoveRandomRoam();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector CenterActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector RoamLocationKey;

	
	UPROPERTY(EditAnywhere, Category = "Roam", meta = (ClampMin = "0.0", ClampMax = "0.95"))
	float MinRadiusRatio = 0.4f;

	
	UPROPERTY(EditAnywhere, Category = "Roam", meta = (ClampMin = "0.0"))
	float VerticalJitter = 80.f;
};
