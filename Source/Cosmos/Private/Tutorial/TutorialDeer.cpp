#include "Tutorial/TutorialDeer.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"

ATutorialDeer::ATutorialDeer()
{
	PrimaryActorTick.bCanEverTick = false;

	HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
	SetRootComponent(HitBox);
	HitBox->InitBoxExtent(FVector(70.0f, 35.0f, 75.0f));
	HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HitBox->SetCollisionObjectType(ECC_Enemies);
	HitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	HitBox->SetCollisionResponseToChannel(ECC_Weapon, ECR_Block);
	HitBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	HitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	HitBox->SetGenerateOverlapEvents(false);
	HitBox->SetCanEverAffectNavigation(false);

	DeerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DeerMesh"));
	DeerMesh->SetupAttachment(HitBox);
	DeerMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -75.0f));
	DeerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DeerMesh->SetGenerateOverlapEvents(false);
	DeerMesh->SetCanEverAffectNavigation(false);

	BreathingAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("BreathingAudio"));
	BreathingAudio->SetupAttachment(HitBox);
	BreathingAudio->bAutoActivate = false; // BeginPlay에서 거리 설정을 적용한 뒤 재생합니다.
}

void ATutorialDeer::BeginPlay()
{
	Super::BeginPlay();

	if (!BreathingSound || bIsDead)
	{
		return;
	}

	// 가까이 갈수록 크게, 멀어지면 들리지 않게 거리 감쇠를 직접 지정합니다.
	BreathingAudio->SetSound(BreathingSound);
	BreathingAudio->bOverrideAttenuation = true;
	BreathingAudio->AttenuationOverrides.bAttenuate = true;
	BreathingAudio->AttenuationOverrides.bSpatialize = true;
	BreathingAudio->AttenuationOverrides.AttenuationShape = EAttenuationShape::Sphere;
	BreathingAudio->AttenuationOverrides.AttenuationShapeExtents = FVector(BreathingFullVolumeRadius, 0.0f, 0.0f);
	BreathingAudio->AttenuationOverrides.FalloffDistance = BreathingFalloffDistance;

	// BeginPlay는 타이틀 일시정지 중이라 이때 틀면 들리지 않는 거리의 루프 사운드로 취급돼 버려질 수 있습니다.
	// 그래서 플레이어(카메라)가 들리는 거리에 들어오면 재생하고, 벗어나면 멈춥니다. 타이머는 일시정지 중엔 돌지 않습니다.
	FTimerHandle BreathingRangeTimer;
	GetWorldTimerManager().SetTimer(BreathingRangeTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		if (bIsDead)
		{
			GetWorldTimerManager().ClearAllTimersForObject(this);
			return;
		}

		const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if (!PC || !PC->PlayerCameraManager)
		{
			return;
		}

		const float AudibleRange = BreathingFullVolumeRadius + BreathingFalloffDistance;
		const bool bInRange = FVector::DistSquared(PC->PlayerCameraManager->GetCameraLocation(), GetActorLocation())
			<= FMath::Square(AudibleRange);

		if (bInRange && !BreathingAudio->IsPlaying())
		{
			BreathingAudio->Play();
		}
		else if (!bInRange && BreathingAudio->IsPlaying())
		{
			BreathingAudio->Stop(); // 경계에서는 감쇠로 이미 무음이라 끊겨도 티가 나지 않습니다.
		}
	}), 0.25f, true, 0.0f);
}

void ATutorialDeer::TakeHitAlt(float Damage, EWeaponType Weapon, const FHitResult& Hit)
{
	// 맞은 느낌: 죽은 뒤에 맞아도 피는 튀지만, 산탄이 한꺼번에 맞을 땐 0.05초에 한 번만 냅니다.
	const float Now = GetWorld()->GetTimeSeconds();
	if (Hit.bBlockingHit && FMath::IsFinite(Damage) && Damage > 0.0f
		&& (LastHitFeedbackTime < 0.0f || Now - LastHitFeedbackTime >= 0.05f))
	{
		LastHitFeedbackTime = Now;
		const FRotator Rotation = FRotationMatrix::MakeFromZ(Hit.ImpactNormal).Rotator();
		if (HitVFX)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, HitVFX, Hit.ImpactPoint, Rotation, HitVFXScale,
				true, true, ENCPoolMethod::AutoRelease, true);
		}
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, Hit.ImpactPoint);
		}
	}

	TakeHit(Damage, Weapon);
}

void ATutorialDeer::TakeHit(float Damage, EWeaponType Weapon)
{
	// 샷건의 여러 판정이나 연타에도 진행 이벤트는 한 번만 보냅니다.
	if (bIsDead || IsActorBeingDestroyed() || !FMath::IsFinite(Damage) || Damage <= 0.0f)
	{
		return;
	}

	bIsDead = true;
	BreathingAudio->Stop(); // 죽으면 숨소리가 끊깁니다.
	OnDied();
	OnDeerKilled.Broadcast(this);
}

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTutorialDeerTest, "Cosmos.Tutorial.Deer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTutorialDeerTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Test world"), World))
	{
		return false;
	}
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	ATutorialDeer* Deer = World->SpawnActor<ATutorialDeer>();
	if (!TestNotNull(TEXT("Deer"), Deer))
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		return false;
	}

	// 실제 무기가 사용하는 채널/오브젝트 쿼리로 사슴이 감지되는지 검사합니다.
	FHitResult ShotHit;
	const FVector Start(-200.0f, 0.0f, 0.0f), End(200.0f, 0.0f, 0.0f);
	TestTrue(TEXT("Shotgun trace finds deer"), World->LineTraceSingleByChannel(ShotHit, Start, End, ECC_Weapon));
	TestTrue(TEXT("Shotgun hit belongs to deer"), ShotHit.GetActor() == Deer);
	TArray<FHitResult> NailHits;
	FCollisionObjectQueryParams Objects;
	Objects.AddObjectTypesToQuery(ECC_Enemies);
	TestTrue(TEXT("Nail sweep finds deer"), World->SweepMultiByObjectType(
		NailHits, Start, End, FQuat::Identity, Objects, FCollisionShape::MakeSphere(20.0f)));
	TestTrue(TEXT("Nail hit belongs to deer"), NailHits.ContainsByPredicate(
		[Deer](const FHitResult& Hit) { return Hit.GetActor() == Deer; }));
	IDamageable* Damageable = Cast<IDamageable>(Deer);
	TestNotNull(TEXT("Existing damage interface"), Damageable);
	Deer->TakeHit(0.0f, EWeaponType::Shotgun);
	Deer->TakeHit(-10.0f, EWeaponType::Nail);
	TestFalse(TEXT("Non-positive damage ignored"), Deer->IsDead());
	Deer->TakeHit(1.0f, EWeaponType::Shotgun);
	TestTrue(TEXT("One valid hit kills"), Deer->IsDead());
	TestFalse(TEXT("Dead deer is not destroyed"), Deer->IsActorBeingDestroyed());
	Deer->TakeHit(100.0f, EWeaponType::Nail);
	TestTrue(TEXT("Repeated hit preserves dead state"), Deer->IsDead());
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}
#endif
