// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTT_FlyingTo.generated.h"

class AFlyingBase;
/**
 * 
 */
struct FBTFlyMemory
{
	FVector CachedGoal = FVector::ZeroVector;
	float ElapsedTime = 0.0f;
};
UCLASS()
class COSMOS_API UBTT_FlyingTo : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTT_FlyingTo();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory, float DeltaSeconds) override;
	
	virtual uint16 GetInstanceMemorySize() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;
	UPROPERTY(EditAnywhere, Category = "Fly")
	bool bAddFlyOffset = true;
	UPROPERTY(EditAnywhere, Category = "Fly")
	float MaxDuration = 0.0f;

private:
	bool CalcGoal(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,const AFlyingBase* Flyer, FVector& OutGoal) const;
};
