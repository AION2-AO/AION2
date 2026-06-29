#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "AI/AIMonsterControllerBase.h"
#include "GameplayTagContainer.h"
#include "STC_PhaseEquals.generated.h"

class AActor;

USTRUCT(BlueprintType)
struct FSTC_PhaseEqualsInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TObjectPtr<AAIMonsterControllerBase> AIMonsterController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag PhaseTag;


	UPROPERTY(EditAnywhere)
	bool bInvert = false;
};

USTRUCT(DisplayName = "Phase Equals", Category = "AION2")
struct AION2_API FSTC_PhaseEquals : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTC_PhaseEqualsInstanceData;
	
	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceData,
		const IStateTreeBindingLookup& BindingLookup,
		EStateTreeNodeFormatting Formatting) const override;
#endif
};


