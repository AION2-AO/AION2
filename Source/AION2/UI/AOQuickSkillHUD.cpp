// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AOQuickSkillHUD.h"
#include "UI/AOSkillQuickSlotWidget.h"

#include "Data/DA_AbilitySet.h"

#include "Character/Daeva/Daeva.h"


void UAOQuickSkillHUD::BindToASC(UAbilitySystemComponent* InASC)
{
    Super::BindToASC(InASC);

    // Exception Handling
    if (!BoundASC)
    {        
        return;
    }
   
    const ADaeva* Daeva = Cast<ADaeva>(BoundASC->GetAvatarActor());
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

    // AbilitySet 안됐을 때 확인용
    //UE_LOG(
    //    LogTemp, 
    //    Warning, 
    //    TEXT("UAOQuickSkillHUD::BindToASC... AbilitySetName: %s"), 
    //    *GetPathNameSafe(AbilitySet)
    //);

    for (const TPair<int32, TObjectPtr<UAOSkillQuickSlotWidget>>& Pair : SkillSlotByAbilityID)
    {
        const int32 AbilityID = Pair.Key;
        UAOSkillQuickSlotWidget* SlotWidget = Pair.Value;

        if (!SlotWidget)
        {
            continue;
        }

        FGAData AbilityData;
        if (!AbilitySet->GetAbilityDataByInputID(AbilityID, AbilityData))
        {
            continue;
        }

        SlotWidget->InitSkillSlot(AbilityData);
    }
}

void UAOQuickSkillHUD::NativeConstruct()
{
	Super::NativeConstruct();

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


}
