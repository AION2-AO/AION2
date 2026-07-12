// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Monster_TH_OrbGimmickAttack.h"
#include "Character/Monster/Boss/Thalythra/Talythra.h"
#include "Character/Daeva/Daeva.h"
#include "Components/CapsuleComponent.h"
#include "Physics/Collision.h"
#include "Character/Monster/Boss/Thalythra/Shield/TalythraGimmickShield.h"
#include "GAS/AOGameplayTags.h"

void UGA_Monster_TH_OrbGimmickAttack::OnCheckAttackHitEvent(FGameplayEventData Payload)
{

	// 그러면 여기서 쉴드에서 해당 플레이어가 해당 쉴드 안에 있는지 체크하면 되겟네. 

	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	ATalythra* pTalythra = Cast<ATalythra>(GetAvatarActorFromActorInfo());

	if (!pTalythra)
	{
		return;
	}


	TArray<ATalythraGimmickShield*> ArrayGimmickShields = pTalythra->Get_ArrayOrbShield();

	for (auto& iter : ArrayGimmickShields)
	{
		iter->SetPlayerShieldColor();
	}

	uint8 SafeOrbColor = static_cast<uint8>(pTalythra->Get_AttackOrbColor());

	pTalythra->CheckIsInSafeZone(AttackData, SafeOrbColor);
}

void UGA_Monster_TH_OrbGimmickAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		if (PendingTag.IsValid())
		{
			UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

			if (ASC)
			{
				ASC->RemoveLooseGameplayTag(PendingTag);
				ASC->RemoveLooseGameplayTag(GIMMICK_MONSTER);
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

}


