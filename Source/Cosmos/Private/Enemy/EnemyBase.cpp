#include "Enemy/EnemyBase.h"
#include "Character/HealthComponent.h"
#include "Enemy/EnemyAIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "BrainComponent.h" 
#include "TimerManager.h"
#include "Engine/World.h"
#include "Animation/AnimMontage.h"
#include "Kismet/GameplayStatics.h"
//for debug
#include "DrawDebugHelpers.h" 
#include "Engine/OverlapResult.h" //  FOverlapResult 

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
	//for move / 움직임 기본 세팅
	Movement->MaxWalkSpeed = RunSpeed;
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	//for Only walk on Nav place / Nav 위에서만 움직이게 
	Movement->SetMovementMode(MOVE_NavWalking);
	// no clip each enemy, / 적들이 알아서 비켜가게
	Movement->bUseRVOAvoidance = true;
	Movement->AvoidanceConsiderationRadius = 125.0f; //No Clip Range
	//DIsable unnecessary Functions / 이동 방향으로 알아서 회전
	Movement->bUseControllerDesiredRotation = false;
	Movement->bOrientRotationToMovement = true;
	//disable unused function in cmc / 안쓰는 기능 꺼서 계산 줄이기
	Movement->GetNavAgentPropertiesRef().bCanCrouch = false;
	Movement->GetNavAgentPropertiesRef().bCanFly = false;
	Movement->GetNavAgentPropertiesRef().bCanJump = false;
	Movement->GetNavAgentPropertiesRef().bCanSwim = false;
	//Disable Capsule Collision Overlap Each Monsters. ->  Use RVOAvoidance up there / 콜리전 계산 중지, 겹치기 방지는 위에 RVO가 함
	Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	Capsule->SetGenerateOverlapEvents(false);
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
	else
	{
		return;
	}
}

void AEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	Super::EndPlay(EndPlayReason);
}

float AEnemyBase::EnemyAttack() {
	if (!AttackMontage) {
		GetWorldTimerManager().SetTimer(AttackHitTimerHandle, this, &AEnemyBase::AttackHitCheck, AttackPreDelay, false);
		return AttackPreDelay + 0.5f;
	}
	const float Duration = PlayAnimMontage(AttackMontage);
	return Duration > 0.0f ? Duration : 1.0f;
}

void AEnemyBase::TakeHit(float Damage, EWeaponType Weapon)
{
	if (Damage <= 0.f || !HealthComponent || HealthComponent->IsDead())
	{
		return;
	}
	HealthComponent->ApplyDamage(Damage);
	if (HealthComponent->IsDead())
	{
		return;
	}
	if (HitSound)
	{
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}
	if (HitReactMontage)
	{
		PlayAnimMontage(HitReactMontage);
	}

}

void AEnemyBase::HandleDeath()
{
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		AI->StopMovement();
	}
	GetCharacterMovement()->DisableMovement();
	SetActorEnableCollision(false);

	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}
	GetWorldTimerManager().SetTimer(DeathTimerHandle, [this]() { Destroy(); }, DeathDelay, false);
}

void AEnemyBase::AttackHitCheck()
{
	if (HealthComponent && HealthComponent->IsDead())
	{
		return;
	}
	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());
	}
	//set radius
	const FVector Start = GetActorLocation() + GetActorForwardVector() * AttackOffset;
	// const FVector End = Start + GetActorForwardVector() * AttackRange;
	TArray<FOverlapResult> AttackHits;
	//ignore self
	FCollisionQueryParams Params(SCENE_QUERY_STAT(DetectPlayer), false, this);

	const bool bHit = GetWorld()->OverlapMultiByChannel(AttackHits, Start, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(AttackRange), Params);
	// for DEBUUYG
#if ENABLE_DRAW_DEBUG
	DrawDebugSphere(GetWorld(), Start, AttackRange, 12,
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
