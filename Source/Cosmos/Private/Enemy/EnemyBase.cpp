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
	Movement->AvoidanceConsiderationRadius = 150.0f; //No Clip Range
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
	if (HealthComponent)
	{
		//	HealthComponent->OnDeath.AddDynamic(this, &EnemyBase::HandleDeath);
	}
	else
	{
		return;
	}
}

// Called every frame


void AEnemyBase::SetMovementSpeed(float NewSpeed)
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = NewSpeed;
	}
}


//void AEnemyBase::PostInitializeComponents()
//{
//	Super::PostInitializeComponents();
//}

void AEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	Super::EndPlay(EndPlayReason);
}

void AEnemyBase::HandleDeath()
{

}
float AEnemyBase::EnemyAttack() {
	if (!AttackMontage) {
		return 0.3f;
	}
	const float Duration = PlayAnimMontage(AttackMontage);
	return Duration > 0.0f ? Duration : 1.0f;
}
// AEnemyBase.cpp
void AEnemyBase::TakeHit(float Damage, EWeaponType Weapon)
{

}