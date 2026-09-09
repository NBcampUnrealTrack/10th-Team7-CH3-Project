#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

UCLASS()
class COSMOS_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeaponBase();
	
	void TryAttack();
	void ApplyDamage(AActor* HitActor, const FHitResult& HitResult);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float BaseDamage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float AttackInterval;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float AttackRange;

	float LastAttackTime;

	virtual void PerformAttack() PURE_VIRTUAL(AWeaponBase::PerformAttack, );
	virtual float GetCurrentDamage() const PURE_VIRTUAL(AWeaponBase::GetCurrentDamage, return 0.f;);
	virtual void ApplyEnchant() PURE_VIRTUAL(AWeaponBase::ApplyEnchant, );
};
