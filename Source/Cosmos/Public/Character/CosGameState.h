//#include "CoreMinimal.h"
//#include "GameFramework/GameStateBase.h"
//#include "CosGameState.generated.h"
//
//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStateChanged);
////이 신호를 받으면 GetkillCount처럼 함수를 직접 불러 최신값 읽어가는 방식
//
//UCLASS()
//class COSMOS_API ACosGameState : public AGameStateBase
//{
//	GENERATED_BODY()
//	
//public:
//	int32 GetKillCount() const { return KillCount; }
//	int32 GetWaveIndex() const { return WaveIndex; }
//	
//	//킬수/ 웨이브/ 점수 바꾸는 함수
//	void AddKill();
//	void SetWaveIndex(int32 NewWaveIndex);
//	void AddScore(int32 Amount);
//
//	UPROPERTY(BlueprintAssignable)
//	FOnStateChanged OnStateChanged;
//
//protected:
//	int32 KillCount = 0;
//	int32 WaveIndex = 0;
//	int32 Score = 0;
//};
