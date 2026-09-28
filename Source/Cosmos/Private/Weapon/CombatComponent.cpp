#include "Weapon/CombatComponent.h"
#include "Weapon/NailWeapon.h"
#include "Weapon/ShotgunWeapon.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"                    
#include "GameFramework/CharacterMovementComponent.h"   
#include "Components/ChildActorComponent.h"
#include "GameFramework/PlayerController.h"
#include "Character/HealthComponent.h"
#include "Data/CosGameInstance.h"
#include "Weapon/CosLegacyCameraShake.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	DodgeCameraShake = UCosLegacyCameraShake::StaticClass(); // 회피 기본 흔들림
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay(); //부모가 해둔 초기화가 있을 수 있으니 관례적 호출

	PotionCount = GetMaxPotionCount();
	OnPotionCountChanged.Broadcast(PotionCount);

	APawn* OwnerPawn = Cast<APawn>(GetOwner()); // 이 컴포넌트가 붙어있는 캐릭터를 가져옴 입력 컴포넌트에 접근하기 위해
	if (!IsValid(OwnerPawn))
	{
		return;
	}

	// [회피] 이동 컴포넌트 접근용 캐릭터 캐싱 + 기본 마찰값 저장
	OwnerCharacter = Cast<ACharacter>(OwnerPawn);
	if (IsValid(OwnerCharacter))
	{
		UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
		if (IsValid(MoveComp))
		{
			DefaultGroundFriction = MoveComp->GroundFriction;
			DefaultBrakingDeceleration = MoveComp->BrakingDecelerationWalking;
		}
	}

	TArray<UChildActorComponent*> ChildActorComponents; // 배열 생성
	OwnerPawn->GetComponents<UChildActorComponent>(ChildActorComponents); // 캐릭터가 가진 child컴포넌트를 배열에 담기 ,GetComponents는 결과를 인자에 넣음.

	for (UChildActorComponent* ChildActorComp : ChildActorComponents) // 배열을 인자를 하나씩 비교함
	{
		if (!IsValid(ChildActorComp)) //방어코드
		{
			continue;
		}

		AActor* ChildActor = ChildActorComp->GetChildActor(); // 실제 액터 꺼내서 붙이기

		if (ANailWeapon* Nail = Cast<ANailWeapon>(ChildActor)) // ChildActor가 ANailWeapon 타입이라면 ANailWeapon 타입 포인터를 돌려줌. 그렇기때문에 맞다면 if 실행
		{
			NailWeapon = Nail;
		}
		else if (AShotgunWeapon* Shotgun = Cast<AShotgunWeapon>(ChildActor))
		{
			ShotgunWeapon = Shotgun;
		}
	}

	bCombatReady = true;       // 포션, 무기 초기화 완료
	OnCombatReady.Broadcast(); // 기다리던 쪽(컨트롤러)에 준비 완료 알림

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()); // Pawn 조종하는 컨트롤러 가져오기
	if (!IsValid(PC))
	{
		return;
	}

	// IMC 관리하는 시스템 가져오기
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		if (IsValid(CombatMappingContext)) // IMC 지정되어 있는지.
		{
			Subsystem->AddMappingContext(CombatMappingContext, 0); // IMC 등록
		}
	}

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(OwnerPawn->InputComponent)) // 입력을 Enhanced Input 타입으로 캐스팅
	{
		if (IsValid(NailAttackAction))// IA 지정되어 있는지.
		{
			EIC->BindAction(NailAttackAction, ETriggerEvent::Started, this, &UCombatComponent::OnNailAttack); // 좌클릭시 대못공격
		}
		if (IsValid(ShotgunAttackAction))
		{
			EIC->BindAction(ShotgunAttackAction, ETriggerEvent::Started, this, &UCombatComponent::OnShotgunAttack); // 우클릭시 샷건공격
		}
		if (IsValid(ReloadAction))
		{
			EIC->BindAction(ReloadAction, ETriggerEvent::Started, this, &UCombatComponent::OnReload); // R 누르면 재장전
		}
		if (IsValid(UsePotionAction))
		{
			EIC->BindAction(UsePotionAction, ETriggerEvent::Started, this, &UCombatComponent::OnUsePotion); // E 누르면 포션
		}
		if (IsValid(DodgeAction)) // [회피]
		{
			EIC->BindAction(DodgeAction, ETriggerEvent::Started, this, &UCombatComponent::TryDodge); // Shift 누르면 회피
		}
	}
}

bool UCombatComponent::CanAttack() const // 장전중 공격 방지
{
	if (IsValid(ShotgunWeapon) && ShotgunWeapon->IsReloading())
	{
		return false;
	}

	return true;
}

// 공격하는 함수랑 연결
void UCombatComponent::OnNailAttack()
{

	if (!CanAttack())
	{
		return;
	}

	if (IsValid(NailWeapon))
	{
		NailWeapon->TryAttack();
	}
}

void UCombatComponent::OnShotgunAttack()
{
	if (!CanAttack())
	{
		return;
	}

	if (IsValid(ShotgunWeapon))
	{
		ShotgunWeapon->TryAttack();
	}
}

void UCombatComponent::OnReload()
{
	if (IsValid(ShotgunWeapon))
	{
		ShotgunWeapon->Reload();
	}
}

void UCombatComponent::OnUsePotion()
{
	UHealthComponent* HealthComponent = GetOwner()->FindComponentByClass<UHealthComponent>(); // HealthComponent 얻기
	if (!IsValid(HealthComponent) || HealthComponent->IsDead())
	{
		return;
	}
	if (PotionCount <= 0 || HealthComponent->GetCurrentHealth() >= HealthComponent->GetMaxHealth())
	{
		return;
	}
	UCosGameInstance* GI = Cast<UCosGameInstance>(GetWorld()->GetGameInstance());
	if (!IsValid(GI))
	{
		return;
	}

	PotionCount--;
	HealthComponent->Heal(static_cast<float>(GI->GetPotionHealAmount()));
	OnPotionCountChanged.Broadcast(PotionCount);
	OnPotionUsed.Broadcast();
}

int32 UCombatComponent::GetMaxPotionCount() const
{
	const UCosGameInstance* GI = Cast<UCosGameInstance>(GetWorld()->GetGameInstance());
	return GI ? GI->GetPotionMaxCount() : 0;
}

void UCombatComponent::RefillPotions()
{
	PotionCount = GetMaxPotionCount();
	OnPotionCountChanged.Broadcast(PotionCount);
}

// 누르고 있는 방향키 방향으로 순간 이동
void UCombatComponent::TryDodge()
{
	if (!IsValid(OwnerCharacter) || !bCanDodge || bIsDodging)
	{
		return;
	}

	UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
	if (!IsValid(MoveComp) || !MoveComp->IsMovingOnGround())
	{
		return;
	}

	FVector DodgeDirection = OwnerCharacter->GetLastMovementInputVector(); // 현재 방향키 입력 방향
	DodgeDirection.Z = 0.0f;

	if (DodgeDirection.IsNearlyZero()) // 입력 없으면 뒤로
	{
		DodgeDirection = -OwnerCharacter->GetActorForwardVector();
	}
	DodgeDirection.Normalize();

	MoveComp->GroundFriction = 0.0f;              // 회피 중 마찰 제거
	MoveComp->BrakingDecelerationWalking = 0.0f;

	OwnerCharacter->LaunchCharacter(DodgeDirection * DodgeStrength, true, false);
	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (IsValid(PC) && DodgeCameraShake != nullptr)
	{
		PC->ClientStartCameraShake(DodgeCameraShake, DodgeShakeScale);
	}
	if (IsValid(DodgeSound))
	{
		UGameplayStatics::PlaySound2D(this, DodgeSound, DodgeSoundVolume);
	}
	bIsDodging = true;
	bCanDodge = false;

	GetWorld()->GetTimerManager().SetTimer(
		DodgeTimerHandle, this, &UCombatComponent::EndDodge, DodgeDuration, false);
	GetWorld()->GetTimerManager().SetTimer(
		DodgeCooldownTimerHandle, this, &UCombatComponent::ResetDodgeCooldown, DodgeCooldown, false);
}

//  마찰 복구 + 속도 정리
void UCombatComponent::EndDodge()
{
	bIsDodging = false;

	if (!IsValid(OwnerCharacter))
	{
		return;
	}

	UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
	if (IsValid(MoveComp))
	{
		MoveComp->GroundFriction = DefaultGroundFriction;
		MoveComp->BrakingDecelerationWalking = DefaultBrakingDeceleration;
		MoveComp->Velocity = MoveComp->Velocity.GetClampedToMaxSize(MoveComp->MaxWalkSpeed);
	}
}

// 회피쿨타임 종료
void UCombatComponent::ResetDodgeCooldown()
{
	bCanDodge = true;
}