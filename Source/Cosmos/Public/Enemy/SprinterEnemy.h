// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/EnemyBase.h"
#include "SprinterEnemy.generated.h"
class UNiagaraSystem;
class UNiagaraComponent;
/**
 * 
 */
UCLASS()
class COSMOS_API ASprinterEnemy : public AEnemyBase
{
	GENERATED_BODY()
public:
	ASprinterEnemy();

	virtual float EnemyAttack() override;


protected:
	virtual void PostInitializeComponents() override;

	UFUNCTION(BlueprintCallable, Category = "Sprinter")
	void EnemyExplode(bool bPlayFX);
	void StartChargeVFX();
	void StopChargeVFX();
	//explode time
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprinter")
	float ExplodePreDelay = 0.8f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprinter|FX")
	TObjectPtr<UNiagaraSystem> ChargeVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bomber|FX")
	FVector ChargeVFXScale = FVector(1.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprinter|FX")
	TObjectPtr<UNiagaraSystem> ExplosionVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprinter|FX")
	FVector ExplosionVFXScale = FVector(1.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprinter|FX")
	FSFXVolume ExplosionSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprinter|FX")
	FSFXVolume ExplodingSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprinter")
	float SelfDestructDeathDelay = 0.2f;
private:
	UFUNCTION()
	void HandleExplodeOnDeath();
	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> ChargeVFXComp;

	FTimerHandle ExplodeTimerHandle;
	bool bFuseLit = false;
	bool bIsExploded = false;
};
