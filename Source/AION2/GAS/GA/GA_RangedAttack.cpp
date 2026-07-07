#include "GAS/GA/GA_RangedAttack.h"
#include "GAS/GA/AT/AT_RotateToTarget.h"
#include "GAS/AOGameplayTags.h"
#include "Character/Daeva/Daeva.h"
#include "GAS/AttributeSet/AOAttributeSet.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "GameplayEffect.h"

void UGA_RangedAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

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

	UAnimMontage* AttackMontage =
		Daeva->GetMontageByID(MontageIDToPlay);

	if (!AttackMontage)
	{
		UE_LOG(LogTemp,Error,TEXT("[Attack] Montage is null. MontageID=%d"),static_cast<int32>(MontageIDToPlay));

		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Mana code.
	if (ManaCost > 0.f)
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

		if (!ASC)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		const float CurrentMana = ASC->GetNumericAttribute(UAOAttributeSet::GetManaAttribute());

		if (CurrentMana < ManaCost)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		if (!ManaCostEffect)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		FGameplayEffectSpecHandle ManaCostSpec = MakeOutgoingGameplayEffectSpec(ManaCostEffect, 1.f);

		if (!ManaCostSpec.IsValid())
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		const FGameplayTag SkillManaCost = FGameplayTag::RequestGameplayTag(FName("Data.ManaCost"));

		ManaCostSpec.Data->SetSetByCallerMagnitude(SkillManaCost, -ManaCost);

		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, ManaCostSpec);
	}

	const float SafePlayRate = FMath::Max(MontagePlayRate, 0.01f);

	const float AttackDuration = AttackMontage->GetPlayLength() / SafePlayRate;

	const FGameplayTag AttackSlowDurationTag = FGameplayTag::RequestGameplayTag(FName("Data.AttackSlowDuration"));

	const FGameplayTag AttackingTag = FGameplayTag::RequestGameplayTag(FName("State.Attacking"));

	// GameplayEffectsToApply 안에 있는 모든 GE 적용
	for (const TSubclassOf<UGameplayEffect> GameplayEffect : GameplayEffectsToApply)
	{
		if (!GameplayEffect)
		{
			continue;
		}

		FGameplayEffectSpecHandle SpecHandle =
			MakeOutgoingGameplayEffectSpec(GameplayEffect, 1.0f);

		if (!SpecHandle.IsValid())
		{
			continue;
		}

		/*
			GE_AttackSlow은 Granted Tag에 State.Attacking이 들어있다.
			그 GE만 Duration을 몽타주 길이로 설정한다.
		*/
		const UGameplayEffect* EffectCDO = GameplayEffect->GetDefaultObject<UGameplayEffect>();

		if (EffectCDO &&
			EffectCDO->GetGrantedTags().HasTagExact(AttackingTag))
		{
			SpecHandle.Data->SetSetByCallerMagnitude(AttackSlowDurationTag,	AttackDuration);
		}

		ApplyGameplayEffectSpecToOwner(Handle,ActorInfo,ActivationInfo,	SpecHandle);
	}

	// 공격 시작 시 Sprint GE 제거
	if (!RemoveTagsOnActivate.IsEmpty())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->RemoveActiveEffectsWithGrantedTags(RemoveTagsOnActivate);
		}
	}

	// 공격 몽타주 재생
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,AttackMontage,SafePlayRate,StartSectionName);

	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this,&UGA_RangedAttack::OnMontageTaskFinished);

	MontageTask->OnBlendOut.AddDynamic(this,&UGA_RangedAttack::OnMontageTaskFinished);

	MontageTask->OnInterrupted.AddDynamic(this,&UGA_RangedAttack::OnMontageTaskCancelled);

	MontageTask->OnCancelled.AddDynamic(this,&UGA_RangedAttack::OnMontageTaskCancelled);

	MontageTask->ReadyForActivation();

	// 투사체 스폰 Notify 이벤트 대기
	UAbilityTask_WaitGameplayEvent* WaitHitCheckTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			EVENT_CHECKATTACKHIT
		);

	if (WaitHitCheckTask)
	{
		WaitHitCheckTask->EventReceived.AddDynamic(this,&UGA_RangedAttack::OnLaunchProjectileEvent);

		WaitHitCheckTask->ReadyForActivation();
	}

	FVector Direction = Target->GetActorLocation() - Daeva->GetActorLocation();
	Direction.Z = 0.f;
	Daeva->SetActorRotation(Direction.Rotation());

	UAT_RotateToTarget* RotateTask = UAT_RotateToTarget::RotateToTarget(this, AttackData.AvailableRange, 15.0f);
	RotateTask->ReadyForActivation();
}

void UGA_RangedAttack::OnMontageTaskFinished()
{
	EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,false);
}

void UGA_RangedAttack::OnMontageTaskCancelled()
{
	EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,true);
}

void UGA_RangedAttack::OnLaunchProjectileEvent(FGameplayEventData Payload)
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	AAOCharacter* AOCharacter = Cast<AAOCharacter>(GetAvatarActorFromActorInfo());
	if (!AOCharacter)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Summon Shuriken"));
	AOCharacter->SpawnAttackProjectile(AttackData, ProjectileClass, ProjectileSpawnSocket);
}
