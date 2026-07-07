// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/AOUserWidgetBase.h"
#include "Network/PacketHeader.h"
#include "AODungeonRoomWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonRoomSlotClicked, int32, DungeonId);

class UTextBlock;
class UImage;
class UButton;
class UAOClassSwitcherWidget;
class UOverlay;

/**
 *
 */
UCLASS()
class AION2_API UAODungeonRoomWidget : public UAOUserWidgetBase
{
	GENERATED_BODY()

public:
	void SetDungeonInfo(const Protocol::DungeonInfo& InInfo);
	int32 GetDungeonId() const { return DungeonId; }

	void ClearDungeonInfo();

	// 멤버가 들어오면 갱신
	void AddOrUpdateMemberInfo(const Protocol::DungeonPlayerInfo& MemberInfo);

	// Ready를 반영
	void SetDungeonReady(uint64 PlayerId);

	UPROPERTY(BlueprintAssignable)
	FOnDungeonRoomSlotClicked OnJoinRequested;

protected:
	virtual void NativeConstruct() override;

protected:
	void SetLeaderInfo(const Protocol::DungeonPlayerInfo& InInfo);

	// 이미 존재하는 방에 새로 한 명만 들어옴-> 클래스 지정
	void AddMemberInfo(const Protocol::DungeonPlayerInfo& MemberInfo);

	// 던전 방을 새로 그림
	void SetMemberInfos(const Protocol::DungeonInfo& DungeonInfo);

protected:
	// 
	/*상위 Widget인 DungeonEntranceWidget이 Bind.
	* 이 방 카드를 누름 => broadcast with dungeonId.
	* 서버에 EnterPacket 전송
	*/
	UFUNCTION()
	void HandleJoinClicked();

	// 방장의 클래스 Widget 지정
	UFUNCTION()
	void SetLeaderClassType(uint8 ClassType);

	// 만약 Player가 참가 중인 Room이라면 UI 상태를 바꿔줘야 함
private:
	void ApplyParticipationState(const Protocol::DungeonInfo& DungeonInfo);


protected:

	// 이 방에 들어간 멤버들의 상태 
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TB_LeaderName;

	// 아래로 Class Type 바꾸는 명령 내릴 것.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UAOClassSwitcherWidget> LeaderClassSlot;

	// MemberClass Slot.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UAOClassSwitcherWidget> MemberClassSlot1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UAOClassSwitcherWidget> MemberClassSlot2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UAOClassSwitcherWidget>MemberClassSlot3;

	// 위에서 Bind된 MemberClassSlot Widget들을 편하게 관리하기 위해 Widget 생성자에서 Array로 묶음
	UPROPERTY()
	TArray<TObjectPtr<UAOClassSwitcherWidget>> MemberClassSlots;

	// 이 방 카드의 전체를 버튼으로 삼음
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TB_StatusText;


	// 만약 Player가 참가 중인 Room이라면 UI 보이기 상태를 바꿔줘야 함
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_PlayerJoined;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> Overlay_NotJoin;

	// 이 던전의 ID. 
private:
	int32 DungeonId = 0;
};
