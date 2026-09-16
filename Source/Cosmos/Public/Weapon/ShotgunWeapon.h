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
	bool IsReloading() const { return bIsReloading; } // 재장전동안 모션 안나가도록.
	
	//장전모션 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void OnReloadStarted();
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
	void OnReloadFinished();

	UFUNCTION(BlueprintPure, Category = "Weapon") // getter
	int32 GetCurrentMaxAmmo() const;
	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetCurrentAmmo() const;
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetCurrentReloadTime() const;

	UPROPERTY(BlueprintAssignable, Category = "Weapon") //델리게이트
	FOnAmmoChanged OnAmmoChanged;

protected:
	virtual void BeginPlay() override;
	virtual bool PerformAttack() override; // 공격
	virtual EWeaponType GetWeaponType() const override; // 무기 타입

	//샷건 스탯
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 MaxAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 CurrentAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float ReloadTime;
	bool bIsReloading = false;
	FTimerHandle ReloadTimerHandle;

	//인챈트 적용
	virtual EEnchantStat GetSpeedStatType() const override { return EEnchantStat::RangeFiringRate; }
	virtual EEnchantStat GetDamageAddStatType() const override { return EEnchantStat::RangeDamageAdd; }
	virtual EEnchantStat GetDamageMultiStatType() const override { return EEnchantStat::RangeDamageMulti; }

	UFUNCTION()
	void OnLoadoutChanged(); // 탄약 최대치 줄어들었을때 현재탄약수도 그에 맞게 줄어들도록
};
