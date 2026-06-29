// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/GA/Monster/GA_Boss_ChargeAttack.h"
#include "GAS/AOGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/GE/Monster/GE_Cooldown_Monster_ChargeAttack.h"
#include "Character/Monster/AOMonsterBase.h"


UGA_Boss_ChargeAttack::UGA_Boss_ChargeAttack()
{
	// Ability 자체에 태그를 붙이는 의미 
	// 이 어빌리티는 ChargeAttack 어빌리티다
	// AbilityTags.AddTag(ABILITY_MONSTER_CHARGEATTACK);
	// 이 어빌리티는 ChargeAttack 어빌리티다
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(ABILITY_MONSTER_TH_CHARGEATTACK);
	SetAssetTags(AssetTags);


	// ActivationBlockedTags는 Ability의 발동을 막는 태그 목록입니다.
	// ASC가 Cooldown.Monster.ChargeAttack 태그를 가지고 있으면
	// 이 어빌리티는 실행하지 마라.
	ActivationBlockedTags.AddTag(COOLDOWN_MONSTER_TH_CHARGEATTACK);

	//InstancedPerActor는 “각 Actor마다 해당 GA 인스턴스를 따로 가진다”는 의미
	//예를 들어 GA_Boss_ChargeAttack이 있고, 몬스터가 3마리 있다고 하면,
	//Monster_A ASC
	//	└─ GA_Boss_ChargeAttack 인스턴스
	//
	//	Monster_B ASC
	//	└─ GA_Boss_ChargeAttack 인스턴스
	//
	//	Monster_C ASC
	//	└─ GA_Boss_ChargeAttack 인스턴스

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 보스 AI가 서버에서만 실행한다면 보통 이쪽이 적합
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	// 이 Ability가 Commit될 때 적용할 쿨타임 GE 
	CooldownGameplayEffectClass = UGE_Cooldown_Monster_ChargeAttack::StaticClass();
}

void UGA_Boss_ChargeAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AAOMonsterBase* pMonster = CastChecked<AAOMonsterBase>(ActorInfo->AvatarActor.Get());

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, pMonster->GetMontageByTag(MontageTag));
	MontageTask->OnCompleted.AddDynamic(this, &UGA_Boss_ChargeAttack::OnMontageTaskFinished);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_Boss_ChargeAttack::OnMontageTaskFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Boss_ChargeAttack::OnMontageTaskCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Boss_ChargeAttack::OnMontageTaskCancelled);
	MontageTask->ReadyForActivation();


	UAbilityTask_WaitGameplayEvent* WaitHitCheckTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EVENT_CHECKATTACKHIT);
	WaitHitCheckTask->EventReceived.AddDynamic(this, &UGA_Boss_ChargeAttack::OnCheckAttackHitEvent);
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


void UGA_Boss_ChargeAttack::CancelAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);





}

void UGA_Boss_ChargeAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

}



void UGA_Boss_ChargeAttack::OnMontageTaskFinished()
{
	EndAbility(
		CurrentSpecHandle,
		CurrentActorInfo,
		CurrentActivationInfo,
		true,
		false
	);
}

void UGA_Boss_ChargeAttack::OnMontageTaskCancelled()
{
	EndAbility(
		CurrentSpecHandle,
		CurrentActorInfo,
		CurrentActivationInfo,
		true,
		true
	);
}

void UGA_Boss_ChargeAttack::OnCheckAttackHitEvent(FGameplayEventData Payload)
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


void UGA_Boss_ChargeAttack::AbilityEnd()
{



}

