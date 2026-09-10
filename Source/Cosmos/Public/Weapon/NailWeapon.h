#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "NailWeapon.generated.h"

UCLASS()
class COSMOS_API ANailWeapon : public AWeaponBase
{
	GENERATED_BODY()
	
public:
	ANailWeapon();

protected:
	virtual void PerformAttack() override;
	virtual float GetCurrentDamage() const override;
	virtual void ApplyEnchant() override;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 MaxTargetPerSwing;
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Weapon")
	float SwingRadius = 50.f;
};
