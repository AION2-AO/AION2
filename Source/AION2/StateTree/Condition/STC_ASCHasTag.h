#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "GameplayTagContainer.h"
#include "STC_ASCHasTag.generated.h"

class AActor;

USTRUCT(BlueprintType)
struct FStateTreeASCHasTagConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TObjectPtr<AActor> Actor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Tag;


	UPROPERTY(EditAnywhere)
	bool bInvert = false;
};

USTRUCT(DisplayName = "ASC Has Gameplay Tag", Category = "AION2")
struct AION2_API FSTC_ASCHasTag : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeASCHasTagConditionInstanceData;
	
	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};


