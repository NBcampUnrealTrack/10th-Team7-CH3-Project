#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CosTypes.h" //코스타입 추가
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnHealthChanged,
	float, CurrentHealth,
	float, MaxHealth
);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamaged, float, Damage);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COSMOS_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();

	void ApplyDamage(float Damage, EWeaponType Weapon); //데미지만큼 현재 체력 감소시킴. 체력이 0이면 OnDeath 방송
	void Heal(float Amount);//현재 체력 회복, 최대 체력 넘진 않음

	bool IsDead() const;//현재 체력이 0인지 확인함 

	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetHPAtStart(float MaxHP);
	UFUNCTION()
	void ApplyMaxHealthBonus(float Bonus);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float CurrentHealth;
	

public:	
	//체력변경 시 호출되는 델리게이트
	// CurrentHealth : 현재 체력	/	MaxHealth : 최대 체력
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

	//체력이 0 될때 한 번 호출
	//나중에 게임 오버, 적 사망, 킬카운트? 등에 연결
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDeath OnDeath;
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDamaged OnDamaged;

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float BaseMaxHealth = 100.0f;
};
