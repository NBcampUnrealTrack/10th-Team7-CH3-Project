#include "Data/ShotgunUpgradeButtonWidget.h"
#include "Data/CosGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Button.h"

// 위젯이 생성될 때 자동으로 한 번 호출
// 여기서 각 버튼을 UI와 연결해줌
void UShotgunUpgradeButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (DamageButton)
	{
		DamageButton->OnClicked.AddDynamic(this, &UShotgunUpgradeButtonWidget::OnClickDamage);
	}
	if (FireSpeedButton)
	{
		FireSpeedButton->OnClicked.AddDynamic(this, &UShotgunUpgradeButtonWidget::OnClickFireSpeed);
	}
	if (ReloadButton)
	{
		ReloadButton->OnClicked.AddDynamic(this, &UShotgunUpgradeButtonWidget::OnClickReload);
	}
	if (MagazineButton)
	{
		MagazineButton->OnClicked.AddDynamic(this, &UShotgunUpgradeButtonWidget::OnClickMagazine);
	}

	// AddDynamic으로 구독하는 이유는 샷건이 강화될 때마다 RefreshButtonStates를 자동으로 실행하라는 뜻
	// if문 밖에서 수동으로 RefreshButtonStates를 호출하는 이유는 처음에는 강화가 진행되지 않기 때문에
	// 델리게이트가 자동을 호출 안됨. 따라서 위젯이 처음 화면 뜰 때 위젯을 수동으로 호출해줌
	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		GI->OnLoadoutChange.AddDynamic(this, &UShotgunUpgradeButtonWidget::RefreshButtonStates);
	}
		
	RefreshButtonStates();
}

// 위젯이 사라질 때 호출
// 
void UShotgunUpgradeButtonWidget::NativeDestruct()
{
	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		GI->OnLoadoutChange.RemoveDynamic(this, &UShotgunUpgradeButtonWidget::RefreshButtonStates);
	}

	// Super::NativeDestruct()를 아래에서 호출하는 이유는
	// 소멸 자체가 자식 -> 부모 순으로 진행하는게 일반적.
	// 근데 Super가 위에 있으면 위젯 자체가 내부적으로 정리하기 시작하는데
	// 정리가 시작되기 전 위젯이 걸어둔 외부 구독을 해제 후 Super를 실행해야
	// 자식 -> 부모 순으로 소멸하기 시작
	Super::NativeDestruct();
}

void UShotgunUpgradeButtonWidget::OnClickDamage()
{
	TryUpgrade(EShotgunModuleType::Damage);
}

void UShotgunUpgradeButtonWidget::OnClickFireSpeed()
{
	TryUpgrade(EShotgunModuleType::FireSpeed);
}

void UShotgunUpgradeButtonWidget::OnClickReload()
{
	TryUpgrade(EShotgunModuleType::Reload);
}

void UShotgunUpgradeButtonWidget::OnClickMagazine()
{
	TryUpgrade(EShotgunModuleType::Magazine);
}

void UShotgunUpgradeButtonWidget::TryUpgrade(EShotgunModuleType Type)
{
	UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance());
	if (!GI) return;

	if (GI->UpgradeShotgun(Type))
	{
		UGameplayStatics::PlaySound2D(this, UpgradeSuccessSound);
	}
	else
	{
		UGameplayStatics::PlaySound2D(this, UpgradeFailSound);
	}
}

// 버튼들의 활성/비활성 상태를 갱신하는 함수.
// 레벨이 맥스이면 비활성화
// SetIsEnabled -> 현제 레벨 < 맥스 레벨 : 참이면 버튼 활성화. 그 반대면 버튼 비활성화 (강화가 더 이상 진행하지 못하도록)
void UShotgunUpgradeButtonWidget::RefreshButtonStates()
{
	UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance());
	if (!GI) return;

	if (DamageButton)
	{
		DamageButton->SetIsEnabled(GI->GetModuleLevel(EShotgunModuleType::Damage) < GI->GetMaxUpgradeLevel(EShotgunModuleType::Damage));
	}

	if (FireSpeedButton)
	{
		FireSpeedButton->SetIsEnabled(GI->GetModuleLevel(EShotgunModuleType::FireSpeed) < GI->GetMaxUpgradeLevel(EShotgunModuleType::FireSpeed));
	}

	if (ReloadButton)
	{
		ReloadButton->SetIsEnabled(GI->GetModuleLevel(EShotgunModuleType::Reload) < GI->GetMaxUpgradeLevel(EShotgunModuleType::Reload));
	}

	if (MagazineButton)
	{
		MagazineButton->SetIsEnabled(GI->GetModuleLevel(EShotgunModuleType::Magazine) < GI->GetMaxUpgradeLevel(EShotgunModuleType::Magazine));
	}
}