// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTT_PickFlyingMode.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EPickFlyMode : uint8
{
	Retreat   UMETA(DisplayName = "Retreat"),
	Roam      UMETA(DisplayName = "Roam")
};


UCLASS()
class COSMOS_API UBTT_PickFlyingMode : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_PickFlyingMode();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

protected:

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector OutputKey;
	UPROPERTY(EditAnywhere, Category = "Pick")
	EPickFlyMode Mode = EPickFlyMode::Retreat;

	UPROPERTY(EditAnywhere, Category = "Pick|Retreat")
	float RetreatAngleJitter = 25.f;

	UPROPERTY(EditAnywhere, Category = "Pick|Roam")
	float RoamMaxDistanceRatio = 1.4f;

	UPROPERTY(EditAnywhere, Category = "Pick")
	float VerticalJitter = 60.f;
	
};
