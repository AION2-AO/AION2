#include "Game/AODungeonGameMode.h"

#include "Character/AOCharacter.h"
#include "Character/Daeva/Daeva.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/Controller.h"

#include "Character/Monster/AOMonsterBase.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AAODungeonGameMode::AAODungeonGameMode()
{
	bUseSeamlessTravel = true;
	PrimaryActorTick.bCanEverTick = true;
}

void AAODungeonGameMode::BeginPlay()
{
	Super::BeginPlay();

	FindPlacedBosses();
	InitializePlacedBosses();

	StartDungeon();
}

void AAODungeonGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAODungeonGameMode::FindPlacedBosses()
{
	TArray<AActor*> FoundActors;

	UGameplayStatics::GetAllActorsOfClass(this,AAOMonsterBase::StaticClass(),FoundActors);

	for (AActor* Actor : FoundActors)
	{
		AAOMonsterBase* Boss = Cast<AAOMonsterBase>(Actor);

		if (!Boss)
		{
			continue;
		}

		const int32 BossIndex = Boss->DungeonBossIndex;

		// 0은 일반 몬스터, 1~3만 던전 보스
		if (BossIndex < 1 || BossIndex > 3)
		{
			continue;
		}

		if (PlacedBosses.Contains(BossIndex))
		{
			UE_LOG(LogTemp,Error,TEXT("[Dungeon] Boss %d is duplicated: %s"),BossIndex,*Boss->GetName());
			continue;
		}

		PlacedBosses.Add(BossIndex, Boss);

		UE_LOG(LogTemp,Warning,	TEXT("[Dungeon] Found Boss %d: %s"),BossIndex,*Boss->GetName());
	}

	for (int32 BossIndex = 1; BossIndex <= 3; ++BossIndex)
	{
		if (!PlacedBosses.Contains(BossIndex))
		{
			UE_LOG(	LogTemp,Error,TEXT("[Dungeon] Boss %d is not placed in this map."),	BossIndex);
		}
	}
}

void AAODungeonGameMode::InitializePlacedBosses()
{
	for (const TPair<int32, TObjectPtr<AAOMonsterBase>>& Pair : PlacedBosses)
	{
		AAOMonsterBase* Boss = Pair.Value;

		if (!Boss)
		{
			continue;
		}

		// Boss1도 StartDungeon에서 켜기 때문에 시작 시 전부 꺼 둔다.
		Boss->SetDungeonBossActive(false);
	}
}

void AAODungeonGameMode::StartDungeon()
{
	if (CurrentPhase != EDungeonPhase::Ready)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Dungeon] Start Dungeon"));

	StartBossPhase(3);
}

void AAODungeonGameMode::StartBossPhase(int32 BossNumber)
{
	if (BossNumber < 1 || BossNumber > 3)
	{
		UE_LOG(LogTemp,	Error,TEXT("[Dungeon] Invalid BossNumber: %d"),BossNumber);
		return;
	}

	CurrentBossNumber = BossNumber;

	SetCombatPhase(BossNumber);

	ActivateBoss(BossNumber);

	UE_LOG(LogTemp,Warning,TEXT("[Dungeon] Boss %d Combat Start"),BossNumber);
}

void AAODungeonGameMode::ActivateBoss(int32 BossNumber)
{
	AAOMonsterBase* Boss = PlacedBosses.FindRef(BossNumber);

	if (!Boss)
	{
		UE_LOG(	LogTemp,Error,TEXT("[Dungeon] Boss %d not found."),	BossNumber);
		return;
	}

	CurrentBoss = Boss;

	Boss->SetDungeonBossActive(true);

	UE_LOG(LogTemp,Warning,TEXT("[Dungeon] Boss %d Activated: %s"),BossNumber,*Boss->GetName());
}

void AAODungeonGameMode::NotifyBossDefeated(AAOMonsterBase* DefeatedBoss)
{
	if (!DefeatedBoss)
	{
		return;
	}

	if (CurrentPhase == EDungeonPhase::Cleared || CurrentPhase == EDungeonPhase::Failed)
	{
		return;
	}

	if (DefeatedBoss != CurrentBoss)
	{
		UE_LOG(LogTemp,Warning,	TEXT("[Dungeon] Defeated boss is not current boss: %s"),*DefeatedBoss->GetName());
		return;
	}

	UE_LOG(	LogTemp,Warning,TEXT("[Dungeon] Boss %d Defeated"),	CurrentBossNumber);

	SetDefeatedPhase(CurrentBossNumber);

	// 사망 애니메이션을 보여줄 거면 여기서 바로 끄지 말고,
	// 몽타주 종료 시점에 SetDungeonBossActive(false)를 호출하면 된다.
	DefeatedBoss->SetDungeonBossActive(false);

	CurrentBoss = nullptr;

	if (CurrentBossNumber < 3)
	{
		OpenGateForNextBoss(CurrentBossNumber);

		GetWorldTimerManager().SetTimer(NextBossTimerHandle,this,
			&AAODungeonGameMode::StartNextBoss,
			3.0f,
			false
		);

		return;
	}

	ClearDungeon();
}

void AAODungeonGameMode::StartNextBoss()
{
	StartBossPhase(CurrentBossNumber + 1);
}

void AAODungeonGameMode::ClearDungeon()
{
	CurrentPhase = EDungeonPhase::Cleared;

	ClearAllRespawnTimers();

	UE_LOG(LogTemp, Warning, TEXT("[Dungeon] Dungeon Clear"));

	GiveDungeonReward();

	//RequestReturnToVillage();
}

void AAODungeonGameMode::FailDungeon()
{
	if (CurrentPhase == EDungeonPhase::Failed)
	{
		return;
	}

	CurrentPhase = EDungeonPhase::Failed;
	ClearAllRespawnTimers();

	UE_LOG(LogTemp, Warning, TEXT("Dungeon Failed"));
}

void AAODungeonGameMode::ReturnToVillage()
{
	if (VillageMapPath.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[Dungeon] VillageMapPath is Empty"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Dungeon] Return To Village"));

	GetWorld()->ServerTravel(VillageMapPath);
}

void AAODungeonGameMode::NotifyPlayerDied(APlayerController* DeadPlayerController)
{
	if (!DeadPlayerController)
	{
		return;
	}

	if (CurrentPhase == EDungeonPhase::Cleared || CurrentPhase == EDungeonPhase::Failed)
	{
		return;
	}

	if (DeadPlayerControllers.Contains(DeadPlayerController))
	{
		return;
	}

	APawn* DeadPawn = DeadPlayerController->GetPawn();

	if (!DeadPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Dead player has no Pawn"));
		return;
	}

	const FTransform DeathTransform = DeadPawn->GetActorTransform();

	DeadPlayerControllers.Add(DeadPlayerController);

	const int32 ActivePlayerCount = GetActiveDungeonPlayerCount();
	const int32 AlivePlayerCount = GetAliveDungeonPlayerCount();

	UE_LOG(LogTemp,	Warning, TEXT("[Dungeon] Player Dead. Alive: %d / %d"), AlivePlayerCount, ActivePlayerCount);

	if (AlivePlayerCount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("All Palyers Dead. Dungeon Failed"));

		StartWipeRespawn();
		return;
	}

	StartPlayerRespawnTimer(DeadPlayerController, DeathTransform);
}

void AAODungeonGameMode::SetCombatPhase(int32 BossNumber)
{
	switch (BossNumber)
	{
	case 1:
		CurrentPhase = EDungeonPhase::Boss1Combat;
		break;

	case 2:
		CurrentPhase = EDungeonPhase::Boss2Combat;
		break;

	case 3:
		CurrentPhase = EDungeonPhase::Boss3Combat;
		break;

	default:
		break;
	}
}

void AAODungeonGameMode::SetDefeatedPhase(int32 BossNumber)
{
	switch (BossNumber)
	{
	case 1:
		CurrentPhase = EDungeonPhase::Boss1Defeated;
		break;

	case 2:
		CurrentPhase = EDungeonPhase::Boss2Defeated;
		break;

	case 3:
		CurrentPhase = EDungeonPhase::Boss3Defeated;
		break;

	default:
		break;
	}
}

void AAODungeonGameMode::StartPlayerRespawnTimer(APlayerController* DeadPlayerController, const FTransform& RespawnTransform)
{
	if (!DeadPlayerController)
	{
		return;
	}

	if (RespawnTimerHandles.Contains(DeadPlayerController))
	{
		return;
	}

	PendingRespawnTransforms.Add(DeadPlayerController, RespawnTransform);

	FTimerHandle NewRespawnTimerHandle;

	TWeakObjectPtr<APlayerController> WeakPlayerController = DeadPlayerController;

	FTimerDelegate RespawnDelegate;
	RespawnDelegate.BindLambda([this, WeakPlayerController]()
		{
			if (!WeakPlayerController.IsValid())
			{
				return;
			}

			RespawnPlayer(WeakPlayerController.Get());
		});

	GetWorldTimerManager().SetTimer(NewRespawnTimerHandle, RespawnDelegate, RespawnDelay, false);

	RespawnTimerHandles.Add(DeadPlayerController, NewRespawnTimerHandle);

	UE_LOG(LogTemp,Warning, TEXT("Respawn scheduled: %s / %.1f sec"),*DeadPlayerController->GetName(),RespawnDelay);
}

void AAODungeonGameMode::RespawnPlayer(APlayerController* PlayerController)
{
	UE_LOG(LogTemp, Warning, TEXT("[Dungeon] RespawnPlayer Called"));

	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("[Dungeon] PlayerController is null"));
		return;
	}

	if (CurrentPhase == EDungeonPhase::Failed || CurrentPhase == EDungeonPhase::Cleared)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dungeon] Cannot Respawn. Dungeon Ended."));
		return;
	}

	RespawnTimerHandles.Remove(PlayerController);

	if (!DeadPlayerControllers.Contains(PlayerController))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dungeon] Player is not in DeadPlayerControllers"));
		return;
	}

	const FTransform* RespawnTransform = PendingRespawnTransforms.Find(PlayerController);

	if (!RespawnTransform)
	{
		UE_LOG(LogTemp, Error, TEXT("[Dungeon] Respawn Transform Not Found: %s"), *PlayerController->GetName());
		return;
	}

	APawn* OldPawn = PlayerController->GetPawn();

	if (OldPawn)
	{
		PlayerController->UnPossess();
		OldPawn->Destroy();
	}

	RestartPlayerAtTransform(PlayerController, *RespawnTransform);

	if (ADaeva* RespawnedPlayer = Cast<ADaeva>(PlayerController->GetPawn()))
	{
		RespawnedPlayer->ResetForDungeonRespawn();
	}

	DeadPlayerControllers.Remove(PlayerController);
	PendingRespawnTransforms.Remove(PlayerController);

	UE_LOG(	LogTemp, Warning, TEXT("Player Respawned: %s"),*PlayerController->GetName());
}

AActor* AAODungeonGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<AActor*> FoundPlayerStarts;

	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), FoundPlayerStarts);

	TArray<APlayerStart*> DungeonStarts;

	for (AActor* Actor : FoundPlayerStarts)
	{
		APlayerStart* PlayerStart = Cast<APlayerStart>(Actor);

		if (!PlayerStart)
		{
			continue;
		}

		if (PlayerStart->ActorHasTag(DungeonStartTag))
		{
			DungeonStarts.Add(PlayerStart);
		}
	}

	if (DungeonStarts.IsEmpty())
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	// 접속 순서대로 Start를 하나씩 배정
	// 배열 범위를 넘지 않도록 인덱스 반복.
	const int32 StartIndex = NextDungeonStartIndex % DungeonStarts.Num();
	APlayerStart* SelectedStart = DungeonStarts[StartIndex];

	++NextDungeonStartIndex;

	UE_LOG(LogTemp,Warning,TEXT("[Dungeon] Initial Spawn | Player: %s | Start: %s | Index: %d"),Player ? *Player->GetName() : TEXT("Unknown"),*SelectedStart->GetName(),StartIndex);

	return SelectedStart;
}

int32 AAODungeonGameMode::GetActiveDungeonPlayerCount() const
{
	int32 PlayerCount = 0;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();

		if (!PlayerController)
		{
			continue;
		}

		if (!PlayerController->PlayerState)
		{
			continue;
		}

		++PlayerCount;
	}

	return PlayerCount;
}

int32 AAODungeonGameMode::GetAliveDungeonPlayerCount() const
{
	int32 AlivePlayerCount = 0;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();

		if (!PlayerController)
		{
			continue;
		}

		if (!PlayerController->PlayerState)
		{
			continue;
		}

		if (!DeadPlayerControllers.Contains(PlayerController))
		{
			++AlivePlayerCount;
		}
	}

	return AlivePlayerCount;
}

APlayerStart* AAODungeonGameMode::FindDungeonRespawnPoint() const
{
	TArray<AActor*> FoundPlayerStarts;

	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), FoundPlayerStarts);

	TArray<APlayerStart*> RespawnPoints;

	for (AActor* Actor : FoundPlayerStarts)
	{
		APlayerStart* PlayerStart = Cast<APlayerStart>(Actor);

		if (!PlayerStart)
		{
			continue;
		}

		if (PlayerStart->ActorHasTag(RespawnPlayerStartTag))
		{
			RespawnPoints.Add(PlayerStart);
		}
	}

	if (RespawnPoints.IsEmpty())
	{
		return nullptr;
	}

	const int32 RandomIndex = FMath::RandRange(0, RespawnPoints.Num() - 1);

	return RespawnPoints[RandomIndex];
}

void AAODungeonGameMode::ClearAllRespawnTimers()
{
	for (auto& Pair : RespawnTimerHandles)
	{
		GetWorldTimerManager().ClearTimer(Pair.Value);
	}

	RespawnTimerHandles.Empty();
	PendingRespawnTransforms.Empty();

}

void AAODungeonGameMode::StartWipeRespawn()
{
	ClearAllRespawnTimers();

	GetWorldTimerManager().ClearTimer(WipeRespawnTimerHandle);

	GetWorldTimerManager().SetTimer(WipeRespawnTimerHandle,	this,&AAODungeonGameMode::RespawnAllDeadPlayersAtBossCheckpoint,RespawnDelay,false);

	UE_LOG(	LogTemp,Warning,TEXT("Wipe Respawn Scheduled / Boss %d / %.1f sec"),CurrentBossNumber,RespawnDelay);
}

void AAODungeonGameMode::RespawnAllDeadPlayersAtBossCheckpoint()
{
	APlayerStart* Checkpoint = FindBossRespawnPoint(CurrentBossNumber);

	if (!Checkpoint)
	{
		UE_LOG(LogTemp,	Error,TEXT("Boss %d Respawn Point Not Found."),CurrentBossNumber);

		return;
	}

	TArray<TObjectPtr<APlayerController>> PlayersToRespawn;

	for (APlayerController* PlayerController : DeadPlayerControllers)
	{
		if (PlayerController)
		{
			PlayersToRespawn.Add(PlayerController);
		}
	}

	for (APlayerController* PlayerController : PlayersToRespawn)
	{
		APawn* OldPawn = PlayerController->GetPawn();

		if (OldPawn)
		{
			PlayerController->UnPossess();
			OldPawn->Destroy();
		}

		RestartPlayerAtPlayerStart(PlayerController, Checkpoint);

		if (ADaeva* RespawnedPlayer = Cast<ADaeva>(PlayerController->GetPawn()))
		{
			RespawnedPlayer->ResetForDungeonRespawn();
		}

		UE_LOG(LogTemp,Warning,TEXT("Wipe Respawned: %s / Boss %d"),*PlayerController->GetName(),CurrentBossNumber);
	}

	DeadPlayerControllers.Empty();
	PendingRespawnTransforms.Empty();
	RespawnTimerHandles.Empty();
}

APlayerStart* AAODungeonGameMode::FindBossRespawnPoint(int32 CurrentBossNumber) const
{
	FName TargetTag;

	switch (CurrentBossNumber)
	{
	case 1:
		TargetTag = Boss1RespawnTag;
		break;

	case 2:
		TargetTag = Boss2RespawnTag;
		break;

	case 3:
		TargetTag = Boss3RespawnTag;
		break;

	default:
		return nullptr;
	}

	TArray<AActor*> FoundPlayerStarts;

	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), FoundPlayerStarts);

	for (AActor* Actor : FoundPlayerStarts)
	{
		APlayerStart* PlayerStart = Cast<APlayerStart>(Actor);

		if (PlayerStart && PlayerStart->ActorHasTag(TargetTag))
		{
			return PlayerStart;
		}
	}

	return nullptr;
}

void AAODungeonGameMode::RequestReturnToVillage()
{
	if (CurrentPhase != EDungeonPhase::Cleared)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dungeon] Cannot return to Village before dungeon clear."));
		return;
	}

	ReturnToVillage();
}
