#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class UInputAction;
class UInputMappingContext;
class ANailWeapon;
class AShotgunWeapon;

UCLASS(meta = (BlueprintSpawnableComponent)) //BP_CosCharacter 에서 CombatComponent 표시
class COSMOS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

protected:
	virtual void BeginPlay() override;

	//IA , IMC 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* CombatMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* NailAttackAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ShotgunAttackAction;

	//TryAttack() 호출을 위한 포인터
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	ANailWeapon* NailWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	AShotgunWeapon* ShotgunWeapon;


	void OnNailAttack();
	void OnShotgunAttack();
};