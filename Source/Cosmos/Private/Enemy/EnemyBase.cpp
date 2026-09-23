#include "Enemy/EnemyBase.h"
#include "Enemy/EnemyProjectile.h"
#include "Character/HealthComponent.h"
#include "Enemy/EnemyAIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/CosDataTable.h" 
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "BrainComponent.h" 
#include "TimerManager.h"
#include "Engine/World.h"
#include "Animation/AnimMontage.h"
#include "Character/CosGameState.h" 
#include "Kismet/GameplayStatics.h"
//for debug
#include "DrawDebugHelpers.h" 
#include "Engine/OverlapResult.h" //  FOverlapResult 
#include "BehaviorTree/BlackboardComponent.h" 
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

// Sets default values
AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = false;
	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	USkeletalMeshComponent* MeshComp = GetMesh();

	//capsule Scale
	Capsule->SetCapsuleRadius(25.f);
	//for move / 움직임 기본 세팅
	Movement->MaxWalkSpeed = RunSpeed;
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	//for Only walk on Nav place / Nav 위에서만 움직이게 
	Movement->SetGroundMovementMode(MOVE_NavWalking);
	Movement->SetMovementMode(MOVE_NavWalking);
	// no clip each enemy, / 적들이 알아서 비켜가게
	Movement->bUseRVOAvoidance = true;
	Movement->AvoidanceConsiderationRadius = 200.0f; //No Clip Range
	//DIsable unnecessary Functions / 이동 방향으로 알아서 회전
	Movement->bUseControllerDesiredRotation = false;
	//disable unused function in cmc / 안쓰는 기능 꺼서 계산 줄이기
	Movement->GetNavAgentPropertiesRef().bCanCrouch = false;
	Movement->GetNavAgentPropertiesRef().bCanFly = false;
	Movement->GetNavAgentPropertiesRef().bCanJump = false;
	Movement->GetNavAgentPropertiesRef().bCanSwim = false;
	// avoid hit other collisions / 서로 물리 충돌 방지
	Movement->bEnablePhysicsInteraction = false; 
	//Disable Capsule Collision Overlap Each Monsters. ->  Use RVOAvoidance up there / 콜리전 계산 중지, 겹치기 방지는 위에 RVO가 함
	Capsule->SetCollisionObjectType(ECC_Enemies);                      
	Capsule->SetCollisionResponseToChannel(ECC_Enemies, ECR_Ignore);
	Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	Capsule->SetGenerateOverlapEvents(false);
	Capsule->CanCharacterStepUpOn = ECB_No;


	Movement->bUseFlatBaseForFloorChecks = true;
	Movement->MaxStepHeight = 25.f;
	//avoid monster pop upto the sky
	Movement->MaxDepenetrationWithGeometry = 100.f;
	Movement->MaxDepenetrationWithPawn = 3.f;
	Movement->MaxDepenetrationWithPawnAsProxy = 3.f;
	Movement->bEnablePhysicsInteraction = false; 
	//for Reduce Animations costs / 시야 밖 적들 애니메이션 줄이기
	MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	MeshComp->bEnableUpdateRateOptimizations = true; // Skip Frames of distant enemies / 거리 멀면 애니메이션 줄이기
	MeshComp->SetGenerateOverlapEvents(false); // No Overlap -> use hitbox / 오버렙 안씀, 히트박스로 대체
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->bComponentUseFixedSkelBounds = true; //Skip bound Calc  / 프레임 바운드 계산 안함
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	SetEnemyAtStart();
}

void AEnemyBase::SetEnemyAtStart()
{
	SetMovementSpeed(RunSpeed);

	if (HealthComponent)
	{
		HealthComponent->SetHPAtStart(MaxHP);
	}
	if (EnemyDataTable && !EnemyRowName.IsNone())
	{
		if (const FEnemyData* Row = EnemyDataTable->FindRow<FEnemyData>(EnemyRowName, TEXT("EnemyInit")))
		{
			EnemyData = *Row;

			// 데이터 테이블 값을 기존 스탯 변수에도 반영
			MaxHP = EnemyData.HP;
			AttackDamage = EnemyData.AttackPower;
			RunSpeed = EnemyData.MoveSpeed;
			SoulAmount = EnemyData.SoulDrop; //추가된 부분
		}
	}
	bIsStagger = false;
	LastStaggerTime = -1.f;
	FootstepIndex = (FootStepSounds.Num() > 0)	? FMath::RandRange(0, FootStepSounds.Num() - 1)	: 0;
	SetHitboxesEnabled(true);
}
bool AEnemyBase::IsAlive() const
{
	return HealthComponent && !HealthComponent->IsDead();
}
void AEnemyBase::SetMovementSpeed(float NewSpeed)
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = NewSpeed;
	}
}

void AEnemyBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	CollectHitboxes();

	GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECC_Pawn, bBlockPlayer ? ECR_Block : ECR_Ignore);
	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &AEnemyBase::HandleDeath);
	}
}
void AEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	Super::EndPlay(EndPlayReason);
}
//hitbox collision section
////
void AEnemyBase::CollectHitboxes()
{
	Hitboxes.Reset();
	TInlineComponentArray<UPrimitiveComponent*> Prims(this);

	for (UPrimitiveComponent* Prim : Prims)
	{
		if (!Prim || Prim->GetCollisionProfileName() != HitboxProfileName)
		{
			continue;
		}
		Prim->SetGenerateOverlapEvents(false);
		Prim->SetCanEverAffectNavigation(false);
		Prim->CanCharacterStepUpOn = ECB_No;

		Hitboxes.Add(Prim);
	}
	if (Hitboxes.Num() > 0)
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Weapon, ECR_Ignore);
	}
}

void AEnemyBase::SetHitboxesEnabled(bool bEnabled)
{
	const ECollisionEnabled::Type Mode =
		bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision;

	for (UPrimitiveComponent* Box : Hitboxes)
	{
		if (Box)
		{
			Box->SetCollisionEnabled(Mode);
		}
	}
}
AActor* AEnemyBase::GetAttackTarget() const
{
	return UGameplayStatics::GetPlayerPawn(this, 0);
}
void AEnemyBase::FaceTarget(const AActor* Target)
{
	if (!Target)
	{
		return;
	}
	FRotator NewRot = (Target->GetActorLocation() - GetActorLocation()).Rotation();
	NewRot.Pitch = 0.f;
	NewRot.Roll = 0.f;

	SetActorRotation(NewRot);
}
float AEnemyBase::EnemyAttack() {
	if (bFaceTargetOnAttack)
	{
		FaceTarget(GetAttackTarget());
	}
	PlayRandomSFX(AttackSounds);
	if (!AttackMontage1 && !AttackMontage2) {
		GetWorldTimerManager().SetTimer(AttackHitTimerHandle, this, &AEnemyBase::AttackHitCheck, AttackPreDelay, false);
		return AttackPreDelay + 0.5f;
	}
	float Duration = 0;
	float Random = FMath::FRandRange(0.f, 1.f);
	if (Random <= 0.5f) {
		if (AttackMontage1)
		{
			Duration = PlayAnimMontage(AttackMontage1);
		}
		else
		{
			Duration = PlayAnimMontage(AttackMontage2);
		}
		
	}
	if (Random  > 0.5f) {
		if (AttackMontage2)
		{
			Duration = PlayAnimMontage(AttackMontage2);
		}
		else
		{
			Duration = PlayAnimMontage(AttackMontage1);
		}
	}
	return Duration > 0.0f ? Duration : 1.0f;
}

void AEnemyBase::TakeHit(float Damage, EWeaponType Weapon)
{
	if (ImmuneMelee && Weapon == EWeaponType::Nail)
	{
		PlaySFX(HitbyMeleeSound);
		return;
	}
	if (ImmuneRange && Weapon == EWeaponType::Shotgun)
	{
		PlaySFX(HitbyRangeSound);
		return;
	}
	if (Damage <= 0.f || !IsAlive())
	{
		return;
	}

	HealthComponent->ApplyDamage(Damage, Weapon);
	PlayHitVFX();
	if (!IsAlive())
	{
		return;
	}
	switch (Weapon)
	{
	case EWeaponType::Nail:    PlaySFX(HitbyMeleeSound); break;
	case EWeaponType::Shotgun: PlaySFX(HitbyRangeSound); break;
	default: break;
	}
	if (Damage > StaggerDamage) 
	{
		ApplyStagger();
	}
}
void AEnemyBase::DisableAllCollision()
{
	TInlineComponentArray<UPrimitiveComponent*> Prims(this);
	for (UPrimitiveComponent* Prim : Prims)
	{
		if (Prim)
		{
			Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Prim->SetGenerateOverlapEvents(false);
		}
	}
}
void AEnemyBase::HandleDeath()
{

	if (bDeathHandled) { return; }

	bDeathHandled = true;
	SetCanBeDamaged(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DisableAllCollision();
	SetHitboxesEnabled(false);
	PlaySFX(DeathHitSound);
	ApplyDeathMovement();
	if (UCosGameInstance* GameInstance = Cast<UCosGameInstance>(GetWorld()->GetGameInstance()))
	{ 
		GameInstance->AddSoul(SoulAmount); 
	}
	OnEnemyKilled.Broadcast(this);
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		AI->StopMovement();
		if (AI->BrainComponent)
		{
			AI->BrainComponent->StopLogic(TEXT("Death"));
		}
	}

	//Reset all timers
	GetWorldTimerManager().ClearTimer(AttackHitTimerHandle);
	GetWorldTimerManager().ClearTimer(StaggerTimerHandle);

	PlaySFX(DeathSound);
	if (DeathMontage)
	{
		 PlayAnimMontage(DeathMontage);
	
	}
	if (EnchantPickupClass)
	{
		if (FMath::FRand() <= 0.02f)
		{
			GetWorld()->SpawnActor<AEnchantPickup>(EnchantPickupClass, GetDropLocation(), FRotator::ZeroRotator);
		}
	}
	GetWorldTimerManager().SetTimer(DeathTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]() { Destroy(); }),
		DeathDelay, false);
}
void AEnemyBase::ApplyDeathMovement()
{
//GetCharacterMovement()->DisableMovement();
//GetCharacterMovement()->SetComponentTickEnabled(false);
//GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UCharacterMovementComponent* Movement = GetCharacterMovement();

	Movement->SetAvoidanceEnabled(false);
	Movement->StopMovementImmediately();
	Movement->DisableMovement();
	Movement->SetComponentTickEnabled(false);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void AEnemyBase::AttackHitCheck()
{
	if (!IsAlive())
	{
		return;
	}

	//set radius
	const FVector Start = GetActorLocation() + GetActorForwardVector() * AttackOffset + FVector(0.f, 0.f, AttackHeightOffset);
	// const FVector End = Start + GetActorForwardVector() * AttackRange;
	TArray<FOverlapResult> AttackHits;
	//ignore self
	FCollisionQueryParams Params(SCENE_QUERY_STAT(DetectPlayer), false, this);
	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_Pawn);

	const bool bHit = GetWorld()->OverlapMultiByObjectType(AttackHits, Start, FQuat::Identity, ObjParams,FCollisionShape::MakeSphere(AttackRadius), Params);
	// for DEBUUYG
//#if ENABLE_DRAW_DEBUG
//	DrawDebugSphere(GetWorld(), Start, AttackRadius, 12,
//		bHit ? FColor::Red : FColor::Green, false, 1.0f);
//#endif
	//no hit -> return
	if (!bHit)
	{
		return;
	}
	//Only hit Tagged "Player"
	for (const FOverlapResult& Hit : AttackHits)
	{
		AActor* Target = Hit.GetActor();
		if (!Target || !Target->ActorHasTag(TEXT("Player")))
		{
			continue;
		}
		if (IDamageable* bonk = Cast<IDamageable>(Target))
		{
			bonk->TakeHit(AttackDamage, EWeaponType::None);
		}
		break;
	}
}

void AEnemyBase::ApplyStagger()
{
	if (!IsAlive())
	{
		return;
	}
	const float Now = GetWorld()->GetTimeSeconds();
	if (LastStaggerTime >= 0.f && (Now - LastStaggerTime) < StaggerDelay)
	{
		return;
	}
	LastStaggerTime = Now;
	bIsStagger = true;
//stop animation
	StopAnimMontage();
	GetWorldTimerManager().ClearTimer(AttackHitTimerHandle);
	//stop movement
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		AI->StopMovement();
	}
	SetStaggerBlackboard(true);
	GetCharacterMovement()->StopMovementImmediately();
	//play hit React
	if (HitReactMontage)
	{
		PlayAnimMontage(HitReactMontage);
	}
	GetWorldTimerManager().SetTimer(StaggerTimerHandle,	FTimerDelegate::CreateWeakLambda(this, [this]()	
		{
			bIsStagger = false;
			SetStaggerBlackboard(false);
		}),
		StaggerDuration, false);
}

void AEnemyBase::OnMovementModeChanged(EMovementMode PrevMode, uint8 PrevCustomMode)
{
	Super::OnMovementModeChanged(PrevMode, PrevCustomMode);

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement) { return; }

	if (Movement->NavAgentProps.bCanFly) 
	{
		return; 
	}

	if (Movement->MovementMode == MOVE_Falling)
	{
		Movement->Velocity.Z = 0.f;
		Movement->SetMovementMode(MOVE_NavWalking);
	}
}
FVector AEnemyBase::GetDropLocation() const
{
	return GetActorLocation();
}
FVector AEnemyBase::GetMuzzleLocation() const
{
	const FVector Forward = GetActorForwardVector();

	return GetActorLocation()+ Forward * MuzzleForwardOffset + FVector(0.f, 0.f, MuzzleHeightOffset);
}
FVector AEnemyBase::GetAimLocation() const
{
	if (const AActor* Target = GetAttackTarget())
	{
		return Target->GetActorLocation() + FVector(0.f, 0.f, AimHeightOffset);
	}
	return GetMuzzleLocation() + GetActorForwardVector() * 1000.f;
}
void AEnemyBase::PlaySFXAt(const UObject* WorldContext, const FSFXVolume& SFX, const FVector& Location)
{
	if (!SFX.Sound || !WorldContext)
	{
		return;
	}

	const float FinalPitch = SFX.Pitch * (1.f + FMath::FRandRange(-SFX.RandomPitch, SFX.RandomPitch));


	UGameplayStatics::PlaySoundAtLocation(WorldContext,SFX.Sound,Location,FRotator::ZeroRotator,SFX.Volume,	FinalPitch,	0.f,nullptr,SFX.Concurrency,Cast<AActor>(WorldContext));  
}

void AEnemyBase::PlaySFX(const FSFXVolume& SFX)
{
	PlaySFXAt(this, SFX, GetActorLocation());
}
void AEnemyBase::SetStaggerBlackboard(bool bValue)
{
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AI->GetBlackboardComponent())
		{
			BB->SetValueAsBool(StaggerKeyName, bValue);
		}
	}
}

void AEnemyBase::FireProjectile(float InSpeed, float InDamage)
{
	FireProjectile2(InSpeed, InDamage, 0.f, ProjectileClass);
}
void AEnemyBase::FireProjectile2(float InSpeed, float InDamage, float YawOffset, TSubclassOf<AEnemyProjectile> SelectedProjectile) {
	const TSubclassOf<AEnemyProjectile> ProjectilType = SelectedProjectile ? SelectedProjectile : ProjectileClass;
	if (!ProjectilType)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || !GetAttackTarget())
	{
		return;
	}

	const FVector Muzzle = GetMuzzleLocation();  
	const FVector AimPoint = GetAimLocation();   

	FRotator FireRot = (AimPoint - Muzzle).Rotation();
	FireRot.Yaw += YawOffset;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AEnemyProjectile* Projectile =
		World->SpawnActor<AEnemyProjectile>(ProjectilType, Muzzle, FireRot, SpawnParams))
	{
		Projectile->InitProjectile(InDamage, InSpeed);
	}
}
void AEnemyBase::PlayRandomSFX(const TArray<FSFXVolume>& Sounds)
{
	const int32 Num = Sounds.Num();
	if (Num == 0)
	{
		return;
	}
	const int32 Start = FMath::RandRange(0, Num - 1);
	for (int32 i = 0; i < Num; ++i)
	{
		const FSFXVolume& SFX = Sounds[(Start + i) % Num];
		if (SFX.Sound)
		{
			PlaySFX(SFX);   
			return;
		}
	}
}
void AEnemyBase::PlayFootstep()
{
	if (FootStepSounds.Num() == 0)
	{
		return;
	}
	const AActor* Target = GetAttackTarget();
	if (!Target ||
		FVector::DistSquared(Target->GetActorLocation(), GetActorLocation())
	> FMath::Square(FootstepAudibleDistance))
	{
		return;
	}

	PlaySequentialSFX(FootStepSounds, FootstepIndex);
}
void AEnemyBase::PlaySequentialSFX(const TArray<FSFXVolume>& Sounds, int32& InOutIndex)
{
	const int32 Num = Sounds.Num();
	if (Num == 0)
	{
		return;
	}
	for (int32 i = 0; i < Num; i++)
	{
		const int32 Index = InOutIndex % Num;
		InOutIndex = (Index + 1) % Num;

		if (Sounds[Index].Sound)
		{
			PlaySFX(Sounds[Index]);
			return;
		}
	}
}
void AEnemyBase::PlayHitVFX()
{
	if (!HitVFX)
	{
		return;
	}
	FVector Loc = GetActorLocation();
	FVector ToAttacker = GetActorForwardVector();

	if (const AActor* Attacker = GetAttackTarget())
	{
		const FVector Dir = (Attacker->GetActorLocation() - Loc).GetSafeNormal2D();
		if (!Dir.IsNearlyZero())
		{
			ToAttacker = Dir;
		}
	}

	if (const UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Loc += ToAttacker * Capsule->GetScaledCapsuleRadius();
	}
	Loc.Z += HitVFXHeightOffset;

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this,HitVFX,Loc,ToAttacker.Rotation(),HitVFXScale,true,true,	ENCPoolMethod::AutoRelease,true);                        
}