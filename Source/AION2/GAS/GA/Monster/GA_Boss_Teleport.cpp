// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Boss_Teleport.h"
#include "GAS/AOGameplayTags.h"
#include "GAS/GE/Monster/GE_Cooldown_Monster_Teleport.h"
#include "Character/Monster/Boss/Thalythra/Talythra.h"

UGA_Boss_Teleport::UGA_Boss_Teleport()
{
	// Ability 자체에 태그를 붙이는 의미 
	// 이 어빌리티는 ChargeAttack 어빌리티다
	// AbilityTags.AddTag(ABILITY_MONSTER_CHARGEATTACK);
	// 이 어빌리티는 ChargeAttack 어빌리티다
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(ABILITY_MONSTER_TELEPORT);
	SetAssetTags(AssetTags);


	// ActivationBlockedTags는 Ability의 발동을 막는 태그 목록입니다.
	// ASC가 Cooldown.Monster.ChargeAttack 태그를 가지고 있으면
	// 이 어빌리티는 실행하지 마라.
	ActivationBlockedTags.AddTag(COOLDOWN_MONSTER_TELEPORT);

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
	CooldownGameplayEffectClass = UGE_Cooldown_Monster_Teleport::StaticClass();

}

void UGA_Boss_Teleport::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);


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


void UGA_Boss_Teleport::CancelAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);




}


void UGA_Boss_Teleport::EndAbility(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, 
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);



}
