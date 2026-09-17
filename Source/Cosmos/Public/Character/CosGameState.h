#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CosGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStateChanged);
//이 신호를 받으면 GetkillCount처럼 함수를 직접 불러 최신값 읽어가는 방식

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnStageTimeChanged,
	float,
	RemainingSeconds
);

UCLASS()
class COSMOS_API ACosGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	int32 GetKillCount() const { return KillCount; }
	int32 GetWaveIndex() const { return WaveIndex; }

	//킬수/ 웨이브/ 점수 바꾸는 함수
	void AddKill();
	void SetWaveIndex(int32 NewWaveIndex);
	void AddScore(int32 Amount);

	float GetElapsedWaveTime() const;//추가 : 웨이브 경과시간

	UPROPERTY(BlueprintAssignable)
	FOnStateChanged OnStateChanged;

	UFUNCTION(BlueprintPure)
	int32 GetScore() const { return Score; }

	UFUNCTION(BlueprintPure, Category = "Stage")// 현재 스테이지의 남은 시간을 가져옴. 컨트롤러쪽에서 이 함수를 통해 시간 값을 가져갈 수 있음
	float GetStageRemainingTime() const
	{
		return StageRemainingTime;
	}

	void SetStageRemainingTime(float InRemainingTime);

	UPROPERTY(BlueprintAssignable, Category = "Stage")
	FOnStageTimeChanged OnStageTimeChanged;


protected:
	int32 KillCount = 0;
	int32 WaveIndex = 0;
	int32 Score = 0;
	float WaveStartTime = 0.f;

private:
	UPROPERTY(VisibleAnywhere, Category = "Stage")
	float StageRemainingTime = 300.0f;

};
