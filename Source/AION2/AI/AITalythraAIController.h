// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Types/AIPhaseFlag.h"
#include "AITalythraAIController.generated.h"




/**
 * 
 */
UCLASS()
class AION2_API AAITalythraAIController : public AAIController
{
	GENERATED_BODY()

public:
	AAITalythraAIController();
	
protected:
	virtual void BeginPlay() override; 
	virtual void Tick(float DeltaSeconds) override;
	

public:
	virtual void OnPossess(APawn* InPawn) override; 

	UFUNCTION()
	void TargetPerceptionOn(AActor* Actor, FAIStimulus  Stimlus);


	UFUNCTION()
	AActor* Get_CurrentTargetPlayer() {return CurrentTargetPlayer;}

	FORCEINLINE void Set_Phase(ETalythraPhase _PhaseFlag) { Phase = _PhaseFlag; }



protected:

	// State AI Tree ( 상태 트리 AI ) 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UStateTreeAIComponent> StateTreeAIComponent;

	// AI Percption ( 시야, 청각 등등) 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UAIPerceptionComponent> AIPerceptionComponent;

	// 현재 AI Controller가 조종하고 있는 Character
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class ATalythra> ControlledTalythra;

	// 타겟이 인식 되었는지 판단.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool HasDetectedTarget = false;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
	ETalythraPhase Phase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	ETalythraState State;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetPlayer")
	float DistanceToTarget = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetPlayer")
	TObjectPtr<AActor> CurrentTargetPlayer;


private:
	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<AActor>> ArrayTargetPlayers;

	

};
