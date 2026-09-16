// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTT_PickFlyingMode.h"
#include "Enemy/FlyingBase.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTT_PickFlyingMode::UBTT_PickFlyingMode() 
{
	NodeName = TEXT("Flying Mode");
	bNotifyTick = false;

	TargetActorKey.AddObjectFilter(this,GET_MEMBER_NAME_CHECKED(UBTT_PickFlyingMode, TargetActorKey), AActor::StaticClass());
	OutputKey.AddVectorFilter(this,GET_MEMBER_NAME_CHECKED(UBTT_PickFlyingMode, OutputKey));
}
void UBTT_PickFlyingMode::InitializeFromAsset(UBehaviorTree& Asset)
{
	if (UBlackboardData* BB = GetBlackboardAsset())
	{
		TargetActorKey.ResolveSelectedKey(*BB);
		OutputKey.ResolveSelectedKey(*BB);
	}
}
EBTNodeResult::Type UBTT_PickFlyingMode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* AI = OwnerComp.GetAIOwner();
	if (!BB || !AI)
	{
		return EBTNodeResult::Failed;
	}
	const AFlyingBase* Fly = Cast<AFlyingBase>(AI->GetPawn());
	if (!Fly)
	{
		return EBTNodeResult::Failed;
	}
	const AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Target)
	{
		return EBTNodeResult::Failed;
	}
	const FVector SelfLocation = Fly->GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();
	FVector Goal;
	if (Mode == EPickFlyMode::Retreat)
	{
		//to the player
		FVector AwayDir = (SelfLocation - TargetLocation).GetSafeNormal2D();

		
		if (AwayDir.IsNearlyZero())
		{
			AwayDir = FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f).Vector();
		}

		if (RetreatAngleJitter > 0.f)
		{
			const float Jitter = FMath::FRandRange(-RetreatAngleJitter, RetreatAngleJitter);
			AwayDir = FRotator(0.f, Jitter, 0.f).RotateVector(AwayDir);
		}
		const float GoalDist = Fly->GetDistanceCheck() * Fly->GetRetreatMulti();

		Goal = TargetLocation + AwayDir * GoalDist;
	}
	else
	{
		const float R = Fly->GetRoamRadius();
		const float Dist = FMath::FRandRange(R * 0.5f, R);
		const float Angle = FMath::FRandRange(0.f, 360.f);

		Goal = SelfLocation + FRotator(0.f, Angle, 0.f).Vector() * Dist;

		const float MaxDist = Fly->GetDistanceCheck() * RoamMaxDistanceRatio;
		const FVector FromTarget = (Goal - TargetLocation).GetSafeNormal2D();
		const float   CurDist = FVector::Dist2D(Goal, TargetLocation);

		if (CurDist > MaxDist && !FromTarget.IsNearlyZero())
		{
			Goal = TargetLocation + FromTarget * MaxDist;
		}
	}

	Goal.Z = TargetLocation.Z + Fly->GetFlyHeight()
		+ FMath::FRandRange(-VerticalJitter, VerticalJitter);

	BB->SetValueAsVector(OutputKey.SelectedKeyName, Goal);

	return EBTNodeResult::Succeeded;
}
