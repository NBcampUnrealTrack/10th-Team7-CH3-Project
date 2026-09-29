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

	// 공격 모션
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsSwingLeftToRight() const { return bSwingLeftToRight; }
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetComboWindow() const { return GetCurrentAttackInterval() + ComboWindowBonus; }

protected:
	virtual bool PerformAttack() override;

	virtual EWeaponType GetWeaponType() const override { return EWeaponType::Nail; }; // 타입
	virtual EEnchantStat GetSpeedStatType() const override { return EEnchantStat::MeleeSpeed; } // 공속 타입 
	virtual EEnchantStat GetDamageAddStatType() const override { return EEnchantStat::MeleeDamageAdd; } // 공격력 타입, 깡뎀
	virtual EEnchantStat GetDamageMultiStatType() const override { return EEnchantStat::MeleeDamageMulti; } // 공격력 타입, 배율

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

	// 히트스탑
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void StartHitStop(const TArray<AActor*>& Targets);
	void EndHitStop();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|HitStop")
	float HitStopDuration;          // 멈추는 시간(초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|HitStop", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HitStopTimeDilation;      // 멈출 때 속도 배율 (0.05 = 거의 정지)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|HitStop")
	bool bIsHitStopIncludeOwner;    // 플레이어 캐릭터도 같이 멈출지

	TArray<TPair<TWeakObjectPtr<AActor>, float>> HitStoppedActors; // 멈춘 액터와 원래 속도 배율
	FTimerHandle HitStopTimerHandle;
};
