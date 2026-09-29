#include "UI/CosDialogueWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

TSharedRef<SWidget> UCosDialogueWidget::RebuildWidget()
{
    // BP layouts remain supported; the native fallback supplies a restrained reading panel.
    if (WidgetTree && !WidgetTree->RootWidget)
    {
        UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));
        Root->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        USizeBox* BoxSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("DialogueBoxSize"));
        BoxSize->SetWidthOverride(1440.0f);
        BoxSize->SetMinDesiredHeight(140.0f);

        UBorder* Box = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DialogueBox"));
        // One fill preserves 80% opacity; an overlapping frame would darken the panel.
        Box->SetBrush(FSlateRoundedBoxBrush(FLinearColor(0.0f, 0.0f, 0.0f, 0.8f), 0.0f,
            FLinearColor(0.28f, 0.26f, 0.22f, 0.75f), 1.0f));
        Box->SetBrushColor(FLinearColor::White);
        Box->SetPadding(FMargin(32.0f, 26.0f));
        BoxSize->SetContent(Box);
        UVerticalBox* Lines = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Lines"));
        Box->SetContent(Lines);

        // Engine Roboto provides a plain reading face and the engine's Korean fallback.
        UObject* ReadingFont = LoadObject<UObject>(nullptr, TEXT("/Engine/EngineFonts/Roboto.Roboto"));
        SpeakerLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Speaker"));
        SpeakerLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.64f, 0.52f, 0.34f)));
        FSlateFontInfo SpeakerFont = SpeakerLabel->GetFont();
        SpeakerFont.FontObject = ReadingFont;
        SpeakerFont.TypefaceFontName = TEXT("Bold");
        SpeakerFont.Size = 16;
        SpeakerLabel->SetFont(SpeakerFont);
        Lines->AddChildToVerticalBox(SpeakerLabel)->SetPadding(FMargin(0, 0, 0, 10));

        LineLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Line"));
        LineLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.88f, 0.86f, 0.81f)));
        LineLabel->SetAutoWrapText(true);
        LineLabel->SetLineHeightPercentage(1.3f);
        LineLabel->SetApplyLineHeightToBottomLine(false);
        FSlateFontInfo LineFont = LineLabel->GetFont();
        LineFont.FontObject = ReadingFont;
        LineFont.TypefaceFontName = TEXT("Regular");
        LineFont.Size = 22;
        LineLabel->SetFont(LineFont);
        Lines->AddChildToVerticalBox(LineLabel)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

        UOverlaySlot* BoxSlot = Root->AddChildToOverlay(BoxSize);
        BoxSlot->SetHorizontalAlignment(HAlign_Center);
        BoxSlot->SetVerticalAlignment(VAlign_Bottom);
        BoxSlot->SetPadding(FMargin(32.0f, 0.0f, 32.0f, 52.0f));
        WidgetTree->RootWidget = Root;
    }
    return Super::RebuildWidget();
}

void UCosDialogueWidget::ShowLine(const FText& InSpeaker, const FText& InLine)
{
	ShowLines(InSpeaker, { InLine });
}

void UCosDialogueWidget::ShowLines(const FText& InSpeaker, const TArray<FText>& InLines)
{
	QueuedLines = InLines;

	if (SpeakerLabel)
	{
		SpeakerLabel->SetText(InSpeaker);
		SpeakerLabel->SetVisibility(InSpeaker.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}

	StartLine(0);
}

void UCosDialogueWidget::StartLine(int32 Index)
{
	if (!QueuedLines.IsValidIndex(Index))
	{
		Close();
		return;
	}

	LineIndex = Index;
	FullLine = QueuedLines[Index].ToString();
	RevealedChars = 0.0f;
	HoldElapsed = 0.0f;

	if (LineLabel)
	{
		LineLabel->SetText(FText::GetEmpty());
	}
}

void UCosDialogueWidget::Advance()
{
	StartLine(LineIndex + 1); // 마지막 줄이었으면 닫힙니다.
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
			Advance();
		}
	}
}

FReply UCosDialogueWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	// 글자가 나오는 중이면 끝까지 보여주고, 다 보인 상태면 다음 줄로 넘어갑니다.
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
		Advance();
	}
	return FReply::Handled();
}

void UCosDialogueWidget::Close()
{
	RemoveFromParent();
}
