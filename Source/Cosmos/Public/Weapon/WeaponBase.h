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
	bool ApplyDamage(AActor* HitActor, const FHitResult& HitResult); // 데미저블 판단을 위해 bool형

protected:

	UPROPERTY(VisibleAnyWhere, Category = "Weapon")
	UStaticMeshComponent* WeaponMesh;

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
