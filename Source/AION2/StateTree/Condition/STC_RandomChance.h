#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "STC_RandomChance.generated.h"

USTRUCT(BlueprintType)
struct FSTC_RandomChanceInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float Chance = 0.2f;

	UPROPERTY(EditAnywhere)
	bool bInvert = false;
};

USTRUCT(DisplayName = "Random Chance", Category = "AION2")
struct AION2_API FSTC_RandomChance : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTC_RandomChanceInstanceData;

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
