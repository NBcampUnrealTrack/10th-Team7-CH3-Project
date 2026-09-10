// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CosPlayerController.generated.h"


class UUserWidget;
class UInputMappingContext;
class UHealthComponent;




UCLASS()
class COSMOS_API ACosPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	ACosPlayerController();


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
	void ShowResult(bool bWin, int32 Score);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideResult();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleESCMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideESCMenu();

	UFUNCTION(BlueprintCallable, Category = "UI|Input")
	void SetUIInputMode(bool bUIMode);

	UFUNCTION()
	void UpdateHP(float CurrentHealth, float MaxHealth);



	// 테스트용 임시
	void TestDecreaseAmmo();
	void TestReloadAmmo();
	void TestDecreaseHP();
	void TestHealHP();


	UFUNCTION()
	void UpdateAmmoUI(int32 CurrentAmmo, int32 MaxAmmo);
	
	UFUNCTION()
	void SetupInputComponent();

protected:
	virtual void BeginPlay() override;

};
