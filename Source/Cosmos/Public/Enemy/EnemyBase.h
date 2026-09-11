// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Weapon/Damageable.h"
#include "EnemyBase.generated.h"


class UHealthComponent;
class UAnimMontage;
class UBehaviorTree;

UCLASS()
class COSMOS_API AEnemyBase : public ACharacter, public IDamageable
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyBase();
	// Damageable functions 
	////
	virtual void TakeHit(float Amount, const FHitResult& HitResult) override;
	////
	//UFUNCTION(BlueprintPure, Category = "Enemy|Stats")
	//FORCEINLINE float GetAttackRange() const { return RuntimeStats.AttackRange; }

	void SetMovementSpeed(float NewSpeed);

	//stats getter
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

	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	float EnemyAttack();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called when all components loaded  / 엑터에 포함된 모든 컴포넌트가 생성&초기화되면 엔진이 실행
	//virtual void PostInitializeComponents() override;
	// Called when Destroy, or moved level / 엑터가 파괴되거나 레벨 이동으로 제거 될 때 실행 -> Cleanup
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;
	//HealthComponent
	UPROPERTY(VisibleAnyWhere, BlueprintReadWrite, Category = "AI")
	TObjectPtr<UHealthComponent> HealthComponent;
	//Assets / Need to Set AnimBP <- for using Anim in cpp
	////
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Anim")
	TObjectPtr<UAnimMontage> AttackMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Anim")
	TObjectPtr<UAnimMontage> HitReactMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Anim")
	TObjectPtr<UAnimMontage> DeatMontage;
	//Delay for Dead Animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Anim", meta = (ClampMin = "0.0"))
	float DeathDelay = 2.0f;

	//stats
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float MaxHP = 130.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float AttackRange = 150.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float RunSpeed = 500.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float AttackDamage = 10.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float AttackDelay = 1.5f;
	void InitializaEnemy();

	UFUNCTION()
	void HandleDeath();

	FTimerHandle DeathTimerHandle;

public:
	// NO TICK
	//virtual void Tick(float DeltaTime) override;
};
