#include "Character/CosCharacter.h"
#include "UI/CosPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"// [추가] IMC 등록용 서브시스템
#include "Character/HealthComponent.h"
#include "Weapon/ShotgunWeapon.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ACosCharacter::ACosCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));

	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.0f;
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	NormalSpeed = 600.0f;
	SprintSpeedMultiplier = 1.5f;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
}

void ACosCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// =========================================================================
	// [추가] 테스트용 IMC(Input Mapping Context) 등록
	// =========================================================================
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultIMC)
			{
				Subsystem->AddMappingContext(DefaultIMC, 0);
			}
		}
	}

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		/* =====================================================================
		   [기존 코드 주석 처리] - 해주 님 ACosPlayerController 연동 완료 후 복구용
		========================================================================
		if (ACosPlayerController* PlayerController = Cast<ACosPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&ACosCharacter::Move
				);
			}
			if (PlayerController->JumpAction)
			{
				EnhancedInput->BindAction(
					PlayerController->JumpAction,
					ETriggerEvent::Triggered,
					this,
					&ACosCharacter::StartJump
				);
				EnhancedInput->BindAction(
					PlayerController->JumpAction,
					ETriggerEvent::Completed,
					this,
					&ACosCharacter::StopJump
				);

			}
			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&ACosCharacter::Look
				);
			}
			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Triggered,
					this,
					&ACosCharacter::StartSprint
				);
				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Completed,
					this,
					&ACosCharacter::StopSprint
				);
			}
====================================================================== */

// =========================================================================
// [현재 작동 코드] 테스트용 임시 바인딩
// =========================================================================
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACosCharacter::Move);
		}
		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACosCharacter::StartJump);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACosCharacter::StopJump);
		}
		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACosCharacter::Look);
		}
		if (SprintAction)
		{
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &ACosCharacter::StartSprint);
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &ACosCharacter::StopSprint);
		}//나중에 지워야함
	}
}
void ACosCharacter::Move(const FInputActionValue& Value)
{
	if (!Controller) return;

	const FVector2D MoveInput = Value.Get<FVector2D>();

	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddMovementInput(GetActorForwardVector(), MoveInput.X);
	}

	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddMovementInput(GetActorRightVector(), MoveInput.Y);
	}
}
void ACosCharacter::StartJump(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		Jump();
	}
}
void ACosCharacter::StopJump(const FInputActionValue& Value)
{
	if (!Value.Get<bool>())
	{
		StopJumping();
	}
}
void ACosCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookInput = Value.Get<FVector2D>();

	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
}
void ACosCharacter::StartSprint(const FInputActionValue& Value)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = NormalSpeed * SprintSpeedMultiplier;
	}
}
void ACosCharacter::StopSprint(const FInputActionValue& Value)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	}
}

float ACosCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	// 실제 피해량 계산
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// HealthComponent를 찾아서 데미지 적용
	if (HealthComponent)//수정 : 멤버로 이미 갖고 있으니 바로 접근
	{
		HealthComponent->ApplyDamage(ActualDamage);
	}

	return ActualDamage;
}

AShotgunWeapon* ACosCharacter::GetEquippedWeapon() const
{
	return EquippedWeapon;
}