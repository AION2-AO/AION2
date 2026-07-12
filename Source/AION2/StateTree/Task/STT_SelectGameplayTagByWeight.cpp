#include "STT_SelectGameplayTagByWeight.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FSTT_SelectGameplayTagByWeight::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& Inst = Context.GetInstanceData<FInstanceDataType>(*this);

	UAbilitySystemComponent* ASC = GetASC(Inst);
	if (ASC == nullptr)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (Inst.WeightedTags.IsEmpty())
	{
		return EStateTreeRunStatus::Failed;
	}

	if (Inst.bClearWeightedTagsBeforeSelect)
	{
		for (const FSTT_WeightedGameplayTagEntry& Entry : Inst.WeightedTags)
		{
			if (Entry.Tag.IsValid())
			{
				ASC->SetLooseGameplayTagCount(Entry.Tag, 0);
			}
		}
	}

	float TotalWeight = 0.0f;
	for (const FSTT_WeightedGameplayTagEntry& Entry : Inst.WeightedTags)
	{
		if (Entry.Tag.IsValid() && Entry.Weight > 0.0f)
		{
			TotalWeight += Entry.Weight;
		}
	}

	if (TotalWeight <= 0.0f)
	{
		return EStateTreeRunStatus::Failed;
	}

	const float Pick = FMath::FRandRange(0.0f, TotalWeight);
	float AccumulatedWeight = 0.0f;
	FGameplayTag SelectedTag;

	for (const FSTT_WeightedGameplayTagEntry& Entry : Inst.WeightedTags)
	{
		if (!Entry.Tag.IsValid() || Entry.Weight <= 0.0f)
		{
			continue;
		}

		AccumulatedWeight += Entry.Weight;
		if (Pick <= AccumulatedWeight)
		{
			SelectedTag = Entry.Tag;
			break;
		}
	}

	if (!SelectedTag.IsValid())
	{
		for (int32 Index = Inst.WeightedTags.Num() - 1; Index >= 0; --Index)
		{
			const FSTT_WeightedGameplayTagEntry& Entry = Inst.WeightedTags[Index];
			if (Entry.Tag.IsValid() && Entry.Weight > 0.0f)
			{
				SelectedTag = Entry.Tag;
				break;
			}
		}
	}

	if (!SelectedTag.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	ASC->SetLooseGameplayTagCount(SelectedTag, 1);

	return EStateTreeRunStatus::Succeeded;
}

UAbilitySystemComponent* FSTT_SelectGameplayTagByWeight::GetASC(const FInstanceDataType& Inst) const
{
	AActor* Actor = Inst.Actor;
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
	if (ASC != nullptr)
	{
		return ASC;
	}

	if (AController* Controller = Cast<AController>(Actor))
	{
		if (APawn* Pawn = Controller->GetPawn())
		{
			ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
		}
	}

	return ASC;
}

#if WITH_EDITOR
FText FSTT_SelectGameplayTagByWeight::GetDescription(
	const FGuid& ID,
	FStateTreeDataView InstanceDataView,
	const IStateTreeBindingLookup& BindingLookup,
	EStateTreeNodeFormatting Formatting) const
{
	const FInstanceDataType* Inst = InstanceDataView.GetPtr<FInstanceDataType>();
	if (Inst == nullptr)
	{
		return FText::FromString(TEXT("Select Gameplay Tag By Weight"));
	}

	float TotalWeight = 0.0f;
	int32 ValidTagCount = 0;
	for (const FSTT_WeightedGameplayTagEntry& Entry : Inst->WeightedTags)
	{
		if (Entry.Tag.IsValid() && Entry.Weight > 0.0f)
		{
			TotalWeight += Entry.Weight;
			++ValidTagCount;
		}
	}

	return FText::FromString(FString::Printf(
		TEXT("Select Gameplay Tag By Weight: %d tags, %.1f total weight"),
		ValidTagCount,
		TotalWeight
	));
}
#endif
