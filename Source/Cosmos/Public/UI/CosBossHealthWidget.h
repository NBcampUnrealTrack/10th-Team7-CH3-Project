#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CosBossHealthWidget.generated.h"

class ABoss;
class UScaleBox;
class UProgressBar;
class UTextBlock;

// Shares the combat HUD lifetime; reads the spawned boss's actual health component.
UCLASS()
class COSMOS_API UCosBossHealthWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UCosBossHealthWidget(const FObjectInitializer& ObjectInitializer);
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(Transient)
	TObjectPtr<UScaleBox> Panel;
	UPROPERTY(EditDefaultsOnly, Category = "Boss HUD")
	TSubclassOf<UUserWidget> HealthBarClass;
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> HealthVisual;
	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> HealthBar;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HealthText;
	TWeakObjectPtr<ABoss> Boss;
};
