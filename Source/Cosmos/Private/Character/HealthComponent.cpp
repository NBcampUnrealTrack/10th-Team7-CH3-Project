#include "Character/HealthComponent.h"
#include "Data/CosGameInstance.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	BaseMaxHealth = 100.0f;
	MaxHealth = BaseMaxHealth;
	CurrentHealth = MaxHealth;
}

void UHealthComponent::ApplyDamage(float Damage, EWeaponType Weapon)
{
	if (Damage <= 0.0f || IsDead())//이상한 데미지 방지로, 데미지가 0 이하거나 죽었으면 끝
	{
		return;
	}

	CurrentHealth = FMath::Clamp(//체력 감소(Clamp로 인해 최소 =0, 최대=MaxHealth로 제한해주기)
		CurrentHealth - Damage,
		0.0f,
		MaxHealth
	);

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (IsDead())
	{
		OnDeath.Broadcast();
	}
}

void UHealthComponent::Heal(float Amount)
{
	if (Amount <= 0.0f || IsDead())//이상한 힐 방지. 힐이 0이거나 죽었으면 끝
	{
		return;
	}

	CurrentHealth = FMath::Clamp(//Clamp로 최소, 최대 정함
		CurrentHealth + Amount,
		0.0f,
		MaxHealth
	);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

bool UHealthComponent::IsDead() const
	{
		return CurrentHealth <= 0.0f;
	}
void UHealthComponent::SetHPAtStart(float MaxHP)
{
	MaxHealth = MaxHP;
	CurrentHealth = MaxHealth;
}

void UHealthComponent::ApplyMaxHealthBonus(float Bonus)
{
	const float OldMaxHealth = MaxHealth;
	MaxHealth = BaseMaxHealth + Bonus;

	// 체력 변화량 맡큼 현재 체력도 같이 변화해줌
	CurrentHealth = FMath::Clamp(CurrentHealth + (MaxHealth - OldMaxHealth), 0.0f, MaxHealth);

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}