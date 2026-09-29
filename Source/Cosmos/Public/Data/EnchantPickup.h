#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/EnchantData.h"
#include "Data/CosDataTable.h"
#include "Data/CosGameInstance.h"
#include "EnchantPickup.generated.h"

class USphereComponent;
UCLASS()
class COSMOS_API AEnchantPickup : public AActor
{
	GENERATED_BODY()
	
public:	
	AEnchantPickup();

protected:
	UFUNCTION()
	void OnEnchantOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnEnchantEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	void ActivateEnchant(AActor* Activator);
	void DestroyEnchant();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enchant")
	USceneComponent* Scene;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enchant")
	USkeletalMeshComponent* SkeletalMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enchant")
	USphereComponent* Collision;

	UPROPERTY()
	UEnchantData* GeneratedEnchant;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enchant")
	UDataTable* StatPool;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enchant")
	UDataTable* SkillPool;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* DropSound;
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* PickupSound;

	virtual void BeginPlay() override;
};
