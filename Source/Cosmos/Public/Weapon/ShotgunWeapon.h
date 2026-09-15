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
	void Reload();
	void FinishReload();

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsReloading() const { return bIsReloading; }
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void OnReloadStarted();
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void OnReloadFinished();

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetCurrentMaxAmmo() const;
	int32 GetCurrentAmmo() const;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnAmmoChanged OnAmmoChanged;

protected:
	virtual void BeginPlay() override;
	virtual bool PerformAttack() override;
	virtual EWeaponType GetWeaponType() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 MaxAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 CurrentAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float ReloadTime;
	bool bIsReloading = false;
	FTimerHandle ReloadTimerHandle;

	virtual EEnchantStat GetSpeedStatType() const override { return EEnchantStat::RangeFiringRate; }
	virtual EEnchantStat GetDamageAddStatType() const override { return EEnchantStat::RangeDamageAdd; }
	virtual EEnchantStat GetDamageMultiStatType() const override { return EEnchantStat::RangeDamageMulti; }

	float GetCurrentReloadTime() const;

	UFUNCTION()
	void OnLoadoutChanged();
};
