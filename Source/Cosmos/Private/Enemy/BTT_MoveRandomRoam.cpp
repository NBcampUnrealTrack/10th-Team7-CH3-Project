// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTT_MoveRandomRoam.h"
#include "Enemy/FlyingBase.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTT_MoveRandomRoam::UBTT_MoveRandomRoam() 
{
	NodeName = TEXT("Move Roam Location");

	//just one calc -> no need tick
	bNotifyTick = false;

	CenterActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTT_MoveRandomRoam, CenterActorKey),
	AActor::StaticClass());
	RoamLocationKey.AddVectorFilter(this,GET_MEMBER_NAME_CHECKED(UBTT_MoveRandomRoam, RoamLocationKey));
}
void UBTT_MoveRandomRoam::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (UBlackboardData* BB = GetBlackboardAsset())
	{
		CenterActorKey.ResolveSelectedKey(*BB);
		RoamLocationKey.ResolveSelectedKey(*BB);
	}
}
EBTNodeResult::Type UBTT_MoveRandomRoam::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!BB || !AIController)
	{
		return EBTNodeResult::Failed;
	}
	const AFlyingBase* Flying = Cast<AFlyingBase>(AIController->GetPawn());
	if (!Flying)
	{
		return EBTNodeResult::Failed;
	}
	const AActor* Center = Cast<AActor>(BB->GetValueAsObject(CenterActorKey.SelectedKeyName));
	if (Center)
	{
		return EBTNodeResult::Failed;
	}
	const FVector CenterLocation = Center->GetActorLocation();
	const float MaxR = Flying->GetRoamRadius();
	const float MinR = MaxR * MinRadiusRatio;
	const float Distance = FMath::FRandRange(MinR, MaxR);
	const float Angle = FMath::FRandRange(0.0f, 360.0f);
	const FVector Offset = FRotator(0.0f, Angle, 0.0f).Vector() * Distance;
	const float Height = Flying->GetFlyHeight() + FMath::FRandRange(-VerticalJitter, VerticalJitter);

	FVector RoamGoal = CenterLocation  + Offset;
	RoamGoal.Z = CenterLocation.Z + Height;
	BB->SetValueAsVector(RoamLocationKey.SelectedKeyName, RoamGoal);
	return EBTNodeResult::Succeeded;
}
