#include "Weapon/CombatComponent.h"
#include "Weapon/NailWeapon.h"
#include "Weapon/ShotgunWeapon.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "GameFramework/Pawn.h"
#include "Components/ChildActorComponent.h"
#include "GameFramework/PlayerController.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay(); //부모가 해둔 초기화가 있을 수 있으니 관례적 호출

	APawn* OwnerPawn = Cast<APawn>(GetOwner()); // 이 컴포넌트가 붙어있는 캐릭터를 가져옴 입력 컴포넌트에 접근하기 위해
	if (!IsValid(OwnerPawn))
	{
		return;
	}

	// 팔 없이 무기만 쓰기 위한 임시 코드
	//=========================================================================================

	TArray<UChildActorComponent*> ChildActorComponents;
	OwnerPawn->GetComponents<UChildActorComponent>(ChildActorComponents);

	for (UChildActorComponent* ChildActorComp : ChildActorComponents)
	{
		if (!IsValid(ChildActorComp))
		{
			continue;
		}

		AActor* ChildActor = ChildActorComp->GetChildActor();

		if (ANailWeapon* Nail = Cast<ANailWeapon>(ChildActor))
		{
			NailWeapon = Nail;
		}
		else if (AShotgunWeapon* Shotgun = Cast<AShotgunWeapon>(ChildActor))
		{
			ShotgunWeapon = Shotgun;
		}
	}

	//=========================================================================================

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()); // Pawn 조종하는 컨트롤러 가져오기
	if (!IsValid(PC))
	{
		return;
	}

	// IMC 관리하는 시스템 가져오기
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		if (IsValid(CombatMappingContext)) // IMC 지정되어 있는지.
		{
			Subsystem->AddMappingContext(CombatMappingContext, 0); // IMC 등록
		}
	}

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(OwnerPawn->InputComponent)) // 입력을 Enhanced Input 타입으로 캐스팅
	{
		if (IsValid(NailAttackAction))// IA 지정되어 있는지.
		{
			EIC->BindAction(NailAttackAction, ETriggerEvent::Started, this, &UCombatComponent::OnNailAttack); // 좌클릭시 대못공격
		}
		if (IsValid(ShotgunAttackAction))
		{
			EIC->BindAction(ShotgunAttackAction, ETriggerEvent::Started, this, &UCombatComponent::OnShotgunAttack); // 우클릭시 샷건공격
		}
		if (IsValid(ReloadAction))
		{
			EIC->BindAction(ReloadAction, ETriggerEvent::Started, this, &UCombatComponent::OnReload); // R 누르면 재장전
		}
	}
}


// 공격하는 함수랑 연결
void UCombatComponent::OnNailAttack()
{
	if (!CanAttack())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("OnNailAttack called")); // IMC ,IA 바인딩 확인 
	if (IsValid(NailWeapon))
	{
		NailWeapon->TryAttack();
	}
}

void UCombatComponent::OnShotgunAttack()
{
	if (!CanAttack())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("OnShotgunAttack called"));
	if (IsValid(ShotgunWeapon))
	{
		ShotgunWeapon->TryAttack();
	}
}

void UCombatComponent::OnReload()
{
	if (IsValid(ShotgunWeapon))
	{
		ShotgunWeapon->Reload();
	}
}

bool UCombatComponent::CanAttack() const // 장전중 공격 방지
{
	if (IsValid(ShotgunWeapon) && ShotgunWeapon->IsReloading())
	{
		return false;
	}

	return true;
}