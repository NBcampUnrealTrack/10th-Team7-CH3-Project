#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaveSpawner.generated.h"


class AEnemySpawnPoint; // 어디에 적이 생성될지 에디터 상에 찍어주는 EnemySpawnPoint를 전방선언합니다.
class UDataTable; // 웨이브 데이터 테이블 전방선언


// 델리게이트 선언. 스폰 완료 + 생존 적 0 -> 방송됨. 게임 모드가 구독합니다.
DECLARE_MULTICAST_DELEGATE_OneParam(FOnWaveCleared, int32 /*WaveIndex*/); // OneParam은 인자 1개를 넘긴다, TwoParam은 인자 2개를 넘긴다. 없으면 안넘김.

UCLASS()
class COSMOS_API AWaveSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AWaveSpawner();
	
	// 페이즈 하나를 시작합니다. 우리가 함수 시그니처를 StartWave로 통일했기 때문에 일단 StartWave를 사용합니다. 게임모드가 부르는 유일한 진입점.
	UFUNCTION(BlueprintCallable, Category = "Wave") // BlueprintCallable은 실행핀을 뽑을 수 있는 뭔가를 바꿀 수 있는 함수입니다.
	void StartWave(int32 WaveIndex); // WaveIndex를 매개변수로 넣습니다.

	// 진행 중인 스폰을 중단합니다. 이미 스폰된 적을 지우지는 않습니다. 스폰만 중단합니다.
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StopWave();

	// 살아있는 적을 리턴해줍니다. AliveEnemies 라는 살아있는 적 목록을 담는 배열 안에 .Num 지금 몇 마리 들어있는지 알려줍니다. 3마리 살아있으면 3을 돌려줌.
	UFUNCTION(BlueprintPure, Category = "Wave") // BlueprintPure은 실행핀 없고 값만 돌려줍니다.
	int32 GetAliveCount() const // .cpp로 따로 안빼고 인라인 정의함. 
	{
		return AliveEnemies.Num(); 
	}

	FOnWaveCleared OnWaveCleared; // FOnWaveCleared 델리게이트 타입의 OnWaveCleared 변수를 선언.

protected:
	virtual void BeginPlay() override;

	
private: // 내부함수들이기 때문에 private임. 바깥에 쓰는 것만 public
	// SpawnOne은 적하나를 실제로 만드는 함수입니다. StartWave -> 타이머 시작 -> SpawnOne, SpawnOne, ... -> 5마리 되면 타이머 정지
	void SpawnOne();

	UFUNCTION() // 적이 파괴되었을 때 언리얼이 자동 호출합니다.
				// cpp에서 Enemy->OnDestroyed.AddDynamic(this, ...)으로 등록해두기 때문입니다.
				// AddDynamic은 UFUNCTION() 매크로가 붙은 함수만 받습니다.
	void HandleEnemyDestroyed(AActor* DestroyedActor);


	// 레벨의 AEnemySpawnPoint를 전부 찾아 SpawnPoints에 담아줍니다. 패키징할 때 문제가 있어서 넣어봤습니다.
	// BeginPlay와 StartWave 양쪽에서 부르므로 함수로 일단 분리했습니다만, 9월15일 알파패키징 이후로는 쓸모가 없을 것 같다고 생각중입니다.
	// PIE 할 때랑 패키징 후랑 액터의 순서차이가 날 수도 있나본데요. 아무튼 그렇습니다.
	void CollectSpawnPoints();

	// [DT] 게임모드가 넘긴 번호(1~26)를 DT의 Row Name(Stage1_Wave1의 형식)으로 바꿔줍니다.
	FName MakeRowName(int32 WaveIndex) const;

	// -- 에디터 설정값 
	// 웨이브별 스폰 수가 담긴 데이터 테이블. 에디터 Details에서 DT 에셋을 꽂습니다.

	UPROPERTY(EditAnywhere, Category = "Wave")
	TObjectPtr<UDataTable> WaveDataTable;

	UPROPERTY(EditAnywhere, Category = "Wave|Enemy")
	TSubclassOf<AActor> GhoulClass;

	UPROPERTY(EditAnywhere, Category = "Wave|Enemy")
	TSubclassOf<AActor> EnhancedGhoulClass;

	UPROPERTY(EditAnywhere, Category = "Wave|Enemy")
	TSubclassOf<AActor> GargoyleClass;

	UPROPERTY(EditAnywhere, Category = "Wave|Enemy")
	TSubclassOf<AActor> CrowClass;

	UPROPERTY(EditAnywhere, Category = "Wave|Enemy")
	TSubclassOf<AActor> BossClass;

	UPROPERTY(EditAnywhere, Category = "Wave", meta = (ClampMin = "0.1"))  // 0을 넣으면 무한생성됩니다. 0.1로 막아놓은 것.
	float SpawnInterval = 1.0f; // 스폰하는 인터벌

	// ---런타임 상태--- 아래는 에디터에서 설정하는 값이 아니라 게임이 돌면서 변하는 값들입니다.
	UPROPERTY()
	TArray<TObjectPtr<AEnemySpawnPoint>> SpawnPoints;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> AliveEnemies;
	UPROPERTY()
	TArray<TSubclassOf<AActor>> SpawnQueue;

	FTimerHandle SpawnTimer; // 타이머의 이름표입니다. 타이머 그자체는 아니고 내가 걸어놓은 타이머를 찾기 위한 식별자입니다.
	int32 CurrentWave = 0; // 지금이 몇 번째 웨이브인지 클리어 방송할 때 이 번호를 같이 넘깁니다.
	int32 SpawnedCount = 0; // 이 웨이브에서 지금까지 몇 마리 만들었는지.
	int32 TargetSpawnCount = 0; // 이번 웨이브에 만들 총 마리 수 . 에디터가 아니라 DT에서 채워서 작동함.
	bool bIsSpawning = false; // 아직 스폰중인지?
};
