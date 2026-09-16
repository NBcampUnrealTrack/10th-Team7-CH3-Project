#include "Data/EnchantEquipWidget.h"
#include "Data/CosGameInstance.h"
#include "Data/EnchantData.h"

void UEnchantEquipWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		GI->OnLoadoutChange.AddDynamic(this, &UEnchantEquipWidget::RefreshEquipped);
	}

	RefreshEquipped();
}

void UEnchantEquipWidget::RefreshEquipped()
{
	if (!SlotWidgetClass || !SlotContainer) return;

	SlotContainer->ClearChildren();

	UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance());
	if (!GI) return;

	for (int32 i = 0; i < MaxEnchantSlots; i++)
	{
		UEnchantData* Enchant = GI->EquippedEnchant.IsValidIndex(i) ? GI->EquippedEnchant[i] : nullptr;
		UEnchantSlotWidget* NewSlot = CreateWidget<UEnchantSlotWidget>(this, SlotWidgetClass);
		if (!NewSlot) continue;

		NewSlot->Setup(Enchant, true);
		SlotContainer->AddChildToUniformGrid(NewSlot, 0, i);
	}
}