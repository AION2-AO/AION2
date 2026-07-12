// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AOStoreWidget.h"
#include "Components/Button.h"
#include "Network/PacketHeader.h"
#include "Game/AOGameInstance.h"
#include "AOStoreWidget.h"

void UAOStoreWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// EnterButton 이벤트 바인딩
	if (Button_HealPurchase)
	{
		Button_HealPurchase->OnClicked.AddDynamic(this, &UAOStoreWidget::OnHealPurchase);
	}
}

void UAOStoreWidget::OnHealPurchase()
{
	UAOGameInstance* GI = Cast<UAOGameInstance>(GetWorld()->GetGameInstance());
	if (!GI) return;

	Protocol::C_StorePurchasePacket PurchasePacket;
	uint64 PlayerId = GI->GetMyPlayerId();
	PurchasePacket.set_itemid(1);
	PurchasePacket.set_playerid(PlayerId);
	GI->SendPacket(PurchasePacket, PKT_C_STORE_PURCHASE);
}
