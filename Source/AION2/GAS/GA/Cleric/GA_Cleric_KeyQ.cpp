#include "GAS/GA/Cleric/GA_Cleric_KeyQ.h"
#include "Physics/Collision.h"

void UGA_Cleric_KeyQ::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo * ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData * TriggerEventData)
{
	AAOCharacter* AOCharacter = Cast<AAOCharacter>(ActorInfo->AvatarActor.Get());
	if (!AOCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const int32 Count = ActorInfo->AbilitySystemComponent->GetTagCount(FGameplayTag::RequestGameplayTag(TEXT("Cooldown.Cleric.KeyQ")));
	if (Count >= 3)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_Cleric_KeyQ::OnCheckAttackHitEvent(FGameplayEventData Payload)
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC)
	{
		return;
	}

	AAOCharacter* AOCharacter = Cast<AAOCharacter>(GetAvatarActorFromActorInfo());
	if (!AOCharacter)
	{
		return;
	}

	const float HealValue = SourceASC->GetNumericAttribute(UAOAttributeSet::GetAttackPowerAttribute()) * AttackData.DamageMultiplier;

	{
		float CurrentHealth = SourceASC->GetNumericAttribute(UAOAttributeSet::GetHealthAttribute());
		float MaxHealth = SourceASC->GetNumericAttribute(UAOAttributeSet::GetMaxHealthAttribute());
		float NewHealth = FMath::Min(CurrentHealth + HealValue, MaxHealth);
		SourceASC->SetNumericAttributeBase(UAOAttributeSet::GetHealthAttribute(), NewHealth);

		if (AttackData.HitGameplayCueTag.IsValid())
		{
			FGameplayCueParameters CueParams;
			CueParams.Instigator = AOCharacter;
			CueParams.EffectCauser = AOCharacter;
			SourceASC->ExecuteGameplayCue(AttackData.HitGameplayCueTag, CueParams);
		}
	}

	TArray<FHitResult> OutHitResults;

	const float AttackRange = AttackData.TraceData.Range;
	const float AttackRadius = AttackData.TraceData.Radius;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AttackTrace), false, AOCharacter);

	FVector SweepStart = AOCharacter->GetActorTransform().TransformPosition(AttackData.TraceData.StartOffset);
	FVector SweepEnd = SweepStart + AttackData.TraceData.Direction.GetSafeNormal() * AttackRange;
	FVector CapsuleCenter = SweepStart + (SweepEnd - SweepStart) * 0.5f;

	bool bHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, SweepStart, SweepEnd, FQuat::Identity, CCHANNEL_ATTACK, FCollisionShape::MakeSphere(AttackRadius), Params);

	if (!bHitDetected)
	{
		return;
	}

	AAOCharacter* LowestHealthCharacter = nullptr;
	float LowestHealthPercent = 1.1f;
	for (const FHitResult& HitResult : OutHitResults)
	{
		AAOCharacter* HitCharacter = Cast<AAOCharacter>(HitResult.GetActor());
		if (!IsValid(HitCharacter))
		{
			continue;
		}

		if (HitCharacter->IsDead())
		{
			continue;
		}

		if (AOCharacter->IsEnemy(HitCharacter))
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = HitCharacter->GetAbilitySystemComponent();
		if (!TargetASC)
		{
			continue;
		}

		const float CurrentHealth =
			TargetASC->GetNumericAttribute(UAOAttributeSet::GetHealthAttribute());

		const float MaxHealth =
			TargetASC->GetNumericAttribute(UAOAttributeSet::GetMaxHealthAttribute());

		if (MaxHealth <= 0.f)
		{
			continue;
		}

		const float HealthPercent = CurrentHealth / MaxHealth;

		if (HealthPercent < LowestHealthPercent)
		{
			LowestHealthPercent = HealthPercent;
			LowestHealthCharacter = HitCharacter;
		}
	}

	if (IsValid(LowestHealthCharacter))
	{
		if (UAbilitySystemComponent* TargetASC = LowestHealthCharacter->GetAbilitySystemComponent())
		{
			const float CurrentHealth =
				TargetASC->GetNumericAttribute(UAOAttributeSet::GetHealthAttribute());
			const float MaxHealth =
				TargetASC->GetNumericAttribute(UAOAttributeSet::GetMaxHealthAttribute());
			const float NewHealth = FMath::Min(CurrentHealth + HealValue, MaxHealth);

			TargetASC->SetNumericAttributeBase(UAOAttributeSet::GetHealthAttribute(), NewHealth);

			if (AttackData.HitGameplayCueTag.IsValid())
			{
				FGameplayCueParameters CueParams;
				CueParams.Instigator = LowestHealthCharacter;
				CueParams.EffectCauser = LowestHealthCharacter;
				TargetASC->ExecuteGameplayCue(AttackData.HitGameplayCueTag, CueParams);
			}
		}
	}
}
