#include "Data/EnchantInfoPopup.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UEnchantInfoPopup::Setup(UEnchantData* Enchant, bool bIsEquipped)
{
	if (!Enchant) return;

	if (StatText)
	{
		StatText->SetText(FText::FromString(Enchant->GetStatText()));
	}

	if (ActionButtonLabel)
	{
		ActionButtonLabel->SetText(bIsEquipped
			? FText::FromString(TEXT("UnEquip"))
			: FText::FromString(TEXT("Equip")));
	}
}