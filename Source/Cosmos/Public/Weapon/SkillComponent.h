#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/CosDataTable.h"
#include "SkillComponent.generated.h"

class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSkillStateChanged); // 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSkillActivated);

USTRUCT() // UPROPERTY()로 선언된 배열에 담기려면 리플렉션 등록돼 있어야 함
struct FActiveSkill // 지금 켜져 있는 스킬
{
	GENERATED_BODY()

	UPROPERTY()
	EEnchantSkillType SkillType = EEnchantSkillType::None;

	UPROPERTY()
	float Value = 0.f;

	UPROPERTY()
	float EndTime = 0.f; // 몇 초에 끝나는지
};

UCLASS(meta = (BlueprintSpawnableComponent))
class COSMOS_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USkillComponent();

	UPROPERTY(BlueprintAssignable, Category = "Skill")
	FOnSkillStateChanged OnSkillStateChanged; // 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Skill") // 스킬이 실제로 발동됐을 때만 방송
	FOnSkillActivated OnSkillActivated;

	UFUNCTION(BlueprintCallable, Category = "Skill")
	void ActivateSkills();
	UFUNCTION(BlueprintPure, Category = "Skill")
	bool IsSkillActive(EEnchantSkillType SkillType) const;

	float GetActiveSkillBonus(EEnchantStat Stat) const;

protected:
	virtual void BeginPlay() override;

	void OnUseSkill();

	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Input")
	UInputAction* UseSkillAction;

	UPROPERTY()
	TArray<FActiveSkill> ActiveSkills; // 지금 발동중인 스킬들
	UPROPERTY()
	TMap<EEnchantSkillType, float> SkillReadyTime;
};
