// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "AIMonsterControllerBase.generated.h"

/**
 * 
 */
UCLASS()
class AION2_API AAIMonsterControllerBase : public AAIController
{
	GENERATED_BODY()

public:
	AAIMonsterControllerBase(); 


protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnPossess(APawn* InPawn) override;	

public:
	FORCEINLINE void Set_Phase(FGameplayTag _PhaseTag) { PhaseTag = _PhaseTag; }
	FORCEINLINE void Set_State(FGameplayTag _StateTag) { StateTag = _StateTag; }

	FORCEINLINE FGameplayTag Get_Phase() { return PhaseTag; }
	FORCEINLINE FGameplayTag Get_State() { return StateTag; }

	UFUNCTION()
	virtual void TargetPerceptionOn(AActor* Actor, FAIStimulus  Stimlus);

	UFUNCTION()
	AActor* Get_CurrentTargetPlayer() { return CurrentTargetPlayer; }	



protected:
	// State AI Tree ( 상태 트리 AI ) 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UStateTreeAIComponent> StateTreeAIComponent;

	// AI Percption ( 시야, 청각 등등) 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UAIPerceptionComponent> AIPerceptionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI State", meta = (AllowPrivateAccess = "true"))
	FGameplayTag PhaseTag; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI State", meta = (AllowPrivateAccess = "true"))
	FGameplayTag StateTag;

	// 현재 AI Controller가 조종하고 있는 Character
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class AAOMonsterBase> ControlledMonster;

	// 현재 AI Controller에게 인식된 타겟 플레이어 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetPlayer", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> CurrentTargetPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetPlayer", meta = (AllowPrivateAccess = "true"))
	float DistanceToTarget = 0.f;


	// 타겟이 인식 되었는지 판단.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool HasDetectedTarget = false;


	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<AActor>> ArrayTargetPlayers;

};
