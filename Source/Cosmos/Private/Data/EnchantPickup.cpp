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
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("Overlap!!!")));
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

	UE_LOG(LogTemp, Warning, TEXT("인챈트를 획득합니다!!"));
	if (UCosGameInstance* GameInstance = Cast<UCosGameInstance>(GetWorld()->GetGameInstance()))
	{
		if (GameInstance->CollectEnchant(GeneratedEnchant))
		{
			DestroyEnchant();
		}
		else
		{
			// 획득하지 못하면 (인챈트 인벤토리가 꽉 차서)
			// 어떻게 처리??
			// 일단 지금은 무한 획득 가능하게 되는 거라 딱히 별도의 처리는 불 필요
		}
	}
	
}

void AEnchantPickup::DestroyEnchant()
{
	Destroy();
}