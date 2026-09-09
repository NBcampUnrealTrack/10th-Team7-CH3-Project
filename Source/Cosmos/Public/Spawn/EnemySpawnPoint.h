#pragma once
// EnemySpawnPoint는 "여기서 적이 나온다"는 위치를 레벨에 꽂아두는 용도의 좌표만 들고 있는 액터입니다.
// 스폰위치를 FVector로 하드코딩하는 것이 아니라 에디터 상에 쉽게 액터로 배치하기 위해 쓰는 것입니다.
// AWaveSpawner가 GetAllActorsOfClass로 이 액터를 전부 찾아서, 해당 좌표에 적을 만듭니다.
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawnPoint.generated.h"

UCLASS()
class COSMOS_API AEnemySpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	AEnemySpawnPoint();

	//Details에 표시할 GroupIndex 변수를 선언합니다. 페이즈 n은 GroupIndex가 n인 지점에서만 소환 같은 필터를 쓸 수 있습니다.
	UPROPERTY(EditAnywhere, Category = "Spawn")
	int32 GroupIndex = 0;

	//BillBoardComponent는 에디터 뷰포트에 이 액터가 아이콘으로 표시되어 눈으로 보이게 하는 컴포넌트입니다. 
	// 게임 실행 중에는 안보이고요. 뷰포트 배치용도로 쓰입니다.
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UBillboardComponent> Billboard;

};
