// Fill out your copyright notice in the Description page of Project Settings.

#include "STT_PlayGAByTag.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"


#if WITH_EDITOR
#include "StateTreeNodeDescriptionHelpers.h"
#endif

EStateTreeRunStatus FSTT_PlayGAByTag::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& Inst = Context.GetInstanceData<FInstanceDataType>(*this);

    // 런타임 상태 초기화 (StateTree가 Instance를 재사용할 수 있음)
    Inst.PendingStatus = EStateTreeRunStatus::Running;
    Inst.ActiveSpecHandle = FGameplayAbilitySpecHandle();
    Inst.AbilityEndedHandle.Reset();
    Inst.CachedASC.Reset();

    // 1)Tag 유효성 검사 
	if (!Inst.GATag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[STT_PlayGAByTag] GATag가 비어 있음."));
		return EStateTreeRunStatus::Failed;
	}


    // 2) ASC 유효성 검사
    UAbilitySystemComponent* ASC = GetASC(Inst);
    if (ASC == nullptr)
    {
    	UE_LOG(LogTemp, Warning, TEXT("[STT_PlayGAByTag] ASC 비어 있음."));
    	return EStateTreeRunStatus::Failed;
    }


    // 3) 서버에서 실행되는지 검사 
    if (ASC->GetOwnerActor() == nullptr || ASC->GetOwnerActor()->HasAuthority() == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("[STT_PlayGAByTag] 클라이언트에서 실행됨. Failed 처리."));
        return EStateTreeRunStatus::Failed;
    }
    
    // ASC 캐싱 + 종료 콜백 바인딩
    Inst.CachedASC = ASC;

    FInstanceDataType* InstPtr = &Inst;
    Inst.AbilityEndedHandle = ASC->OnAbilityEnded.AddLambda(
        [this, InstPtr](const FAbilityEndedData& EndedData)
        {
            HandleAbilityEnded(EndedData, InstPtr);
        });
    

    // -----------------------------------------------------------------------------------------
    // GameplayAbility 활성화

    // 5) Tag로 GA Spec 찾기.
    //    bOnlyAbilitiesThatSatisfyTagRequirements=false: Blocked/Required 상태와 무관하게 후보 전부 수집,
    //    실제 활성화 가능 여부는 TryActivateAbility가 판단.
    const FGameplayTagContainer TagContainer(Inst.GATag);
    TArray<FGameplayAbilitySpec*> MatchingSpecs;
    ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(TagContainer, MatchingSpecs, /*bOnlyAbilitiesThatSatisfyTagRequirements=*/ false);

    if (MatchingSpecs.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[STT_PlayGAByTag] Tag '%s'에 매칭되는 GA가 ASC에 없음."), *Inst.GATag.ToString());
        ASC->OnAbilityEnded.RemoveAll(this);
        Inst.AbilityEndedHandle.Reset();
        return EStateTreeRunStatus::Failed;
    }

    // 6) 후보 순회하며 활성화 시도. 첫 성공까지.
    //    Handle은 TryActivate 전에 세팅 — 동기 종료 시 HandleAbilityEnded의 Handle 비교가 성공해야 함.
    bool bActivated = false;
    for (FGameplayAbilitySpec* Candidate : MatchingSpecs)
    {
        if (!Candidate)
        {
            continue;
        }

        Inst.ActiveSpecHandle = Candidate->Handle;
        if (ASC->TryActivateAbility(Inst.ActiveSpecHandle))
        {
            bActivated = true;
            break;
        }
    }

    if (!bActivated)
    {
        UE_LOG(LogTemp, Warning, TEXT("[STT_PlayGAByTag] TryActivateAbility 실패: %s"), *Inst.GATag.ToString());
        Inst.ActiveSpecHandle = FGameplayAbilitySpecHandle();
        ASC->OnAbilityEnded.RemoveAll(this);
        Inst.AbilityEndedHandle.Reset();
        return EStateTreeRunStatus::Failed;
    }

    // 동기 종료 GA였다면 콜백이 이미 PendingStatus를 Succeeded/Failed로 바꿔놨을 수 있음.
    // 그 결과를 그대로 반환.
    return Inst.PendingStatus;
}

EStateTreeRunStatus FSTT_PlayGAByTag::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    const FInstanceDataType& Inst = Context.GetInstanceData<FInstanceDataType>(*this);
    return Inst.PendingStatus;
}

void FSTT_PlayGAByTag::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& Inst = Context.GetInstanceData<FInstanceDataType>(*this);

    if (UAbilitySystemComponent* ASC = Inst.CachedASC.Get())
    {
        // 1) 델리게이트 먼저 끊는다 — Cancel로 인한 재진입 차단.
        ASC->OnAbilityEnded.RemoveAll(this);
        Inst.AbilityEndedHandle.Reset();

        // 2) 옵션에 따라 GA 강제 종료 — 외부에서 State가 끊긴 경우 대비.
        if (Inst.bCancelGAOnExit && Inst.ActiveSpecHandle.IsValid())
        {
            if (FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Inst.ActiveSpecHandle))
            {
                if (Spec->IsActive())
                {
                    ASC->CancelAbilityHandle(Inst.ActiveSpecHandle);
                }
            }
        }
    }

    // 3) 런타임 상태 리셋
    Inst.CachedASC.Reset();
    Inst.ActiveSpecHandle = FGameplayAbilitySpecHandle();
    Inst.PendingStatus = EStateTreeRunStatus::Running;
}

UAbilitySystemComponent* FSTT_PlayGAByTag::GetASC(const FInstanceDataType& Inst) const
{
    if (Inst.Actor == nullptr)
        return nullptr;

    
    return Inst.Actor->FindComponentByClass<UAbilitySystemComponent>();
}

void FSTT_PlayGAByTag::HandleAbilityEnded(const FAbilityEndedData& EndedData, FInstanceDataType* InstPtr) const
{
    if (!InstPtr)
    {
        return;
    }

    // 우리가 띄운 GA가 맞는지 Handle로 식별.
    // ActiveSpecHandle이 아직 invalid한 동기 종료 케이스도 있어, 그때는 첫 콜백을 신뢰한다.
    if (InstPtr->ActiveSpecHandle.IsValid() && EndedData.AbilitySpecHandle != InstPtr->ActiveSpecHandle)
    {
        return;
    }

    InstPtr->PendingStatus = EndedData.bWasCancelled
        ? EStateTreeRunStatus::Failed
        : EStateTreeRunStatus::Succeeded;

    // 여기서 직접 Unbind/Cancel 하지 않는다.
    // Tick이 PendingStatus를 반환하면 StateTree가 ExitState를 호출하고, 거기서 일괄 정리.
}

#if WITH_EDITOR
FText FSTT_PlayGAByTag::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
    const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    const FInstanceDataType* Inst = InstanceDataView.GetPtr<FInstanceDataType>();
    if (!Inst)
    {
        return FText::FromString(TEXT("Play GA By Tag"));
    }

    const FString TagStr = Inst->GATag.IsValid() ? Inst->GATag.ToString() : TEXT("<None>");

    return (Formatting == EStateTreeNodeFormatting::RichText)
        ? FText::FromString(FString::Printf(TEXT("<s>Play GA By Tag</> <b>%s</>"), *TagStr))
        : FText::FromString(FString::Printf(TEXT("Play GA By Tag: %s"), *TagStr));
}
#endif


