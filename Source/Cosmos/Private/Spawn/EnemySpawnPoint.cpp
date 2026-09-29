#include "Spawn/EnemySpawnPoint.h"
#include "Components/BillboardComponent.h"

AEnemySpawnPoint::AEnemySpawnPoint()
{
	// 안써도 지우지 않고 tick을 안쓰는 클래스라는걸 표시하기 위해 false로 돌려놓기만 하는게 관례라고 합니다.
	PrimaryActorTick.bCanEverTick = false;
	
	// 해당 포인터 변수에 컴포넌트를 만들어 넣어줍니다.
	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	SetRootComponent(Billboard);
}

