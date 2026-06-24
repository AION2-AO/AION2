// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/GA/Monster/GA_Boss_ChargeAttack.h"
#include "GAS/AOGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GAS/GE/Monster/GE_Cooldown_Monster_ChargeAttack.h"
#include "Character/Monster/Boss/Thalythra/Talythra.h"




UGA_Boss_ChargeAttack::UGA_Boss_ChargeAttack()
{
	// Ability 자체에 태그를 붙이는 의미 
	// 이 어빌리티는 ChargeAttack 어빌리티다
	// AbilityTags.AddTag(ABILITY_MONSTER_CHARGEATTACK);
	// 이 어빌리티는 ChargeAttack 어빌리티다
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(ABILITY_MONSTER_CHARGEATTACK);
	SetAssetTags(AssetTags);


	// ActivationBlockedTags는 Ability의 발동을 막는 태그 목록입니다.
	// ASC가 Cooldown.Monster.ChargeAttack 태그를 가지고 있으면
	// 이 어빌리티는 실행하지 마라.
	ActivationBlockedTags.AddTag(COOLDOWN_MONSTER_CHARGEATTACK);

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


	//여기서 캐릭터 클래스가져와서 해당 클래스를 cast하여 가져오고 해당 포인터에서 몽타주를 get함수를 통해 가져와서 넣어주기

	// 매개변수 설명
	// 1. 이 Task를 소유하고 있는 어빌리티에 대한 정보 
	// 2. 이 Task를 지정할 수 있는 고유한 이름 "ChargeAttack 
	// 3. 이 캐릭터에서 제공받을 몽타주 에셋 
	// 나머지는 기본 옵션 사용. 
	
	/*ATalythra* pTalytrha = CastChecked<ATalythra>(ActorInfo->AvatarActor.Get());


	UAbilityTask_PlayMontageAndWait* PlayAttackTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("ChargeAttack"), pTalytrha->GetChargeAttackActionMontage());
	PlayAttackTask->OnCompleted.AddDynamic(this, &UGA_Boss_ChargeAttack::OnCompleteCallback);
	PlayAttackTask->OnInterrupted.AddDynamic(this, &UGA_Boss_ChargeAttack::OnInterruptedCallback);
	PlayAttackTask->ReadyForActivation(); */


	// 여기서 비용 / 쿨타임 적용
	// 내부적으로 CooldownGameplayEffectClass를 보고
	// GE_Cooldown_Monster_ChargeAttack을 ASC에 적용함
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
	
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);

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



void UGA_Boss_ChargeAttack::OnCompleteCallback()
{
	// 클라에서도 EndAbility 호출하게 설정 
	bool bReplicatedEndAbility = true; 

	// 취소된것인가.
	bool bWasCancelled = false; 

	


	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);

}

void UGA_Boss_ChargeAttack::OnInterruptedCallback()
{
	// 클라에서도 EndAbility 호출하게 설정 
	bool bReplicatedEndAbility = true;

	// 취소된것인가.
	bool bWasCancelled = true;

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);


}
