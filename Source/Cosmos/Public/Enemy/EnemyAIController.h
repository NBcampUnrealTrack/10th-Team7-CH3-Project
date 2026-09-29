// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
//Dont Use Perception for Normal Monsters
//#include "Perception/AIPerceptionTypes.h"
#include "EnemyAIController.generated.h"

class AEnemyBase;

UCLASS()
class COSMOS_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

protected:

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	//UAIPerceptionComponent* AIPerception;
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	//UAISenseConfig_Sight* SightConfig;
	//UPROPERTY()
	//AActor* CurrentTarget = nullptr;
	//
	//bool bIsChasing = false;
	//FTimerHandle ChaseTimer;
	//FTimerHandle ActionTimerHandle;
	//TWeakObjectPtr<APawn> TargetPawn;
	//TWeakObjectPtr<AEnemyBase> CachedEnemy;
	//
	//virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
	//virtual void StartChase();
	//virtual void TryAttack();
	//virtual void OnAttackFinished();
	//
	//for targeting Player
	//virtual APawn* SelectTarget();
	////for dont cast every time
	//FORCEINLINE AEnemyBase* GetEnemy() const { return CachedEnemy.Get(); }
	////for check is Player alive or not
	//bool IsTargetAlive() const;
	////for check is Player in own Attack Range or not
	//bool IsTargetInRange() const;
	//
	//
	//
	////
	//UPROPERTY(EditDefaultsOnly, Category = "AI")
	//float RetryAI = 1.0f;
	//UPROPERTY(EditDefaultsOnly, Category = "AI")
	//float RetrySearch = 1.0f;

private:
	// FTimerHandle MovingTimer;
	//UPROPERTY(EditAnywhere, Category = "AI") 
	//float Radius = 1000.0f;
};
