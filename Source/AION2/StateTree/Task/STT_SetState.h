// STT_SetPhase.h
#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeNodeBase.h"
#include "GameplayTagContainer.h"
#include "AI/AIMonsterControllerBase.h"
#include "STT_SetState.generated.h"

USTRUCT()
struct AION2_API FSTT_SetStateInstanceData
{
    GENERATED_BODY()

    /** Context로 자동 주입되는 몬스터 컨트롤러 */
    UPROPERTY(EditAnywhere, Category = "Context")
    TObjectPtr<AAIMonsterControllerBase> AIMonsterController = nullptr;

    /** 진입 시 설정할 Phase Tag */
    UPROPERTY(EditAnywhere, Category = "Parameter", meta = (Categories = "State.Monster"))
    FGameplayTag StateTag;
};

USTRUCT(meta = (DisplayName = "Set State", Category = "AION2|State"))
struct AION2_API FSTT_SetState : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FSTT_SetStateInstanceData;
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
    virtual FText GetDescription(const FGuid& ID,
        FStateTreeDataView InstanceDataView,
        const IStateTreeBindingLookup& BindingLookup,
        EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;

#endif
};