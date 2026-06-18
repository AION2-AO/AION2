#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_WaitLanding.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLandDelegate);

UCLASS()
class AION2_API UAT_WaitLanding : public UAbilityTask
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "Jump", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAT_WaitLanding* CreateTask(UGameplayAbility* OwningAbility);

	virtual void Activate() override;
	virtual void OnDestroy(bool AbilityEnded) override;

public:
	UPROPERTY(BlueprintAssignable)
	FLandDelegate OnComplete;

protected:
	UFUNCTION()
	void OnMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode);
};
