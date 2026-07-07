// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AOMMOPlayerController.generated.h"


class UUserWidget;

/**
 * 
 */
UCLASS()
class AION2_API AAOMMOPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// ! 신혜님 UI 코드에서 많이 사용 중이라 삭제하면 안됨 !: 추후 분리하든가 할 것.
	//UUserWidget* GetVillageHUD() const { return VillageHUD; }


	
	
};
