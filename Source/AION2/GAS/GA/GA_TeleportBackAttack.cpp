#include "GAS/GA/GA_TeleportBackAttack.h"

void UGA_TeleportBackAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AAOCharacter* AOCharacter = Cast<ADaeva>(ActorInfo->AvatarActor.Get());
	if (!AOCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AAOCharacter* Target = AOCharacter->GetCurrentTarget();
	if (!IsValid(Target) || FVector::Distance(Target->GetActorLocation(), AOCharacter->GetActorLocation()) > AttackData.AvailableRange)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AOCharacter->TeleportBackToTarget();
}
