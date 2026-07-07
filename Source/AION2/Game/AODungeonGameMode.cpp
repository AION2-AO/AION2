#include "Game/AODungeonGameMode.h"

#include "Character/Daeva/Daeva.h"
#include "Player/AOPlayerState.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/Controller.h"

#include "Character/Monster/AOMonsterBase.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#include "Network/PacketHeader.h"
#include "AION2.h"

AAODungeonGameMode::AAODungeonGameMode()
{
	bUseSeamlessTravel = true;
	PrimaryActorTick.bCanEverTick = true;
	//DefaultPawnClass = APawn::StaticClass();
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

void AAODungeonGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	//Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	UE_LOG(LogTemp, Warning, TEXT("[Dungeon] PreLogin - Options: %s, Address: %s"), *Options, *Address);

	FString ClientToken = UGameplayStatics::ParseOption(Options, TEXT("Token"));
	UE_LOG(LogTemp, Warning, TEXT("[Dungeon] PreLogin - Extracted Token: '%s'"), *ClientToken);

	Protocol::DPlayerInfo* ClientInfo = ValidateToken(ClientToken);
	if (ClientInfo == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[Dungeon] PreLogin - Token validation FAILED for Token: '%s'"), *ClientToken);
		// disconnect
		return;
	}
	int32 Key = UniqueId->GetTypeHash();
	UE_LOG(LogTemp, Warning, TEXT("[Dungeon] PreLogin - Token validation SUCCESS. Adding Key to PendingPlayers: %d"), Key);
	PendingPlayers.Add(Key, *ClientInfo);

	PrePlayers.Remove(ClientToken);
}

void AAODungeonGameMode::PostLogin(APlayerController* NewPlayer)
{
	if (NewPlayer == nullptr || NewPlayer->PlayerState == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[Dungeon] PostLogin: PlayerController or PlayerState is null"));
		if (NewPlayer) NewPlayer->Destroy();
		return;
	}

	const FUniqueNetIdRepl& NetId = NewPlayer->PlayerState->GetUniqueId();

	if (!NetId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[Dungeon] PostLogin: Player has invalid UniqueNetId"));
		NewPlayer->Destroy();
		return;
	}
	int32 UniqueId = NetId->GetTypeHash();

	if (PendingPlayers.Contains(UniqueId) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dungeon] PostLogin: Player not found in PendingPlayers (Key: %d)"), UniqueId);
		NewPlayer->Destroy();
		return;
	}

	Protocol::DPlayerInfo PlayerData = PendingPlayers[UniqueId];
	AAOPlayerState* PlayerState = NewPlayer->GetPlayerState<AAOPlayerState>();
	if (PlayerState)
	{
		FString PlayerName = PlayerData.playername().c_str();
		PlayerState->SetPlayerInfo(PlayerData.playerid(), PlayerName, (uint8)PlayerData.playerclass());
		UE_LOG(LogTemp, Log, TEXT("[Dungeon] PostLogin: Success and SetPlayerInfo (Key: %d), PlayerId: %d"), UniqueId, PlayerData.playerid());
	}

	PendingPlayers.Remove(UniqueId);

	//Super::PostLogin(NewPlayer);
}

void AAODungeonGameMode::InitStartSpot_Implementation(AActor* StartSpot, AController* NewPlayer)
{
	//Super::InitStartSpot_Implementation(StartSpot, NewPlayer);
}

void AAODungeonGameMode::FindPlacedBosses()
{
	TArray<AActor*> FoundActors;

	UGameplayStatics::GetAllActorsOfClass(this, AAOMonsterBase::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		AAOMonsterBase* Boss = Cast<AAOMonsterBase>(Actor);

		if (!Boss)
		{
			continue;
		}

		const int32 BossIndex = Boss->DungeonBossIndex;

		if (BossIndex < 1 || BossIndex > 3)
		{
			continue;
		}

		if (PlacedBosses.Contains(BossIndex))
		{
			continue;
		}

		PlacedBosses.Add(BossIndex, Boss);
	}

	for (int32 BossIndex = 1; BossIndex <= 3; ++BossIndex)
	{
		if (!PlacedBosses.Contains(BossIndex))
		{
			UE_LOG(LogTemp, Error, TEXT("[Dungeon] Boss %d is not placed in this map."), BossIndex);
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

	// 현재 테스트 때문에 3번째 보스부터 시작!
	StartBossPhase(1);
}

void AAODungeonGameMode::StartBossPhase(int32 BossNumber)
{
	if (BossNumber < 1 || BossNumber > 3)
	{
		UE_LOG(LogTemp, Error, TEXT("[Dungeon] Invalid BossNumber: %d"), BossNumber);
		return;
	}

	CurrentBossNumber = BossNumber;

	SetCombatPhase(BossNumber);

	ActivateBoss(BossNumber);

	UE_LOG(LogTemp, Warning, TEXT("[Dungeon] Boss %d Combat Start"), BossNumber);
}

void AAODungeonGameMode::ActivateBoss(int32 BossNumber)
{
	AAOMonsterBase* Boss = PlacedBosses.FindRef(BossNumber);

	if (!Boss)
	{
		UE_LOG(LogTemp, Error, TEXT("[Dungeon] Boss %d not found."), BossNumber);
		return;
	}

	CurrentBoss = Boss;

	Boss->SetDungeonBossActive(true);

	UE_LOG(LogTemp, Warning, TEXT("[Dungeon] Boss %d Activated: %s"), BossNumber, *Boss->GetName());
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
		UE_LOG(LogTemp, Warning, TEXT("[Dungeon] Defeated boss is not current boss: %s"), *DefeatedBoss->GetName());
		return;
	}

	SetDefeatedPhase(CurrentBossNumber);

	// 사망 애니메이션을 보여줄 거면 여기서 바로 끄지 말고,
	// 몽타주 종료 시점에 SetDungeonBossActive(false)를 호출하면 된다.
	DefeatedBoss->SetDungeonBossActive(false);

	CurrentBoss = nullptr;

	if (CurrentBossNumber < 3)
	{
		//OpenGateForNextBoss(CurrentBossNumber);
		GetWorldTimerManager().SetTimer(NextBossTimerHandle,this,&AAODungeonGameMode::StartNextBoss,3.0f,false);

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

	SendDungeonComplete(true);
}

void AAODungeonGameMode::FailDungeon()
{
	if (CurrentPhase == EDungeonPhase::Failed)
	{
		return;
	}

	CurrentPhase = EDungeonPhase::Failed;
	ClearAllRespawnTimers();
}

void AAODungeonGameMode::ReturnToVillage()
{
	if (VillageMapPath.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[Dungeon] VillageMapPath is Empty"));
		return;
	}

	GetWorld()->ServerTravel(VillageMapPath);
}

void AAODungeonGameMode::NotifyPlayerDied(APlayerController* DeadPlayerController, bool bIsFallDeath)
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
		return;
	}

	DeadPlayerControllers.Add(DeadPlayerController);

	const int32 ActivePlayerCount = GetActiveDungeonPlayerCount();
	const int32 AlivePlayerCount = GetAliveDungeonPlayerCount();

	if (bIsFallDeath)
	{
		int32 RespawnBossIndex = CurrentBossNumber - 1;
		RespawnBossIndex = FMath::Clamp(RespawnBossIndex, 1, 3);

		PendingRespawnBossIndices.Add(DeadPlayerController,	RespawnBossIndex);
	}
	else
	{
		const FTransform DeathTransform = DeadPawn->GetActorTransform();

		PendingRespawnTransforms.Add(DeadPlayerController, DeathTransform);
	}

	if (AlivePlayerCount <= 0)
	{
		StartWipeRespawn();
		return;
	}

	StartPlayerRespawnTimer(DeadPlayerController, DeadPawn->GetActorTransform());
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

	// PendingRespawnTransforms.Add(DeadPlayerController, RespawnTransform);

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
}

void AAODungeonGameMode::RespawnPlayer(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	if (CurrentPhase == EDungeonPhase::Failed || CurrentPhase == EDungeonPhase::Cleared)
	{
		return;
	}

	RespawnTimerHandles.Remove(PlayerController);

	if (!DeadPlayerControllers.Contains(PlayerController))
	{
		return;
	}

	//const FTransform* RespawnTransform = PendingRespawnTransforms.Find(PlayerController);
	/*
	if (!RespawnTransform)
	{
		return;
	}*/

	APawn* OldPawn = PlayerController->GetPawn();

	if (OldPawn)
	{
		PlayerController->UnPossess();
		OldPawn->Destroy();
	}

	// 낙사
	if (const int32* RespawnBossIndex = PendingRespawnBossIndices.Find(PlayerController))
	{
		TArray<APlayerStart*> Checkpoints = FindBossRespawnPoint(*RespawnBossIndex);

		if (Checkpoints.IsEmpty())
		{
			return;
		}

		const int32 RespawnIndex = FMath::RandRange(0, Checkpoints.Num() - 1);

		RestartPlayerAtPlayerStart(PlayerController, Checkpoints[RespawnIndex]);
	}
	// 일반 사명
	else if (const FTransform* RespawnTransform = PendingRespawnTransforms.Find(PlayerController))
	{
		RestartPlayerAtTransform(PlayerController, *RespawnTransform);
	}
	else
	{
		return;
	}

	if (ADaeva* RespawnedPlayer = Cast<ADaeva>(PlayerController->GetPawn()))
	{

		RespawnedPlayer->RestorePlayerInfoFromPlayerState();
		RespawnedPlayer->ResetForDungeonRespawn();
	}


	DeadPlayerControllers.Remove(PlayerController);
	PendingRespawnTransforms.Remove(PlayerController);
	PendingRespawnBossIndices.Remove(PlayerController);
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

	const int32 StartIndex = NextDungeonStartIndex % DungeonStarts.Num();
	APlayerStart* SelectedStart = DungeonStarts[StartIndex];

	++NextDungeonStartIndex;

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
	GetWorldTimerManager().SetTimer(WipeRespawnTimerHandle, this, &AAODungeonGameMode::RespawnAllDeadPlayersAtBossCheckpoint, RespawnDelay, false);
}

void AAODungeonGameMode::RespawnAllDeadPlayersAtBossCheckpoint()
{
	TArray<APlayerStart*> Checkpoints = FindBossRespawnPoint(CurrentBossNumber);

	if (Checkpoints.IsEmpty())
	{
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

	for (int32 PlayerIndex = 0; PlayerIndex < PlayersToRespawn.Num(); ++PlayerIndex)
	{
		APlayerController* PlayerController = PlayersToRespawn[PlayerIndex];

		const int32 RespawnIndex = PlayerIndex % Checkpoints.Num();
		APlayerStart* RespawnPoint = Checkpoints[RespawnIndex];

		APawn* OldPawn = PlayerController->GetPawn();

		RestartPlayerAtPlayerStart(PlayerController, RespawnPoint);

		if (ADaeva* RespawnedPlayer = Cast<ADaeva>(PlayerController->GetPawn()))
		{
			RespawnedPlayer->RestorePlayerInfoFromPlayerState();
			RespawnedPlayer->ResetForDungeonRespawn();
		}
	}

	DeadPlayerControllers.Empty();
	PendingRespawnTransforms.Empty();
	RespawnTimerHandles.Empty();
}

TArray<APlayerStart*> AAODungeonGameMode::FindBossRespawnPoint(int32 CurrentBossNumber) const
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
		return {};
	}

	TArray<AActor*> FoundPlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), FoundPlayerStarts);

	TArray<APlayerStart*> RespawnPoints;

	for (AActor* Actor : FoundPlayerStarts)
	{
		APlayerStart* PlayerStart = Cast<APlayerStart>(Actor);

		if (PlayerStart && PlayerStart->ActorHasTag(TargetTag))
		{
			RespawnPoints.Add(PlayerStart);
		}
	}

	return RespawnPoints;
}

APawn* AAODungeonGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	if (!NewPlayer || !StartSpot)
	{
		return nullptr;
	}

	AAOPlayerState* PlayerState = NewPlayer->GetPlayerState<AAOPlayerState>();

	if (!PlayerState)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerState is null."));
		return Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
	}

	const EDaevaClassType ClassType = PlayerState->GetMyClass();

	UE_LOG(LogTemp, Warning, TEXT("[Spawn] %s | Authority: %d | ClassType: %d"), *NewPlayer->GetName(), HasAuthority(), static_cast<uint8>(ClassType));

	const TSubclassOf<APawn>* PawnClass = JobClassMap.Find(ClassType);

	if (!PawnClass || !(*PawnClass))
	{
		UE_LOG(LogTemp, Error, TEXT("Pawn Class Not Found. ClassType : %d"), static_cast<uint8>(PlayerState->GetMyClass()));
		return Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(*PawnClass, StartSpot->GetActorTransform(), SpawnParams);

	UE_LOG(LogTemp, Warning, TEXT("%s Spawned as %s"), *NewPlayer->GetName(), SpawnedPawn ? *SpawnedPawn->GetName() : TEXT("NULL"));
	SpawnedPlayers.Add(SpawnedPawn);
	return SpawnedPawn;
}

void AAODungeonGameMode::SetPrePlayerInfo(const Protocol::S_DungeonStartDediPacket& PlayerInfo)
{
	UE_LOG(LogTemp, Warning, TEXT("[Dungeon] SetPrePlayerInfo - Total players: %d"), PlayerInfo.preplayersinfos_size());
	for (int i = 0; i < PlayerInfo.preplayersinfos_size(); ++i)
	{
		Protocol::DediDungeonInfo DungeonInfo = PlayerInfo.preplayersinfos(i);
		FString Token = UTF8_TO_TCHAR(DungeonInfo.clienttoken().c_str());
		UE_LOG(LogTemp, Warning, TEXT("[Dungeon] SetPrePlayerInfo - Adding PrePlayer: Name: %s, Token: %s"), UTF8_TO_TCHAR(DungeonInfo.clientname().c_str()), *Token);
		Protocol::DPlayerInfo DPlayerInfo;
		DPlayerInfo.set_playerid(DungeonInfo.clientid());
		DPlayerInfo.set_playername(DungeonInfo.clientname());
		DPlayerInfo.set_playerclass(DungeonInfo.clientclass());
		PrePlayers.Add(Token, DPlayerInfo);
	}
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

//void AAODungeonGameMode::SendDungeonComplete()
//{
//	Protocol::C_DungeonMapLoadCompletePacket MapPkt;
//	MapPkt.set_dungeonid(MyDungeonId);
//	SEND_PACKET(MapPkt, PKT_C_DUNGEOMMAPCOMPLETE);
//}

//H.Y
void AAODungeonGameMode::SendDungeonComplete(bool bIsClear)
{
	Protocol::C_DungeonMapLoadCompletePacket MapPkt;
	MapPkt.set_dungeonid(MyDungeonId);
	SEND_PACKET(MapPkt, PKT_C_DUNGEOMMAPCOMPLETE);
}
//

Protocol::DPlayerInfo* AAODungeonGameMode::ValidateToken(FString Token)
{
	auto ClientInfo = PrePlayers.Find(Token);
	if (ClientInfo)
	{
		return ClientInfo;
	}
	return nullptr;
}
