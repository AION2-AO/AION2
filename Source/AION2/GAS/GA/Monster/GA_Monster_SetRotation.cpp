// Fill out your copyright notice in the Description page of Project Settings.



#include "GA_Monster_SetRotation.h"
#include "Character/Monster/AOMonsterBase.h"
#include "AI/AIMonsterControllerBase.h"

void UGA_Monster_SetRotation::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, 
	const FGameplayEventData* TriggerEventData)
{
	AAOMonsterBase* Monster = Cast<AAOMonsterBase>(ActorInfo->AvatarActor.Get());

	if (Monster == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 서버 권위에서만 회전 (컨트롤러 회전은 서버 권위)
	if (Monster->HasAuthority() == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	const FRotator NewRot(0.f, DesiredYaw, 0.f);
	Monster->SetActorRotation(NewRot);

	// 핵심: ControlRotation도 같은 값으로 맞춰야 다음 틱에 안 되돌아감
	if (AAIController* AIContoller = Cast<AAIController>(Monster->GetController()))	
	{
		AIContoller->SetControlRotation(NewRot);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_Monster_SetRotation::CancelAbility(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, 
	bool bReplicateCancelAbility)
{

	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UGA_Monster_SetRotation::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, 
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
