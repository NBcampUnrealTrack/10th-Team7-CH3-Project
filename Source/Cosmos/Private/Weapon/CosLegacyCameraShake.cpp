#include "Weapon/CosLegacyCameraShake.h"


UCosLegacyCameraShake::UCosLegacyCameraShake()
{
	OscillationDuration = 0.2f;              // 흔들리는 시간
	OscillationBlendInTime = 0.05f;          // 시작할 때 부드럽게
	OscillationBlendOutTime = 0.1f;          // 끝날 때 부드럽게

	RotOscillation.Pitch.Amplitude = 1.5f;   // 위아래 흔들림 세기
	RotOscillation.Pitch.Frequency = 15.f;   // 위아래 흔들림 빠르기
	RotOscillation.Pitch.InitialOffset = EOO_OffsetRandom; // 매번 다른 지점에서 시작

	RotOscillation.Yaw.Amplitude = 1.5f;     // 좌우 흔들림 세기
	RotOscillation.Yaw.Frequency = 15.f;
	RotOscillation.Yaw.InitialOffset = EOO_OffsetRandom;
}