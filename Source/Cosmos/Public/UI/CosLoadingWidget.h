#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CosLoadingWidget.generated.h"

// 화면 전체를 덮는 기본 로딩 화면. 위젯 에셋 없이도 검은 배경 + "LOADING..." 문구로 동작합니다.
// BP에서 상속해 디자이너로 꾸미면 그 레이아웃을 그대로 씁니다.
UCLASS(Blueprintable)
class COSMOS_API UCosLoadingWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	// 디자이너 레이아웃이 없을 때 표시할 문구
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loading")
	FText LoadingText = FText::FromString(TEXT("LOADING..."));
};
