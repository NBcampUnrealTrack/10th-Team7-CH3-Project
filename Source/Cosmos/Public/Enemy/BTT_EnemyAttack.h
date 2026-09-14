#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_EnemyAttack.generated.h"

struct FBTPerformAttackMemory
{
	float RemainingTime = 0.f;
};
/**
 *
 */
UCLASS()
class COSMOS_API UBTT_EnemyAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_EnemyAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	virtual uint16 GetInstanceMemorySize() const override;
protected:


	//put short delay Each Attacks
	UPROPERTY(EditAnywhere, Category = "Setting", meta = (ClampMin = "0.05"))
	float MinDuration = 0.1f;
};

