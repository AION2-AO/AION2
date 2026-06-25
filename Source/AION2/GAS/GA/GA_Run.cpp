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
		UE_LOG(LogTemp, Error, TEXT("[Run] AttributeSet is null"));

		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (AttributeSet->GetStamina() <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Run] Not enough stamina"));

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

			UE_LOG(LogTemp, Log, TEXT("[Run] MoveSpeed Effect Applied"));
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
			RunStaminaDrainEffectHandle =
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

			UE_LOG(LogTemp, Log, TEXT("[Run] Stamina Drain Effect Applied"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Run] RunStaminaDrainEffect is null"));
	}

	if (!StaminaChangedDelegateHandle.IsValid())
	{
		StaminaChangedDelegateHandle =
			ASC->GetGameplayAttributeValueChangeDelegate(
				UAOAttributeSet::GetStaminaAttribute()
			).AddUObject(this, &UGA_Run::OnStaminaChanged);
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Run Start] Stamina: %.1f / %.1f"),
		AttributeSet->GetStamina(),
		AttributeSet->GetMaxStamina()
	);
}

void UGA_Run::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo
)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	UE_LOG(LogTemp, Warning, TEXT("[Run End] Input Released"));

	EndAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		true,
		false
	);
}

void UGA_Run::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue > 0.0f)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Run End] Stamina Empty"));

	EndAbility(
		CurrentSpecHandle,
		CurrentActorInfo,
		CurrentActivationInfo,
		true,
		false
	);
}

void UGA_Run::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	UAbilitySystemComponent* ASC =
		ActorInfo && ActorInfo->AbilitySystemComponent.IsValid()
		? ActorInfo->AbilitySystemComponent.Get()
		: nullptr;

	if (ASC)
	{
		if (RunMoveSpeedEffectHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(RunMoveSpeedEffectHandle);
			RunMoveSpeedEffectHandle.Invalidate();

			UE_LOG(LogTemp, Log, TEXT("[Run End] MoveSpeed Effect Removed"));
		}

		if (RunStaminaDrainEffectHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(RunStaminaDrainEffectHandle);
			RunStaminaDrainEffectHandle.Invalidate();

			UE_LOG(LogTemp, Log, TEXT("[Run End] Stamina Drain Effect Removed"));
		}

		if (StaminaChangedDelegateHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(
				UAOAttributeSet::GetStaminaAttribute()
			).Remove(StaminaChangedDelegateHandle);

			StaminaChangedDelegateHandle.Reset();
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[Run End] Ability Ended"));

	Super::EndAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		bReplicateEndAbility,
		bWasCancelled
	);
}