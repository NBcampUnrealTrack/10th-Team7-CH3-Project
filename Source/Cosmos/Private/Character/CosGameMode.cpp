#include "Character/CosGameMode.h"
#include "Character/CosCharacter.h"

ACosGameMode::ACosGameMode()
{
	DefaultPawnClass = ACosCharacter::StaticClass();
}

