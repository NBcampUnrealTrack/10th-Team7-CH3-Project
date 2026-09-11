#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/EnchantData.h"
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
	UStaticMeshComponent* StaticMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enchant")
	USphereComponent* Collision;

	UPROPERTY()
	UEnchantData* GeneratedEnchant;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enchant")
	UDataTable* StatPool;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enchant")
	UDataTable* SkillPool;

	virtual void BeginPlay() override;
};
