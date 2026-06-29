#include "STC_PhaseEquals.h"
#include "StateTreeExecutionContext.h"

bool FSTC_PhaseEquals::TestCondition(FStateTreeExecutionContext& Context) const
{

    // InstanceData 꺼내기
    const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    // 1) AIController 유효성
    if (InstanceData.AIMonsterController == nullptr)
    {
        return false;
    }

    // 2) 비교 기준 태그 유효성
    if (!InstanceData.PhaseTag.IsValid())
    {
        return false;
    }


    // 3) 현재 Phase 가져와서 비교
    const FGameplayTag CurrentPhase = InstanceData.AIMonsterController->Get_Phase(); 
    const bool bMatches = CurrentPhase.MatchesTagExact(InstanceData.PhaseTag);


    return bMatches; 
  
}

#if WITH_EDITOR
FText FSTC_PhaseEquals::GetDescription(const FGuid& ID, FStateTreeDataView InstanceData, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    const FInstanceDataType* Data = InstanceData.GetPtr<FInstanceDataType>();
    if (Data == nullptr)
    {
        return FText::FromString(TEXT("Phase Equals"));
    }

    const FString InvertPrefix = Data->bInvert ? TEXT("NOT ") : TEXT("");
    const FString TagString = Data->PhaseTag.IsValid()
        ? Data->PhaseTag.ToString()
        : TEXT("<None>");

    return FText::FromString(FString::Printf(TEXT("%sPhase Equals: %s"),
        *InvertPrefix, *TagString));
}
#endif