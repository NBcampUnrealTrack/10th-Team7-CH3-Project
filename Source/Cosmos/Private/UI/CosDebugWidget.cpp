#include "UI/CosDebugWidget.h"
#include "UI/CosPlayerController.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"

TSharedRef<SWidget> UCosDebugWidget::RebuildWidget()
{
#if !UE_BUILD_SHIPPING
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
		WidgetTree->RootWidget = Root;
		UBorder* Background = WidgetTree->ConstructWidget<UBorder>();
		Background->SetBrushColor(FLinearColor(0.015f, 0.012f, 0.01f, 0.94f));
		Background->SetPadding(FMargin(18));
		UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(Background);
		PanelSlot->SetAnchors(FAnchors(1, 0));
		PanelSlot->SetAlignment(FVector2D(1, 0));
		PanelSlot->SetPosition(FVector2D(-24, 155));
		PanelSlot->SetSize(FVector2D(450, 520));
		StatusText = WidgetTree->ConstructWidget<UTextBlock>();
		FSlateFontInfo Font = StatusText->GetFont();
		Font.Size = 16;
		StatusText->SetFont(Font);
		StatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.76f, 0.56f)));
		StatusText->SetAutoWrapText(true);
		Background->SetContent(StatusText);
		SetVisibility(ESlateVisibility::HitTestInvisible);
	}
#endif
	return Super::RebuildWidget();
}

void UCosDebugWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
#if !UE_BUILD_SHIPPING
	RefreshDelay -= InDeltaTime;
	if (RefreshDelay <= 0 && StatusText)
	{
		RefreshDelay = 0.2f;
		if (const ACosPlayerController* PC = Cast<ACosPlayerController>(GetOwningPlayer()))
		{
			StatusText->SetText(FText::FromString(PC->GetDebugStatus() + TEXT(
				"\n\nF1   디버그 도움말 닫기\nF2   보스전 바로 시작\nCtrl + 1~7   해당 날짜 시작\n"
				"F3   무적 켜기 / 끄기\nF4   체력 · 탄약 · 포션 충전\nF5   현재 적 처치 (보상 발생)\n"
				"F6   현재 날짜 다시 시작\nF7   배속 1 → 2 → 0.25\nF9   보스 최대 체력 10% 피해\n"
				"\n콘솔(~): CosDebugSoul 1000\nCosDebugPlayerDamage 10\nCosDebugSpeed 1\n"
				"\n날짜 이동 시 전장 · 무적 · 배속 초기화\n소울 · 장비는 유지됩니다. 개발 빌드 전용.")));
		}
	}
#endif
}
