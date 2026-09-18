// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CosTypes.h"
#include "EnemyProjectile.generated.h"
class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;
UCLASS()
class COSMOS_API AEnemyProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AEnemyProjectile();

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void InitProjectile(float InDamage, float InSpeed);

protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void PlaySFXProjectile(const FSFXVolume& SFX);
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USphereComponent> CollisionComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UStaticMeshComponent> MeshComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UProjectileMovementComponent> PMovement;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|FX")
	TObjectPtr<UNiagaraSystem> ImpactFX;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|FX")
	FSFXVolume ImpactSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|FX")
	FSFXVolume HitPlayerSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float LifeMaxSeconds = 5.f;
private:
	float Damage = 0.f;
	bool bImpacted = false;
};
