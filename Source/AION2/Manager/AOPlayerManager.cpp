// Fill out your copyright notice in the Description page of Project Settings.


#include "AOPlayerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Game/AOGameInstance.h"
#include "Character/Daeva/Daeva.h"

UAOPlayerManager::UAOPlayerManager()
{
	static ConstructorHelpers::FClassFinder<APawn> AssassinClassRef(TEXT(""));
	if (AssassinClassRef.Succeeded())
	{
		JobClassMap.Add(1, AssassinClassRef.Class);
	}

	static ConstructorHelpers::FClassFinder<APawn> ClericClassRef(TEXT(""));
	if (ClericClassRef.Succeeded())
	{
		JobClassMap.Add(2, ClericClassRef.Class);
	}

	static ConstructorHelpers::FClassFinder<APawn> RangerClassRef(TEXT(""));
	if (RangerClassRef.Succeeded())
	{
		JobClassMap.Add(3, RangerClassRef.Class);
	}

	static ConstructorHelpers::FClassFinder<APawn> TemplarClassRef(TEXT(""));
	if (TemplarClassRef.Succeeded())
	{
		JobClassMap.Add(4, TemplarClassRef.Class);
	}
}

void UAOPlayerManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GameInstance = Cast<UAOGameInstance>(GetGameInstance());
	World = GetWorld();
	PlayerClass = ADaeva::StaticClass();
}

void UAOPlayerManager::HandleLogin(uint64 PlayerId, uint8 ClassType)
{
	// 애니메이션 넣기
	GameInstance->SetMyPlayerId(PlayerId);
	GameInstance->SetMyPlayerClass(ClassType);
}

void UAOPlayerManager::HandleSpawn(uint64 PlayerId, uint8 ClassType, FVector SpawnLocation)
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
			MyPlayer = World->SpawnActor<ADaeva>(SpawnClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
		}

		else
		{
			ADaeva* NewPlayer = World->SpawnActor<ADaeva>(SpawnClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
			PlayerInfos.try_emplace(PlayerId, NewPlayer);
		}
	}
}
