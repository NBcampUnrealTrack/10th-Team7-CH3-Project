#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CosDataTable.generated.h"

UENUM(BlueprintType)
enum class EEnchantCategory : uint8
{
	None,
	All,
	Range,
	Melee,
	Misc
};

UENUM(BlueprintType)
enum class EEnchantStatKind : uint8
{
	None,
	Damage, Speed, MaxAmmo, ReloadSpeed, MaxTarget, AttackRange,
	MaxHealth, MovementSpeed, SprintMulti, MaxPotion, PotionValue,
	SkillCooldown, SoulGain, GainHeal
};

UENUM(BlueprintType)
enum class EEnchantValueType : uint8
{
	None,
	Flat,
	Percent,
	Integer
};

UENUM(BlueprintType)
enum class EEnchantStat : uint8
{
	None = 0,

	// All
	AllDamageAdd = 10, 
	AllDamageMulti = 11, 
	AllSpeed = 12,

	// Range
	RangeDamageAdd = 20, 
	RangeDamageMulti = 21, 
	RangeFiringRate = 22, 
	RangeMaxAmmo = 23,
	RangeReloadSpeed = 24,

	// Melee
	MeleeDamageAdd = 30, 
	MeleeDamageMulti = 31, 
	MeleeSpeed = 32, 
	MeleeMaxTarget = 33,
	MeleeRange = 34,

	//Misc
	MaxHealth = 40, 
	MovementSpeed = 41, 
	SprintSpeedMulti = 42,
	MaxPotionAdd = 43,
	IncreasePotionValue = 44, 
	DecreaseSkillCooldown = 45, 
	IncreaseSoulValue = 46,
	GainHeal = 47
};

FORCEINLINE uint8 GetCategory(EEnchantStat Stat)
{
	return static_cast<uint8>(Stat) / 10;
}

UENUM(BlueprintType)
enum class EEnchantSkillType : uint8
{
	None = 0,
	// All
	IncreaseAllSpeed = 10, IncreaseAllDamage = 11,
	// Range
	IncreaseShotgunDamage = 20, IncreaseShotgunFiringRate = 21, IncreaseRangeReloadSpeed = 22, InfinityAmmo = 23,
	// Melee
	IncreaseMeleeDamage = 30, IncreaseMeleeAttackSpeed = 31, IncreaseMeleeMaxTarget = 32,
	// Misc
	IncreaseMovementSpeed = 40, MadePotion = 41
};

FORCEINLINE uint8 GetSkillCategory(EEnchantSkillType Skill)
{
	return static_cast<uint8>(Skill) / 10;
}



USTRUCT(BlueprintType)
struct FEnemyData : public FTableRowBase
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float HP = 0.f; // ü��
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AttackPower = 0.f; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MoveSpeed = 0.f; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DashSpeed = 0.f; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AttackSpeed = 0.f; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float BulletSpeed = 0.f; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float FlightAltitude = 0.f; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float TurnRadius = 0.f;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 SoulDrop = 0; 
};

USTRUCT(BlueprintType)
struct FWaveData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 StageIndex = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 WaveIndex	= 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float LimitTime = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 GhoulCount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 EnhancedGhoulCount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 GargoyleCount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 CrowCount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 BossCount = 0;
};

UENUM(BlueprintType)
enum class EShotgunModuleType : uint8
{
	Damage,
	FireSpeed,
	Reload,
	Magazine
};

USTRUCT(BlueprintType)
struct FUpgradeData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EShotgunModuleType  Type = EShotgunModuleType::Damage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Level = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Cost = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float EffectValue = 0.f;

};

USTRUCT(BlueprintType)
struct FEnchantStatData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EEnchantStat StatType = EEnchantStat::None; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EEnchantCategory Category = EEnchantCategory::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EEnchantStatKind StatKind = EEnchantStatKind::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EEnchantValueType ValueType = EEnchantValueType::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MinValue = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxValue = 0.f;
};

USTRUCT(BlueprintType)
struct FEnchantSkillData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EEnchantSkillType SkillType = EEnchantSkillType::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MinValue = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxValue = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MinDuration = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxDuration = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MinCooldown = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxCooldown = 0.f;
};

UCLASS()
class COSMOS_API UCosDataTable : public UDataTable
{
	GENERATED_BODY()
	
};
