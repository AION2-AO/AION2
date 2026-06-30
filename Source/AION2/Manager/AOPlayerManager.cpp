// Fill out your copyright notice in the Description page of Project Settings.


#include "AOPlayerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Game/AOGameInstance.h"
#include "Character/ServerCharacter/MMODaeva.h"
#include "Player/AOPlayerController.h"
#include "Player/AOPlayerState.h"
#include "UI/AOPlayerHUDWidget.h"
#include "UI/AOMainHUDWidget.h"
#include "AOQuickSlotComponent.h"

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
	GameInstance->SetMyPlayerId(PlayerId);
	GameInstance->SetMyPlayerClass(ClassType);
}

void UAOPlayerManager::HandleSpawn(uint64 PlayerId, FString PlayerName, uint8 ClassType, FVector SpawnLocation, FRotator SpawnRotation)
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

				AAOPlayerController* PlayerController = Cast<AAOPlayerController>(GetWorld()->GetFirstPlayerController());
				if (PlayerController != nullptr)
				{
					PlayerController->Possess(MyPlayer);
					UAOQuickSlotComponent* InventoryComp = MyPlayer->FindComponentByClass<UAOQuickSlotComponent>();

					if (InventoryComp == nullptr)
					{
						UE_LOG(LogTemp, Warning, TEXT("Inventory Is Nat Vaild"));
						return;
					}

					UAOMainHUDWidget* MainHUD = PlayerController->GetMainHUD();
					if (MainHUD == nullptr) return;

					UAOPlayerHUDWidget* PlayerHUD = MainHUD->GetPlayerHUDWidget();
					if (PlayerHUD == nullptr) return;
				
					for (const auto& Pair : MyItems)
					{
						const Protocol::ItemData& Item = Pair.Value;

						int32 InstanceId = Item.iteminstancedid();
						int32 TemplateId = Item.itemtemplateid();
						int32 SlotIndex = Item.slotindex();
						int32 Count = Item.count();

						FAOSlotData SlotData;
						SlotData.ItemInstancedId = InstanceId;
						SlotData.ItemTemplateId = TemplateId;
						SlotData.SlotIndex = SlotIndex;
						SlotData.Count = Count;
						FItemData TemplateData;

						if (InventoryComp->FindItemTemplateData(TemplateId, TemplateData))
						{
							PlayerHUD->UpdateItemQuickSlot(SlotIndex, SlotData, TemplateData);
						}
					}
				}
			}
		}

		else
		{
			AMMODaeva* NewPlayer = GetWorld()->SpawnActor<AMMODaeva>(SpawnClass, SpawnLocation, SpawnRotation, SpawnParams);
			UE_LOG(LogTemp, Log, TEXT("Create NewPlayer: %d"), PlayerId);
			Players.Add(PlayerId, NewPlayer);
		}

		FPlayerInfo PlayerInfo(PlayerId, PlayerName, ClassType);
		PlayerInfos.Add(PlayerId, PlayerInfo);
	}

	for (auto& player : Players)
	{
		UE_LOG(LogTemp, Log, TEXT("Stored Players: %d"), player.Key);
	}
}

void UAOPlayerManager::HandleItem(Protocol::S_ItemDataPacket Items)
{
	if (Items.playeritems_size() > 0)
	{
		int32 ItemCount = Items.playeritems_size();
		if (ItemCount == 0) return;

		if (Items.playeritems_size() > 0)
		{
			for (int i = 0; i < ItemCount; i++)
			{
				const Protocol::ItemData& Item = Items.playeritems(i);
				MyItems.Add(Item.iteminstancedid(), Item);
			}
		}
	}
}

void UAOPlayerManager::HnadleMove(uint64 PlayerId, FVector NewLocation, FRotator NewRotation, FVector NewVel)
{
	if (!GameInstance)
		return;
	UE_LOG(LogTemp, Log, TEXT("Handle_S_Move: %d"), PlayerId);

	if (GameInstance->GetMyPlayerId() == PlayerId) return;
	auto PlayerRef = Players.Find(PlayerId);
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
	auto PlayerRef = Players.Find(PlayerId);
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
