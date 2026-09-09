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
	void FinishReload();

protected:
	virtual void PerformAttack() override;
	virtual float GetCurrentDamage() const override;
	virtual void ApplyEnchant() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 MaxAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 CurrentAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float ReloadTime;

	bool bIsReloading = false;

	FTimerHandle ReloadTimerHandle;

};
