#include "Character/CosGameMode.h"
#include "Character/CosCharacter.h"
#include "Character/CosGameState.h"//CosGameState 연결
#include "Spawn/WaveSpawner.h"
#include "Kismet/GameplayStatics.h"
#include "Character/HealthComponent.h"

ACosGameMode::ACosGameMode()
{
	DefaultPawnClass = ACosCharacter::StaticClass();
	GameStateClass = ACosGameState::StaticClass();
}

void ACosGameMode::BeginPlay()
{
	Super::BeginPlay();

	WaveSpawner = Cast<AWaveSpawner>(//현재 월드에 AWaveSpawner인 액터 찾아서 WaveSpawner에 저장
		UGameplayStatics::GetActorOfClass(//나중에 WaveSpawner->StartWave(1); 이렇게 호출
			GetWorld(),
			AWaveSpawner::StaticClass()
		)
	);

	//웨이브 스포너에서 웨이브 클리어 신호 받도록 연결
	//웨이브 끝나면 HandleWaveCleared가 호출
	if (WaveSpawner)
	{
		WaveSpawner->OnWaveCleared.AddUObject(
			this,
			&ACosGameMode::HandleWaveCleared
		);
	}

	BindPlayerDeath();
}

void ACosGameMode::StartNextWave()
{//CurrentWaveIndex+1-> 게임 CosGameState 에 현재 웨이브 전달 ->WaveSpawner가  웨이브 시작함
	++CurrentWaveIndex;

	if (WaveSpawner)//웨이브 스포너에 현재 웨이브 번호 전달, 적 스폰 시작함
	{
		WaveSpawner->StartWave(CurrentWaveIndex);
	}

	//게임스테이트에서도 웨이브 번호 전달
	if (ACosGameState* GS = GetGameState<ACosGameState>())
	{
		GS->SetWaveIndex(CurrentWaveIndex);
	}
}

void ACosGameMode::StartGame()//타이들에서 게임 시작 누르면 전투(임시) 맵으로 이동하도록함
{
	UGameplayStatics::OpenLevel(this, FName("L_AlphaTestMap"));
}

void ACosGameMode::HandleWaveCleared(int32 WaveIndex)
{
	if (WaveIndex == 26)//보스 나오게하기
	{
		return;
	}

	if (WaveIndex % 5 == 0)//대장간 5,10,15,20,25 클리어 시 대장간 
	{
		return;
	}

	StartNextWave();// 그 외는 다음 웨이브로 넘어가게
}

void ACosGameMode::HandlePlayerDeath()//현재 플레이어 컨트롤러 가져오게
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	if (!PlayerController)
	{
		return;
	}
	//잠시 후 플레이어 리스폰
	GetWorldTimerManager().SetTimer(
		RespawnTimer,
		this,
		&ACosGameMode::ResPawnPlayer,
		2.0f,
		false
	);
}

void ACosGameMode::ResPawnPlayer()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	if (!PlayerController)
	{
		return;
	}
	
	//기존 캐릭터 없애고 새로운 캐릭터 생성하고 빙의(웨이브 초기화인지, 싹 초기화인지 모르겠어서 임시로 둠)
	RestartPlayer(PlayerController);
}

void ACosGameMode::BindPlayerDeath()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);

	if (!PlayerPawn)
	{
		return;
	}

	UHealthComponent* HealthComp =
		PlayerPawn->FindComponentByClass<UHealthComponent>();

	if (!HealthComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] 플레이어 헬스 컴포넌트를 찾을 수 없음"));
		return;
	}

	HealthComp->OnDeath.RemoveDynamic(//중복 방지
		this,
		&ACosGameMode::HandlePlayerDeath
	);

	HealthComp->OnDeath.AddDynamic(
		this,
		&ACOsGameMode::HandlePlayerDeath
	);

	UE_LOG(LogTemp, Log, TEXT("[GameMode] 플레이어 사망 이벤트 바인딩"))
}
