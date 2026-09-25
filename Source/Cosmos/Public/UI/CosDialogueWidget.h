#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CosDialogueWidget.generated.h"

class UTextBlock;

// 화면 아래쪽에 한 줄 대사를 타자 치듯 보여주는 기본 대사창. 위젯 에셋 없이도 동작합니다.
// 클릭하면 남은 글자를 한 번에 보여주고, 다 보인 상태에서 클릭하거나 HoldSeconds가 지나면 닫힙니다.
UCLASS(Blueprintable)
class COSMOS_API UCosDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ShowLine(const FText& InSpeaker, const FText& InLine);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 초당 표시할 글자 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (ClampMin = "1.0"))
	float CharsPerSecond = 18.0f;

	// 다 보인 뒤 자동으로 닫히기까지(초). 0이면 클릭할 때까지 남아 있습니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (ClampMin = "0.0"))
	float HoldSeconds = 4.0f;

private:
	void Close();

	// BP에서 꾸밀 때 같은 이름의 TextBlock을 두면 그걸 씁니다.
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SpeakerLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LineLabel;

	FString FullLine;
	float RevealedChars = 0.0f;
	float HoldElapsed = 0.0f;
};
