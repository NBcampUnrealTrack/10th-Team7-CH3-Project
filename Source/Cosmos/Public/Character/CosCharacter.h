#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Weapon/Damageable.h"//[추가] 데미져블 인터페이스 사용하게
#include "CosCharacter.generated.h"

class UHealthComponent;
class UCameraComponent;
// [추가] Enhanced Input 클래스 전방 선언
class UInputMappingContext;
class UInputAction;
// ㄴ 테스트용

UCLASS()
class COSMOS_API ACosCharacter : public ACharacter, public IDamageable// IDamageable 상속 추가
{
	GENERATED_BODY()

public:
	ACosCharacter();
	void TakeHit(float Damage, EWeaponType Weapon) override;//추가한 IDamageable 구현부분

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float NormalSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeedMultiplier;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void Move(const FInputActionValue& Value);
	UFUNCTION()
	void StartJump(const FInputActionValue& Value);
	UFUNCTION()
	void StopJump(const FInputActionValue& Value);
	UFUNCTION()
	void Look(const FInputActionValue& Value);
	UFUNCTION()
	void StartSprint(const FInputActionValue& Value);
	UFUNCTION()
	void StopSprint(const FInputActionValue& Value);
};
