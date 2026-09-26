#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CosIntroVideoWidget.generated.h"

class UMediaPlayer;
class UMediaSource;
class UMediaTexture;
class UMediaSoundComponent;
class UImage;

DECLARE_MULTICAST_DELEGATE(FOnIntroVideoFinished);

// 게임 시작 전에 전체 화면으로 오프닝 영상을 재생하는 위젯. 위젯 에셋 없이도 동작합니다.
// 영상이 끝나거나, 아무 키/마우스 버튼을 누르면 OnFinished를 한 번 알립니다.
// 게임이 일시정지된 상태에서도 영상과 소리가 재생됩니다.
UCLASS(Blueprintable)
class COSMOS_API UCosIntroVideoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 영상을 열고 재생합니다. 열지 못하면 false를 반환하고 OnFinished는 부르지 않습니다.
	bool PlayIntro(UMediaSource* MediaSource);

	// 영상을 멈추고 OnFinished를 알립니다. 여러 번 불러도 한 번만 알립니다.
	UFUNCTION(BlueprintCallable, Category = "Intro")
	void FinishIntro();

	FOnIntroVideoFinished OnFinished;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 화면 오른쪽 아래에 표시할 건너뛰기 안내. 비우면 표시하지 않습니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro")
	FText SkipText = FText::FromString(TEXT("아무 키나 눌러 건너뛰기"));

	// 시작 버튼을 누른 입력이 곧바로 건너뛰기로 이어지지 않도록, 이 시간(초) 동안은 건너뛰기를 막습니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro", meta = (ClampMin = "0.0"))
	float SkipBlockSeconds = 0.5f;

	// 영상 해상도. 화면 비율이 달라도 이 비율을 유지하며 화면에 맞춥니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intro")
	FVector2D VideoSize = FVector2D(1920.0, 1080.0);

private:
	UFUNCTION()
	void HandleEndReached();

	UFUNCTION()
	void HandleOpenFailed(FString FailedUrl);

	bool TrySkip();
	void StopMedia();

	UPROPERTY(Transient)
	TObjectPtr<UMediaPlayer> MediaPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UMediaTexture> MediaTexture;

	UPROPERTY(Transient)
	TObjectPtr<UMediaSoundComponent> MediaSound;

	UPROPERTY(Transient)
	TObjectPtr<UImage> VideoImage;

	double PlayStartRealTime = 0.0;
	bool bFinished = false;
};
