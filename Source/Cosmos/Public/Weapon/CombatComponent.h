#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class UInputAction;
class UInputMappingContext;
class ANailWeapon;
class AShotgunWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPotionCountChanged, int32, NewCount);
UCLASS(meta = (BlueprintSpawnableComponent)) //BP_CosCharacter 에서 CombatComponent 표시
class COSMOS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	//UI쪽에서 써야돼서 public
	FORCEINLINE AShotgunWeapon* GetShotgunWeapon() const { return ShotgunWeapon; }
	FORCEINLINE ANailWeapon* GetNailWeapon() const { return NailWeapon; }
	
	UPROPERTY(BlueprintAssignable)
	FOnPotionCountChanged OnPotionCountChanged;

	int32 GetPotionCount() const { return PotionCount; }
	/*UFUNCTION(BlueprintPure, Category = "Potion")
	int32 GetCurrentPotionCount() const;
	UFUNCTION(BlueprintPure, Category = "Potion")
	int32 GetCurrentMaxPotionCount() const;
	UFUNCTION(BlueprintPure, Category = "Potion")
	float GetCurrentPotionHealAmount() const;*/

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool CanAttack() const;

protected:
	virtual void BeginPlay() override;

	//IA , IMC 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* CombatMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* NailAttackAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ShotgunAttackAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ReloadAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* UsePotionAction;

	//포션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion")
	int32 PotionCount;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion")
	float PotionHealAmount;

	//TryAttack() 호출을 위한 포인터
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	ANailWeapon* NailWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	AShotgunWeapon* ShotgunWeapon;

	//입력 연결 함수
	void OnNailAttack();
	void OnShotgunAttack();
	void OnReload();
	void OnUsePotion();
};