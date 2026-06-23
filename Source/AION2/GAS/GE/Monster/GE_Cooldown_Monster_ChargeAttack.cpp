// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GE/Monster/GE_Cooldown_Monster_ChargeAttack.h"
#include "GAS/AOGameplayTags.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"



UGE_Cooldown_Monster_ChargeAttack::UGE_Cooldown_Monster_ChargeAttack()
{
	// 1. 이 GE는 일정 시간 유지되는 효과 
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	// 2. 쿨타임 시간 15초 

	DurationMagnitude = FScalableFloat(15.0f);

	// 3. TargetTags GameplayEffectComponent 생성
	UTargetTagsGameplayEffectComponent* TargetTagsComponent =
		CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(
			TEXT("TargetTagsComponent")
		);

	// 4. GE의 컴포넌트 배열에 등록
	GEComponents.Add(TargetTagsComponent);

	// 5. 이 GE가 적용되는 동안 Target ASC에 부여할 태그 설정
	FInheritedTagContainer GrantedTags;
	GrantedTags.Added.AddTag(COOLDOWN_MONSTER_CHARGEATTACK);
	GrantedTags.CombinedTags.AddTag(COOLDOWN_MONSTER_CHARGEATTACK);

	TargetTagsComponent->SetAndApplyTargetTagChanges(GrantedTags);

}
