#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeTaskBase.h"
#include "STT_SelectGameplayTagByWeight.generated.h"

class AActor;
class UAbilitySystemComponent;

USTRUCT(BlueprintType)
struct AION2_API FSTT_WeightedGameplayTagEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter")
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameter", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float Weight = 1.0f;
};

USTRUCT()
struct AION2_API FSTT_SelectGameplayTagByWeightInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	TArray<FSTT_WeightedGameplayTagEntry> WeightedTags;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bClearWeightedTagsBeforeSelect = true;
};

USTRUCT(meta = (DisplayName = "Select Gameplay Tag By Weight", Category = "AION2|Ability"))
struct AION2_API FSTT_SelectGameplayTagByWeight : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_SelectGameplayTagByWeightInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(
		const FGuid& ID,
		FStateTreeDataView InstanceDataView,
		const IStateTreeBindingLookup& BindingLookup,
		EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif

private:
	UAbilitySystemComponent* GetASC(const FInstanceDataType& Inst) const;
};
