#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CosTypes.h"
#include "Damageable.generated.h"

UINTERFACE(MinimalAPI)
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

class COSMOS_API IDamageable
{
	GENERATED_BODY()

public:
	virtual void TakeHit(float Damage, EWeaponType Weapon) = 0;
	virtual void TakeHitAlt(float Damage, EWeaponType Weapon, const FHitResult& Hit)
	{
		TakeHit(Damage, Weapon);
	}
};
