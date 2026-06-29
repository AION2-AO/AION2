// STT_SetPhase.cpp
#include "STT_SetState.h"

EStateTreeRunStatus FSTT_SetState::EnterState(FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{
    const FInstanceDataType& Instance = Context.GetInstanceData(*this);

    if (!IsValid(Instance.AIMonsterController))
    {
        return EStateTreeRunStatus::Failed;
    }

    Instance.AIMonsterController->Set_State(Instance.StateTag);

    return EStateTreeRunStatus::Succeeded;
}

#if WITH_EDITOR
FText FSTT_SetState::GetDescription(const FGuid& ID,
    FStateTreeDataView InstanceDataView,
    const IStateTreeBindingLookup& BindingLookup,
    EStateTreeNodeFormatting Formatting) const
{
    const FInstanceDataType& Instance = InstanceDataView.Get<FInstanceDataType>();

    const FText TagText = Instance.StateTag.IsValid()
        ? FText::FromName(Instance.StateTag.GetTagName())
        : FText::FromString(TEXT("<None>"));

    // Rich 포매팅(노드에 굵게/색상 표시), Text 포매팅(툴팁/검색용 평문)
    return Formatting == EStateTreeNodeFormatting::RichText
        ? FText::Format(NSLOCTEXT("AION2", "SetPhaseRich", "<b>Set State</> <b>{0}</>"), TagText)
        : FText::Format(NSLOCTEXT("AION2", "SetPhasePlain", "Set State {0}"), TagText);
}
#endif