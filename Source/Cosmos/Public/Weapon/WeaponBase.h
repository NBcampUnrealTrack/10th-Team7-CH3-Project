#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CosTypes.h"
#include "WeaponBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitConfirmed, AActor*, Target); //FOnHitConfirmed 델리게이트 타입 생성

UCLASS()
class COSMOS_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeaponBase();
	
	void TryAttack();
	bool ApplyDamage(AActor* HitActor, const FHitResult& HitResult); // 데미저블 판단을 위해 bool형

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon") // 애니메이션 구현 위해 이벤트로 호출
	void OnAttackPlayed();

	UPROPERTY(BlueprintAssignable, Category = "Weapon") // Bind Event to OnHitConfirmed 설정
	FOnHitConfirmed OnHitConfirmed; //방송하기 위한 변수 

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	UStaticMeshComponent* WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float BaseDamage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float AttackInterval;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float AttackRange;

	float LastAttackTime;

	virtual void PerformAttack() PURE_VIRTUAL(AWeaponBase::PerformAttack, ); // 공격
	virtual float GetCurrentDamage() const PURE_VIRTUAL(AWeaponBase::GetCurrentDamage, return 0.f;); // 데미지 getter
	virtual EWeaponType GetWeaponType() const PURE_VIRTUAL(AWeaponBase::GetWeaponType, return EWeaponType::None;); // 무기 타입
	virtual void ApplyEnchant() PURE_VIRTUAL(AWeaponBase::ApplyEnchant, ); // 인챈트 

	bool GetTraceStartAndDirection(FVector& OutStart, FVector& OutDirection) const; // 트레이스 방향을 무기 기준이 아니라 카메라 시점으로 바꿔줌.
};
