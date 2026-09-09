#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CosGameInstance.h"
#include "CosDataTable.generated.h"

USTRUCT(BlueprintType)
struct FEnemyData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 체력, 공격력, 이동속도, 돌진속도, 공격속도, 투사체 속도, 비행 고도, 선회 반경, 영혼 조각 드롭량
	// 몬스터 스탯 구조체
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float HP = 0.f; // 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AttackPower = 0.f; // 공격력
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MoveSpeed = 0.f; // 이동속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DashSpeed = 0.f; // 돌진 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AttackSpeed = 0.f; // 공격 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float BulletSpeed = 0.f; // 투사체 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float FlightAltitude = 0.f; // 비행 고도
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float TurnRadius = 0.f;	// 선회 반경
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 SoulDrop = 0; // 소울 드롭량
};

USTRUCT(BlueprintType)
struct FWaveData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 WaveIndex = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 PhaseIndex = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float LimitTime = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 GhoulCount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 EnhancedGhoulCount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 GargoyleCount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 CrowCount = 0;
};

USTRUCT(BlueprintType)
struct FUpgradeData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EShotgunModuleType  Type = EShotgunModuleType::Muzzle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Level = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Cost = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float EffectValue = 0.f;

};

UCLASS()
class COSMOS_API UCosDataTable : public UDataTable
{
	GENERATED_BODY()
	
};
