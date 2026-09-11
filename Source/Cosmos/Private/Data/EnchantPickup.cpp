#include "Data/EnchantPickup.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Data/EnchantGenerator.h"


AEnchantPickup::AEnchantPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Collision->SetupAttachment(Scene);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(Collision);

	// 이벤트 바인딩
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AEnchantPickup::OnEnchantOverlap);
	Collision->OnComponentEndOverlap.AddDynamic(this, &AEnchantPickup::OnEnchantEndOverlap);

}

void AEnchantPickup::BeginPlay()
{
	Super::BeginPlay();

	GeneratedEnchant = UEnchantGenerator::GenerateRandomEnchant(this, StatPool, SkillPool);

	if (!GeneratedEnchant)
	{
		UE_LOG(LogTemp, Warning, TEXT("Enchant Make Fail"));
	}
	
}

void AEnchantPickup::OnEnchantOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	// 중요. 캐릭터 태그가 Player라 되어 있어야 인챈트를 획득할 수 있음
	if (OtherActor && OtherActor->ActorHasTag("Player"))
	{
		ActivateEnchant(OtherActor);
	}
}

void AEnchantPickup::OnEnchantEndOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
)
{

}

void AEnchantPickup::ActivateEnchant(AActor* Activator)
{
	if (!GeneratedEnchant)
	{
		return;
	}

	if (UCosGameInstance* GameInstance = Cast<UCosGameInstance>(GetWorld()->GetGameInstance()))
	{
		GameInstance->EquipEnchant(GeneratedEnchant);
	}
	DestroyEnchant();
}

void AEnchantPickup::DestroyEnchant()
{
	Destroy();
}