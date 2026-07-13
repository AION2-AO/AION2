// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "AOCooldownTagProvider.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UAOCooldownTagProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * CoolTownTag를 사용하는 여러 Type의 class가 공통적으로 Tag를 제공하도록 하는 interface.
 */
class AION2_API IAOCooldownTagProvider
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual bool GetUICooldownTag(FGameplayTag& OutCooldownTag) const = 0;
	
};
