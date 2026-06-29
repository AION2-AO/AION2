// STT_SetPhase.cpp
#include "STT_SetPhase.h"

EStateTreeRunStatus FSTT_SetPhase::EnterState(FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{
    const FInstanceDataType& Instance = Context.GetInstanceData(*this);

    if (!IsValid(Instance.AIMonsterController))
    {
        return EStateTreeRunStatus::Failed;
    }

    Instance.AIMonsterController->Set_Phase(Instance.PhaseTag);

    return EStateTreeRunStatus::Succeeded;
}

#if WITH_EDITOR
FText FSTT_SetPhase::GetDescription(const FGuid& ID,
    FStateTreeDataView InstanceDataView,
    const IStateTreeBindingLookup& BindingLookup,
    EStateTreeNodeFormatting Formatting) const
{
    const FInstanceDataType& Instance = InstanceDataView.Get<FInstanceDataType>();

    const FText TagText = Instance.PhaseTag.IsValid()
        ? FText::FromName(Instance.PhaseTag.GetTagName())
        : FText::FromString(TEXT("<None>"));

    // Rich 포매팅(노드에 굵게/색상 표시), Text 포매팅(툴팁/검색용 평문)
    return Formatting == EStateTreeNodeFormatting::RichText
        ? FText::Format(NSLOCTEXT("AION2", "SetPhaseRich", "<b>Set Phase</> <b>{0}</>"), TagText)
        : FText::Format(NSLOCTEXT("AION2", "SetPhasePlain", "Set Phase {0}"), TagText);
}
#endif