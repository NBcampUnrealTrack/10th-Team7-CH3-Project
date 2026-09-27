// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Weapon/Damageable.h"
#include "Data/EnchantPickup.h"
#include "Data/CosDataTable.h"
#include "EnemyBase.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnemyKilled, AEnemyBase*);
class UNiagaraSystem;
class UHealthComponent;
class UAnimMontage;
class UBehaviorTree;
class USoundBase;
class AEnemyProjectile;
UCLASS()
class COSMOS_API AEnemyBase : public ACharacter, public IDamageable
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyBase();
	// Damageable functions 
	////
	virtual void TakeHit(float Damage, EWeaponType Weapon) override;
	virtual void TakeHitAlt(float Damage, EWeaponType Weapon, const FHitResult& Hit) override;
	////
	//UFUNCTION(BlueprintPure, Category = "Enemy|Stats")
	//FORCEINLINE float GetAttackRange() const { return RuntimeStats.AttackRange; }

	void SetMovementSpeed(float NewSpeed);

	FOnEnemyKilled OnEnemyKilled;
	//stats getter
	UFUNCTION(BlueprintPure, Category = "AI|Getters")
	FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTreeAsset; }
	UFUNCTION(BlueprintPure, Category = "AI|Getters")
	FORCEINLINE float GetMaxHP() const { return MaxHP; }
	UFUNCTION(BlueprintPure, Category = "AI|Getters")
	FORCEINLINE float GetAttackRange() const { return AttackRange; }
	UFUNCTION(BlueprintPure, Category = "AI|Getters")
	FORCEINLINE float GetRunSpeed() const { return RunSpeed; }
	UFUNCTION(BlueprintPure, Category = "AI|Getters")
	FORCEINLINE float GetAttackDamage() const { return AttackDamage; }
	UFUNCTION(BlueprintPure, Category = "AI|Getters")
	FORCEINLINE float GetAttackDelay() const { return AttackDelay; }
	UFUNCTION(BlueprintPure, Category = "AI|Projectile")
	FVector GetMuzzleLocation() const;
	UFUNCTION(BlueprintPure, Category = "Enemy|Projectile")
	FVector GetAimLocation() const;

	//status getter
	UFUNCTION(BlueprintPure, Category = "AI|Getters")
	bool IsAlive() const;
	UFUNCTION(BlueprintPure, Category = "AI|Getters")
	bool IsStagger() const { return bIsStagger; };
	UFUNCTION(BlueprintPure, Category = "AI|Getters")
	virtual AActor* GetAttackTarget() const;
	//attack hit
	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	virtual void AttackHitCheck();
	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	virtual float EnemyAttack();
	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	virtual void ApplyStagger();
	UFUNCTION(BlueprintCallable, Category = "AI|SFX")
	void PlayFootstep();
	static void PlaySFXAt(const UObject* WorldContext, const FSFXVolume& SFX, const FVector& Location);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called when all components loaded  / 엑터에 포함된 모든 컴포넌트가 생성&초기화되면 엔진이 실행
	virtual void PostInitializeComponents() override;
	// Called when Destroy, or moved level / 엑터가 파괴되거나 레벨 이동으로 제거 될 때 실행 -> Cleanup
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// for new hitbox collision
	UPROPERTY(EditDefaultsOnly, Category = "AI|Hitbox")
	FName HitboxProfileName = TEXT("EnemyHitbox");
	UPROPERTY(Transient)
	TArray<TObjectPtr<UPrimitiveComponent>> Hitboxes;
	void CollectHitboxes();
	void SetHitboxesEnabled(bool bEnabled);

	UPROPERTY(EditDefaultsOnly, Category = "AI|Data")
	TObjectPtr<UDataTable> EnemyDataTable;
	UPROPERTY(EditDefaultsOnly, Category = "AI|Data")
	FName EnemyRowName;
	UPROPERTY(BlueprintReadOnly, Category = "AI|Data")
	FEnemyData EnemyData;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	FName StaggerKeyName = TEXT("bIsStagger");
	//HealthComponent
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	TObjectPtr<UHealthComponent> HealthComponent;
	UPROPERTY(EditAnywhere, Category = "AI|Drop")
	TSubclassOf<AEnchantPickup> EnchantPickupClass;
	//Assets / Need to Set AnimBP <- for using Anim in cpp
	////
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Anim")
	TObjectPtr<UAnimMontage> AttackMontage1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Anim")
	TObjectPtr<UAnimMontage> AttackMontage2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Anim")
	TObjectPtr<UAnimMontage> HitReactMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Anim")
	TObjectPtr<UAnimMontage> DeathMontage;
	//Sounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|SFX")
	TArray<FSFXVolume> FootStepSounds;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|SFX", meta = (ClampMin = "0.0"))
	float FootstepAudibleDistance = 1400.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|SFX")
	TArray<FSFXVolume> AttackSounds;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|SFX")
	FSFXVolume HitbyMeleeSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|SFX")
	FSFXVolume HitbyRangeSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|SFX")
	FSFXVolume DeathHitSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|SFX")
	FSFXVolume DeathSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|SFX")
	FSFXVolume ProjectileHitSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|SFX")
	FSFXVolume HitPlayerSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI|SFX")
	bool bIsExplodeDeath = false;
	//Effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|VFX")
	TObjectPtr<UNiagaraSystem> BloodHitVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|VFX")
	FVector BloodHitVFXScale = FVector(1.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|VFX")
	TObjectPtr<UNiagaraSystem> MeleeHitVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|VFX")
	FVector MeleeHitVFXScale = FVector(1.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|VFX")
	TObjectPtr<UNiagaraSystem> RangeHitVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|VFX")
	FVector RangeHitVFXScale = FVector(1.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|VFX")
	float HitVFXHeightOffset = 0.f;
	//
	void PlayHitVFX(EWeaponType weapon);
	UFUNCTION(BlueprintCallable, Category = "AI|SFX")
	void PlaySFX(const FSFXVolume& SFX);
	UFUNCTION(BlueprintCallable, Category = "AI|SFX")
	void PlayRandomSFX(const TArray<FSFXVolume>& Sounds);
	void PlaySequentialSFX(const TArray<FSFXVolume>& Sounds, int32& InOutIndex);
	bool ResolveImpactPoint(const FHitResult& Hit, FVector& OutPoint, FVector& OutNormal) const;

	//Delay for Dead Animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Anim", meta = (ClampMin = "0.0"))
	float DeathDelay = 2.0f;
	//stats
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float MaxHP = 130.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float AttackRange = 120.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float RunSpeed = 500.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float AttackDamage = 15.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float AttackSpeed = 1200.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float AttackDelay = 1.5f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float AttackPreDelay = 0.5f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float AttackRadius = 80.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float AttackOffset = 80.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float AttackHeightOffset = 0.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float StaggerDuration = 0.3f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float StaggerDelay = 0.5f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	int32 SoulAmount = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	bool ImmuneRange = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	bool ImmuneMelee = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float StaggerDamage = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Combat")
	bool bFaceTargetOnAttack = true;
	UFUNCTION(BlueprintCallable, Category = "AI|Projectile")
	virtual void FireProjectile(float InSpeed, float InDamage);
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Projectile")
	TSubclassOf<AEnemyProjectile> ProjectileClass; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Projectile")
	float MuzzleForwardOffset = 60.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Projectile")
	float MuzzleHeightOffset = 40.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Projectile")
	float AimHeightOffset = 50.f;
	UPROPERTY(EditDefaultsOnly, Category = "AI|Hitbox")
	bool bBlockPlayer = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|VFX")
	float HitVFXInterval = 0.05f;
	UFUNCTION()
	void HandleDeath();
	virtual void ApplyDeathMovement();
	void FireProjectile2(float InSpeed, float InDamage, float YawOffset, TSubclassOf<AEnemyProjectile> SelectedProjectile);
	void SetEnemyAtStart();
	void FaceTarget(const AActor* Target);
	void SetStaggerBlackboard(bool bValue);
	virtual FVector GetDropLocation() const;

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
private:
	void DisableAllCollision();
	FVector LastHitPoint = FVector::ZeroVector;
	FVector LastHitNormal = FVector::ZeroVector;
	bool bHasLastHit = false;
	float LastHitVFXTime = -1.f;
	FTimerHandle DeathTimerHandle;
	FTimerHandle AttackHitTimerHandle;
	FTimerHandle StaggerTimerHandle;
	bool  bIsStagger = false;
	float LastStaggerTime = -1.f;
	bool bDeathHandled = false;
	int32 FootstepIndex = 0;
};
