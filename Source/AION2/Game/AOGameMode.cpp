// Fill out your copyright notice in the Description page of Project Settings.


#include "AOGameMode.h"
#include "Game/AOGameInstance.h"
#include "Manager/AONetworkManager.h"

#include "Character/Daeva/Daeva.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

AAOGameMode::AAOGameMode()
{
	DefaultPawnClass = nullptr;
}

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
}

void AAOGameMode::NotifyPlayerDied(AController* DeadController)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!DeadController)
	{
		return;
	}

	if (RespawnTimerHandles.Contains(DeadController))
	{
		return;
	}

	APawn* DeadPawn = DeadController->GetPawn();

	if (!DeadPawn)
	{
		return;
	}

	const FTransform RespawnTransform = DeadPawn->GetActorTransform();

	FTimerHandle RespawnTimerHandle;

	TWeakObjectPtr<AController> WeakController = DeadController;

	FTimerDelegate RespawnDelegate;
	RespawnDelegate.BindLambda([this, WeakController, RespawnTransform]()
		{
			if (!WeakController.IsValid())
			{
				return;
			}

			RespawnPlayerImmediately(WeakController.Get(), RespawnTransform);
		}
	);

	GetWorldTimerManager().SetTimer(RespawnTimerHandle,RespawnDelegate,Respawn,false);

	RespawnTimerHandles.Add(DeadController, RespawnTimerHandle);
}

void AAOGameMode::NotifyPlayerRespawnImmediately(AController* DeadController)
{
	APawn* DeadPawn = DeadController->GetPawn();
	if (!DeadPawn)
	{
		return;
	}

	RespawnPlayerImmediately(DeadController, DeadPawn->GetActorTransform());
}

void AAOGameMode::RespawnPlayerImmediately(AController* DeadController, const FTransform& RespawnTransform)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!DeadController)
	{
		return;
	}

	if (FTimerHandle* TimerHandle = RespawnTimerHandles.Find(DeadController))
	{
		GetWorldTimerManager().ClearTimer(*TimerHandle);
		RespawnTimerHandles.Remove(DeadController);
	}

	APawn* OldPawn = DeadController->GetPawn();

	if (OldPawn)
	{
		DeadController->UnPossess();
		OldPawn->Destroy();
	}

	RestartPlayerAtTransform(DeadController, RespawnTransform);

	if (ADaeva* RespawnedPlayer = Cast<ADaeva>(DeadController->GetPawn()))
	{
		RespawnedPlayer->RestorePlayerInfoFromPlayerState();
		RespawnedPlayer->ResetForDungeonRespawn();
	}
}
