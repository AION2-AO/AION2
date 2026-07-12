// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DungeonClearWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "TimerManager.h"
#include "Player/AOPlayerController.h"
#include "Game/AODungeonGameMode.h"

void UDungeonClearWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Check)
		Btn_Check->OnClicked.AddUniqueDynamic(this, &UDungeonClearWidget::OnCheckClicked);
}

void UDungeonClearWidget::SetDungeonClearWidget(int32 Gold)
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	if (GoldPrice)
	{
		GoldPrice->SetText(FText::AsNumber(Gold));
	}

	if(Clear1)
	{
		Clear1->SetVisibility(ESlateVisibility::Visible);
	}

	if (Clear2)
	{
		Clear2->SetVisibility(ESlateVisibility::Visible);
	}

	RemainingTime = 3;

	if (CountdownText)
	{
		CountdownText->SetText(FText::AsNumber(RemainingTime));
	}

	GetWorld()->GetTimerManager().ClearTimer(ClearTimer);
	GetWorld()->GetTimerManager().SetTimer(
		ClearTimer,
		this,
		&UDungeonClearWidget::UpdateCountdown,
		1.0f,
		true
	);

}

void UDungeonClearWidget::DungeonReward()
{
	if (Clear1)
	{
		Clear1->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (Clear2)
	{
		Clear2->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (ClearPopup)
	{
		ClearPopup->SetVisibility(ESlateVisibility::Visible);
	}

	APlayerController* PC = GetOwningPlayer();
	if (PC && PC->IsLocalController())
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}

}

void UDungeonClearWidget::UpdateCountdown()
{
	RemainingTime--;

	if (RemainingTime <= 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(ClearTimer);

		DungeonReward();
		return;
	}

	if (CountdownText)
	{
		CountdownText->SetText(FText::AsNumber(RemainingTime));
	}
}

void UDungeonClearWidget::OnCheckClicked()
{
	//AAODungeonGameMode* GameMode = Cast<AAODungeonGameMode>(GetWorld()->GetAuthGameMode());
	//if (GameMode)
	//{
	//	GameMode->SendDungeonCompleteRequest();
	//}
	AAOPlayerController* PlayerController = Cast<AAOPlayerController>(GetOwningPlayer());

	if (!PlayerController)
	{
		return;
	}

	PlayerController->ServerRequestDungeonComplete();

	FInputModeGameOnly InputMode;
	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = false;
	RemoveFromParent();
}
