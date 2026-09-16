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
	return (GetWorld()->GetTimeSeconds() - LastAttackTime) >= GetAttackDelay();
}
void AFlyingBase::AttackHitCheck()
{
	if (!IsAlive())
	{
		return;
	}
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
	const FRotator AcutalFire = AimOffset.Rotation();
	const FVector MuzzleLocation = GetActorLocation() + GetActorForwardVector() * MuzzleOffset;
	//spawn setting
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn; //always shoot.
	PlaySFX(AttackSound);
	GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, AcutalFire, SpawnParams);
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