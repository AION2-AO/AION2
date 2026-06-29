// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "AORaidGameState.generated.h"

/**
 * Game Mode가 정의한 규칙에 따라 게임의 현재 상태를 추적
 * => 모든 Client에 복제됨
 */
UCLASS()
class AION2_API AAORaidGameState : public AGameState
{
	GENERATED_BODY()
	
	// TODO: 일단 비워두고, 추가 구현 시 내용 추가.
	//UPROPERTY(ReplicatedUsing = OnRep_BossHP)
	//float BossHP;
	//
	//UPROPERTY(ReplicatedUsing = OnRep_BossMaxHP)
	//float BossMaxHP;
	//
	//UPROPERTY(ReplicatedUsing = OnRep_RaidPhase)
	//int32 RaidPhase;
	
	// + 레이드 멤버 목록
};
