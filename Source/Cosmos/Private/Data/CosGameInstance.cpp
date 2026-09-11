#include "Data/CosGameInstance.h"
#include "Data/EnchantData.h"


// 재화를 얻는 로직
// 몬스터 쪽에서 사망 처리 로직에 AddSoul 호출
// if (UCosGameInstance* GameInstance = Cast<UCosGameInstance>(GetWorld()->GetGameInstance()))
// { GameInstance->AddSoul(EnemyData.SoulDrop); }
void UCosGameInstance::AddSoul(int32 Amount)
{
	if (Amount <= 0) return;
	Soul += Amount;
	OnCurrencyChanged.Broadcast(Soul);
}

// 재화를 사용할 때 상태 업데이트 로직
// if (UCosGameInstance* GameInstance = Cast<UCosGameInstance>(GetWorld()->GetGameInstance()))
// { 
//		if (GameInstance->TrySpendSoul(Amount)
//		{
//			성공: 실제로 구매/업그레이드 적용
//		}
//		else
//		{	
//			실패: 재화 부족 처리 (UI 메시지 등)
//		}
// }
bool UCosGameInstance::SpendSoul(int32 Amount)
{
	if (Amount <= 0 || Soul < Amount) return false;
	Soul -= Amount;
	OnCurrencyChanged.Broadcast(Soul);
	return true;
}

bool UCosGameInstance::CollectEnchant(UEnchantData* Enchant)
{
	if (!Enchant) return false;

	const int32 MaxCollectedEnchantCount = 10;

	if (CollectedEnchant.Num() >= 10)
	{
		UE_LOG(LogTemp, Warning, TEXT("인챈트 인벤토리가 꽉 찼습니다."));
		return false;
	}

	CollectedEnchant.Add(Enchant);
	OnLoadoutChange.Broadcast();

	return true;
}

// 인챈트 드랍
// 몬스터 클래스에서 몬스터가 죽을 때 이 코드를 넣으면 인챈트가 드랍
// GetWorld()->SpawnActor<AEnchantPickup>(EnchantPickupClass, GetActorLocation(), FRotator::ZeroRotator);

bool UCosGameInstance::EquipEnchant(UEnchantData* Enchant)
{
	UE_LOG(LogTemp, Warning, TEXT("인챈트 장착"));
	if (!Enchant || !CollectedEnchant.Contains(Enchant)) return false;
	
	const int32 MaxEquippedEnchantCount = 3;

	if (CollectedEnchant.Num() >= 3)
	{
		UE_LOG(LogTemp, Warning, TEXT("인첸트를 더 이상 장착할 수 없습니다."));
		return false;
	}

	EquippedEnchant.Add(Enchant);
	OnLoadoutChange.Broadcast();

	return true;
}

void UCosGameInstance::UnEquipEnchant(UEnchantData* Enchant)
{
	
}