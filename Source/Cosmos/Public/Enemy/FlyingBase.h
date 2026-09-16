// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/EnemyBase.h"
#include "FlyingBase.generated.h"

/**
 * 
 */
UCLASS()
class COSMOS_API AFlyingBase : public AEnemyBase
{
	GENERATED_BODY()

public:
	AFlyingBase();

	UFUNCTION(BlueprintPure, Category = "AI|Gettes")
	FORCEINLINE float GetFlyHeight() const { return FlyHeight; }
	UFUNCTION(BlueprintPure, Category = "AI|Gettes")
	FORCEINLINE float GetRoamRadius() const { return RoamRadius; } // roam after attack
	UFUNCTION(BlueprintPure, Category = "AI|Getters")
	FORCEINLINE float GetDistanceCheck() const { return DistanceCheck; } // check ditance between player and monster
	UFUNCTION(BlueprintPure, Category = "AI|Getters")
	FORCEINLINE float GetRetreatMulti() const { return RetreatMulti; } // Retreat distance, multi * distancecheck
	UFUNCTION(BlueprintPure, Category = "AI|Getters")
	FORCEINLINE float GetArriveRadius() const { return ArriveCheck; } // check is enemy got distance of player or not

	UFUNCTION(BlueprintPure, Category = "AI|Fly")
	FVector GetFlyLocation(const FVector& Ground) const;
	
	UFUNCTION(BlueprintPure, Category = "AI|Combat")
	bool CanAttack() const;

protected:
	virtual void BeginPlay()override;
	virtual void AttackHitCheck()override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Fly")
	float FlyHeight = 600.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Fly")
	float RoamRadius = 100.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Fly")
	float ArriveCheck = 100.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Fly")
	float GroundCheck = 5000.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Fly")
	float DistanceCheck = 800.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Fly")
	float RetreatMulti = 1.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Attack")
	TSubclassOf<AActor> ProjectileClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Attack")
	float MuzzleOffset = 60.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Attack")
	float AimHeightOffset = 50.f;
private:
	float LastAttackTime = -1.f;
};
