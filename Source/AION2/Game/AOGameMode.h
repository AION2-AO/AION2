// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "AOGameMode.generated.h"

/**
 * 
 */
UCLASS()
class AION2_API AAOGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	AAOGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
private:
	class UAOGameInstance* GameInst;
	class UAONetworkManager* NetworkManager;
};
