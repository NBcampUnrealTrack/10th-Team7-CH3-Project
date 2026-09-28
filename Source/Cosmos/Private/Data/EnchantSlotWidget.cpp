#include "Data/EnchantSlotWidget.h"
#include "Data/EnchantInventoryWidget.h"
#include "Data/CosGameInstance.h"

void UEnchantSlotWidget::Setup(UEnchantData* InEnchant, bool InIsEquipped)
{
	MyEnchant = InEnchant;
	bIsEquipped = InIsEquipped;

	if (!MyEnchant)
	{
		SetToolTipText(FText::GetEmpty());
		OnEnchantSet();
		return;
	}

	SetToolTipText(FText::FromString(GetStatText()));
	OnEnchantSet();
}


FString UEnchantSlotWidget::GetStatText() const
{
	return MyEnchant ? MyEnchant->GetStatText() : FString();
}

FReply UEnchantSlotWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
	
	UE_LOG(LogTemp, Warning, TEXT("Slot clicked. MyEnchant valid: %d"), MyEnchant != nullptr);

	if (MyEnchant)
	{
		if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
		{
			GI->OnEnchantInfoRequested.Broadcast(MyEnchant, bIsEquipped);
		}
	}

	return FReply::Handled();
}