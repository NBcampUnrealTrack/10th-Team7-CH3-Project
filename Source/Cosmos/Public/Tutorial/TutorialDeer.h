#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon/Damageable.h"
#include "TutorialDeer.generated.h"

class UBoxComponent;
class USkeletalMeshComponent;
class UAnimSequence;
class ATutorialDeer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTutorialDeerKilled, ATutorialDeer*, Deer);

// Day 1의 무저항 대상. 게임모드/재화/스폰 진행은 사망 알림을 구독한 쪽이 담당합니다.
UCLASS(Blueprintable)
class COSMOS_API ATutorialDeer : public AActor, public IDamageable
{
	GENERATED_BODY()

public:
	ATutorialDeer();
	virtual void TakeHit(float Damage, EWeaponType Weapon) override;

	UFUNCTION(BlueprintPure, Category = "Tutorial|Deer")
	bool IsDead() const { return bIsDead; }

	// 실제 피격으로 사망할 때만 한 번 방송합니다. Destroy()/레벨 종료로는 방송하지 않습니다.
	UPROPERTY(BlueprintAssignable, Category = "Tutorial|Deer")
	FOnTutorialDeerKilled OnDeerKilled;

protected:
	// 기존 샷건 Weapon 채널과 대못 Enemies 오브젝트 검색에 모두 대응합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial|Deer")
	TObjectPtr<UBoxComponent> HitBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial|Deer")
	TObjectPtr<USkeletalMeshComponent> DeerMesh;

	// 사슴 외형과 호환되는 애니메이션을 BP에서 지정합니다. 없으면 기본적으로 외형을 숨깁니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tutorial|Deer")
	TObjectPtr<UAnimSequence> DeathAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tutorial|Deer")
	bool bHideMeshWithoutDeathAnimation = true;

	// 필요할 때만 BP에서 소리/이펙트/별도 사망 연출을 붙입니다. 컷신은 강제하지 않습니다.
	UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial|Deer")
	void OnDeathPresentation(EWeaponType KillingWeapon);

private:
	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Tutorial|Deer")
	bool bIsDead = false;
};
