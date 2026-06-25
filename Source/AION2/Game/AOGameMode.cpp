// Fill out your copyright notice in the Description page of Project Settings.


#include "AOGameMode.h"
#include "Game/AOGameInstance.h"
#include "Manager/AONetworkManager.h"

void AAOGameMode::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay - AMyGameMode"));

	GameInst = Cast<UAOGameInstance>(GetWorld()->GetGameInstance());
	check(GameInst && "GameInstance is invalid!");

	if (GameInst->GetNetworkManager())
	{
		NetworkManager = GameInst->GetNetworkManager();
	}
}

void AAOGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (NetworkManager)
		NetworkManager->ProcessQueuePackets();
}
