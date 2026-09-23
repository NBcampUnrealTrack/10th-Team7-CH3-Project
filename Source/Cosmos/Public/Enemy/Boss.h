// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "Boss.generated.h"

/**
 * 
 */
UCLASS()
class COSMOS_API ABoss : public AEnemyBase
{
	GENERATED_BODY()
	
public: 
	ABoss();

	virtual void TakeHit(float Damage, EWeaponType Weapon) override; 
	virtual void Tick(float DeltaSeconds) override;
	void BeginTeleport();

	UFUNCTION(BlueprintPure, Category = "Boss")
	FORCEINLINE bool IsTeleporting() const { return bTeleporting; }
protected:
	virtual void BeginPlay() override;
	virtual float EnemyAttack() override; 

	bool CheckBossRange() const;
	float StartBurst();
	float StartSpread();

	UFUNCTION()
	void FireBurst(); 
	UFUNCTION()
	void FireSpread();
	UFUNCTION()
	void StartTeleport(); //begin -> Start -> Moved -> finish
	UFUNCTION()
	void MovedTeleport();
	UFUNCTION()
	void FinishTeleport();

	FVector PickBossPointLocation();
	void RequestTeleport();
	void ResetTeleportCount();
	void SetTeleportBlackboard(bool bValue);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Rotation")
	float TurnSpeed = 180.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Attack")
	int32 BurstCount = 7;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Attack")
	float BurstInterval = 0.15f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Attack")
	float BurstStartDelay = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Attack")
	int32 SpreadCount = 10;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Attack")
	float SpreadAngle = 60.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Attack")
	float SpreadDelay = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Attack")
	float PatternEndDelay = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Teleport")
	FName TeleportKeyName = TEXT("bNeedTeleport");
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Teleport")
	FName BossPointTag = TEXT("BossPoint");
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Telport")
	float TeleportInterval = 30.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Telport")
	float TeleportHPLimit = 0.2f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Telport")
	int32 TeleportMeleeCount = 3;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Teleport", meta = (ClampMin = "0.1"))
	float TeleportDelay = 6.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Telport")
	float SinkDepth = 500.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Teleport")
	bool bHideWhileTeleporting = true;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Projectile")
	TSubclassOf<AEnemyProjectile> BurstProjectileClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile")
	FSFXVolume BurstSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile")
	FSFXVolume BurstProjectileSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Projectile")
	float BurstSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Projectile")
	float BurstDamage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Projectile")
	TSubclassOf<AEnemyProjectile> SpreadProjectileClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile")
	FSFXVolume SpreadSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Projectile")
	FSFXVolume SpreadProjectileSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Projectile")
	float SpreadSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Projectile")
	float SpreadDamage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Anim")
	TObjectPtr<UAnimMontage> BurstMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Anim")
	TObjectPtr<UAnimMontage> SpreadMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Anim")
	TObjectPtr<UAnimMontage> TeleportStartMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Anim")
	TObjectPtr<UAnimMontage> TeleportEndMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Telport")
	FSFXVolume SinkSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Telport")
	FSFXVolume RiseSound;

private:
	FTimerHandle TeleportTimerHandle;
	bool bTeleporting = false;
	FTimerHandle MechTimerHandle;
	int32 BurstRemaining = 0;
	int32 MeleeAttackCount = 0;
	float DamageTakenCheck = 0.f;
	
	float LastTeleportTime = 0.f;
	bool bTeleportPending = false;

	TArray<TWeakObjectPtr<AActor>> BossPoints;

	int32 CurrentPointIndex = INDEX_NONE;
};
