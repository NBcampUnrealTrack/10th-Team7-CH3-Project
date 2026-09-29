#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CosDebugWidget.generated.h"

class UTextBlock;

UCLASS()
class COSMOS_API UCosDebugWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText;
	float RefreshDelay = 0.0f;
};
