#include "UI/CosIntroVideoWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScaleBox.h"
#include "Components/TextBlock.h"
#include "MediaPlayer.h"
#include "MediaSource.h"
#include "MediaTexture.h"
#include "MediaSoundComponent.h"

TSharedRef<SWidget> UCosIntroVideoWidget::RebuildWidget()
{
	// BP 디자이너로 만든 레이아웃이 없을 때만 기본 화면을 코드로 만듭니다.
	// 디자이너로 꾸밀 때는 영상을 보여줄 Image의 이름을 "VideoImage"로 지어 주세요.
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));

		UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Background"));
		Background->SetBrushColor(FLinearColor::Black);
		if (UOverlaySlot* BgSlot = Root->AddChildToOverlay(Background))
		{
			BgSlot->SetHorizontalAlignment(HAlign_Fill);
			BgSlot->SetVerticalAlignment(VAlign_Fill);
		}

		UScaleBox* VideoBox = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass(), TEXT("VideoBox"));
		VideoBox->SetStretch(EStretch::ScaleToFit);
		VideoBox->SetContent(WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("VideoImage")));
		if (UOverlaySlot* VideoSlot = Root->AddChildToOverlay(VideoBox))
		{
			VideoSlot->SetHorizontalAlignment(HAlign_Fill);
			VideoSlot->SetVerticalAlignment(VAlign_Fill);
		}

		if (!SkipText.IsEmpty())
		{
			UTextBlock* SkipLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SkipLabel"));
			SkipLabel->SetText(SkipText);
			SkipLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.75f, 0.7f, 0.7f)));
			FSlateFontInfo Font = SkipLabel->GetFont();
			Font.Size = 16;
			SkipLabel->SetFont(Font);
			if (UOverlaySlot* SkipSlot = Root->AddChildToOverlay(SkipLabel))
			{
				SkipSlot->SetHorizontalAlignment(HAlign_Right);
				SkipSlot->SetVerticalAlignment(VAlign_Bottom);
				SkipSlot->SetPadding(FMargin(0.0f, 0.0f, 40.0f, 30.0f));
			}
		}

		WidgetTree->RootWidget = Root;
	}

	VideoImage = Cast<UImage>(GetWidgetFromName(TEXT("VideoImage")));

	// 키 입력으로 건너뛸 수 있도록 포커스를 받습니다.
	SetIsFocusable(true);

	return Super::RebuildWidget();
}

bool UCosIntroVideoWidget::PlayIntro(UMediaSource* MediaSource)
{
	if (!IsValid(MediaSource))
	{
		return false;
	}

	MediaPlayer = NewObject<UMediaPlayer>(this);
	MediaPlayer->PlayOnOpen = true;
	MediaPlayer->SetLooping(false);
	MediaPlayer->OnEndReached.AddDynamic(this, &UCosIntroVideoWidget::HandleEndReached);
	MediaPlayer->OnMediaOpenFailed.AddDynamic(this, &UCosIntroVideoWidget::HandleOpenFailed);

	MediaTexture = NewObject<UMediaTexture>(this);
	MediaTexture->AutoClear = true;
	MediaTexture->ClearColor = FLinearColor::Black; // 첫 프레임이 나오기 전까지 검은 화면
	MediaTexture->SetMediaPlayer(MediaPlayer);
	MediaTexture->UpdateResource();

	if (IsValid(VideoImage))
	{
		VideoImage->SetBrushResourceObject(MediaTexture);
		VideoImage->SetDesiredSizeOverride(VideoSize);
	}

	// 영상 소리는 액터에 붙은 사운드 컴포넌트로만 나오므로 소유 컨트롤러에 붙입니다.
	// 타이틀에서 걸어둔 일시정지 중에도 들리도록 UI 사운드로 재생합니다.
	if (APlayerController* PC = GetOwningPlayer())
	{
		MediaSound = NewObject<UMediaSoundComponent>(PC);
		MediaSound->bIsUISound = true;
		MediaSound->SetTickableWhenPaused(true);
		MediaSound->SetMediaPlayer(MediaPlayer);
		MediaSound->RegisterComponent();
		MediaSound->Activate(true);
	}

	if (!MediaPlayer->OpenSource(MediaSource))
	{
		StopMedia();
		return false;
	}

	if (UWorld* World = GetWorld())
	{
		PlayStartRealTime = World->GetRealTimeSeconds();
	}
	bFinished = false;
	return true;
}

void UCosIntroVideoWidget::FinishIntro()
{
	if (bFinished)
	{
		return;
	}
	bFinished = true;

	StopMedia();
	OnFinished.Broadcast();
}

void UCosIntroVideoWidget::NativeDestruct()
{
	StopMedia();
	Super::NativeDestruct();
}

FReply UCosIntroVideoWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	TrySkip();
	return FReply::Handled();
}

FReply UCosIntroVideoWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	TrySkip();
	return FReply::Handled();
}

bool UCosIntroVideoWidget::TrySkip()
{
	const UWorld* World = GetWorld();
	if (World && World->GetRealTimeSeconds() - PlayStartRealTime < SkipBlockSeconds)
	{
		return false;
	}

	FinishIntro();
	return true;
}

void UCosIntroVideoWidget::HandleEndReached()
{
	FinishIntro();
}

void UCosIntroVideoWidget::HandleOpenFailed(FString FailedUrl)
{
	UE_LOG(LogTemp, Warning, TEXT("[Intro] 인트로 영상을 열지 못했습니다: %s"), *FailedUrl);
	FinishIntro();
}

void UCosIntroVideoWidget::StopMedia()
{
	if (IsValid(MediaPlayer))
	{
		MediaPlayer->OnEndReached.RemoveAll(this);
		MediaPlayer->OnMediaOpenFailed.RemoveAll(this);
		MediaPlayer->Close();
	}

	if (IsValid(MediaSound))
	{
		MediaSound->DestroyComponent(); // 등록 해제되면서 소리도 멈춥니다.
		MediaSound = nullptr;
	}
}
