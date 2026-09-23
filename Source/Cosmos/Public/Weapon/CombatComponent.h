#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class UInputAction;
class UInputMappingContext;
class ANailWeapon;
class AShotgunWeapon;
class UCosGameInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPotionCountChanged, int32, NewCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPotionUsed);
UCLASS(meta = (BlueprintSpawnableComponent)) //BP_CosCharacter 에서 CombatComponent 표시
class COSMOS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	
	UPROPERTY(BlueprintAssignable)
	FOnPotionCountChanged OnPotionCountChanged;
	UPROPERTY(BlueprintAssignable)
	FOnCombatReady OnCombatReady; // 초기화 완료 알림
	UPROPERTY(BlueprintAssignable) // 추가: 포션을 실제로 마셨을 때만 방송
	FOnPotionUsed OnPotionUsed;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsCombatReady() const { return bCombatReady; }

	//UI쪽에서 써야돼서 public
	FORCEINLINE AShotgunWeapon* GetShotgunWeapon() const { return ShotgunWeapon; }
	FORCEINLINE ANailWeapon* GetNailWeapon() const { return NailWeapon; }
	
	UFUNCTION(BlueprintPure, Category = "Potion")
	int32 GetPotionCount() const { return PotionCount; }
	UFUNCTION(BlueprintPure, Category = "Potion")
	int32 GetMaxPotionCount() const;
	UFUNCTION(BlueprintCallable, Category = "Potion")
	void RefillPotions();

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
	int32 PotionCount = 0;

	//TryAttack() 호출을 위한 포인터
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	ANailWeapon* NailWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	AShotgunWeapon* ShotgunWeapon;
	
	bool bCombatReady = false; // 초기화 완료 여부

	//입력 연결 함수
	void OnNailAttack();
	void OnShotgunAttack();
	void OnReload();
	void OnUsePotion();
};