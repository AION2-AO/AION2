#include "GAS/GA/GA_Run.h"

#include "AbilitySystemComponent.h"
#include "GAS/AttributeSet/AOAttributeSet.h"

void UGA_Run::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

	const UAOAttributeSet* AttributeSet = ASC->GetSet<UAOAttributeSet>();
	if (!AttributeSet)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (AttributeSet->GetStamina() <= 0.0f)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (RunMoveSpeedEffect)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle =
			ASC->MakeOutgoingSpec(RunMoveSpeedEffect, 1.0f, Context);

		if (SpecHandle.IsValid())
		{
			RunMoveSpeedEffectHandle =
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Run] RunMoveSpeedEffect is null"));
	}

	if (RunStaminaDrainEffect)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle =
			ASC->MakeOutgoingSpec(RunStaminaDrainEffect, 1.0f, Context);

		if (SpecHandle.IsValid())
		{
			RunStaminaDrainEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Run] RunStaminaDrainEffect is null"));
	}

	if (!StaminaChangedDelegateHandle.IsValid())
	{
		StaminaChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(UAOAttributeSet::GetStaminaAttribute()).AddUObject(this, &UGA_Run::OnStaminaChanged);
	}
}

void UGA_Run::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	EndAbility(	Handle,	ActorInfo,	ActivationInfo,	true,false);
}

void UGA_Run::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue > 0.0f)
	{
		return;
	}


	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Run::EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,const FGameplayAbilityActivationInfo ActivationInfo,bool bReplicateEndAbility,bool bWasCancelled)
{
	UAbilitySystemComponent* ASC =	ActorInfo && ActorInfo->AbilitySystemComponent.IsValid() ? ActorInfo->AbilitySystemComponent.Get()	: nullptr;

	if (ASC)
	{
		if (RunMoveSpeedEffectHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(RunMoveSpeedEffectHandle);
			RunMoveSpeedEffectHandle.Invalidate();
		}

		if (RunStaminaDrainEffectHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(RunStaminaDrainEffectHandle);
			RunStaminaDrainEffectHandle.Invalidate();
		}

		if (StaminaChangedDelegateHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UAOAttributeSet::GetStaminaAttribute()).Remove(StaminaChangedDelegateHandle);

			StaminaChangedDelegateHandle.Reset();
		}
	}
	
	Super::EndAbility(Handle,ActorInfo,ActivationInfo,bReplicateEndAbility,bWasCancelled);
}