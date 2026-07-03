// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/AOUIManager.h"
#include "Player/AOPlayerController.h"
#include "Game/AOGameInstance.h"

void UAOUIManager::Initialize(FSubsystemCollectionBase& collection)
{
	Super::Initialize(collection);
	WidgetCache.Empty();
}

void UAOUIManager::Deinitialize()
{
	WidgetCache.Empty();
	Super::Deinitialize();
}

UUserWidget* UAOUIManager::ShowWidget(TSoftClassPtr<UUserWidget> WidgetClass, EUILayer Layer)
{
	if (WidgetClass.IsNull()) return nullptr;

	// 캐시에 기존 인스턴스가 있는지 확인
	UUserWidget** FoundWidget = WidgetCache.Find(WidgetClass);
	UUserWidget* WidgetInstance = FoundWidget ? *FoundWidget : nullptr;

	AAOPlayerController* PC = Cast<AAOPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PC || !PC->IsLocalPlayerController()) return nullptr;

	// 없으면 생성
	if (!WidgetInstance)
	{
		UClass* LoadedClass = WidgetClass.LoadSynchronous();
		if (!LoadedClass) return nullptr;

		WidgetInstance = CreateWidget<UUserWidget>(PC, LoadedClass);
		if (WidgetInstance)
		{
			WidgetCache.Add(WidgetClass, WidgetInstance);
		}
	}

	if (WidgetInstance && !WidgetInstance->IsInViewport())
	{
		int ZOrder = GetZOrderForLayer(Layer);
		WidgetInstance->AddToViewport(ZOrder);
	}
	return WidgetInstance;
}

void UAOUIManager::HideWidget(UUserWidget* WidgetInstance)
{
	if (WidgetInstance && WidgetInstance->IsInViewport())
	{
		WidgetInstance->RemoveFromParent();
	}
}

UUserWidget* UAOUIManager::GetWidgetByClass(TSubclassOf<UUserWidget> WidgetClass) const
{
	for (auto& Pair : WidgetCache)
	{
		if (Pair.Value && Pair.Value->GetClass()->IsChildOf(WidgetClass))
		{
			return Pair.Value;
		}
	}
	return nullptr;
}

int UAOUIManager::GetZOrderForLayer(EUILayer Layer) const
{
	switch (Layer)
	{
	case EUILayer::Background: return 0;
	case EUILayer::Default:    return 10;
	case EUILayer::PopUp:      return 20;
	case EUILayer::System:     return 30;
	}
	return 10;
}
