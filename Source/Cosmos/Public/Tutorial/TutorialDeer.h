#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon/Damageable.h"
#include "TutorialDeer.generated.h"

class UBoxComponent;
class USkeletalMeshComponent;
class UAudioComponent;
class USoundBase;
class UNiagaraSystem;
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
	virtual void TakeHitAlt(float Damage, EWeaponType Weapon, const FHitResult& Hit) override;

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

	// 살아 있는 동안 반복 재생하는 헐떡이는 소리. 죽으면 멈춥니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial|Deer|Sound")
	TObjectPtr<UAudioComponent> BreathingAudio;

	// 헐떡이는 소리. 끊기지 않도록 Looping을 켠 사운드(큐)를 넣으세요. 비워두면 재생하지 않습니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial|Deer|Sound")
	TObjectPtr<USoundBase> BreathingSound;

	// 이 거리(cm) 안에서는 최대 음량으로 들립니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial|Deer|Sound", meta = (ClampMin = "0.0"))
	float BreathingFullVolumeRadius = 300.0f;

	// 위 반경 바깥으로 이 거리(cm)만큼 가면서 점점 작아져 들리지 않게 됩니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial|Deer|Sound", meta = (ClampMin = "0.0"))
	float BreathingFalloffDistance = 1200.0f;

	// 총알이 맞은 지점에서 터지는 이펙트(피 튀김 등). 비워두면 재생하지 않습니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial|Deer|Hit")
	TObjectPtr<UNiagaraSystem> HitVFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial|Deer|Hit")
	FVector HitVFXScale = FVector(1.0f);

	// 총알이 맞은 지점에서 나는 소리. 비워두면 재생하지 않습니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial|Deer|Hit")
	TObjectPtr<USoundBase> HitSound;

	virtual void BeginPlay() override;

	// 사망 신호만 보냅니다. 래그돌/애니메이션/콜리전 등 사망 연출은 BP에서 처리합니다.
	UFUNCTION(BlueprintImplementableEvent, Category = "Tutorial|Deer")
	void OnDied();

private:
	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = "Tutorial|Deer", meta = (AllowPrivateAccess = "true"))
	bool bIsDead = false;

	// 샷건 산탄 여러 발이 한 번에 맞아도 이펙트·소리가 과하게 겹치지 않게 합니다.
	float LastHitFeedbackTime = -1.0f;
};
