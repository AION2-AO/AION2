// Fill out your copyright notice in the Description page of Project Settings.


#include "AOPlayerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Game/AOGameInstance.h"
#include "Character/Daeva/Daeva.h"
#include "Character/ServerCharacter/MMODaeva.h"
#include "Player/AOPlayerController.h"
#include "Player/AOPlayerState.h"
#include "UI/AOPlayerHUDWidget.h"
#include "UI/AOMainHUDWidget.h"
#include "AOChattingWidget.h"
#include "AOQuickSlotComponent.h"
#include "Item/AOItemDataBase.h"
#include "AbilitySystemComponent.h"
#include "GAS/AttributeSet/AOAttributeSet.h"
#include "Game/AODungeonGameMode.h"

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

void UAOPlayerManager::HandleLogin(const uint64 PlayerId, const uint8 ClassType)
{
	GameInstance->SetMyPlayerId(PlayerId);
	GameInstance->SetMyPlayerClass(ClassType);
}

void UAOPlayerManager::HandleSpawn(const uint64 PlayerId, const FString PlayerName, uint8 ClassType, FVector SpawnLocation, FRotator SpawnRotation)
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
			//MyPlayer = GetWorld()->SpawnActor<ADaeva>(SpawnClass, SpawnLocation, SpawnRotation, SpawnParams);
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
							InventoryComp->InitializeQuickSlot(SlotIndex, TemplateId, InstanceId, Count);
							PlayerHUD->UpdateItemQuickSlot(SlotIndex, SlotData, TemplateData);
						}
					}
				}
			}
		}

		else
		{
			AMMODaeva* NewPlayer = GetWorld()->SpawnActor<AMMODaeva>(SpawnClass, SpawnLocation, SpawnRotation, SpawnParams);
			if (NewPlayer)
			{
				NewPlayer->SetMyId(PlayerId);
				NewPlayer->SetMyClass(ClassType);
			}
			UE_LOG(LogTemp, Log, TEXT("Create NewPlayer: %d, Location: %f, %f, %f "), PlayerId, SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
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

void UAOPlayerManager::HandleChatting(FString SenderName, FString SendMessage)
{
	AAOPlayerController* PlayerController = Cast<AAOPlayerController>(MyPlayer->GetController());
	if (PlayerController && PlayerController->GetMainHUD())
	{
		if (UAOChattingWidget* ChatWidget = PlayerController->GetMainHUD()->ChattingWidget)
		{
			ChatWidget->AddChatMessage(SenderName, SendMessage);
		}
	}
}

void UAOPlayerManager::HandleStorePurchase(Protocol::ItemData ItemInfo)
{
	AAOPlayerController* PlayerController = Cast<AAOPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PlayerController != nullptr)
	{
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

		int32 InstanceId = ItemInfo.iteminstancedid();
		int32 TemplateId = ItemInfo.itemtemplateid();
		int32 SlotIndex = ItemInfo.slotindex();
		int32 Count = ItemInfo.count();

		FAOSlotData SlotData;
		SlotData.ItemInstancedId = InstanceId;
		SlotData.ItemTemplateId = TemplateId;
		SlotData.SlotIndex = SlotIndex;
		SlotData.Count = Count;
		FItemData TemplateData;

		if (InventoryComp->FindItemTemplateData(TemplateId, TemplateData))
		{
			InventoryComp->InitializeQuickSlot(SlotIndex, TemplateId, InstanceId, Count);
			PlayerHUD->UpdateItemQuickSlot(SlotIndex, SlotData, TemplateData);
		}
	}
}

void UAOPlayerManager::HandleUseItem(const Protocol::S_UseItemPacket& Pkt)
{
	int32 SlotIndex = Pkt.slotindex();
	UAOQuickSlotComponent* QuickSlotComp = MyPlayer->GetQuickSlotComponent();
	if (QuickSlotComp)
	{
		FAOSlotData SlotData;
		FItemData TemplateData;
		if (QuickSlotComp->GetItemDataFromSlot(SlotIndex, SlotData, TemplateData))
		{
			QuickSlotComp->InitializeQuickSlot(SlotIndex, SlotData.ItemTemplateId, SlotData.ItemInstancedId, Pkt.count());

			AAOPlayerController* PC = Cast<AAOPlayerController>(MyPlayer->GetController());
			if (!PC)return;

			UAOMainHUDWidget* MainHUD = PC->GetMainHUD();
			if (!MainHUD)return;

			UAOPlayerHUDWidget* PlayerHUD = MainHUD->GetPlayerHUDWidget();
			if (!PlayerHUD)return;

			FAOSlotData UpdatedSlotData = SlotData;
			UpdatedSlotData.Count = Pkt.count();
			PlayerHUD->UpdateItemQuickSlot(Pkt.slotindex(), UpdatedSlotData, TemplateData);

		}
	}

	UAbilitySystemComponent* ASC = MyPlayer->GetAbilitySystemComponent();
	if (ASC)
	{
		if (Pkt.effecttype() == "GE_Health")
		{
			float CurrentHealth = ASC->GetNumericAttribute(UAOAttributeSet::GetHealthAttribute());
			float MaxHealth = ASC->GetNumericAttribute(UAOAttributeSet::GetMaxHealthAttribute());
			float NewHealth = FMath::Min(CurrentHealth + Pkt.effectvalue(), MaxHealth);

			// 캐릭터의 Health 어트리뷰트 베이스 값 갱신 (자동으로 OnHealthChanged 트리거)
			ASC->SetNumericAttributeBase(UAOAttributeSet::GetHealthAttribute(), NewHealth);

			UE_LOG(LogTemp, Log, TEXT("[UseItem] Healed %s HP: %.1f -> %.1f"), *MyPlayer->GetName(), CurrentHealth, NewHealth);
		}
	}
}

void UAOPlayerManager::HandleDungeonSetPlayerInfo(const Protocol::S_DungeonStartDediPacket& Info)
{
	AAODungeonGameMode* GameMode = Cast<AAODungeonGameMode>(GetWorld()->GetAuthGameMode());
	GameMode->SetPrePlayerInfo(Info);
}

void UAOPlayerManager::HandleDisconnect(uint64 RemovePlayerId)
{
	auto PlayerRef = Players.Find(RemovePlayerId);
	if (PlayerRef)
	{
		auto Player = PlayerRef->Get();
		Player->Destroy();

		Players.Remove(RemovePlayerId);
		PlayerInfos.Remove(RemovePlayerId);
	}
}


#pragma region Dungeon State

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

void UAOPlayerManager::ClearMyDungeonRoomState()
{
	MyDungeonRoomState = FPlayerDungeonRoomState();
}

bool UAOPlayerManager::TryUpdateMyDungeonRoomState(const Protocol::DungeonInfo& DungeonInfo)
{
	if (!GameInstance)
	{
		return false;
	}

	const uint64 MyPlayerId = GameInstance->GetMyPlayerId();

	if (DungeonInfo.has_leaderinfo() && DungeonInfo.leaderinfo().memberid() == MyPlayerId)
	{
		MyDungeonRoomState.DungeonId = DungeonInfo.dungeonid();
		MyDungeonRoomState.EntranceState = EDungeonEntranceState::Leader;
		MyDungeonRoomState.ReadyState = EReadyState::Ready;
		return true;
	}

	for (int32 Index = 0; Index < DungeonInfo.members_size(); ++Index)
	{
		const Protocol::DungeonPlayerInfo& MemberInfo = DungeonInfo.members(Index);

		if (MemberInfo.memberid() == MyPlayerId)
		{
			MyDungeonRoomState.DungeonId = DungeonInfo.dungeonid();
			MyDungeonRoomState.EntranceState = EDungeonEntranceState::Member;
			MyDungeonRoomState.ReadyState = MemberInfo.isready() ? EReadyState::Ready : EReadyState::Preparing;
			return true;
		}
	}

	return false;
}

void UAOPlayerManager::UpdateMyDungeonRoomStateFromList(const google::protobuf::RepeatedPtrField<Protocol::DungeonInfo>& DungeonInfos)
{
	for (const Protocol::DungeonInfo& DungeonInfo : DungeonInfos)
	{
		if (TryUpdateMyDungeonRoomState(DungeonInfo))
		{
			return;
		}
	}

	//ClearMyDungeonRoomState();
}

void UAOPlayerManager::UpdateMyDungeonEnterState(int32 DungeonId, const Protocol::DungeonPlayerInfo& EnterPlayer)
{
	if (!GameInstance)
	{
		return;
	}

	if (EnterPlayer.memberid() != GameInstance->GetMyPlayerId())
	{
		return;
	}

	MyDungeonRoomState.DungeonId = DungeonId;
	MyDungeonRoomState.EntranceState = EDungeonEntranceState::Member;
	MyDungeonRoomState.ReadyState = EnterPlayer.isready() ? EReadyState::Ready : EReadyState::Preparing;
}

void UAOPlayerManager::UpdateMyDungeonReadyState(int32 DungeonId, uint64 PlayerId)
{
	if (!GameInstance)
	{
		return;
	}

	if (PlayerId != GameInstance->GetMyPlayerId())
	{
		return;
	}

	if (MyDungeonRoomState.DungeonId != DungeonId)
	{
		return;
	}

	MyDungeonRoomState.ReadyState = EReadyState::Ready;
}

#pragma endregion
