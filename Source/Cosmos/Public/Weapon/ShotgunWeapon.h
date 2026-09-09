#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "ShotgunWeapon.generated.h"

UCLASS()
class COSMOS_API AShotgunWeapon : public AWeaponBase
{
	GENERATED_BODY()
	
public:
	AShotgunWeapon();
	void ApplyUpgrade();
	void Reload();

protected:
	virtual void PerformAttack() override;
	virtual float GetCurrentDamage() const override;

private:
	int32 MaxAmmo;
	int32 CurrentAmmo;
	float ReloadTime;
};
