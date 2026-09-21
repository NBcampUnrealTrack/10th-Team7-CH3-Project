#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "ShotgunWeapon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChanged, int32, NewAmmo, int32, MaxAmmo);

UCLASS()
class COSMOS_API AShotgunWeapon : public AWeaponBase
{
	GENERATED_BODY()
	
public:
	AShotgunWeapon();
	void Reload();
	void FinishReload();
	
	UPROPERTY(BlueprintAssignable, Category = "Weapon") //델리게이트
	FOnAmmoChanged OnAmmoChanged;

	//장전모션 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void OnReloadStarted();
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void OnReloadFinished();
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsReloading() const { return bIsReloading; } // 재장전동안 모션 안나가도록.

	//Getter. UI에서 써야해서 public
	UFUNCTION(BlueprintPure, Category = "Weapon") 
	int32 GetCurrentMaxAmmo() const;
	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetCurrentAmmo() const;
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetCurrentReloadTime() const;

protected:
	virtual void BeginPlay() override; // 샷건탄 초기화
	virtual bool PerformAttack() override; // 공격

	void BroadcastAmmo(); // 현재/최대 탄약을 함께 방송

	virtual EWeaponType GetWeaponType() const override { return EWeaponType::Shotgun; } //타입
	virtual EEnchantStat GetSpeedStatType() const override { return EEnchantStat::RangeFiringRate; } // 공속 타입
	virtual EEnchantStat GetDamageAddStatType() const override { return EEnchantStat::RangeDamageAdd; } // 공격 타입, 깡뎀
	virtual EEnchantStat GetDamageMultiStatType() const override { return EEnchantStat::RangeDamageMulti; }// 공격 타입, 배율
	virtual float GetUpgradeBonus(EEnchantStat Stat) const override;
	UFUNCTION()
	void OnLoadoutChanged(); // 탄약 최대치 줄어들었을때 현재탄약수도 그에 맞게 줄어들도록

	//샷건 스탯
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 MaxAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 CurrentAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float ReloadTime;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float ShotRadius = 10.f;
	bool bIsReloading = false;
	FTimerHandle ReloadTimerHandle;
};
