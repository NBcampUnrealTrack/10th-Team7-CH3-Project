// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Boss.h"        
#include "Enemy/EnemyBase.h" 
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"

ABoss::ABoss()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	bFaceTargetOnAttack = false;
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	//boss dont movbe
	Movement->DefaultLandMovementMode = MOVE_None;
	Movement->SetMovementMode(MOVE_None);
	Movement->bUseRVOAvoidance = false;
	Movement->bOrientRotationToMovement = false;
	Movement->bEnablePhysicsInteraction = false;
	Movement->MaxDepenetrationWithPawn = 0.f;

	USkeletalMeshComponent* MeshComp = GetMesh();
	MeshComp->SetCollisionProfileName(TEXT("EnemyHitbox"));
	MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	MeshComp->bEnableUpdateRateOptimizations = false;
}


void ABoss::BeginPlay()
{
	AEnemyBase::BeginPlay();

	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsWithTag(this, BossPointTag, Found);

	Found.Sort([](const AActor& A, const AActor& B)
		{
			return A.GetName() < B.GetName();
		});

	BossPoints.Reset();
	for (AActor* Point : Found)
	{
		if (Point)
		{
			BossPoints.Add(Point);
		}
	}

	float BestDistSq = TNumericLimits<float>::Max();
	for (int32 i = 0; i < BossPoints.Num(); ++i)
	{
		if (!BossPoints[i].IsValid())
		{
			continue;
		}
		const float DistSq = FVector::DistSquared(GetActorLocation(), BossPoints[i]->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			CurrentPointIndex = i;
		}
	}

	ResetTeleportCount();
}

float ABoss::EnemyAttack()
{
	if (const UWorld* World = GetWorld())
	{
		if ((World->GetTimeSeconds() - LastTeleportTime) >= TeleportInterval)
		{
			RequestTeleport();
		}
	}
	if (CheckBossRange())
	{
		++MeleeAttackCount;
		if (MeleeAttackCount >= TeleportMeleeCount)
		{
			RequestTeleport();  
		}
		return AEnemyBase::EnemyAttack();
	}
	if (bFaceTargetOnAttack)
	{
		FaceTarget(GetAttackTarget());
	}
	return FMath::RandBool() ? StartBurst() : StartSpread();
}
bool ABoss::CheckBossRange() const
{
	const AActor* Target = GetAttackTarget();
	if (!Target)
	{
		return false;
	}
	const float DistSq = FVector::DistSquaredXY(GetActorLocation(), Target->GetActorLocation());
	return DistSq <= FMath::Square(GetAttackRange());
}

float ABoss::StartBurst()
{
	BurstRemaining = FMath::Max(1, BurstCount);
	if (BurstMontage)
	{
		PlayAnimMontage(BurstMontage);
	}
	PlaySFX(BurstSound);
	GetWorldTimerManager().SetTimer(MechTimerHandle, this, &ABoss::FireBurst, BurstInterval, true,	BurstStartDelay);
	return BurstStartDelay + BurstInterval * BurstRemaining + PatternEndDelay;
}
void ABoss::FireBurst()
{
	if (!IsAlive() || IsTeleporting())
	{
		GetWorldTimerManager().ClearTimer(MechTimerHandle);
		return;
	}
	PlaySFX(BurstProjectileSound);
	FaceTarget(GetAttackTarget());
	FireProjectile2(BurstSpeed, BurstDamage, 0.f, BurstProjectileClass);
	if (--BurstRemaining <= 0)
	{
		GetWorldTimerManager().ClearTimer(MechTimerHandle);
	}
}

float ABoss::StartSpread()
{
	if (SpreadMontage)
	{
		PlayAnimMontage(SpreadMontage);
	}

	PlaySFX(SpreadSound);
	PlaySFX(SpreadProjectileSound);
	GetWorldTimerManager().SetTimer(MechTimerHandle, this, &ABoss::FireSpread,SpreadDelay, false);
	return SpreadDelay + PatternEndDelay;
}


void ABoss::FireSpread()
{
	if (!IsAlive() || IsTeleporting())
	{
		return;
	}
	FaceTarget(GetAttackTarget());

	const int32 Num = FMath::Max(1, SpreadCount);
	const float Step = (Num > 1) ? (SpreadAngle / static_cast<float>(Num - 1)) : 0.f;
	const float Start = (Num > 1) ? (-SpreadAngle * 0.5f) : 0.f;

	for (int32 i = 0; i < Num; ++i)
	{
		FireProjectile2(SpreadSpeed, SpreadDamage, Start + Step * i, SpreadProjectileClass);
	}
}
void ABoss::TakeHit(float Damage, EWeaponType Weapon)
{
	const bool bWasAlive = IsAlive();

	AEnemyBase::TakeHit(Damage, Weapon);

	if (bWasAlive && Damage > 0.f)
	{
		DamageTakenCheck += Damage;

		if (DamageTakenCheck >= GetMaxHP() * TeleportHPLimit)
		{
			RequestTeleport();
		}
	}
}

//teleport section
//////////////////
void ABoss::RequestTeleport()
{
	if (bTeleportPending || IsTeleporting() || !IsAlive())
	{
		return;
	}
	if (BossPoints.Num() <= 1)
	{
		return;  
	}

	bTeleportPending = true;
	SetTeleportBlackboard(true);
}
void ABoss::ResetTeleportCount()
{
	MeleeAttackCount = 0;
	DamageTakenCheck = 0.f;
	bTeleportPending = false;

	if (const UWorld* World = GetWorld())
	{
		LastTeleportTime = World->GetTimeSeconds();
	}
}
void ABoss::SetTeleportBlackboard(bool bValue)
{
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AI->GetBlackboardComponent())
		{
			BB->SetValueAsBool(TeleportKeyName, bValue);
		}
	}
}
void ABoss::BeginTeleport()
{
	if (IsTeleporting() || !IsAlive())
	{
		return;
	}
	bTeleporting = true;

	GetWorldTimerManager().ClearTimer(MechTimerHandle);
	BurstRemaining = 0;

	PlaySFX(SinkSound);

	const float SinkTime = PlayAnimMontage(TeleportStartMontage);

	GetWorldTimerManager().SetTimer(TeleportTimerHandle, this, &ABoss::StartTeleport, SinkTime, false);
}
void ABoss::StartTeleport()
{
	if (!IsAlive())
	{
		bTeleporting = false;
		return;
	}
	SetActorEnableCollision(false);

	FVector Location = GetActorLocation();
	Location.Z -= SinkDepth;
	SetActorLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);

	if (bHideWhileTeleporting)
	{
		SetActorHiddenInGame(true);
	}

	GetWorldTimerManager().SetTimer(TeleportTimerHandle, this,	&ABoss::MovedTeleport,TeleportDelay, false);
}
void ABoss::MovedTeleport()
{
	const FVector Dest = PickBossPointLocation();
	SetActorLocation(Dest, false, nullptr, ETeleportType::TeleportPhysics);

	FaceTarget(GetAttackTarget());

	if (bHideWhileTeleporting)
	{
		SetActorHiddenInGame(false);
	}
	SetActorEnableCollision(true);

	PlaySFX(RiseSound);
	ResetTeleportCount();

	const float RiseTime = PlayAnimMontage(TeleportEndMontage);

	GetWorldTimerManager().SetTimer(TeleportTimerHandle, this,	&ABoss::FinishTeleport, FMath::Max(RiseTime, 0.01f), false);
}
void ABoss::FinishTeleport()
{
	bTeleporting = false;
	SetTeleportBlackboard(false);
}
FVector ABoss::PickBossPointLocation()
{
	
	TArray<int32> BossSpawnPoints;
	BossSpawnPoints.Reserve(BossPoints.Num());

	for (int32 i = 0; i < BossPoints.Num(); ++i)
	{
		if (i != CurrentPointIndex && BossPoints[i].IsValid())
		{
			BossSpawnPoints.Add(i);
		}
	}
	if (BossSpawnPoints.Num() == 0)
	{
		return GetActorLocation();
	}

	const int32 Pick = BossSpawnPoints[FMath::RandRange(0, BossSpawnPoints.Num() - 1)];
	CurrentPointIndex = Pick;

	FVector Dest = BossPoints[Pick]->GetActorLocation();
	if (const UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Dest.Z += Capsule->GetScaledCapsuleHalfHeight();
	}

	return Dest;
}

void ABoss::Tick(float DeltaSeconds)
{
	AEnemyBase::Tick(DeltaSeconds);
		if (!IsAlive() || IsTeleporting())
	{
		return;
	}
	const AActor* Target = GetAttackTarget();
	if (!Target)
	{
		return;
	}

	FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.f;
	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	const FRotator Current = GetActorRotation();
	const FRotator Desired(0.f, ToTarget.Rotation().Yaw, 0.f);
	SetActorRotation(FMath::RInterpConstantTo(Current, Desired, DeltaSeconds, TurnSpeed));
}