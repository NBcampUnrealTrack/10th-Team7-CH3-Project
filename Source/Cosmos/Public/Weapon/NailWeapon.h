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

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsSwingLeftToRight() const { return bSwingLeftToRight; }
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetComboWindow() const { return GetCurrentAttackInterval() + ComboWindowBonus; }

protected:
	virtual bool PerformAttack() override;
	virtual EWeaponType GetWeaponType() const override;
	virtual EEnchantStat GetSpeedStatType() const override { return EEnchantStat::MeleeSpeed; }
	virtual EEnchantStat GetDamageAddStatType() const override { return EEnchantStat::MeleeDamageAdd; }
	virtual EEnchantStat GetDamageMultiStatType() const override { return EEnchantStat::MeleeDamageMulti; }

	int32 GetCurrentMaxTarget() const;
	float GetCurrentRange() const;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 MaxTargetPerSwing;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float SwingRadius;

	// 공격 모션을 위한 것들
	void UpdateSwingDirection();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float ComboWindowBonus;
	bool bSwingLeftToRight;
	float LastSwingTime;
};
