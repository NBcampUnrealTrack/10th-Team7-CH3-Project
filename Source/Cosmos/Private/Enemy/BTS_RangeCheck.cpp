// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTS_RangeCheck.h"
#include "Enemy/EnemyBase.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "Kismet/GameplayStatics.h"

UBTS_RangeCheck::UBTS_RangeCheck()
{
	NodeName = TEXT("Check Range");
	Interval = 0.5f;
	//for seperate check timing for frame
	RandomDeviation = 0.1f;
	bNotifyTick = true;
	//dont use interval on start
	bCallTickOnSearchStart = true;
	//avoid pick wrong key
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTS_RangeCheck, TargetActorKey), AActor::StaticClass());
	InAttackRangeKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTS_RangeCheck, InAttackRangeKey));
}

void UBTS_RangeCheck::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	//transfer key Nmame->Blackboard ID
	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		TargetActorKey.ResolveSelectedKey(*BBAsset);
		InAttackRangeKey.ResolveSelectedKey(*BBAsset);
	}
}

void UBTS_RangeCheck::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!BB || !AIController)
	{
		return;
	}
	AEnemyBase* Enemy = Cast<AEnemyBase>(AIController->GetPawn());
	if (!Enemy)
	{
		return;
	}
	APawn* Target = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	BB->SetValueAsObject(TargetActorKey.SelectedKeyName, Target);
	bool bInRange = false;
	if (Target)
	{
		const float DistSq = FVector::DistSquared(Enemy->GetActorLocation(), Target->GetActorLocation());
		const float Range = Enemy->GetAttackRange() * ExtraRange;
		bInRange = (DistSq <= FMath::Square(Range));
	}
	BB->SetValueAsBool(InAttackRangeKey.SelectedKeyName, bInRange);
}
