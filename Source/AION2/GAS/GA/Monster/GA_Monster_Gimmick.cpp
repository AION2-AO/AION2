// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/Monster/GA_Monster_Gimmick.h"
#include "AbilitySystemComponent.h"
#include "GAS/AOGameplayTags.h"


void UGA_Monster_Gimmick::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{


	if(HasAuthority(&CurrentActivationInfo))
	{
		if(PendingTag.IsValid())
		{
			UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get(); 
			
			if(ASC)
			{
				ASC->RemoveLooseGameplayTag(PendingTag);
				ASC->RemoveLooseGameplayTag(GIMMICK_MONSTER);
				ASC->AddLooseGameplayTag(DoneTag);
			}

		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);



}
