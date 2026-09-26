#include "UI/CosDialogueWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

TSharedRef<SWidget> UCosDialogueWidget::RebuildWidget()
{
	// BP 디자이너 레이아웃이 없을 때만 기본 대사창을 코드로 만듭니다.
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));
		Root->SetVisibility(ESlateVisibility::SelfHitTestInvisible); // 대사창 밖 클릭은 대장간 UI로 넘깁니다.

		UBorder* Box = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DialogueBox"));
		Box->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f));
		Box->SetPadding(FMargin(32.0f, 20.0f));

		UVerticalBox* Lines = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Lines"));

		SpeakerLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Speaker"));
		SpeakerLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.6f, 0.3f)));
		FSlateFontInfo SpeakerFont = SpeakerLabel->GetFont();
		SpeakerFont.Size = 18;
		SpeakerLabel->SetFont(SpeakerFont);

		LineLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Line"));
		LineLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.9f, 0.86f)));
		LineLabel->SetAutoWrapText(true);
		FSlateFontInfo LineFont = LineLabel->GetFont();
		LineFont.Size = 24;
		LineLabel->SetFont(LineFont);

		Lines->AddChildToVerticalBox(SpeakerLabel);
		Lines->AddChildToVerticalBox(LineLabel);
		Box->SetContent(Lines);

		if (UOverlaySlot* BoxSlot = Root->AddChildToOverlay(Box))
		{
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
			BoxSlot->SetVerticalAlignment(VAlign_Bottom);
			BoxSlot->SetPadding(FMargin(160.0f, 0.0f, 160.0f, 80.0f));
		}

		WidgetTree->RootWidget = Root;
	}

	return Super::RebuildWidget();
}

void UCosDialogueWidget::ShowLine(const FText& InSpeaker, const FText& InLine)
{
	FullLine = InLine.ToString();
	RevealedChars = 0.0f;
	HoldElapsed = 0.0f;

	if (SpeakerLabel)
	{
		SpeakerLabel->SetText(InSpeaker);
		SpeakerLabel->SetVisibility(InSpeaker.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (LineLabel)
	{
		LineLabel->SetText(FText::GetEmpty());
	}
}

void UCosDialogueWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const int32 Total = FullLine.Len();
	if (RevealedChars < Total)
	{
		// 타자 치듯 한 글자씩 보여줍니다.
		RevealedChars = FMath::Min(RevealedChars + InDeltaTime * CharsPerSecond, static_cast<float>(Total));
		if (LineLabel)
		{
			LineLabel->SetText(FText::FromString(FullLine.Left(FMath::FloorToInt(RevealedChars))));
		}
		return;
	}

	if (HoldSeconds > 0.0f)
	{
		HoldElapsed += InDeltaTime;
		if (HoldElapsed >= HoldSeconds)
		{
			Close();
		}
	}
}

FReply UCosDialogueWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 글자가 나오는 중이면 끝까지 보여주고, 다 보인 상태면 닫습니다.
	if (RevealedChars < FullLine.Len())
	{
		RevealedChars = FullLine.Len();
		if (LineLabel)
		{
			LineLabel->SetText(FText::FromString(FullLine));
		}
	}
	else
	{
		Close();
	}
	return FReply::Handled();
}

void UCosDialogueWidget::Close()
{
	RemoveFromParent();
}
