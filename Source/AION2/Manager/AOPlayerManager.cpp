// Fill out your copyright notice in the Description page of Project Settings.


#include "AOPlayerManager.h"
#include "Game/AOGameInstance.h"
#include "Game/AODungeonGameMode.h"

#include "Character/Daeva/Daeva.h"
#include "Character/ServerCharacter/MMODaeva.h"
#include "Player/AOPlayerController.h"

#include "UI/AOPlayerHUDWidget.h"
#include "UI/AOMainHUDWidget.h"
#include "UI/GoldWidget.h"
#include "UI/AOChattingWidget.h"
#include "UI/AOQuickSlotComponent.h"

#include "Item/AOItemDataBase.h"
#include "AbilitySystemComponent.h"
#include "GAS/AttributeSet/AOAttributeSet.h"
#include "Kismet/GameplayStatics.h"

#include "Manager/AOSoundManager.h"

UAOPlayerManager::UAOPlayerManager()
{
	static ConstructorHelpers::FClassFinder<APawn> AssassinClassRef(TEXT("/Game/Blueprint/Daeva/Assassin/BP_MMOAssassin"));
	if (AssassinClassRef.Succeeded())
	{
		JobClassMap.Add(1, AssassinClassRef.Class);
	}

	static ConstructorHelpers::FClassFinder<APawn> ClericClassRef(TEXT("/Game/Blueprint/Daeva/Cleric/BP_MMOCleric"));
	if (ClericClassRef.Succeeded())
	{
		JobClassMap.Add(2, ClericClassRef.Class);
	}

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

void UAOPlayerManager::HandleLogin(Protocol::S_LoginSuccessPacket& LoginPacket)
{
	uint64 PlayerId = LoginPacket.playerinfo().playerid();
	uint8 PlayerClass = static_cast<uint8>(LoginPacket.playerinfo().playerclass());
	float PlayerHp = static_cast<float>(LoginPacket.hp());
	GameInstance->SetMyPlayerId(PlayerId);
	GameInstance->SetMyPlayerClass(LoginPacket.playerinfo().playerclass());

	MyGold = LoginPacket.gold();
	FString PlayerName = UTF8_TO_TCHAR(LoginPacket.playerinfo().playernickname().c_str());

	FPlayerInfo NewInfo(PlayerId, PlayerName, PlayerClass, PlayerHp);
	PlayerInfos.Add(PlayerId, NewInfo);
}

void UAOPlayerManager::HandleSpawn(const uint64 PlayerId, const FString& PlayerName, uint8& ClassType, FVector& SpawnLocation, FRotator& SpawnRotation)
{
	if (!GameInstance)
		return;
	UAOSoundManager::Get(this)->PlayBGM(TEXT("VillageBGM"));

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (JobClassMap.Contains(ClassType))
	{
		UClass* SpawnClass = JobClassMap[ClassType].Get();
		if (GameInstance->GetMyPlayerId() == PlayerId)
		{
			if (IsValid(MyPlayer))
			{
				MyPlayer->SetActorLocationAndRotation(SpawnLocation, SpawnRotation);
			}
			else
			{
				MyPlayer = GetWorld()->SpawnActor<AMMODaeva>(SpawnClass, SpawnLocation, SpawnRotation, SpawnParams);

				if (!MyPlayer) return;
				MyPlayer->SetMyId(PlayerId);
				MyPlayer->SetMyClass(ClassType);

				AAOPlayerController* PlayerController = Cast<AAOPlayerController>(GetWorld()->GetFirstPlayerController());

				if (!PlayerController) return;
				PlayerController->Possess(MyPlayer);

				auto PlayerInfo = PlayerInfos.Find(PlayerId);
				int Hp = 100;
				if (PlayerInfo)
					Hp = PlayerInfo->PlayerHp;
				// === SuYeon: PlayerState�� info�� ��������� ���� ===
				if (AAOPlayerState* AOPlayerState = PlayerController->GetPlayerState<AAOPlayerState>())
				{
					AOPlayerState->SetPlayerInfo(PlayerId, PlayerName, ClassType, Hp);
				}

				if (UAOMainHUDWidget* MainHUD = PlayerController->GetMainHUD())
				{
					if (UAOPlayerHUDWidget* PlayerHUD = MainHUD->GetPlayerHUDWidget())
					{
						PlayerHUD->ChangeClassIcon(static_cast<EDaevaClassType>(ClassType));
					}
				}


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
				UGoldWidget* GoldWidget = MainHUD->GoldWidget;
				if (!GoldWidget) return;
				GoldWidget->SetGold(FString::FromInt(MyGold));
			}
		}

		else
		{
			if (Players.Contains(PlayerId) && IsValid(Players[PlayerId]))
			{
				Players[PlayerId]->SetActorLocationAndRotation(SpawnLocation, SpawnRotation);
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
		}

		FPlayerInfo PlayerInfo(PlayerId, PlayerName, ClassType, 100);
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

void UAOPlayerManager::HnadleMove(uint64 PlayerId, FVector& NewLocation, FRotator& NewRotation, FVector& NewVel)
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

void UAOPlayerManager::HandleStorePurchase(Protocol::ItemData ItemInfo, int32 Gold)
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
		MyGold = Gold;
		UGoldWidget* GoldWidget = MainHUD->GoldWidget;
		GoldWidget->SetGold(FString::FromInt(MyGold));
	}
}

void UAOPlayerManager::HandleUseItem(const Protocol::S_UseItemPacket& Pkt)
{
	int32 SlotIndex = Pkt.slotindex();
	int32 TargetItemId = -1;

	for (auto& Pair : MyItems)
	{
		Protocol::ItemData& Item = Pair.Value;
		if (Item.slotindex() == SlotIndex)
		{
			TargetItemId = Item.iteminstancedid();
			break;
		}
	}

	if (TargetItemId != -1)
	{
		if (Pkt.count() <= 0)
		{
			MyItems.Remove(TargetItemId);
		}
		else
		{
			MyItems.Find(TargetItemId)->set_count(Pkt.count());
		}
	}

	UAOQuickSlotComponent* QuickSlotComp = MyPlayer->GetQuickSlotComponent();
	if (QuickSlotComp)
	{
		FAOSlotData SlotData;
		FItemData TemplateData;
		if (QuickSlotComp->GetItemDataFromSlot(SlotIndex, SlotData, TemplateData))
		{
			FAOSlotData UpdatedSlotData = SlotData;
			UpdatedSlotData.Count = Pkt.count();

			if (Pkt.count() <= 0)
			{
				QuickSlotComp->InitializeQuickSlot(SlotIndex, 0, 0, 0);
				UpdatedSlotData.ItemTemplateId = 0;
				UpdatedSlotData.ItemInstancedId = 0;
			}
			else
			{
				QuickSlotComp->InitializeQuickSlot(SlotIndex, SlotData.ItemTemplateId, SlotData.ItemInstancedId, Pkt.count());
			}

			AAOPlayerController* PC = Cast<AAOPlayerController>(MyPlayer->GetController());
			if (PC)
			{
				UAOMainHUDWidget* MainHUD = PC->GetMainHUD();
				if (MainHUD)
				{
					UAOPlayerHUDWidget* PlayerHUD = MainHUD->GetPlayerHUDWidget();
					if (PlayerHUD)
					{
						PlayerHUD->UpdateItemQuickSlot(Pkt.slotindex(), UpdatedSlotData, TemplateData);
					}
				}
			}
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

			ASC->SetNumericAttributeBase(UAOAttributeSet::GetHealthAttribute(), NewHealth);

			UE_LOG(LogTemp, Log, TEXT("[UseItem] Healed %s HP: %.1f -> %.1f"), *MyPlayer->GetName(), CurrentHealth, NewHealth);
		}
	}
}

void UAOPlayerManager::HandleDungeonSetPlayerInfo(const Protocol::S_DungeonStartDediPacket& Info)
{
	AAODungeonGameMode* GameMode = Cast<AAODungeonGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->SetPrePlayerInfo(Info);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HandleDungeonSetPlayerInfo failed: GameMode is null or not AAODungeonGameMode."));
	}
}

void UAOPlayerManager::HandleDungeonEnd(int32 Gold)
{
	MyGold = Gold;
	//FSoftObjectPath LevelPath(FString::Printf(TEXT("/Game/Map/Lobby")))
	UGameplayStatics::OpenLevel(this, TEXT("/Game/Map/Village"));
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
	ClearMyDungeonRoomState();
	AAOPlayerController* PC = Cast<AAOPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		PC->ClientTravel(ServerURL, ETravelType::TRAVEL_Absolute);
	}
}

void UAOPlayerManager::HandleSetPvpState(Protocol::EPvpState State)
{
	if (State == Protocol::EPvpState::PVP_STATE_ACTIVE)
	{
		MyPlayer->SetPvpState(true, 600);
	}
	else if (State == Protocol::EPvpState::PVP_STATE_ACTIVE)
	{
		MyPlayer->SetPvpState(false, 600);
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

uint8 UAOPlayerManager::GetPlayerClassType(uint64 PlayerId) const
{
	if (const FPlayerInfo* Info = PlayerInfos.Find(PlayerId))
	{
		return Info->ClassType;
	}
	if (GameInstance && GameInstance->GetMyPlayerId() == PlayerId)
	{
		return static_cast<uint8>(GameInstance->GetMyPlayerClass());
	}
	return 0;
}

#pragma endregion

void UAOPlayerManager::HandleDash(const uint64 PlayerId, FVector& NewLocation, FRotator& NewRotation, FVector& NewVel)
{
	if (!GameInstance)
		return;
	UE_LOG(LogTemp, Log, TEXT("HandleDash: %d"), PlayerId);

	if (GameInstance->GetMyPlayerId() == PlayerId) return;
	auto PlayerRef = Players.Find(PlayerId);
	if (PlayerRef == nullptr) return;
	AMMODaeva* MMOPlayer = *PlayerRef;
	if (MMOPlayer)
	{
		MMOPlayer->ReceiveDashPacket(NewLocation, NewRotation, NewVel);
	}
}

void UAOPlayerManager::HandleJump(const uint64 PlayerId, bool bIsGliding)
{
	if (!GameInstance)
		return;

	if (GameInstance->GetMyPlayerId() == PlayerId) return;

	auto PlayerRef = Players.Find(PlayerId);
	if (PlayerRef == nullptr) return;

	AMMODaeva* MMOPlayer = *PlayerRef;
	if (MMOPlayer)
	{
		MMOPlayer->ReceiveJumpPacket(bIsGliding);
	}
}

void UAOPlayerManager::HandleAttack(Protocol::S_AttackResultPacket& Pkt)
{
	AMMODaeva* TargetPlayer = nullptr;

	if (MyPlayer && MyPlayer->GetMyId() == Pkt.targetid())
	{
		TargetPlayer = MyPlayer;
	}
	else if (auto PlayerRef = Players.Find(Pkt.targetid()))
	{
		TargetPlayer = *PlayerRef;
	}

	if (TargetPlayer)
	{
		UAbilitySystemComponent* MyASC = TargetPlayer->GetAbilitySystemComponent();
		if (MyASC)
		{
			MyASC->SetNumericAttributeBase(UAOAttributeSet::GetHealthAttribute(), Pkt.targethp());
		}
		if (Pkt.isdead())
		{
			EMontageID PlayMontageID = EMontageID::Die;
			TargetPlayer->PlayMontageWithSection(PlayMontageID, 2.0f, NAME_None); 
		}
	}

	if (MyPlayer && MyPlayer->GetMyId() != Pkt.attackerid())
	{
		if (auto PlayerRef = Players.Find(Pkt.attackerid()))
		{
			AMMODaeva* AttackerPlayer = *PlayerRef;
			if (AttackerPlayer)
			{
				EMontageID PlayMontageID = static_cast<EMontageID>(Pkt.skillid());
				AttackerPlayer->PlayMontageWithSection(PlayMontageID, 1.0f, NAME_None);
			}
		}
	}
}