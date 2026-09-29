// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTT_FlyingTo.h"
#include "Enemy/FlyingBase.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTT_FlyingTo::UBTT_FlyingTo()
{
	NodeName = TEXT("Fly Movement");
	//need this for move every tick;
	bNotifyTick = true;
	//expose object and vector on editor dropdown
	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTT_FlyingTo, TargetKey), AActor::StaticClass());
	TargetKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTT_FlyingTo, TargetKey));
}
uint16 UBTT_FlyingTo::GetInstanceMemorySize() const
{
	return sizeof(FBTFlyMemory);
}
void UBTT_FlyingTo::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (UBlackboardData* BB = GetBlackboardAsset())
	{
		TargetKey.ResolveSelectedKey(*BB);
	}
}
EBTNodeResult::Type UBTT_FlyingTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}
	AFlyingBase* Flying = Cast<AFlyingBase>(AIController->GetPawn());
	if (!Flying || !Flying->IsAlive())
	{
		return EBTNodeResult::Failed;
	}
	FBTFlyMemory* Memory = reinterpret_cast<FBTFlyMemory*>(NodeMemory);
	Memory->ElapsedTime = 0.f;
	FVector Goal;
	if (!CalcGoal(OwnerComp, NodeMemory, Flying, Goal))
	{
		return EBTNodeResult::Failed;
	}
	const float Arrived = FMath::Square(Flying->GetArriveRadius());
	if (FVector::DistSquared(Flying->GetActorLocation(), Goal) <= Arrived)
	{
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::InProgress;
}
bool UBTT_FlyingTo::CalcGoal(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, const AFlyingBase* Flyer, FVector& OutGoal) const
{
	const UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB || !Flyer)
	{
		return false;
	}
	if (TargetKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
	{
		const AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));
		if (!TargetActor)
		{
			return false;
		}
		OutGoal = TargetActor->GetActorLocation();
		if (bAddFlyOffset)
		{
			OutGoal.Z += Flyer->GetFlyHeight();
		}
		return true;
	}

	if (TargetKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		OutGoal = BB->GetValueAsVector(TargetKey.SelectedKeyName);
		return BB->IsVectorValueSet(TargetKey.SelectedKeyName);
	}
	return false;
}
void UBTT_FlyingTo::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) 
{
	FBTFlyMemory* Memory = reinterpret_cast<FBTFlyMemory*>(NodeMemory);
	
		Memory->ElapsedTime += DeltaSeconds;
		if (Memory->ElapsedTime > MaxDuration)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}
		AAIController* AIController = OwnerComp.GetAIOwner();
		AFlyingBase* Flying = AIController ? Cast<AFlyingBase>(AIController->GetPawn()) : nullptr;

		if (!Flying || !Flying->IsAlive())
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}
		FVector Goal;
		if (!CalcGoal(OwnerComp, NodeMemory, Flying, Goal)) 
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}
		const FVector Current = Flying->GetActorLocation();
		const FVector ToGoal = Goal - Current;
		const float AcceptSq = FMath::Square(Flying->GetArriveRadius());
		if (ToGoal.SizeSquared() <= AcceptSq)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}
		Flying->AddMovementInput(ToGoal.GetSafeNormal(), 1.f);
}
