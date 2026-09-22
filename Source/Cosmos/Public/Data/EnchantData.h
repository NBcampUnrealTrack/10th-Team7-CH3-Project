#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CosDataTable.h"
#include "EnchantData.generated.h"

USTRUCT(BlueprintType)
struct FEnchantRolledStat
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EEnchantStat StatType = EEnchantStat::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EEnchantValueType ValueType = EEnchantValueType::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float RolledValue = 0.f;
};

UCLASS()
class COSMOS_API UEnchantData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText EnchantDescription;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FEnchantRolledStat> RolledStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bHasSkill = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EEnchantSkillType SkillType = EEnchantSkillType::IncreaseAllDamage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float SkillValue = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float SkillDuration = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float SkillCooldown = 0.f;

	UFUNCTION(BlueprintPure, Category = "Enchant")
	static FText GetStatLabel(EEnchantStat stat);

	UFUNCTION(BlueprintPure, Category = "Enchant")
	static FText GetSkillLabel(EEnchantSkillType Type);

	UFUNCTION(BlueprintPure, Category = "Enchant")
	static EEnchantValueType GetSkillValueType(EEnchantSkillType Type);
	
};
