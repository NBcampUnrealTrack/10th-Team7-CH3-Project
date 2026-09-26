// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CosPlayerController.generated.h"



class UUserWidget;
class UInputMappingContext;
class UHealthComponent;
class AShotgunWeapon;
class UCameraShakeBase;
class UCameraComponent;
class USoundBase;
class UAudioComponent;
class UMediaPlayer;
class UMediaSource;
class UCosDialogueWidget;
class UCosIntroVideoWidget;

USTRUCT(BlueprintType)
struct FWaveResultData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 WaveNumber = 0;

	UPROPERTY(BlueprintReadOnly)
	float ElapsedSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 SoulEarned = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalSoul = 0;
};

UCLASS()
class COSMOS_API ACosPlayerController : public APlayerController
{

	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateEnemyCountUI(int32 RemainingEnemies, int32 TotalEnemies);

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio")
	void OnForgeBGMRequested();

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio")
	void OnCombatBGMRequested();

	// 보스 스테이지 시작 시 호출. BP에서 구현하지 않으면 기본 전투 BGM을 재생합니다.
	UFUNCTION(BlueprintNativeEvent, Category = "Audio")
	void OnBossBGMRequested();

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio")
	void OnGameOverBGMRequested();

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio")
	void OnTitleBGMRequested();

	UFUNCTION(BlueprintImplementableEvent, Category = "Audio")
	void OnStopBGMRequested();

	UFUNCTION()
	void HandleGameOverRequested();

	ACosPlayerController();

	UFUNCTION()
	void UpdateSoulUI(int32 CurrentSoul);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateWaveUI(int32 CurrentWave);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Classes")
	TSubclassOf<UUserWidget> TitleWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Instances")
	TObjectPtr<UUserWidget> TitleWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Classes")
	TSubclassOf<UUserWidget> CombatHUDClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Instances")
	TObjectPtr<UUserWidget> CombatHUDInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Classes")
	TSubclassOf<UUserWidget> ForgeWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Instances")
	TObjectPtr<UUserWidget> ForgeWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Classes")
	TSubclassOf<UUserWidget> ResultWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Instances")
	TObjectPtr<UUserWidget> ResultWidgetInstance;

	// 결과창 → 대장간 사이에 띄우는 로딩 화면. 기본값은 검은 배경 + LOADING 문구입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Classes")
	TSubclassOf<UUserWidget> LoadingWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Instances")
	TObjectPtr<UUserWidget> LoadingWidgetInstance;

	// 대장간 배경 영상을 재생하는 미디어 플레이어. 이게 재생을 시작해야 로딩 화면을 걷습니다. 비우면 최소 시간만 기다립니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Loading")
	TObjectPtr<UMediaPlayer> ForgeMediaPlayer;

	// 영상이 빨리 준비돼도 로딩 화면을 최소 이만큼(초) 보여줍니다. 깜빡임 방지.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Loading", meta = (ClampMin = "0.0"))
	float ForgeLoadingMinSeconds = 1.0f;

	// 영상이 끝내 준비되지 않아도 이 시간(초)이 지나면 로딩 화면을 걷습니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Loading", meta = (ClampMin = "0.0"))
	float ForgeLoadingMaxSeconds = 5.0f;

	// Day 1에 쓰러졌다가 대장간에서 깨어날 때 한 번 띄우는 대사
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Dialogue")
	TSubclassOf<UCosDialogueWidget> DialogueWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Dialogue")
	FText FirstForgeSpeaker;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Dialogue", meta = (MultiLine = true))
	FText FirstForgeLine = FText::FromString(TEXT("정신이 들어? 여기는 대장간이야. 무기를 강화해 줄게."));

	// 타이틀에서 시작을 누른 뒤, 게임이 시작되기 전에 재생할 오프닝 영상 위젯
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Intro")
	TSubclassOf<UCosIntroVideoWidget> IntroVideoWidgetClass;

	// 오프닝 영상(File Media Source 에셋). 비워두면 아래 IntroMoviePath의 파일을 찾아 재생합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Intro")
	TObjectPtr<UMediaSource> IntroMediaSource;

	// IntroMediaSource가 비었을 때 재생할 파일(Content 폴더 기준). 파일이 없으면 영상 없이 바로 시작합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Intro")
	FString IntroMoviePath = TEXT("Movies/Intro.mp4");

	// 켜면 게임을 켠 뒤 처음 시작할 때만 영상을 보여주고, 재시작할 때는 건너뜁니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Intro")
	bool bPlayIntroOnlyOnce = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Classes")
	TSubclassOf<UUserWidget> ESCWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Instances")
	TObjectPtr<UUserWidget> ESCWidgetInstance;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<class UInputAction> ESCAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSubclassOf<UCameraShakeBase> CosLegacyCameraShakeClass; // 피격 시 재생할 카메라 흔들림

	void UpdateKillUI(int32 KillCount);

	UFUNCTION()
	void HandleStageTimeChanged(float RemainingSeconds);

	void UpdateStageTimeUI(float RemainingSeconds);

	UFUNCTION()
	void RefreshCombatHUD();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowTitleWidget();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideTitleWidget();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowCombatHUD();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void CloseCombatHUD();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowGameHUD();

	UFUNCTION(BlueprintPure, Category = "HUD")
	UUserWidget* GetHUDWidget() const;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void OpenForgeWidget();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void CloseForgeWidget();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowResult(const FWaveResultData& ResultData);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideResult();

	// Day 1 종료 연출. 입력을 막고 시점이 풀썩 쓰러지며 화면이 어두워진 뒤 결과창을 띄웁니다.
	// 대장간이 열릴 때 시점을 되돌리고 화면을 밝혀 "깨어나는" 것으로 이어집니다.
	void PlayCollapseThenShowResult(const FWaveResultData& ResultData);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleESCMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideESCMenu();

	UFUNCTION(BlueprintCallable, Category = "UI|Input")
	void SetUIInputMode(bool bUIMode);

	UFUNCTION()
	void SetupCharacterBindings();

	UFUNCTION()
	void OnCharacterDeath();

	UFUNCTION()
	void UpdateHP(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void UpdateAmmoUI(int32 CurrentAmmo, int32 MaxAmmo);

	UFUNCTION(BlueprintCallable)
	void ShowGameOver(int32 Score);

	UFUNCTION(BlueprintCallable)
	void HideGameOver();

	UFUNCTION()
	void HandleForgeRequested();
/*
	// 테스트용 임시
	void TestDecreaseAmmo();
	void TestReloadAmmo();
	void TestDecreaseHP();
	void TestHealHP();
*/

	UFUNCTION()
	void SetupInputComponent();

	UFUNCTION()
	void HandleGameStateChanged();

	UFUNCTION()
	void UpdatePotionUI(int32 CurrentPotion);

	UFUNCTION()
	void HandleKillConfirmed(AActor* Victim); // 킬 마커 표시하기 위한 함수

	UFUNCTION()
	void HandleHitConfirmed(AActor* Target); // 추가

	UFUNCTION()
	void HandlePlayerDamaged(float Damage);

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;

	// 마지막 적을 잡은 뒤 쓰러지기 전까지 심박 소리만 들리는 시간(초)
	UPROPERTY(EditDefaultsOnly, Category = "Day1|Collapse", meta = (ClampMin = "0.0"))
	float HeartbeatDuration = 2.5f;

	// 쓰러지기 전부터 재생할 심박 소리. 화면이 어두워지는 동안 서서히 작아집니다.
	// 구간보다 짧은 소리면 반복되도록 Looping을 켠 사운드(큐)를 넣으세요. 비워두면 재생하지 않습니다.
	UPROPERTY(EditDefaultsOnly, Category = "Day1|Collapse")
	TObjectPtr<USoundBase> HeartbeatSound;

	// 심박이 들리는 동안 화면이 붉게 번쩍이는 횟수. 0이면 번쩍이지 않습니다.
	UPROPERTY(EditDefaultsOnly, Category = "Day1|Collapse|Flash", meta = (ClampMin = "0"))
	int32 HeartbeatFlashCount = 2;

	// 심박 시작 후 첫 번쩍임까지(초)
	UPROPERTY(EditDefaultsOnly, Category = "Day1|Collapse|Flash", meta = (ClampMin = "0.0"))
	float HeartbeatFlashFirstDelay = 0.3f;

	// 번쩍임 사이 간격(초). 심박 소리 박자에 맞추세요.
	UPROPERTY(EditDefaultsOnly, Category = "Day1|Collapse|Flash", meta = (ClampMin = "0.05"))
	float HeartbeatFlashInterval = 0.9f;

	UPROPERTY(EditDefaultsOnly, Category = "Day1|Collapse|Flash")
	FLinearColor HeartbeatFlashColor = FLinearColor(0.6f, 0.0f, 0.0f);

	// 번쩍이는 순간의 진하기(0~1)
	UPROPERTY(EditDefaultsOnly, Category = "Day1|Collapse|Flash", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HeartbeatFlashAlpha = 0.45f;

	// 번쩍인 뒤 사라지는 시간(초)
	UPROPERTY(EditDefaultsOnly, Category = "Day1|Collapse|Flash", meta = (ClampMin = "0.0"))
	float HeartbeatFlashDuration = 0.5f;

	// 시점이 바닥까지 떨어지는 데 걸리는 시간(초)
	UPROPERTY(EditDefaultsOnly, Category = "Day1|Collapse", meta = (ClampMin = "0.01"))
	float CollapseFallDuration = 0.9f;

	// 카메라가 내려가는 거리(cm). 눈높이에서 바닥 근처까지.
	UPROPERTY(EditDefaultsOnly, Category = "Day1|Collapse", meta = (ClampMin = "0.0"))
	float CollapseCameraDrop = 140.0f;

	// 옆으로 쓰러지며 기울어지는 각도(도). 음수면 반대쪽으로 쓰러집니다.
	UPROPERTY(EditDefaultsOnly, Category = "Day1|Collapse", meta = (ClampMin = "-89.0", ClampMax = "89.0"))
	float CollapseRoll = 75.0f;

	// 쓰러진 뒤 시선의 위아래 각도(도). 0이면 수평.
	UPROPERTY(EditDefaultsOnly, Category = "Day1|Collapse", meta = (ClampMin = "-89.0", ClampMax = "89.0"))
	float CollapseEndPitch = 0.0f;

	// 아래 시간들은 심박이 끝나고 쓰러지기 시작한 순간부터 셉니다.
	// 쓰러지기 시작한 뒤 화면이 어두워지기 시작할 때까지(초)
	UPROPERTY(EditDefaultsOnly, Category = "Day1|Collapse", meta = (ClampMin = "0.0"))
	float CollapseFadeDelay = 0.4f;

	// 화면이 완전히 검게 되기까지 걸리는 시간(초)
	UPROPERTY(EditDefaultsOnly, Category = "Day1|Collapse", meta = (ClampMin = "0.0"))
	float CollapseFadeDuration = 1.6f;

	// 쓰러지기 시작한 뒤 결과창이 뜰 때까지(초)
	UPROPERTY(EditDefaultsOnly, Category = "Day1|Collapse", meta = (ClampMin = "0.0"))
	float CollapseResultDelay = 3.0f;

	// 대장간에서 깨어날 때 화면이 밝아지는 시간(초)
	UPROPERTY(EditDefaultsOnly, Category = "Day1|Collapse", meta = (ClampMin = "0.0"))
	float WakeUpFadeDuration = 1.5f;

	// 바닥에 털썩 닿는 순간 재생할 소리. 비워두면 재생하지 않습니다.
	UPROPERTY(EditDefaultsOnly, Category = "Day1|Collapse")
	TObjectPtr<USoundBase> CollapseSound;

private:
	// 오프닝 영상을 띄웠으면 true. 영상이 끝나면 ShowGameHUD를 다시 불러 게임을 시작합니다.
	bool TryPlayIntro();
	UMediaSource* ResolveIntroMediaSource();
	void HandleIntroFinished();

	UPROPERTY(Transient)
	TObjectPtr<UCosIntroVideoWidget> IntroVideoWidgetInstance;

	bool bIntroChecked = false; // 이 레벨에서 영상 재생 여부를 이미 판단했는지

	void CheckForgeLoading();
	void HideLoadingWidget();
	FTimerHandle ForgeLoadingTimer;
	float ForgeLoadingStartTime = 0.0f;

	void ShowFirstForgeDialogue();
	bool bPendingFirstForgeDialogue = false;

	void BeginCollapseFall();
	void PlayHeartbeatFlash();
	void UpdateCollapseCamera();
	void StartCollapseFade();
	void FinishCollapse();
	void RecoverFromCollapse();

	FTimerHandle CollapseFallTimer;
	FTimerHandle HeartbeatFlashTimer;
	int32 HeartbeatFlashesLeft = 0;
	FTimerHandle CollapseCameraTimer;
	FTimerHandle CollapseFadeTimer;
	FTimerHandle CollapseResultTimer;

	FWaveResultData PendingCollapseResult;
	TWeakObjectPtr<UCameraComponent> CollapseCamera;
	TWeakObjectPtr<UAudioComponent> HeartbeatAudio;
	FVector CollapseCameraStartLocation = FVector::ZeroVector;
	float CollapseStartPitch = 0.0f;
	float CollapseStartTime = 0.0f;
	bool bCollapseImpactPlayed = false;
	bool bIsCollapsing = false; // 쓰러지는 중. 결과창이 뜨기 전까지 입력을 막습니다.
	bool bIsCollapsed = false;  // 대장간에서 깨어나기 전까지 true
};
