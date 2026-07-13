#include "GAS/GA/GA_Dash.h"

#include "Character/Daeva/Daeva.h"
#include "Character/AOCharacterMovementComponent.h"
#include "GAS/AOGameplayTags.h"
#include "GAS/AttributeSet/AOAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UGA_Dash::ActivateAbility(
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

	// Dash Cost GE를 CommitAbility에서 사용 중이라면
	// 대시 시작 전에 스태미나를 먼저 검사한다.
	if (AttributeSet->GetStamina() <= 0.0f)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (!MovementComp)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 공중 / 비행 / 글라이드 중에는 순간 대시 불가
	if (MovementComp->IsFalling() || MovementComp->IsFlying() || (MovementComp->MovementMode == MOVE_Custom && MovementComp->CustomMovementMode == static_cast<uint8>(EAOMovementMode::Glide)))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Cost / Cooldown GameplayEffect 적용
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
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

	UAnimMontage* DashMontage = nullptr;

	float MontagePlayRate = 1.0f;
	if (ASC->HasMatchingGameplayTag(STATE_COMBAT))
	{
		DashMontage = Daeva->GetMontageByID(EMontageID::CombatDash);
		MontagePlayRate = 1.3f;
	}
	else
	{
		DashMontage = Daeva->GetMontageByID(EMontageID::Dash);
	}

	if (!DashMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask =	UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(	this,NAME_None,	DashMontage,MontagePlayRate,Daeva->HasMoveInput() ? FName("Forward") : FName("Back"));

	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_Dash::OnDashFinished);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_Dash::OnDashFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Dash::OnDashCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Dash::OnDashCancelled);

	MontageTask->ReadyForActivation();
}

void UGA_Dash::OnDashFinished()
{
	EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,false);
}

void UGA_Dash::OnDashCancelled()
{
	EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,true);
}