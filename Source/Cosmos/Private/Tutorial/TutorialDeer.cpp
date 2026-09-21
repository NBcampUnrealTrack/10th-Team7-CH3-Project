#include "Tutorial/TutorialDeer.h"
#include "Animation/AnimSequence.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"

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
}

void ATutorialDeer::TakeHit(float Damage, EWeaponType Weapon)
{
	// 샷건의 여러 판정이나 연타에도 진행 이벤트는 한 번만 보냅니다.
	if (bIsDead || IsActorBeingDestroyed() || !FMath::IsFinite(Damage) || Damage <= 0.0f)
	{
		return;
	}

	bIsDead = true;
	SetActorEnableCollision(false);
	if (DeathAnimation && DeerMesh->GetSkeletalMeshAsset())
	{
		DeerMesh->PlayAnimation(DeathAnimation, false);
	}
	else if (bHideMeshWithoutDeathAnimation)
	{
		DeerMesh->SetVisibility(false, true);
	}

	OnDeathPresentation(Weapon);
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
	TestFalse(TEXT("Dead deer collision disabled"), Deer->GetActorEnableCollision());
	TestFalse(TEXT("Dead deer no longer blocks shots"), World->LineTraceSingleByChannel(ShotHit, Start, End, ECC_Weapon));
	Deer->TakeHit(100.0f, EWeaponType::Nail);
	TestTrue(TEXT("Repeated hit preserves dead state"), Deer->IsDead());
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}
#endif
