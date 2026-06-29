// Fill out your copyright notice in the Description page of Project Settings.


#include "AOPlayerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Game/AOGameInstance.h"
#include "Character/ServerCharacter/MMODaeva.h"
#include "Player/AOPlayerController.h"

UAOPlayerManager::UAOPlayerManager() 
{
	static ConstructorHelpers::FClassFinder<APawn> AssassinClassRef(TEXT("/Game/Blueprint/Daeva/Assassin/BP_MMOAssassin"));
	if (AssassinClassRef.Succeeded())
	{
		JobClassMap.Add(1, AssassinClassRef.Class);
	}

	//static ConstructorHelpers::FClassFinder<APawn> ClericClassRef(TEXT(""));
	//if (ClericClassRef.Succeeded())
	//{
	//	JobClassMap.Add(2, ClericClassRef.Class);
	//}

	static ConstructorHelpers::FClassFinder<APawn> RangerClassRef(TEXT("/Game/Blueprint/Daeva/Ranger/BP_MMORanger"));
	if (RangerClassRef.Succeeded())
	{
		JobClassMap.Add(3, RangerClassRef.Class);
	}

	static ConstructorHelpers::FClassFinder<APawn> TemplarClassRef(TEXT("/Game/Blueprint/Daeva/Templar/BP_MMOTemplar"));
	if (TemplarClassRef.Succeeded())
	{
		JobClassMap.Add(4, TemplarClassRef.Class);
	}
}

void UAOPlayerManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GameInstance = Cast<UAOGameInstance>(GetGameInstance());
	PlayerClass = ADaeva::StaticClass();
}

void UAOPlayerManager::HandleLogin(uint64 PlayerId, uint8 ClassType)
{
	// 애니메이션 넣기
	GameInstance->SetMyPlayerId(PlayerId);
	GameInstance->SetMyPlayerClass(ClassType);
}

void UAOPlayerManager::HandleSpawn(uint64 PlayerId, uint8 ClassType, FVector SpawnLocation, FRotator SpawnRotation)
{
	if (!GameInstance)
		return;

	FActorSpawnParameters SpawnParams; 
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (JobClassMap.Contains(ClassType))
	{
		UClass* SpawnClass = JobClassMap[ClassType].Get();
		if (GameInstance->GetMyPlayerId() == PlayerId)
		{
			MyPlayer = GetWorld()->SpawnActor<AMMODaeva>(SpawnClass, SpawnLocation, SpawnRotation, SpawnParams);
			if (MyPlayer != nullptr)
			{
				MyPlayer->SetMyId(PlayerId);
				MyPlayer->SetMyClass(ClassType);

				UE_LOG(LogTemp, Log, TEXT("HandleSpawn - SetMyId: %d"), PlayerId);

				AAOPlayerController* PlayerController = Cast<AAOPlayerController>(GetWorld()->GetFirstPlayerController());
				if (PlayerController != nullptr)
				{
					PlayerController->Possess(MyPlayer);
				}
			}
		}

		else
		{
			AMMODaeva* NewPlayer = GetWorld()->SpawnActor<AMMODaeva>(SpawnClass, SpawnLocation, SpawnRotation, SpawnParams);
			UE_LOG(LogTemp, Log, TEXT("Create NewPlayer: %d"), PlayerId);
			PlayerInfos.Add(PlayerId, NewPlayer);
		}
	}

	for (auto& player : PlayerInfos)
	{
		UE_LOG(LogTemp, Log, TEXT("Stored Players: %d"), player.Key);
	}

}

void UAOPlayerManager::HnadleMove(uint64 PlayerId, FVector NewLocation, FRotator NewRotation, FVector NewVel)
{
	if (!GameInstance)
		return;
	UE_LOG(LogTemp, Log, TEXT("Handle_S_Move: %d"), PlayerId);

	if (GameInstance->GetMyPlayerId() == PlayerId) return;
	auto PlayerRef = PlayerInfos.Find(PlayerId);
	if (PlayerRef == nullptr)return;
	auto Player = PlayerRef->Get();
	if (Player)
	{
		Player->ReceiveMovePacket(NewLocation, NewRotation, NewVel);
	}
}

void UAOPlayerManager::HandleDungeonCreate(int32 DungeonId)
{
	if (!GameInstance)
		return;
	uint64 PlayerId = GameInstance->GetMyPlayerId();
	auto PlayerRef = PlayerInfos.Find(PlayerId);
	if (PlayerRef == nullptr)return;
	auto Player = PlayerRef->Get();
	if (Player)
	{
		Player->SetDungeonId(DungeonId);
	}

}

void UAOPlayerManager::HandleDungeonEnter(int32 DungeonId)
{

}

void UAOPlayerManager::HandleDungeonStart(FString ServerURL)
{
	AAOPlayerController* PC = Cast<AAOPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		PC->ClientTravel(ServerURL, ETravelType::TRAVEL_Absolute);
	}
}
