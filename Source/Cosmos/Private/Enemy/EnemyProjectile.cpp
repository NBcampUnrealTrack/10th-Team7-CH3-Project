// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/EnemyProjectile.h"
#include "Enemy/EnemyBase.h"
#include "Weapon/Damageable.h"     
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

#include "Engine/World.h"
// Sets default values
AEnemyProjectile::AEnemyProjectile()
{
 	PrimaryActorTick.bCanEverTick = false;
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));
	SetRootComponent(CollisionComp);
	CollisionComp->InitSphereRadius(12.0f);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionProfileName(TEXT("EnemyProjectile"));
	CollisionComp->SetGenerateOverlapEvents(false);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(CollisionComp);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetGenerateOverlapEvents(false);
	MeshComp->SetCastShadow(false);
	MeshComp->bReceivesDecals = false;

	PMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
	PMovement->SetUpdatedComponent(CollisionComp);
	PMovement->InitialSpeed = 1200.f; // hard coding for test, it will be change when spawn itself
	PMovement->MaxSpeed = 1200.f; // hard coding for test, it will be change when spawn itself
	PMovement->bRotationFollowsVelocity = true;
	PMovement->bShouldBounce = false;
	PMovement->ProjectileGravityScale = 0.0f;
	PMovement->bSweepCollision = true;
	
	bReplicates = false;
	SetReplicateMovement(false);
}

void AEnemyProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (CollisionComp)
	{
		CollisionComp->OnComponentHit.AddDynamic(this, &AEnemyProjectile::OnHit);
	}
	SetLifeSpan(LifeMaxSeconds);
}
void AEnemyProjectile::InitProjectile(float InDamage, float InSpeed)
{
	Damage = InDamage;
	bImpacted = false;
	if (PMovement && InSpeed > 0.f) 
	{
		PMovement->InitialSpeed = InSpeed;
		PMovement->MaxSpeed = InSpeed;
		PMovement->Velocity = GetActorForwardVector() * InSpeed;
	}
}
void AEnemyProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bImpacted)
	{
		return;
	}
	bImpacted = true;
	const bool bHitPlayer =	OtherActor	&& OtherActor != this	&& OtherActor != GetInstigator() && OtherActor->ActorHasTag(TEXT("Player"));
	if (bHitPlayer)
	{
			if (IDamageable* Target = Cast<IDamageable>(OtherActor))
		{
			Target->TakeHitAlt(Damage, EWeaponType::None, Hit);
			AEnemyBase::PlaySFXAt(this, HitPlayerSound, Hit.ImpactPoint);
		}
		else
		{
				AEnemyBase::PlaySFXAt(this, ImpactSound, Hit.ImpactPoint);
		}
	}
	if (ImpactFX) {
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(	this, ImpactFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(),FVector(1.f), true, true, ENCPoolMethod::AutoRelease);
	}
	Destroy();
}

void AEnemyProjectile::PlaySFXProjectile(const FSFXVolume & SFX)
	{
		if (!SFX.Sound)
		{
			return;
		}
		const float FinalResult = SFX.Pitch * (1.f + FMath::FRandRange(-SFX.RandomPitch, SFX.RandomPitch));
		UGameplayStatics::PlaySoundAtLocation(this, SFX.Sound, GetActorLocation(), SFX.Volume, FinalResult);
	}


