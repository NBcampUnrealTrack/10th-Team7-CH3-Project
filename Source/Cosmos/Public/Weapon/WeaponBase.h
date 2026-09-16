#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CosTypes.h"
#include "Data/CosDataTable.h"
#include "WeaponBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitConfirmed, AActor*, Target); //FOnHitConfirmed 델리게이트 타입 생성

UCLASS()
class COSMOS_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeaponBase();
	
	void TryAttack(); // 공격 속도 판단
	bool TryApplyDamage(AActor* HitActor); // 데미저블 판단을 위해 bool형

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon") // 애니메이션 구현 위해 이벤트로 호출
	void OnAttackPlayed();

	UPROPERTY(BlueprintAssignable, Category = "Weapon") // Bind Event to OnHitConfirmed 설정
	FOnHitConfirmed OnHitConfirmed; //방송하기 위한 변수 

protected:
	virtual bool PerformAttack() PURE_VIRTUAL(AWeaponBase::PerformAttack, return false; ); // 탄 없을때 공격모션 나가지 않도록 bool
	virtual EWeaponType GetWeaponType() const PURE_VIRTUAL(AWeaponBase::GetWeaponType, return EWeaponType::None;); // 무기 타입
	bool GetTraceStartAndDirection(FVector & OutStart, FVector & OutDirection) const; // 트레이스 방향을 무기 기준이 아니라 카메라 시점으로 바꿔줌.
	float LastAttackTime;

	//메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	UStaticMeshComponent* WeaponMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	USceneComponent* WeaponRoot;
	//무기 스탯
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float BaseDamage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float AttackInterval;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float AttackRange;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetCurrentAttackInterval() const;
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetCurrentDamage() const;
	virtual EEnchantStat GetSpeedStatType() const { return EEnchantStat::None; }
	virtual EEnchantStat GetDamageAddStatType() const { return EEnchantStat::None; }
	virtual EEnchantStat GetDamageMultiStatType() const { return EEnchantStat::None; }

	float GetEnchantStat(EEnchantStat Stat) const; // 인챈트로 얼마나 스탯이 늘어나는지.
};
