#pragma once

#include "CoreMinimal.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeTaskBase.h"
#include "STT_SetMoveSpeedMultiplier.generated.h"

class AActor;
class UCharacterMovementComponent;

USTRUCT()
struct AION2_API FSTT_SetMoveSpeedMultiplierInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float Multiplier = 1.2f;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> CachedMovement = nullptr;

	UPROPERTY(Transient)
	float SavedMaxWalkSpeed = 0.0f;

	UPROPERTY(Transient)
	bool bApplied = false;
};

USTRUCT(meta = (DisplayName = "Set Move Speed Multiplier", Category = "AION2|Movement"))
struct AION2_API FSTT_SetMoveSpeedMultiplier : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_SetMoveSpeedMultiplierInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual void ExitState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(
		const FGuid& ID,
		FStateTreeDataView InstanceDataView,
		const IStateTreeBindingLookup& BindingLookup,
		EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif

private:
	UCharacterMovementComponent* GetMovementComponent(AActor* Actor) const;
};
