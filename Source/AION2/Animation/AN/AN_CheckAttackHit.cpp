#include "Animation/AN/AN_CheckAttackHit.h"
#include "GAS/AOGameplayTags.h"

#include "AbilitySystemBlueprintLibrary.h"

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
		}
	}
}
