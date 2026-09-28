#include "Character/CosCharacter.h"
#include "UI/CosPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"// [추가] IMC 등록용 서브시스템
#include "Character/HealthComponent.h"
#include "Weapon/ShotgunWeapon.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapon/CombatComponent.h"
#include "Data/CosGameInstance.h"
#include "Kismet/GameplayStatics.h"

ACosCharacter::ACosCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(RootComponent);
	CameraComp->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
	CameraComp->bUsePawnControlRotation = true;

	NormalSpeed = 600.0f;
	SprintSpeedMultiplier = 1.5f;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
}

void ACosCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{

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
					ETriggerEvent::Started,
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
		/*	if (PlayerController->SprintAction)
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
			}*/
		}
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

void ACosCharacter::TakeHit(float Damage, EWeaponType Weapon)
{
	// Honor Unreal's damage flag (including the development God command).
	if (!CanBeDamaged()) return;
	if (HealthComponent)
	{
		HealthComponent->ApplyDamage(Damage, Weapon);
	}
	if (DamageShake && Damage > 0.f)
	{
		PlaySFXPlayer(HitReactSound);
		const float Scale = FMath::Clamp(Damage / DamageShakeReference, 0.4f, 2.f);
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->ClientStartCameraShake(DamageShake, Scale);
		}
		PlayDamagedScreen(Damage);
	}
}


void ACosCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		GI->OnLoadoutChange.AddDynamic(this, &ACosCharacter::OnLoadoutChanged);
	}

	OnLoadoutChanged();
}

void ACosCharacter::OnLoadoutChanged()
{
	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		if (HealthComponent)
		{
			HealthComponent->ApplyMaxHealthBonus(GI->GetTotalStat(EEnchantStat::MaxHealth));
		}
	}

	RecalculateMovementSpeed();
	UE_LOG(LogTemp, Warning, TEXT("MaxHealth=%.1f, WalkSpeed=%.1f"), HealthComponent->GetMaxHealth(), GetCharacterMovement()->MaxWalkSpeed);
}

void ACosCharacter::RecalculateMovementSpeed()
{
	if (!GetCharacterMovement()) return;

	UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance());
	const float SpeedBonus = GI ? GI->GetTotalStat(EEnchantStat::MovementSpeed) : 0.0f;

	const float CurrentBaseSpeed = NormalSpeed + SpeedBonus;

	GetCharacterMovement()->MaxWalkSpeed = CurrentBaseSpeed;
}

void ACosCharacter::PlaySFXPlayer(const FSFXVolume& SFX) const
{
	if (!SFX.Sound)
	{
		return;
	}
	const float FinalPitch = SFX.Pitch * (1.f + FMath::FRandRange(-SFX.RandomPitch, SFX.RandomPitch));
	UGameplayStatics::PlaySound2D(this,	SFX.Sound,SFX.Volume,FinalPitch,0.f,SFX.Concurrency,this,false);
}
void ACosCharacter::PlayDamagedScreen(float Damage)
{
	if (Damage <= 0.f)
	{
		return;
	}
	const float Intensity = FMath::Clamp(Damage / DamagedScreenAmount, 0.2f, 1.f);
	OnPlayerDamaged.Broadcast(Intensity);
}
