#include "STT_SetMoveSpeedMultiplier.h"

#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

EStateTreeRunStatus FSTT_SetMoveSpeedMultiplier::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Inst = Context.GetInstanceData<FInstanceDataType>(*this);

	if (Inst.bApplied)
	{
		ExitState(Context, Transition);
	}

	Inst.CachedMovement = nullptr;
	Inst.SavedMaxWalkSpeed = 0.0f;
	Inst.bApplied = false;

	if (Inst.Multiplier <= 0.0f)
	{
		return EStateTreeRunStatus::Failed;
	}

	UCharacterMovementComponent* Movement = GetMovementComponent(Inst.Actor);
	if (!Movement)
	{
		return EStateTreeRunStatus::Failed;
	}

	Inst.CachedMovement = Movement;
	Inst.SavedMaxWalkSpeed = Movement->MaxWalkSpeed;
	Inst.bApplied = true;

	Movement->MaxWalkSpeed = Inst.SavedMaxWalkSpeed * Inst.Multiplier;

	return EStateTreeRunStatus::Succeeded;
}

void FSTT_SetMoveSpeedMultiplier::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Inst = Context.GetInstanceData<FInstanceDataType>(*this);

	if (Inst.bApplied)
	{
		if (UCharacterMovementComponent* Movement = Inst.CachedMovement.Get())
		{
			Movement->MaxWalkSpeed = Inst.SavedMaxWalkSpeed;
		}
	}

	Inst.CachedMovement = nullptr;
	Inst.SavedMaxWalkSpeed = 0.0f;
	Inst.bApplied = false;
}

UCharacterMovementComponent* FSTT_SetMoveSpeedMultiplier::GetMovementComponent(AActor* Actor) const
{
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	if (ACharacter* Character = Cast<ACharacter>(Actor))
	{
		return Character->GetCharacterMovement();
	}

	if (AController* Controller = Cast<AController>(Actor))
	{
		if (ACharacter* Character = Cast<ACharacter>(Controller->GetPawn()))
		{
			return Character->GetCharacterMovement();
		}
	}

	return nullptr;
}

#if WITH_EDITOR
FText FSTT_SetMoveSpeedMultiplier::GetDescription(
	const FGuid& ID,
	FStateTreeDataView InstanceDataView,
	const IStateTreeBindingLookup& BindingLookup,
	EStateTreeNodeFormatting Formatting) const
{
	const FInstanceDataType* Inst = InstanceDataView.GetPtr<FInstanceDataType>();
	const float Multiplier = Inst ? Inst->Multiplier : 1.0f;
	const FText MultiplierText = FText::AsNumber(Multiplier);

	return Formatting == EStateTreeNodeFormatting::RichText
		? FText::Format(NSLOCTEXT("AION2", "SetMoveSpeedMultiplierRich", "<b>Set Move Speed</> x<b>{0}</>"), MultiplierText)
		: FText::Format(NSLOCTEXT("AION2", "SetMoveSpeedMultiplierPlain", "Set Move Speed x{0}"), MultiplierText);
}
#endif
