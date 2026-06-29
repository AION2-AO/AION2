#include "GAS/GA/GA_RangeCheckAttack.h"

void UGA_RangeCheckAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ADaeva* Daeva = Cast<ADaeva>(ActorInfo->AvatarActor.Get());
	if (!Daeva)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AAOCharacter* Target = Daeva->GetCurrentTarget();
	if (!IsValid(Target) || FVector::Distance(Target->GetActorLocation(), Daeva->GetActorLocation()) > AttackData.AvailableRange)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}
