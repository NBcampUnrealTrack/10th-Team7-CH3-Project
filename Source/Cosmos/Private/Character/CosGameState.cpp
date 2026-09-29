#include "Character/CosGameState.h"

void ACosGameState::AddKill()//OnEnemyKilled 구독해야함
{
	++KillCount;
	OnStateChanged.Broadcast();
}

void ACosGameState::SetWaveIndex(int32 NewWaveIndex)
{
	WaveIndex = NewWaveIndex;
	WaveStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f; // 새 웨이브 시작 시각 기록
	OnStateChanged.Broadcast();
}

float ACosGameState::GetElapsedWaveTime() const
{
	if (GetWorld())
	{
		return GetWorld()->GetTimeSeconds() - WaveStartTime;
	}
	return 0.f;
}

void ACosGameState::AddScore(int32 Amount)
{
	if (Amount <= 0)//점수 감소를 방지
	{
		return;
	}
	Score += Amount;
	OnStateChanged.Broadcast();
}

void ACosGameState::SetStageRemainingTime(float InRemainingTime)
{
	StageRemainingTime = FMath::Max(0.0f, InRemainingTime);

	OnStageTimeChanged.Broadcast(StageRemainingTime);
}