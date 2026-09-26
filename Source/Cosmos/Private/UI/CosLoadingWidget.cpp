#include "UI/CosLoadingWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"

TSharedRef<SWidget> UCosLoadingWidget::RebuildWidget()
{
	// BP 디자이너로 만든 레이아웃이 없을 때만 기본 화면을 코드로 만듭니다.
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Background"));
		Background->SetBrushColor(FLinearColor::Black);
		Background->SetHorizontalAlignment(HAlign_Center);
		Background->SetVerticalAlignment(VAlign_Center);

		UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LoadingLabel"));
		Label->SetText(LoadingText);
		Label->SetColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.75f, 0.7f)));
		FSlateFontInfo Font = Label->GetFont();
		Font.Size = 28;
		Label->SetFont(Font);

		Background->SetContent(Label);
		WidgetTree->RootWidget = Background;
	}

	return Super::RebuildWidget();
}
