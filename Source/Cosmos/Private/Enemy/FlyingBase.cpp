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
	//set MaxFlySpeed
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
FVector AFlyingBase::GetMuzzleLocation() const
{
	return GetActorLocation() + GetActorForwardVector() * MuzzleOffset;
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
	PlaySFX(AttackSound);
	if (!ProjectileClass)
	{
		return;
	}
	//only target main player
	const APawn* TargetPlayer = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!TargetPlayer)
	{
		return;
	}
	//for shoot to body not foot
	const FVector AimOffset = TargetPlayer->GetActorLocation() + FVector(0.f, 0.f, AimHeightOffset);
	const FVector MuzzleLocation = GetActorLocation() + GetActorForwardVector() * MuzzleOffset;
	const FRotator FirePoint = (AimOffset - MuzzleLocation).Rotation();
	//spawn setting
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn; //always shoot.

	GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, FirePoint, SpawnParams);
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