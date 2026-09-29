// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/FlyingBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AFlyingBase::AFlyingBase()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	//change move_navwalk -> move_flying
	Movement->DefaultLandMovementMode = MOVE_Flying;
	Movement->SetMovementMode(MOVE_Flying);
	//remove gravity
	Movement->GravityScale = 0.0f;
	//need maxflyspeed not maxwalkspeed
	Movement->MaxFlySpeed = RunSpeed;
	//for smooth slide when stop
	Movement->BrakingDecelerationFlying = 2048.f;
	// change setting from enemybase -> open fly
	Movement->GetNavAgentPropertiesRef().bCanFly = true;
	Movement->GetNavAgentPropertiesRef().bCanWalk = false;
	// check RVO again
	Movement->bUseRVOAvoidance = true;
	Movement->AvoidanceConsiderationRadius = 200.f;

	Movement->RotationRate = FRotator(0.f, 360.f, 0.f);
}
void AFlyingBase::BeginPlay() 
{
	Super::BeginPlay();
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxFlySpeed = GetRunSpeed();
	}
	LastAttackTime = -1.f;
	
}
bool AFlyingBase::CanAttack() const
{
	if (LastAttackTime < 0.0f)
	{
		return true;
	}
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}
	return (World->GetTimeSeconds() - LastAttackTime) >= GetAttackDelay();
}
void AFlyingBase::AttackHitCheck()
{
	if (!IsAlive())
	{
		return;
	}
	if (const UWorld* World = GetWorld())
	{
		LastAttackTime = World->GetTimeSeconds();
	}
	PlayRandomSFX(AttackSounds);
	FireProjectile(AttackSpeed, AttackDamage);
}

FVector AFlyingBase::GetFlyLocation(const FVector& Ground) const
{
	const FVector TraceStart = Ground + FVector(0.0f, 0.0f, GroundCheck * 0.4f);
	const FVector TraceEnd = Ground - FVector(0.0f, 0.0f, GroundCheck);
	FHitResult Hit;
	//ignore self, only need to find ground(worldstatic)
	FCollisionQueryParams Params(SCENE_QUERY_STAT(FindGroundForFilght), false, this);
	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params))
	{
		//if found ground -> add fly height on it
		return FVector(Ground.X, Ground.Y, Hit.ImpactPoint.Z + FlyHeight);
	}
	return Ground + FVector(0.0f, 0.0f, FlyHeight);
}
FVector AFlyingBase::GetDropLocation() const
{
	const FVector Self = GetActorLocation();
	const FVector TraceEnd = Self - FVector(0.f, 0.f, GroundCheck);

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(EnemyDropGround), false, this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Self, TraceEnd, ECC_WorldStatic, Params))
	{
		return Hit.ImpactPoint + FVector(0.f, 0.f, 20.f);
	}
		return Self;
}
void AFlyingBase::ApplyDeathMovement()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	Capsule->SetCollisionObjectType(ECC_WorldDynamic);
	Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	Capsule->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Capsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Movement->SetAvoidanceEnabled(false);
	Movement->StopMovementImmediately();
	Movement->GravityScale = DeathGravityScale;
	Movement->SetMovementMode(MOVE_Falling);
}
void AFlyingBase::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	if (IsAlive())
	{
		return;
	}
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->DisableMovement();
	Movement->SetComponentTickEnabled(false);  

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}