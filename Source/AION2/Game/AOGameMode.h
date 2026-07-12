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

	// TEST.KHY
public:
	UFUNCTION(BlueprintCallable)
	void NotifyPlayerDied(AController* DeadController);
	void NotifyPlayerRespawnImmediately(AController* DeadController);

protected:
	void RespawnPlayerImmediately(AController* DeadController, const FTransform& RespawnTransform);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Test|Respawn")
	float Respawn = 3.0f;

	TMap<TObjectPtr<AController>, FTimerHandle> RespawnTimerHandles;
	
private:
	class UAOGameInstance* GameInst;
	class UAONetworkManager* NetworkManager;
};
