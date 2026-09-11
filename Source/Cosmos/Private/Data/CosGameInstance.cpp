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

// 인챈트 드랍
// 몬스터 클래스에서 몬스터가 죽을 때 이 코드를 넣으면 인챈트가 드랍
// GetWorld()->SpawnActor<AEnchantPickup>(EnchantPickupClass, GetActorLocation(), FRotator::ZeroRotator);

void UCosGameInstance::EquipEnchant(UEnchantData* Enchant)
{
	UE_LOG(LogTemp, Warning, TEXT("인챈트 장착"));
	UE_LOG(LogTemp, Warning, TEXT("EquipEnchant called - Enchant: %s"), Enchant ? TEXT("valid") : TEXT("null"));
}

void UCosGameInstance::UnEquipEnchant(UEnchantData* Enchant)
{
	
}