// Fill out your copyright notice in the Description page of Project Settings.

#include "AOQuickSlotComponent.h"

UAOQuickSlotComponent::UAOQuickSlotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAOQuickSlotComponent::BeginPlay()
{
	Super::BeginPlay();

	QuickSlots.SetNum(MaxQuickSlots);
	for (int32 i = 0; i < MaxQuickSlots; ++i)
	{
		QuickSlots[i].SlotIndex = i;
	}
}

void UAOQuickSlotComponent::InitializeQuickSlot(int32 SlotIndex, int32 ItemTemplateId, int32 ItemInstancedId, int32 Count)
{
	if (!QuickSlots.IsValidIndex(SlotIndex))
	{
		return;
	}

	FAOSlotData& Slot = QuickSlots[SlotIndex];
	Slot.ItemTemplateId = ItemTemplateId;
	Slot.ItemInstancedId = ItemInstancedId;
	Slot.Count = Count;
	Slot.SlotIndex = SlotIndex;

	if (OnQuickSlotChanged.IsBound())
	{
		OnQuickSlotChanged.Broadcast();
	}
}

bool UAOQuickSlotComponent::GetItemDataFromSlot(int32 SlotIndex, FAOSlotData& OutSlotData, FItemData& OutTemplateData)
{
	if (!QuickSlots.IsValidIndex(SlotIndex))
	{
		return false;
	}

	const FAOSlotData& Slot = QuickSlots[SlotIndex];
	if (!Slot.IsValid())
	{
		return false;
	}

	UAOItemDataBase* DataBase = ItemDataBaseAsset.LoadSynchronous();
	if (!DataBase)
	{
		return false;
	}

	if (DataBase->FindItemTemplate(Slot.ItemTemplateId, OutTemplateData))
	{
		OutSlotData = Slot;
		return true;
	}

	return false;
}

bool UAOQuickSlotComponent::FindItemTemplateData(int32 ItemTemplateId, FItemData& OutTemplateData)
{
	UAOItemDataBase* DataBase = ItemDataBaseAsset.LoadSynchronous();
	if (DataBase)
	{
		return DataBase->FindItemTemplate(ItemTemplateId, OutTemplateData);
	}
	return false;
}
