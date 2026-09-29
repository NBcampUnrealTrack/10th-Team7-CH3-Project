#include "UI/CosBossHealthWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Character/CosGameMode.h"
#include "Character/HealthComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/ScaleBox.h"
#include "Components/SizeBox.h"
#include "Enemy/Boss.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

UCosBossHealthWidget::UCosBossHealthWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // A hard class reference also keeps the shared HUD textures in cooked builds.
    static ConstructorHelpers::FClassFinder<UUserWidget> SharedHealthBar(TEXT("/Game/Cosmos/WBP/WBP_HPBar"));
    HealthBarClass = SharedHealthBar.Class;
}

TSharedRef<SWidget> UCosBossHealthWidget::RebuildWidget()
{
    if (WidgetTree && !WidgetTree->RootWidget)
    {
        UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
        WidgetTree->RootWidget = Root;
        Panel = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass(), TEXT("BossHealthPanel"));
        Panel->SetStretch(EStretch::ScaleToFit);
        UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(Panel);
        PanelSlot->SetAnchors(FAnchors(0.20f, 0.0f, 0.80f, 0.0f));
        PanelSlot->SetOffsets(FMargin(0.0f, 0.0f, 0.0f, 150.0f));
        USizeBox* DesignSize = WidgetTree->ConstructWidget<USizeBox>();
        DesignSize->SetWidthOverride(834.0f);
        DesignSize->SetHeightOverride(200.0f);
        Panel->SetContent(DesignSize);
        if (HealthBarClass)
        {
            HealthVisual = CreateWidget<UUserWidget>(GetOwningPlayer(), HealthBarClass);
            if (HealthVisual)
            {
                DesignSize->SetContent(HealthVisual);
                HealthBar = Cast<UProgressBar>(HealthVisual->GetWidgetFromName(TEXT("HPBar")));
                HealthText = Cast<UTextBlock>(HealthVisual->GetWidgetFromName(TEXT("HPText")));
            }
        }
        Panel->SetVisibility(ESlateVisibility::Collapsed);
        SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    return Super::RebuildWidget();
}

void UCosBossHealthWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	const ACosGameMode* GM = GetWorld()->GetAuthGameMode<ACosGameMode>();
	if (!Panel || !HealthBar || !HealthText || !GM || !GM->IsBossStage())
	{
		if (Panel) Panel->SetVisibility(ESlateVisibility::Collapsed);
		Boss.Reset();
		return;
	}
	if (!Boss.IsValid())
	{
		for (TActorIterator<ABoss> It(GetWorld()); It; ++It)
		{
			if (It->IsAlive()) { Boss = *It; break; }
		}
	}
	const UHealthComponent* Health = Boss.IsValid() ? Boss->FindComponentByClass<UHealthComponent>() : nullptr;
	if (!Health || Health->IsDead())
	{
		Panel->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	Panel->SetVisibility(ESlateVisibility::HitTestInvisible);
	const float MaxHealth = Health->GetMaxHealth();
	HealthBar->SetPercent(MaxHealth > 0.0f ? FMath::Clamp(Health->GetCurrentHealth() / MaxHealth, 0.0f, 1.0f) : 0.0f);
	HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Health->GetCurrentHealth(), MaxHealth)));
}
