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
};
