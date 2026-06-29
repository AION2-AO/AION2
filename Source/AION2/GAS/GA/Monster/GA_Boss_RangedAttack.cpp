// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Boss_RangedAttack.h"
#include "GAS/AOGameplayTags.h"
#include "GAS/GE/Monster/GE_Cooldown_Monster_Teleport.h"
#include "Character/Monster/AOMonsterBase.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"


UGA_Boss_RangedAttack::UGA_Boss_RangedAttack()
{
	// Ability 자체 태그
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(ABILITY_MONSTER_TH_RANGEDATTACK);
	SetAssetTags(AssetTags);


	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 보스 AI가 서버에서만 실행한다면 보통 이쪽이 적합
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;


}

void UGA_Boss_RangedAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);


	AAOMonsterBase* pMonster = CastChecked<AAOMonsterBase>(ActorInfo->AvatarActor.Get());

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, pMonster->GetMontageByTag(MontageTag));
	MontageTask->OnCompleted.AddDynamic(this, &UGA_Boss_RangedAttack::OnMontageTaskFinished);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_Boss_RangedAttack::OnMontageTaskFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Boss_RangedAttack::OnMontageTaskCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Boss_RangedAttack::OnMontageTaskCancelled);
	MontageTask->ReadyForActivation();


	UAbilityTask_WaitGameplayEvent* WaitHitCheckTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EVENT_CHECKATTACKHIT);
	WaitHitCheckTask->EventReceived.AddDynamic(this, &UGA_Boss_RangedAttack::OnCheckAttackHitEvent);
	WaitHitCheckTask->ReadyForActivation();



	// 여기서 비용 / 쿨타임 적용
	// 내부적으로 CooldownGameplayEffectClass를 보고
	// GE_Cooldown_Monster_ChargeAttack을 ASC에 적용함
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;

	}


	// 여기서 CommitAbility()가 true를 반환했다는 뜻은 :
	// 
	// 이 Ability를 실제로 사용해도 된다.
	// 	→ Cost 검사 통과
	// 	→ Cooldown 검사 통과
	// 	→ Cost 적용
	// 	→ Cooldown GE 적용
	// 
	// 	입니다.
	// 
	// 	반대로 false면 :
	// 
	// 이 Ability를 지금 확정해서 사용할 수 없다.


}


void UGA_Boss_RangedAttack::CancelAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);




}


void UGA_Boss_RangedAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, 
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);



}




void UGA_Boss_RangedAttack::OnMontageTaskFinished()
{
	EndAbility(
		CurrentSpecHandle,
		CurrentActorInfo,
		CurrentActivationInfo,
		true,
		false
	);
}

void UGA_Boss_RangedAttack::OnMontageTaskCancelled()
{
	EndAbility(
		CurrentSpecHandle,
		CurrentActorInfo,
		CurrentActivationInfo,
		true,
		true
	);
}

void UGA_Boss_RangedAttack::OnCheckAttackHitEvent(FGameplayEventData Payload)
{
	if (!HasAuthority(&CurrentActivationInfo))
	{
		return;
	}

	AAOCharacter* AOCharacter =
		Cast<AAOCharacter>(GetAvatarActorFromActorInfo());

	if (!AOCharacter)
	{
		return;
	}

	AOCharacter->CheckAttackHit(AttackData);
}


void UGA_Boss_RangedAttack::AbilityEnd()
{



}
