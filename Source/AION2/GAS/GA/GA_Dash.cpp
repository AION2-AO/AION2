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
		UE_LOG(LogTemp, Error, TEXT("[Dash] AttributeSet is null"));

		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Dash Cost GE를 CommitAbility에서 사용 중이라면
	// 대시 시작 전에 스태미나를 먼저 검사한다.
	if (AttributeSet->GetStamina() <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dash] Not enough stamina"));

		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		UE_LOG(LogTemp, Error, TEXT("[Dash] Character is null"));

		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (!MovementComp)
	{
		UE_LOG(LogTemp, Error, TEXT("[Dash] MovementComponent is null"));

		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 공중 / 비행 / 글라이드 중에는 순간 대시 불가
	if (MovementComp->IsFalling() ||
		MovementComp->IsFlying() ||
		(MovementComp->MovementMode == MOVE_Custom &&
			MovementComp->CustomMovementMode == static_cast<uint8>(EAOMovementMode::Glide)))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dash] Dash unavailable in current movement mode"));

		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Cost / Cooldown GameplayEffect 적용
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dash] CommitAbility failed"));

		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ADaeva* Daeva = Cast<ADaeva>(ActorInfo->AvatarActor.Get());
	if (!Daeva)
	{
		UE_LOG(LogTemp, Error, TEXT("[Dash] Daeva is null"));

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
		UE_LOG(LogTemp, Error, TEXT("[Dash] Dash montage is null"));

		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			DashMontage,
			MontagePlayRate,
			Daeva->HasMoveInput() ? FName("Forward") : FName("Back")
		);

	if (!MontageTask)
	{
		UE_LOG(LogTemp, Error, TEXT("[Dash] Failed to create montage task"));

		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_Dash::OnDashFinished);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_Dash::OnDashFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Dash::OnDashCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Dash::OnDashCancelled);

	MontageTask->ReadyForActivation();

	UE_LOG(LogTemp,Log,TEXT("[Dash Start] Stamina: %.1f / %.1f"),AttributeSet->GetStamina(),AttributeSet->GetMaxStamina());
}

void UGA_Dash::OnDashFinished()
{
	EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,false);
}

void UGA_Dash::OnDashCancelled()
{
	UE_LOG(LogTemp, Warning, TEXT("[Dash Cancelled]"));

	EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,true);
}