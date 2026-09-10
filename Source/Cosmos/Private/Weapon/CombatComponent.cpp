#include "Weapon/CombatComponent.h"
#include "Weapon/NailWeapon.h"
#include "Weapon/ShotgunWeapon.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::BeginPlay()
{
	
}


void UCombatComponent::OnNailAttack()
{
	if (NailWeapon)
	{
		NailWeapon->TryAttack();
	}
}

void UCombatComponent::OnShotgunAttack()
{
	if (ShotgunWeapon)
	{
		ShotgunWeapon->TryAttack();
	}
}