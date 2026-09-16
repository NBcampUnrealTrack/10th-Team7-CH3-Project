#include "Data/EnchantInventoryWidget.h"
#include "Data/CosGameInstance.h"
#include "Data/EnchantData.h"
#include "Data/EnchantSlotWidget.h"
#include "Components/VerticalBox.h"

void UEnchantInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct(); // UswerWidget의 BeginPlay 같은 것

	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		GI->OnLoadoutChange.AddDynamic(this, &UEnchantInventoryWidget::RefreshList);
	}

	RefreshList();
}

// 인챈트 장착 UI을 켰을 때 한 번 발생
// 그 이후에는 델리게이트를 이용해 OnLoadoutChange가 방송될 때마다 자동으로 실행
// 기본 구조는 기존에 저장되어 있는 슬롯을 다 비운 후 슬롯을 다시 채움.
// 왜 이렇게? 계속 지우고 채우는 행위를 하다 보니 데이터가 어긋날 일이 없다.
// 또한 변
// 하지만 계속 지우고 채우는 행위를 반복하는 것은 비효율적.
void UEnchantInventoryWidget::RefreshList()
{
	UE_LOG(LogTemp, Warning, TEXT("RefreshList called"));

	// 연결된 슬롯 위젯 클래스가 없거나 슬롯을 어디에 놓을지 가리키는 자리가 없으면 return
	if (!SlotWidgetClass || !SlotContainer)
	{
		UE_LOG(LogTemp, Warning, TEXT("RefreshList early return - SlotWidgetClass:%d SlotContainer:%d"),
			SlotWidgetClass != nullptr, SlotContainer != nullptr);
		return;
	}

	// 우선 기존 컨테이너 (슬롯 자리)를 지워주고
	SlotContainer->ClearChildren();

	UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance());
	if (!GI) return;

	const int32 StartIndex = CurrentPage * SlotsPerPage;
	const int32 EndIndex = FMath::Min(StartIndex + SlotsPerPage, GI->CollectedEnchant.Num());

	// 인챈트 인벤토리에서 순환을 돌며 각 인챈트에 접근
	// 접근한 뒤 각 인챈트를 Setup(들어갈 슬롯에 인챈트 정보를 전달 (저장)
	// 다음 컨테이너에 슬롯을 채움
	UE_LOG(LogTemp, Warning, TEXT("StartIndex:%d EndIndex:%d Num:%d SlotsPerPage:%d ColumsPerRow:%d"),
		StartIndex, EndIndex, GI->CollectedEnchant.Num(), SlotsPerPage, ColumsPerRow);

	for (int32 i = StartIndex; i < EndIndex; i++)
	{
		UEnchantData* Enchant = GI->CollectedEnchant[i];

		UEnchantSlotWidget* NewSlot = CreateWidget<UEnchantSlotWidget>(this, SlotWidgetClass);
		if (!NewSlot) continue;

		const bool bIsEquipped = GI->EquippedEnchant.Contains(Enchant);
		NewSlot->Setup(Enchant, false, this);

		const int32 LocalIndex = i - StartIndex;
		SlotContainer->AddChildToUniformGrid(NewSlot, LocalIndex / ColumsPerRow, LocalIndex % ColumsPerRow);
	}
}

void UEnchantInventoryWidget::OnEquipButtonClicked()
{
	if (!SelectedEnchant) return;

	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		if (!GI->EquipEnchant(SelectedEnchant))
		{
			// 장착 실패 메시지
		}
	}
}

void UEnchantInventoryWidget::OnUnEquipButtonClicked()
{
	if (!SelectedEnchant) return;
	
	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		GI->UnEquipEnchant(SelectedEnchant);
	}
}

void UEnchantInventoryWidget::NextPage()
{
	UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance());
	if (!GI) return;

	const int32 MaxPage = FMath::Max(0, GI->CollectedEnchant.Num() - 1 / SlotsPerPage);
	if (CurrentPage < MaxPage) 
	{ 
		CurrentPage++; 
		RefreshList();
	}
}

void UEnchantInventoryWidget::PrevPage()
{
	if (CurrentPage > 0)
	{
		CurrentPage--;
		RefreshList();
	}
}

void UEnchantInventoryWidget::ToggleEquip(UEnchantData* Enchant, bool bCurrentlyEquipped)
{
	if (!Enchant) return;

	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		if (bCurrentlyEquipped)
		{
			GI->UnEquipEnchant(Enchant);
		}
		else
		{
			GI->EquipEnchant(Enchant);
		}
	}
}