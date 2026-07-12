#include "GAS/GA/Cleric/GA_Cleric_KeyE.h"
#include "Physics/Collision.h"

void UGA_Cleric_KeyE::OnCheckAttackHitEvent(FGameplayEventData Payload)
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

		if (UAbilitySystemComponent* TargetASC = HitCharacter->GetAbilitySystemComponent())
		{
			float CurrentHealth = TargetASC->GetNumericAttribute(UAOAttributeSet::GetHealthAttribute());
			float MaxHealth = TargetASC->GetNumericAttribute(UAOAttributeSet::GetMaxHealthAttribute());
			float NewHealth = FMath::Min(CurrentHealth + HealValue, MaxHealth);

			TargetASC->SetNumericAttributeBase(UAOAttributeSet::GetHealthAttribute(), NewHealth);

			if (AttackData.HitGameplayCueTag.IsValid())
			{
				FGameplayCueParameters CueParams;
				CueParams.Instigator = HitCharacter;
				CueParams.EffectCauser = HitCharacter;
				TargetASC->ExecuteGameplayCue(AttackData.HitGameplayCueTag, CueParams);
			}
		}
	}
}
