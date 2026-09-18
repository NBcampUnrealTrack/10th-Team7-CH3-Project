#include "Enemy/EnemyBase.h"
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
	Movement->AvoidanceConsiderationRadius = 150.0f; //No Clip Range
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

	Movement->bUseFlatBaseForFloorChecks = true;
	Movement->MaxStepHeight = 25.f;
	//avoid monster pop upto the sky
	Movement->MaxDepenetrationWithPawn = 30.f;
	Movement->MaxDepenetrationWithGeometry = 100.f;
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
	if (Damage <= 0.f || !IsAlive())
	{
		return;
	}
	HealthComponent->ApplyDamage(Damage, EWeaponType::None);
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
	ApplyStagger();

}

void AEnemyBase::HandleDeath()
{
	PlaySFX(DeathHitSound);
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
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->SetComponentTickEnabled(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision); 
	//Reset all timers
	GetWorldTimerManager().ClearTimer(AttackHitTimerHandle);
	GetWorldTimerManager().ClearTimer(StaggerTimerHandle);


	if (DeathMontage)
	{
		 PlayAnimMontage(DeathMontage);
		 PlaySFX(DeathSound);
	}
	UWorld* World = GetWorld();
	if (World)
	{
		World->SpawnActor<AEnchantPickup>(EnchantPickupClass, GetActorLocation(), FRotator::ZeroRotator);
	}
	GetWorldTimerManager().SetTimer(DeathTimerHandle, [this]() { Destroy(); }, DeathDelay, false);
}

void AEnemyBase::AttackHitCheck()
{
	if (!IsAlive())
	{
		return;
	}
	PlaySFX(AttackSound);
	//set radius
	const FVector Start = GetActorLocation() + GetActorForwardVector() * AttackOffset;
	// const FVector End = Start + GetActorForwardVector() * AttackRange;
	TArray<FOverlapResult> AttackHits;
	//ignore self
	FCollisionQueryParams Params(SCENE_QUERY_STAT(DetectPlayer), false, this);

	const bool bHit = GetWorld()->OverlapMultiByChannel(AttackHits, Start, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(AttackRadius), Params);
	// for DEBUUYG
#if ENABLE_DRAW_DEBUG
	DrawDebugSphere(GetWorld(), Start, AttackRadius, 12,
		bHit ? FColor::Red : FColor::Green, false, 1.0f);
#endif
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
		if (UBlackboardComponent* BB = AI->GetBlackboardComponent())
		{
			BB->SetValueAsBool(StaggerKeyName, true);
		}
	}
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
void AEnemyBase::PlaySFX(const FSFXVolume& SFX)
{
	if (!SFX.Sound)
	{
		return;
	}
	const float FinalResult = SFX.Pitch * (1.f + FMath::FRandRange(-SFX.RandomPitch, SFX.RandomPitch));
	UGameplayStatics::PlaySoundAtLocation(this, SFX.Sound, GetActorLocation(), SFX.Volume, FinalResult);
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
