// Fill out your copyright notice in the Description page of Project Settings.


#include "GE_Cooldown_Monster.h"
#include "GAS/AOGameplayTags.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"


UGE_Cooldown_Monster::UGE_Cooldown_Monster()
{
    // 1. Duration 타입으로 지정
    DurationPolicy = EGameplayEffectDurationType::HasDuration;

    // 2. Duration을 SetByCaller로 받도록 설정
    FGameplayEffectModifierMagnitude DurationMag;
    FSetByCallerFloat SetByCaller;
    SetByCaller.DataTag = FGameplayTag::RequestGameplayTag(TEXT("Data.Cooldown.Duration"));
    DurationMag = FGameplayEffectModifierMagnitude(SetByCaller);
    DurationMagnitude = DurationMag;

}
