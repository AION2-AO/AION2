#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_JumpAndWaitLanding.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FJumpDelegate);

UCLASS()
class AION2_API UAT_JumpAndWaitLanding : public UAbilityTask
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "Jump", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAT_JumpAndWaitLanding* CreateTask(UGameplayAbility* OwningAbility);

	virtual void Activate() override;
	virtual void OnDestroy(bool AbilityEnded) override;

public:
	UPROPERTY(BlueprintAssignable)
	FJumpDelegate OnComplete;

protected:
	UFUNCTION()
	void OnLandedCallback(const FHitResult& Hit);
};
