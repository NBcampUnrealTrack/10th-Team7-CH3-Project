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

	UPROPERTY(BlueprintAssignable, Category = "Weapon") // Bind Event to OnHitConfirmed 설정
	FOnHitConfirmed OnHitConfirmed; //방송하기 위한 변수 

protected:

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	UStaticMeshComponent* WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float BaseDamage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float AttackInterval;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float AttackRange;

	float LastAttackTime;

	virtual void PerformAttack() PURE_VIRTUAL(AWeaponBase::PerformAttack, );
	virtual float GetCurrentDamage() const PURE_VIRTUAL(AWeaponBase::GetCurrentDamage, return 0.f;);
	virtual EWeaponType GetWeaponType() const PURE_VIRTUAL(AWeaponBase::GetWeaponType, return EWeaponType::None;);
	virtual void ApplyEnchant() PURE_VIRTUAL(AWeaponBase::ApplyEnchant, );
};
