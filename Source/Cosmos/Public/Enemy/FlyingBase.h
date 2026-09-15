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
	FORCEINLINE float GetRoamRadius() const { return RoamRadius; }
	UFUNCTION(BlueprintPure, Category = "AI|Getters")
	FORCEINLINE float GetArriveCheck() const { return ArriveCheck; }

	UFUNCTION(BlueprintPure, Category = "AI|Fly")
	FVector GetFlyLocation(const FVector& Ground) const;
	
protected:
	virtual void BeginPlay()override;
	virtual void AttackHitCheck()override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Fly")
	float FlyHeight = 600.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Fly")
	float RoamRadius = 1000.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Fly")
	float ArriveCheck = 100.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Fly")
	float GroundCheck = 5000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Attack")
	TSubclassOf<AActor> ProjectileClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Attack")
	float MuzzleOffset = 60.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Attack")
	float AimHeightOffset = 50.f;
};
