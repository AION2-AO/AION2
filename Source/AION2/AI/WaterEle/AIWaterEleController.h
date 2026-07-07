// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/AIMonsterControllerBase.h"
#include "AIWaterEleController.generated.h"

/**
 * 
 */
class AWaterEle;
UCLASS()
class AION2_API AAIWaterEleController : public AAIMonsterControllerBase
{
	GENERATED_BODY()

protected:
	AAIWaterEleController();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	virtual void OnPossess(APawn* InPawn) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class AWaterEle> ControlledWaterEle;

private:
	UPROPERTY(EditAnywhere, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAOAttributeSet> ControlledMonsterAttributeSet;
	
};
