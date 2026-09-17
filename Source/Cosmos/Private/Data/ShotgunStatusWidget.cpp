#include "Data/ShotgunStatusWidget.h"
#include "Data/CosGameInstance.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UShotgunStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		GI->OnLoadoutChange.AddDynamic(this, &UShotgunStatusWidget::RefreshAll);
	}

	RefreshAll();
}

void UShotgunStatusWidget::NativeDestruct()
{
	if (UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance()))
	{
		GI->OnLoadoutChange.RemoveDynamic(this, &UShotgunStatusWidget::RefreshAll);
	}

	Super::NativeDestruct();
}

// RefreshOne()을 호출해서 슬라이더와 텍스를 갱신하는 함수
// 즉 샷건의 스테이터스의 상태를 업데이트 하는 함수
void UShotgunStatusWidget::RefreshAll()
{
	RefreshOne(EShotgunModuleType::Damage, DamageProgressBar, DamageValueText);
	RefreshOne(EShotgunModuleType::FireSpeed, FireSpeedProgressBar, FireSpeedValueText);
	RefreshOne(EShotgunModuleType::Reload, ReloadProgressBar, ReloadValueText);
	RefreshOne(EShotgunModuleType::Magazine, nullptr, MagazineValueText);
}

// 각 모듈 하나에 대해 현재 레벨이랑 실제값, 최대 레벨을 물어봐서 슬라이더와 텍스트에 정보를 전달하는 함수
void UShotgunStatusWidget::RefreshOne(EShotgunModuleType Type, UProgressBar* ProgressBar, UTextBlock* ValueText)
{
	UCosGameInstance* GI = Cast<UCosGameInstance>(GetGameInstance());
	if (!GI || !ValueText) return;

	const int32 Level = GI->GetModuleLevel(Type);
	const int32 MaxLevel = GI->GetMaxUpgradeLevel(Type);

	if (ProgressBar)
	{
		ProgressBar->SetPercent(MaxLevel > 0 ? static_cast<float>(Level) / static_cast<float>(MaxLevel) : 0.0f);
	}

	ValueText->SetText(FormatShotgunStatText(Type, GI->GetShotgunStat(Type), Level));
}

// 모듈마다 단위가 다르기 때문에 타입별로 표시 문구를 다르게 설정하기 위한 함수
FText UShotgunStatusWidget::FormatShotgunStatText(EShotgunModuleType Type, float Value, int32 Level) const
{
	const double DisplayValue = static_cast<double>(Value);

	switch (Type)
	{
	case EShotgunModuleType::Damage:
		return FText::FromString(FString::Printf(TEXT("Add Damage: %.0f"), DisplayValue));
	case EShotgunModuleType::FireSpeed:
		return FText::FromString(FString::Printf(TEXT("Add Fire Speed: %.2fs"), DisplayValue));
	case EShotgunModuleType::Reload:
		return FText::FromString(FString::Printf(TEXT("Add Reload Speed: %.1fs"), DisplayValue));
	case EShotgunModuleType::Magazine:
		return FText::FromString(FString::Printf(TEXT("Add Magazine: %.0f"), DisplayValue));
	default:
		return FText::FromString(FString::Printf(TEXT("%.2f"), DisplayValue));
	}
}