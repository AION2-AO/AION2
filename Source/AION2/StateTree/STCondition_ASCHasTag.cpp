#include "STCondition_ASCHasTag.h"

#include "StateTreeExecutionContext.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

bool FStateTreeASCHasTagCondition::TestCondition(
	FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AActor* Actor = InstanceData.Actor;
	if (!IsValid(Actor))
	{
		return false;
	}

	if (!InstanceData.Tag.IsValid())
	{
		return false;
	}

	UAbilitySystemComponent* ASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);

	// 만약 Actor가 AIController이고 ASC는 Pawn에 붙어있는 구조라면
	if (ASC == nullptr)
	{
		if (AController* Controller = Cast<AController>(Actor))
		{
			APawn* Pawn = Controller->GetPawn();
			if (IsValid(Pawn))
			{
				ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
			}
		}
	}

	if (ASC == nullptr)
	{
		return false;
	}

	const bool bHasTag = ASC->HasMatchingGameplayTag(InstanceData.Tag);

	return InstanceData.bInvert ? !bHasTag : bHasTag;
}