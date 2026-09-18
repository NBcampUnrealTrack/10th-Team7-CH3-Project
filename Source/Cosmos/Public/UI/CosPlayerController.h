// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CosPlayerController.generated.h"



class UUserWidget;
class UInputMappingContext;
class UHealthComponent;
class AShotgunWeapon;

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

private:
	int32 CachedMaxAmmo = 0;

public:

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateEnemyCountUI(int32 RemainingEnemies, int32 TotalEnemies);

	ACosPlayerController();
	FTimerHandle WeaponBindRetryTimer;

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
	void UpdateAmmoUI(int32 CurrentAmmo);

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

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override; 
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
};
