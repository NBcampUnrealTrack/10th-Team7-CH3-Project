#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EnchantGenerator.generated.h"

UCLASS()
class COSMOS_API UEnchantGenerator : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Enchant")
	static UEnchantData* GenerateRandomEnchant(UObject* Outer, const UDataTable* StatPool,
		const UDataTable* SkillPool);
};
