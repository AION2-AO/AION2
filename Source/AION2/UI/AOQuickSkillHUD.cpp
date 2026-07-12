// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/AOQuickSkillHUD.h"
#include "UI/AOSkillQuickSlotWidget.h"

#include "Data/DA_AbilitySet.h"
#include "Data/AOSkillSlotViewData.h"

#include "Character/Daeva/Daeva.h"

#include "Gas/AOGameplayTags.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"

void UAOQuickSkillHUD::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // ============= Initialize SkillSlotByAbilityID ============

    // 456
    SkillSlotByAbilityID.Add(static_cast<int32>(EAbilityID::LB_1), Skill_R);
    SkillSlotByAbilityID.Add(static_cast<int32>(EAbilityID::LB_2), Skill_R);
    SkillSlotByAbilityID.Add(static_cast<int32>(EAbilityID::LB_3), Skill_R);

    // 789
    SkillSlotByAbilityID.Add(static_cast<int32>(EAbilityID::RB_1), Skill_T);
    SkillSlotByAbilityID.Add(static_cast<int32>(EAbilityID::RB_2), Skill_T);
    SkillSlotByAbilityID.Add(static_cast<int32>(EAbilityID::RB_3), Skill_T);

    // 10 11 12 13
    SkillSlotByAbilityID.Add(static_cast<int32>(EAbilityID::Key1), Skill_1);
    SkillSlotByAbilityID.Add(static_cast<int32>(EAbilityID::Key2), Skill_2);
    SkillSlotByAbilityID.Add(static_cast<int32>(EAbilityID::Key3), Skill_3);
    SkillSlotByAbilityID.Add(static_cast<int32>(EAbilityID::Key4), Skill_4);

    // 14 15
    SkillSlotByAbilityID.Add(static_cast<int32>(EAbilityID::KeyQ), Skill_Q);
    SkillSlotByAbilityID.Add(static_cast<int32>(EAbilityID::KeyE), Skill_E);


    // ============= Initialize SkillSlotArray ============
    SkillSlotArray.Add(Skill_1);
    SkillSlotArray.Add(Skill_2);
    SkillSlotArray.Add(Skill_3);
    SkillSlotArray.Add(Skill_4);

    SkillSlotArray.Add(Skill_Q);
    SkillSlotArray.Add(Skill_E);
    SkillSlotArray.Add(Skill_R);
    SkillSlotArray.Add(Skill_T);
}

void UAOQuickSkillHUD::BindToASC(UAbilitySystemComponent* InASC)
{
    if (!InASC)
    {
        UnbindFromASC();;
        return;
    }

    // 중복 Binding 방지
    UnbindComboInputCompletedDelegate();
    UnbindComboDelegates();
    UnbindCooldownDelegates();

    Super::BindToASC(InASC);

    // Exception Handling
    if (!BoundASC)
    {        
        return;
    }
   
    ADaeva* Daeva = Cast<ADaeva>(BoundASC->GetAvatarActor());
    if (!Daeva)
    {
        Daeva = Cast<ADaeva>(GetOwningPlayerPawn());
    }

    // 한 번 더 넣어줬는데 안되면 return;
    if (!Daeva)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAOQuickSkillHUD::BindToASC Daeva is null"));
        return;
    }

    const UDA_AbilitySet* AbilitySet = Daeva->GetCombatAbilitySet();

    // Class Character Blueprint에 DA_CombatAbilitySet_{Class}을 할당하지 않은 경우.
    if (!AbilitySet)
    {
        // AbilitySet 안됐을 때 확인용
        UE_LOG(
            LogTemp, 
            Warning, 
            TEXT("UAOQuickSkillHUD::BindToASC...CharacterName: %s,  AbilitySetName: %s"), 
            *GetNameSafe(Daeva), *GetPathNameSafe(AbilitySet)
        );
        return;
    }

    // Button Init하는 부분 바꿔줌.
    InitSkillSlots(AbilitySet);

    BindComboInputCompletedDelegate(Daeva);
    BindComboDelegates();
    BindCooldownDelegates();
}

void UAOQuickSkillHUD::UnbindFromASC()
{
    // 기존 Bind 정리.
    UnbindComboInputCompletedDelegate();
    UnbindComboDelegates();
    UnbindCooldownDelegates();

    Super::UnbindFromASC();
}

void UAOQuickSkillHUD::InitSkillSlots(const UDA_AbilitySet* InAbilitySet)
{
    if (!InAbilitySet)
    {
        return;
    }

    // 혹시 모르니 한 번 Clear 해주기.
    for (UAOSkillQuickSlotWidget* SlotWidget : SkillSlotArray)
    {
        SlotWidget->ClearSkillSlotViewData();
    }


    // Ability의 ViewData 넣어주기.
    for (const TPair<int32, TObjectPtr<UAOSkillQuickSlotWidget>>& Pair : SkillSlotByAbilityID)
    {
        const int32 AbilityID = Pair.Key;
        UAOSkillQuickSlotWidget* SlotWidget = Pair.Value;

        // 화면에 존재할 거라고 생각하지만, Exception Handling.
        if (!SlotWidget)
        {
            continue;
        }

        // 해당 클래스가 이 Key에 구현한 GA_Attack 정보가 없으면 return.
        FGAData AbilityData;
        if (!InAbilitySet->GetAbilityDataByInputID(AbilityID, AbilityData))
        {
            continue;
        }

        // ViewData를 만들어서 넣어줌
        FAOSkillSlotViewData ViewData;
        ViewData.AbilityID = AbilityData.AbilityID;
        ViewData.Icon = AbilityData.Icon;
        ViewData.AbilityLevel = AbilityData.AbilityLevel;

        // Cooldown Tag는 Ability 자체의 쿨다운 Tag를 읽어와서 넣어주기 
        if (AbilityData.Ability)
        {
            const UGameplayAbility* AbilityCDO =
                AbilityData.Ability->GetDefaultObject<UGameplayAbility>();

            if (AbilityCDO)
            {
                const FGameplayTagContainer* CooldownTags =
                    AbilityCDO->GetCooldownTags();

                if (CooldownTags && !CooldownTags->IsEmpty())
                {
                    ViewData.CooldownTag = CooldownTags->First();
                }
            }
        }

        /*
        * TODO(SuYeon): 나중에.
        * SlotWidget은 이 정보를 바탕으로 다시 CoolTime 등에 대해 PlayerStatus 위에 띄운다.
        * 다른 class로 분리해서 PlayerStatus Widget에 넣은 뒤,
        * 해당 상위 Widget을 통해 소통하는 게 나을 듯.
        */
        
        SlotWidget->AddSkillSlotViewData(ViewData);
    }

    // 처음 것으로 초기화 해주기.
    for (UAOSkillQuickSlotWidget* SlotWidget : SkillSlotArray)
    {
        if (SlotWidget->GetSlotSkillCount() > 0)
        {
            SlotWidget->SetCurrentSkillIndex(0);

        }
    }
}

void UAOQuickSkillHUD::BindComboDelegates()
{
    if (!BoundASC)
    {
        return;
    }

    // 콤보 태그 변화 구독: NewOrRemoved에 구독하고 있으므로, 각각의 1번 공격은 Combo가 아님 =>  따로 구독 안해도 됨.
   // // Left Button
   // BoundASC->RegisterGameplayTagEvent(COMBO_AVAILABLE_LB2, EGameplayTagEventType::NewOrRemoved)
   //     .AddUObject(this, &UAOQuickSkillHUD::HandleLBComboTagChanged);
   //
   // BoundASC->RegisterGameplayTagEvent(COMBO_AVAILABLE_LB3, EGameplayTagEventType::NewOrRemoved)
   //     .AddUObject(this, &UAOQuickSkillHUD::HandleLBComboTagChanged);
   //
   // // Right Button
   // BoundASC->RegisterGameplayTagEvent(COMBO_AVAILABLE_RB2, EGameplayTagEventType::NewOrRemoved)
   //     .AddUObject(this, &UAOQuickSkillHUD::HandleRBComboTagChanged);
   //
   // BoundASC->RegisterGameplayTagEvent(COMBO_AVAILABLE_RB3, EGameplayTagEventType::NewOrRemoved)
   //     .AddUObject(this, &UAOQuickSkillHUD::HandleRBComboTagChanged);



    auto BindComboTag = [this](FGameplayTag Tag, void (UAOQuickSkillHUD::* Func)(FGameplayTag, int32))
        {
            FDelegateHandle Handle =
                BoundASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved)
                .AddUObject(this, Func);

            ComboTagDelegateHandles.Emplace(Tag, Handle);
        };

    BindComboTag(COMBO_AVAILABLE_LB2, &UAOQuickSkillHUD::HandleLBComboTagChanged);
    BindComboTag(COMBO_AVAILABLE_LB3, &UAOQuickSkillHUD::HandleLBComboTagChanged);
    BindComboTag(COMBO_AVAILABLE_RB2, &UAOQuickSkillHUD::HandleRBComboTagChanged);
    BindComboTag(COMBO_AVAILABLE_RB3, &UAOQuickSkillHUD::HandleRBComboTagChanged);

    
}

void UAOQuickSkillHUD::UnbindComboDelegates()
{
    if (!BoundASC)
    {
        ComboTagDelegateHandles.Empty();
        return;
    }

    for (const TPair<FGameplayTag, FDelegateHandle>& Pair : ComboTagDelegateHandles)
    {
        if (Pair.Value.IsValid())
        {
            BoundASC->RegisterGameplayTagEvent(Pair.Key, EGameplayTagEventType::NewOrRemoved)
                .Remove(Pair.Value);
        }
    }

    ComboTagDelegateHandles.Empty();
}

void UAOQuickSkillHUD::BindCooldownDelegates()
{
    if (!BoundASC)
    {
        return;
    }

    // 같은 CooldownTag가 실수로 여러 슬롯에 들어가 있으면 delegate가 중복 등록됨!
    // 슬롯마다 태그가 유니크하다는 설계여도 방어적으로 한 번만 등록.
    TSet<FGameplayTag> BoundTags;

    for (UAOSkillQuickSlotWidget* SlotWidget : SkillSlotArray)
    {
        if (!SlotWidget)
        {
            continue;
        }

        // 일단 빠른 구현을 위해 슬롯당 ViewData가 하나라고 가정.
        // => 현재 ViewData(이 시점에서 첫번째)만 읽으면 해당 슬롯의 쿨다운 태그를 알 수 있다.
        // 나중에 해당 Slot의 다른 Skill에 대해서도 구현한다고 하면, 순회하면서 처리해줘야 함.
        const FAOSkillSlotViewData* ViewData = SlotWidget->GetCurrentSkillSlotViewData();
        if (!ViewData || !ViewData->CooldownTag.IsValid())
        {
            continue;
        }

        const FGameplayTag CooldownTag = ViewData->CooldownTag;
        if (BoundTags.Contains(CooldownTag))
        {
            continue;
        }

        BoundTags.Add(CooldownTag);

        // NewOrRemoved => ASC의 CooldownTag count가 생길 때, 사라질 때 호출됨.
        // 해당 Event는 어떤 Slot에 해당 Skill이 있는지 전달해줄 수 없으므로, 
        // Handler에서 Slot을 순회해서 검사해줌.
        FDelegateHandle Handle =
            BoundASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved)
            .AddUObject(this, &UAOQuickSkillHUD::HandleCooldownTagChanged);

        CooldownTagDelegateHandles.Emplace(CooldownTag, Handle);
    }
}

void UAOQuickSkillHUD::UnbindCooldownDelegates()
{
    if (!BoundASC)
    {
        CooldownTagDelegateHandles.Empty();
        return;
    }

    for (const TPair<FGameplayTag, FDelegateHandle>& Pair : CooldownTagDelegateHandles)
    {
        const FGameplayTag CooldownTag = Pair.Key;
        const FDelegateHandle& Handle = Pair.Value;

        if (!CooldownTag.IsValid() || !Handle.IsValid())
        {
            continue;
        }

        // RegisterGameplayTagEvent는 태그별 delegate list를 반환한다.
        // 등록할 때 받은 handle로 정확히 같은 바인딩만 제거한다.
        BoundASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved)
            .Remove(Handle);
    }

    CooldownTagDelegateHandles.Empty();
}

void UAOQuickSkillHUD::HandleCooldownTagChanged(FGameplayTag CooldownTag, int32 NewCount)
{
    	UAOSkillQuickSlotWidget* SlotWidget = FindCurrentSlotByCooldownTag(CooldownTag);
	if (!SlotWidget)
	{
		return;
	}

	if (NewCount > 0)
	{
		float RemainingTime = 0.0f;
		float Duration = 0.0f;

		if (GetCooldownTime(CooldownTag, RemainingTime, Duration))
		{
			SlotWidget->StartCooldown(RemainingTime, Duration);
		}
	}
	else
	{
		SlotWidget->StopCooldown();
	}
}

UAOSkillQuickSlotWidget* UAOQuickSkillHUD::FindCurrentSlotByCooldownTag(FGameplayTag CooldownTag) const
{
    if (!CooldownTag.IsValid())
    {
        return nullptr;
    }

    for (UAOSkillQuickSlotWidget* SlotWidget : SkillSlotArray)
    {
        if (!SlotWidget)
        {
            continue;
        }

        const FAOSkillSlotViewData* ViewData = SlotWidget->GetCurrentSkillSlotViewData();
        if (ViewData && ViewData->CooldownTag == CooldownTag)
        {
            return SlotWidget;
        }
    }

    return nullptr;
}

bool UAOQuickSkillHUD::GetCooldownTime(FGameplayTag CooldownTag, float& OutRemainingTime, float& OutDuration) const
{
    OutRemainingTime = 0.0f;
    OutDuration = 0.0f;

    if (!BoundASC || !CooldownTag.IsValid())
    {
        return false;
    }

    FGameplayTagContainer CooldownTags;
    CooldownTags.AddTag(CooldownTag);

    // ASC에 적용 중인 Active GameplayEffect 중,
    // CooldownTag를 가진 효과를 찾는다.
    const FGameplayEffectQuery Query =
        FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTags);

    const TArray<TPair<float, float>> TimeRemainingAndDurations =
        BoundASC->GetActiveEffectsTimeRemainingAndDuration(Query);

    if (TimeRemainingAndDurations.IsEmpty())
    {
        return false;
    }

    // 같은 태그로 여러 GE가 잡히는 경우 가장 긴 남은 시간을 UI에 표시한다.
    for (const TPair<float, float>& TimePair : TimeRemainingAndDurations)
    {
        const float RemainingTime = TimePair.Key;
        const float Duration = TimePair.Value;

        if (RemainingTime > OutRemainingTime)
        {
            OutRemainingTime = RemainingTime;
            OutDuration = Duration;
        }
    }

    return OutRemainingTime > 0.0f && OutDuration > 0.0f;
}

void UAOQuickSkillHUD::HandleLBComboTagChanged(FGameplayTag Tag, int32 NewCount)
{
    // 콤보 가능 태그가 사라짐.
    if (NewCount <= 0)
    {
        return;
    }

    // 아직 이 함수에서는 안 쓰고 있음을 명시.
    (void)Tag;

    if (NewCount > 0)
    {
        Skill_R->HandleComboInput();  
    }
    else
    {
        Skill_R->ResetComboInput();
    }
}

void UAOQuickSkillHUD::HandleRBComboTagChanged(FGameplayTag Tag, int32 NewCount)
{
    if (NewCount <= 0)
    {
        return;
    }

    // 아직 이 함수에서는 안 쓰고 있음을 명시.
    (void)Tag;

    if (NewCount > 0)
    {
        Skill_T->HandleComboInput();
    }
    else
    {
        Skill_T->ResetComboInput();  
    }
}

void UAOQuickSkillHUD::BindComboInputCompletedDelegate(ADaeva* InDaeva)
{
    if (!IsValid(InDaeva))
    {
        return;
    }

    BoundComboInputDaeva = InDaeva;

    ComboInputCompletedDelegateHandle =
        InDaeva->OnComboInputCompleted.AddUObject(
            this,
            &UAOQuickSkillHUD::HandleComboInputCompleted
        );
}

void UAOQuickSkillHUD::UnbindComboInputCompletedDelegate()
{
    ADaeva* Daeva = BoundComboInputDaeva.Get();

    if (Daeva && ComboInputCompletedDelegateHandle.IsValid())
    {
        Daeva->OnComboInputCompleted.Remove(
            ComboInputCompletedDelegateHandle
        );
    }

    ComboInputCompletedDelegateHandle.Reset();
    BoundComboInputDaeva.Reset();
}

void UAOQuickSkillHUD::HandleComboInputCompleted(int32 SlotBaseInputId)
{
    const TObjectPtr<UAOSkillQuickSlotWidget>* SlotPtr =
        SkillSlotByAbilityID.Find(SlotBaseInputId);

    if (!SlotPtr)
    {
        return;
    }

    UAOSkillQuickSlotWidget* SlotWidget = SlotPtr->Get();
    if (!IsValid(SlotWidget))
    {
        return;
    }

    SlotWidget->ResetComboInput();
}

void UAOQuickSkillHUD::PlaySkillPressedFeedback(int32 InputId)
{
    if (!SkillSlotByAbilityID.Contains(InputId))
    {
        // Invalid 입력이 들어옴: return.
        // Dash, Jump 등의 Non-Skill 입력이 있음!
        return;
    }

    // GAS Input으로 그냥 받았는데, AbilityID랑 다를 수도 있음. 
    // 지금 Slot은 AbilityID로 Mapping되어있으므로, 확인 차 Log 찍어보기.
    // => 같은 ID로 들어가고 있었으나, 한 번 눌러도 중복 입력이 좀 많이 들어가는 상태.
    UE_LOG(LogTemp, Warning, TEXT("Input ID: %d"), InputId);

    // 해당 Id의 Slot effect 재생하도록 하기.
    SkillSlotByAbilityID[InputId]->PlaySkillPressedFeedback();
}
