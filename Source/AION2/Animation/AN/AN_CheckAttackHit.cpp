#include "Animation/AN/AN_CheckAttackHit.h"
#include "GAS/AOGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Interface/CombatInterface.h"

void UAN_CheckAttackHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp)
	{
		AActor* OwnerActor = MeshComp->GetOwner();
		if (OwnerActor)
		{
			FGameplayEventData PayloadData;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, EVENT_CHECKATTACKHIT, PayloadData);

			// GAS 비활성 캐릭터(MMODaeva 등)를 위해 직접 타격 감지 실행
			if (ICombatInterface* CombatChar = Cast<ICombatInterface>(OwnerActor))
			{
				FAttackData AttackData;
				AttackData.DamageMultiplier = 1.0f;
				AttackData.bRestoreManaOnHit = true; // 기본 공격
				AttackData.TraceData.Range = 250.0f;
				AttackData.TraceData.Radius = 60.0f;
				AttackData.TraceData.StartOffset = FVector::ZeroVector;
				AttackData.TraceData.Direction = OwnerActor->GetActorForwardVector();

				CombatChar->CheckAttackHit(AttackData);
			}
		}
	}
}
