#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "STT_PlayGAByTag.generated.h"

class UAbilitySystemComponent;
class AAIController;

/**
 * Instance Data: Task 1회 실행분의 모든 런타임 상태.
 * Task struct는 stateless이며, 모든 state는 여기로 들어간다.
 */
USTRUCT()
struct AION2_API FSTT_PlayGAByTagInstanceData
{
    GENERATED_BODY()

    // === Context (StateTree가 자동 주입) ===
    UPROPERTY(EditAnywhere, Category = "Context")
    TObjectPtr<AAIController> AIController = nullptr;

    UPROPERTY(EditAnywhere, Category = "Context")
    TObjectPtr<AActor> Actor = nullptr;

    // === Parameters (디자이너 노출) ===
    /** 활성화할 GA를 식별할 GameplayTag. GA의 AbilityTags와 매칭. */
    UPROPERTY(EditAnywhere, Category = "Parameter", meta = (Categories = "Ability"))
    FGameplayTag GATag;

    /** ExitState 시 GA가 아직 실행 중이면 강제 Cancel. */
    UPROPERTY(EditAnywhere, Category = "Parameter")
    bool bCancelGAOnExit = true;

    // === Runtime State (Transient) ===
    UPROPERTY(Transient)
    TWeakObjectPtr<UAbilitySystemComponent> CachedASC;

    /** 활성화된 GA의 Spec Handle. Cancel/식별에 사용. */
    FGameplayAbilitySpecHandle ActiveSpecHandle;

    /** OnAbilityEnded 델리게이트 핸들. ExitState에서 RemoveAll로 정리되지만 보조용. */
    FDelegateHandle AbilityEndedHandle;

    /** EnterState에서 결정된 초기 RunStatus. Tick에서 콜백 결과를 반환하기 위해 보관. */
    EStateTreeRunStatus PendingStatus = EStateTreeRunStatus::Running;
};

/**
 * Tag로 GA를 활성화하고, 해당 GA가 종료될 때 Succeeded/Failed로 마무리하는 StateTree Task.
 *
 * 흐름:
 *   EnterState  → ASC 캐싱 → OnAbilityEnded 바인딩 → TryActivateAbilityByTag → Running 반환
 *   GA 종료 콜백 → PendingStatus 세팅
 *   Tick        → PendingStatus 반환 (Running이면 계속 대기)
 *   ExitState   → RemoveAll(this) → (옵션) CancelAbilityHandle
 */

USTRUCT(meta = (DisplayName = "Play GA By Tag", Category = "AION2|Ability"))
struct AION2_API FSTT_PlayGAByTag : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

public:
    using FInstanceDataType = FSTT_PlayGAByTagInstanceData;

    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
    virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;


#if WITH_EDITOR
    virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
        const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const override;
#endif


private:
    UAbilitySystemComponent* GetASC(const FInstanceDataType& Inst) const;
    
    void HandleAbilityEnded(const FAbilityEndedData& EndedData, FInstanceDataType* InstPtr) const;




};