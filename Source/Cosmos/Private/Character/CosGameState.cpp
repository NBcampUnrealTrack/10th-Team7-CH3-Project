#include "Character/CosGameState.h"

void ACosGameState::AddKill()
{
	++KillCount;
	OnStateChanged.Broadcast();
}

void ACosGameState::SetWaveIndex(int32 NewWaveIndex)
{
	WaveIndex = NewWaveIndex;
	OnStateChanged.Broadcast();
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