// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTS_FlyingEnemyCheck.h"
#include "Enemy/FlyingBase.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "Kismet/GameplayStatics.h"

UBTS_FlyingEnemyCheck::UBTS_FlyingEnemyCheck()
{
	NodeName = TEXT("Flying Enemyt Service");
	//work every sec
	Interval = 0.3f;
	//make frame randomly
	RandomDeviation = 0.05f;
	bNotifyTick = true;
	//when start, update interval once
	bCallTickOnSearchStart = true;
	//Show DropDown
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTS_FlyingEnemyCheck, TargetActorKey), AActor::StaticClass());
	InRangeKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTS_FlyingEnemyCheck, InRangeKey));
	CanAttackKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTS_FlyingEnemyCheck, CanAttackKey));
	CloseToPlayerKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTS_FlyingEnemyCheck,  CloseToPlayerKey));
}
void UBTS_FlyingEnemyCheck::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (UBlackboardData* BB = GetBlackboardAsset())
	{
		TargetActorKey.ResolveSelectedKey(*BB);
		InRangeKey.ResolveSelectedKey(*BB);
		CanAttackKey.ResolveSelectedKey(*BB);
		CloseToPlayerKey.ResolveSelectedKey(*BB);
	}
}
void UBTS_FlyingEnemyCheck::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) 
{
		Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
		UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
		AAIController* AI = OwnerComp.GetAIOwner();
		if (!BB || !AI)
		{
			return;
		}
		const AFlyingBase* Flying = Cast<AFlyingBase>(AI->GetPawn());
		if (!Flying)
		{
			return;
		}
		APawn* Target = UGameplayStatics::GetPlayerPawn(OwnerComp.GetWorld(), 0);

		BB->SetValueAsObject(TargetActorKey.SelectedKeyName, Target);
		BB->SetValueAsBool(CanAttackKey.SelectedKeyName, Flying->CanAttack());
		//if player dead, stop attack
		if (!Target)
		{
			BB->SetValueAsBool(InRangeKey.SelectedKeyName, false);
			BB->SetValueAsBool(CloseToPlayerKey.SelectedKeyName, false);
			return;
		}
		const float DistSq = FVector::DistSquaredXY(Flying->GetActorLocation(), Target->GetActorLocation());
		const float AttackRange = Flying->GetAttackRange() * RangeTolerance;	
		BB->SetValueAsBool(InRangeKey.SelectedKeyName, DistSq <= FMath::Square(AttackRange));

		const float Long = Flying->GetDistanceCheck();BB->SetValueAsBool(CloseToPlayerKey.SelectedKeyName, DistSq < FMath::Square(Long));
}
