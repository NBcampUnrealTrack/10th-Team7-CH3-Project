// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/SprinterEnemy.h"
#include "Character/HealthComponent.h"
#include "Weapon/Damageable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"  
#include "NiagaraComponent.h"
ASprinterEnemy::ASprinterEnemy()
{

}


void ASprinterEnemy::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &ASprinterEnemy::HandleExplodeOnDeath);
	}
}


float ASprinterEnemy::EnemyAttack()
{
	if (!IsAlive() || bFuseLit || bIsExploded)
	{
		return 0.1f;
	}
	bFuseLit = true;
	StartChargeVFX();
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		AI->StopMovement();
	}
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}
	if (bFaceTargetOnAttack)
	{
		FaceTarget(GetAttackTarget());
	}

	PlaySFX(ExplodingSound);

	if (AttackMontage1)
	{
		PlayAnimMontage(AttackMontage1);  
	}
	GetWorldTimerManager().SetTimer(ExplodeTimerHandle,FTimerDelegate::CreateWeakLambda(this, [this]() { EnemyExplode(true); }), ExplodePreDelay, false);
	return ExplodePreDelay + 0.1f;
}


void ASprinterEnemy::EnemyExplode(bool bPlayFX)
{
	if (bIsExploded)
	{
		return;
	}
	bIsExploded = true;

	GetWorldTimerManager().ClearTimer(ExplodeTimerHandle);
	StopChargeVFX();
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Center = GetActorLocation() + FVector(0.f, 0.f, AttackHeightOffset);
	if (bPlayFX)
	{
		if (ExplosionVFX)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(	this, ExplosionVFX, Center, FRotator::ZeroRotator, ExplosionVFXScale,true, true, ENCPoolMethod::AutoRelease, true);
		}
		PlaySFXAt(this, ExplosionSound, Center);
	}

	TArray<FOverlapResult> Hits;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(BomberExplode), false, this);
	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_Pawn);

	const bool bHit = World->OverlapMultiByObjectType(
		Hits, Center, FQuat::Identity, ObjParams,
		FCollisionShape::MakeSphere(AttackRadius), Params);



	if (bHit)
	{
		TSet<AActor*> AlreadyDamaged;

		for (const FOverlapResult& Hit : Hits)
		{
			AActor* Target = Hit.GetActor();
			if (!Target || !Target->ActorHasTag(TEXT("Player")) || AlreadyDamaged.Contains(Target))
			{
				continue;
			}
			AlreadyDamaged.Add(Target);

			if (IDamageable* Damageable = Cast<IDamageable>(Target))
			{
				Damageable->TakeHit(GetAttackDamage(), EWeaponType::None);
			}
		}
	}

	if (HealthComponent && !HealthComponent->IsDead())
	{
		bIsExplodeDeath = true;
		DeathDelay = SelfDestructDeathDelay;
		if (USkeletalMeshComponent* MeshComp = GetMesh())
		{
			MeshComp->SetHiddenInGame(true);
		}
		HealthComponent->ApplyDamage(GetMaxHP() * 10.f + 1000.f, EWeaponType::None);
	}
}

void ASprinterEnemy::HandleExplodeOnDeath()
{
	StopChargeVFX();
	GetWorldTimerManager().ClearTimer(ExplodeTimerHandle);
	bFuseLit = false;
}

void ASprinterEnemy::StartChargeVFX()
{
	if (!ChargeVFX || ChargeVFXComp)
	{
		return;
	}
	ChargeVFXComp = UNiagaraFunctionLibrary::SpawnSystemAttached(ChargeVFX,GetCapsuleComponent(),NAME_None,FVector::ZeroVector,FRotator::ZeroRotator,EAttachLocation::SnapToTarget, false,true,	ENCPoolMethod::None,true);
	if (ChargeVFXComp)
	{
		ChargeVFXComp->SetRelativeScale3D(ChargeVFXScale);
	}
}


void ASprinterEnemy::StopChargeVFX()
{
	if (ChargeVFXComp)
	{
		ChargeVFXComp->Deactivate();      
		ChargeVFXComp->DestroyComponent(); 
		ChargeVFXComp = nullptr;
	}
}