#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "ShotgunWeapon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmmoChanged, int32, NewAmmo);

UCLASS()
class COSMOS_API AShotgunWeapon : public AWeaponBase
{
	GENERATED_BODY()
	
public:
	AShotgunWeapon();
	void ApplyUpgrade();
	void Reload();
	void FinishReload();

	int32 GetCurrentAmmo() const;

	FORCEINLINE int32 GetMaxAmmo() const { return MaxAmmo; }

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnAmmoChanged OnAmmoChanged;

protected:
	virtual void PerformAttack() override;
	virtual float GetCurrentDamage() const override;
	virtual EWeaponType GetWeaponType() const override;
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
