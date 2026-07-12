#include "STC_RandomChance.h"

#include "StateTreeExecutionContext.h"

bool FSTC_RandomChance::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const float ClampedChance = FMath::Clamp(InstanceData.Chance, 0.0f, 1.0f);
	const bool bPassed = FMath::FRand() <= ClampedChance;

	return InstanceData.bInvert ? !bPassed : bPassed;
}

#if WITH_EDITOR
FText FSTC_RandomChance::GetDescription(const FGuid& ID, FStateTreeDataView InstanceData,
	const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	const FInstanceDataType* Data = InstanceData.GetPtr<FInstanceDataType>();
	if (Data == nullptr)
	{
		return FText::FromString(TEXT("Random Chance"));
	}

	const FString InvertPrefix = Data->bInvert ? TEXT("NOT ") : TEXT("");
	const float Percent = FMath::Clamp(Data->Chance, 0.0f, 1.0f) * 100.0f;

	return FText::FromString(FString::Printf(TEXT("%sRandom Chance: %.0f%%"),
		*InvertPrefix, Percent));
}
#endif
