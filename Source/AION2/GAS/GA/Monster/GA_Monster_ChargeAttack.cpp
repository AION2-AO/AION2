// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/Monster/GA_Monster_ChargeAttack.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/AOGameplayTags.h"
#include "Character/Monster/AOMonsterBase.h"
#include "Components/CapsuleComponent.h"

void UGA_Monster_ChargeAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, 
	const FGameplayEventData* TriggerEventData)
{

	// 몽타주 재생은 Action에서 처리
	Super::ActivateAbility(Handle,ActorInfo, ActivationInfo, TriggerEventData); 

	UAbilityTask_WaitGameplayEvent* WaitChargeHitStartCheckTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EVENT_CHECKATTACKHIT_BEGIN);

	WaitChargeHitStartCheckTask->EventReceived.AddDynamic(this, &UGA_Monster_ChargeAttack::OnHitCheckBegin);
	WaitChargeHitStartCheckTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitChargeHitEndCheckTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EVENT_CHECKATTACKHIT_END);
	WaitChargeHitEndCheckTask->EventReceived.AddDynamic(this, &UGA_Monster_ChargeAttack::OnHitCheckEnd);
	WaitChargeHitEndCheckTask->ReadyForActivation();


}

void UGA_Monster_ChargeAttack::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	UnBindCapsuleOverlap(); // 안전망 - 중간 캔슬 대비 
	HitActors.Reset();
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UGA_Monster_ChargeAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{

	UnBindCapsuleOverlap(); // 안전망 - 중간 캔슬 대비 
	HitActors.Reset();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

}

void UGA_Monster_ChargeAttack::OnHitCheckBegin(FGameplayEventData Payload)
{
	if (HasAuthority(&CurrentActivationInfo) == false)
		return; 

	HitActors.Reset();
	BindCapsuleOverlap();
}

void UGA_Monster_ChargeAttack::OnHitCheckEnd(FGameplayEventData Payload)
{
	if (HasAuthority(&CurrentActivationInfo) == false)
		return;


	UnBindCapsuleOverlap();
	HitActors.Reset();

}

void UGA_Monster_ChargeAttack::OnCapsuleBindOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, 
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// 서버에서만 로직 실행할 수 있게 설정 
	if (HasAuthority(&CurrentActivationInfo) == false)
		return; 

	// 해당 Actor이 충돌대상이 맞는지.
	if (IsVaildTarget(OtherActor) == false)
		return; 


	// 해당 Actor가 이미 충돌처리 된 것인지.
	if (HitActors.Contains(OtherActor))
		return; 

	
	HitActors.Add(OtherActor);


	if (AAOCharacter* AOTarget = Cast<AAOCharacter>(OtherActor))
	{
		AAOCharacter* Monster = CastChecked<AAOCharacter>(GetAvatarActorFromActorInfo());

		bool bCameraShake = false; 
		Monster->OnAttackSucceeded(AttackData, AOTarget, SweepResult, bCameraShake);
		
	}
}


void UGA_Monster_ChargeAttack::BindCapsuleOverlap()
{
	AAOMonsterBase* Monster = CastChecked<AAOMonsterBase>(GetAvatarActorFromActorInfo());
	
	if(UCapsuleComponent* Capsule = Monster->GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		Capsule->OnComponentBeginOverlap.AddDynamic(this, &UGA_Monster_ChargeAttack::OnCapsuleBindOverlap);
		bHitCheckActive = true; 
	}


}

void UGA_Monster_ChargeAttack::UnBindCapsuleOverlap()
{
	if (!bHitCheckActive)
		return; 

	AAOMonsterBase* Monster = Cast<AAOMonsterBase>(GetAvatarActorFromActorInfo());
	if(Monster == nullptr)
	{
		bHitCheckActive = false; 
		return; 
	}


	if(UCapsuleComponent* Capsule = Monster->GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		Capsule->OnComponentBeginOverlap.RemoveDynamic(this, &UGA_Monster_ChargeAttack::OnCapsuleBindOverlap);
	}

	bHitCheckActive = false; 

}

bool UGA_Monster_ChargeAttack::IsVaildTarget(AActor* OtherActor) const
{
	if (!IsValid(OtherActor))
		return false;

	// 자기 자신 제외
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (OtherActor == Avatar)
		return false;

	// AAOCharacter 계열만 (플레이어 캐릭터)
	AAOCharacter* AsCharacter = Cast<AAOCharacter>(OtherActor);
	if (!AsCharacter)
		return false;

	// 아군(다른 몬스터) 제외
	if (AsCharacter->IsA<AAOMonsterBase>())
		return false;

	// TODO: 팀 시스템 있으면 여기서 팀 체크
	// if (UAOTeamFunctionLibrary::AreActorsFriendly(Avatar, OtherActor)) return false;


	return true;
}
