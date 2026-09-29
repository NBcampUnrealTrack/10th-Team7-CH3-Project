#include "UI/CosPlayerController.h"
#include "UI/CosDebugWidget.h"
#include "Character/CosCharacter.h"
#include "Character/CosGameMode.h"
#include "Character/HealthComponent.h"
#include "Data/CosGameInstance.h"
#include "Enemy/Boss.h"
#include "Tutorial/TutorialDeer.h"
#include "Weapon/CombatComponent.h"
#include "Weapon/ShotgunWeapon.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "InputCoreTypes.h"

void ACosPlayerController::DebugMessage(const FString& Message) const
{
#if !UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Display, TEXT("[CosDebug] %s"), *Message);
	if (GEngine) GEngine->AddOnScreenDebugMessage(7310, 4.0f, FColor(225, 195, 125), Message);
#endif
}

bool ACosPlayerController::HandleDebugKey(const FInputKeyEventArgs& EventArgs)
{
#if !UE_BUILD_SHIPPING
	if (EventArgs.Event != IE_Pressed) return false;
	if (IsInputKeyDown(EKeys::LeftControl) || IsInputKeyDown(EKeys::RightControl))
	{
		const FKey DayKeys[] = {EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five, EKeys::Six, EKeys::Seven};
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(DayKeys); ++Index)
		{
			if (EventArgs.Key == DayKeys[Index]) { CosDebugDay(Index + 1); return true; }
		}
	}
	if (EventArgs.Key == EKeys::F1) { CosDebugHelp(); return true; }
	if (EventArgs.Key == EKeys::F2) { CosDebugBoss(); return true; }
	if (EventArgs.Key == EKeys::F3) { CosDebugGod(); return true; }
	if (EventArgs.Key == EKeys::F4) { CosDebugRefill(); return true; }
	if (EventArgs.Key == EKeys::F5) { CosDebugKillEnemies(); return true; }
	if (EventArgs.Key == EKeys::F6) { CosDebugRestart(); return true; }
	if (EventArgs.Key == EKeys::F7)
	{
		const float Speed = UGameplayStatics::GetGlobalTimeDilation(this);
		CosDebugSpeed(Speed < 0.5f ? 1.0f : (Speed < 1.5f ? 2.0f : 0.25f));
		return true;
	}
	if (EventArgs.Key == EKeys::F9) { CosDebugBossDamage(); return true; }
#endif
	return false;
}

void ACosPlayerController::CosDebugHelp()
{
#if !UE_BUILD_SHIPPING
	if (DebugOverlayInstance)
	{
		DebugOverlayInstance->RemoveFromParent();
		DebugOverlayInstance = nullptr;
		return;
	}
	DebugOverlayInstance = CreateWidget<UCosDebugWidget>(this);
	if (DebugOverlayInstance) DebugOverlayInstance->AddToViewport(200);
#endif
}

void ACosPlayerController::CosDebugDay(int32 Day)
{
#if !UE_BUILD_SHIPPING
	if (Day < 1 || Day > 7) { DebugMessage(TEXT("Day must be 1..7")); return; }
	if (bDebugTravelPending) return;
	bDebugTravelPending = true;
	SetPause(false);
	UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
	if (UCosGameInstance* GI = GetGameInstance<UCosGameInstance>()) GI->bPendingGameplayReveal = false;
	DebugMessage(FString::Printf(TEXT("Restarting battlefield at Day %d"), Day));
	UGameplayStatics::OpenLevel(this, TEXT("/Game/Cosmos/Maps/L_BlockoutAstra"), true,
		FString::Printf(TEXT("CosDebugDay=%d"), Day));
#endif
}

void ACosPlayerController::CosDebugBoss()
{
#if !UE_BUILD_SHIPPING
	CosDebugDay(7);
#endif
}

void ACosPlayerController::CosDebugRestart()
{
#if !UE_BUILD_SHIPPING
	if (const ACosGameMode* GM = GetWorld()->GetAuthGameMode<ACosGameMode>()) CosDebugDay(GM->GetCurrentDay());
#endif
}

void ACosPlayerController::CosDebugGod()
{
#if !UE_BUILD_SHIPPING
	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->SetCanBeDamaged(!ControlledPawn->CanBeDamaged());
		DebugMessage(ControlledPawn->CanBeDamaged() ? TEXT("Invulnerability OFF") : TEXT("Invulnerability ON"));
	}
#endif
}

void ACosPlayerController::CosDebugRefill()
{
#if !UE_BUILD_SHIPPING
	if (APawn* ControlledPawn = GetPawn())
	{
		if (UHealthComponent* Health = ControlledPawn->FindComponentByClass<UHealthComponent>())
		{
			if (Health->IsDead()) { DebugMessage(TEXT("Player dead: use CosDebugRestart")); return; }
			Health->Heal(Health->GetMaxHealth());
		}
		if (UCombatComponent* Combat = ControlledPawn->FindComponentByClass<UCombatComponent>())
		{
			Combat->RefillPotions();
			if (AShotgunWeapon* Shotgun = Combat->GetShotgunWeapon()) Shotgun->RefillAmmo();
		}
		RefreshCombatHUD();
		DebugMessage(TEXT("Health, ammo and potions refilled"));
	}
#endif
}

void ACosPlayerController::CosDebugKillEnemies()
{
#if !UE_BUILD_SHIPPING
	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsOfClass(this, AEnemyBase::StaticClass(), Enemies);
	int32 Killed = 0;
	for (AActor* Actor : Enemies)
	{
		AEnemyBase* Enemy = Cast<AEnemyBase>(Actor);
		if (IsValid(Enemy) && Enemy->IsAlive())
		{
			Enemy->TakeHit(Enemy->GetMaxHP() * 100.0f + 1.0f, EWeaponType::None);
			++Killed;
		}
	}
	DebugMessage(FString::Printf(TEXT("Killed %d enemies (normal rewards and progression)"), Killed));
#endif
}

void ACosPlayerController::CosDebugSpeed(float Speed)
{
#if !UE_BUILD_SHIPPING
	if (!FMath::IsFinite(Speed) || Speed < 0.1f || Speed > 5.0f)
	{
		DebugMessage(TEXT("Speed must be 0.1..5.0; use 1 for normal"));
		return;
	}
	UGameplayStatics::SetGlobalTimeDilation(this, Speed);
	DebugMessage(FString::Printf(TEXT("Game speed %.2fx"), Speed));
#endif
}

void ACosPlayerController::CosDebugBossDamage(float Percent)
{
#if !UE_BUILD_SHIPPING
	if (!FMath::IsFinite(Percent) || Percent <= 0 || Percent > 100)
	{
		DebugMessage(TEXT("Boss damage percent must be > 0 and <= 100"));
		return;
	}
	for (TActorIterator<ABoss> It(GetWorld()); It; ++It)
	{
		if (It->IsAlive())
		{
			It->TakeHit(It->GetMaxHP() * Percent / 100.0f, EWeaponType::None);
			DebugMessage(FString::Printf(TEXT("Boss damaged: %.0f%% of max HP"), Percent));
			return;
		}
	}
	DebugMessage(TEXT("No living boss"));
#endif
}

void ACosPlayerController::CosDebugPlayerDamage(float Damage)
{
#if !UE_BUILD_SHIPPING
	if (!FMath::IsFinite(Damage) || Damage <= 0 || Damage > 1000000) return;
	if (ACosCharacter* ControlledPawn = Cast<ACosCharacter>(GetPawn()))
	{
		ControlledPawn->TakeHit(Damage, EWeaponType::None);
		DebugMessage(FString::Printf(TEXT("Player damage requested: %.0f (respects invulnerability)"), Damage));
	}
#endif
}

void ACosPlayerController::CosDebugSoul(int32 Amount)
{
#if !UE_BUILD_SHIPPING
	if (Amount <= 0 || Amount > 1000000) { DebugMessage(TEXT("Soul amount must be 1..1000000")); return; }
	if (UCosGameInstance* GI = GetGameInstance<UCosGameInstance>())
	{
		if (GI->GetSoul() > MAX_int32 - Amount) return;
		GI->AddSoul(Amount);
		UpdateSoulUI(GI->GetSoul());
		DebugMessage(FString::Printf(TEXT("Added %d Soul"), Amount));
	}
#endif
}

FString ACosPlayerController::GetDebugStatus() const
{
#if !UE_BUILD_SHIPPING
	const ACosGameMode* GM = GetWorld()->GetAuthGameMode<ACosGameMode>();
	const APawn* ControlledPawn = GetPawn();
	const UHealthComponent* Health = ControlledPawn ? ControlledPawn->FindComponentByClass<UHealthComponent>() : nullptr;
	return FString::Printf(TEXT("COSMOS  |  DEVELOPMENT\nDay %d   무적 %s   배속 %.2fx\n플레이어 체력 %.0f / %.0f"),
		GM ? GM->GetCurrentDay() : 0, ControlledPawn && !ControlledPawn->CanBeDamaged() ? TEXT("ON") : TEXT("OFF"),
		UGameplayStatics::GetGlobalTimeDilation(this), Health ? Health->GetCurrentHealth() : 0,
		Health ? Health->GetMaxHealth() : 0);
#else
	return FString();
#endif
}
