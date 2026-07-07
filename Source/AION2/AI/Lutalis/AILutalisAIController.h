// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/AIMonsterControllerBase.h"
#include "AILutalisAIController.generated.h"

/**
 * 
 */
UCLASS()
class AION2_API AAILutalisAIController : public AAIMonsterControllerBase
{
	GENERATED_BODY()


public:
	AAILutalisAIController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	virtual void TargetPerceptionOn(AActor* Actor, FAIStimulus Stimlus) override;

public:
	//virtual void OnPossess(APawn* InPawn) override;

protected:

	// 현재 AI Controller가 조종하고 있는 Character
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class ALutalis> ControlledLutails;


private:
	// AttributeSet
	UPROPERTY(EditAnywhere, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAOAttributeSet> ControlledMonsterAttributeSet;
};
