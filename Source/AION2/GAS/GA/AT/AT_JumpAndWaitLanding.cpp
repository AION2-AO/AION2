#include "GAS/GA/AT/AT_JumpAndWaitLanding.h"

#include "GameFramework/Character.h"

UAT_JumpAndWaitLanding* UAT_JumpAndWaitLanding::CreateTask(UGameplayAbility* OwningAbility)
{
	UAT_JumpAndWaitLanding* NewTask = NewAbilityTask<UAT_JumpAndWaitLanding>(OwningAbility);
	return NewTask;
}

void UAT_JumpAndWaitLanding::Activate()
{
	Super::Activate();

	ACharacter* Character = CastChecked<ACharacter>(GetAvatarActor());
	Character->LandedDelegate.AddDynamic(this, &UAT_JumpAndWaitLanding::OnLandedCallback);
	Character->Jump();

	SetWaitingOnAvatar();
}

void UAT_JumpAndWaitLanding::OnDestroy(bool AbilityEnded)
{
	if (ACharacter* Character = Cast<ACharacter>(GetAvatarActor()))
	{
		Character->LandedDelegate.RemoveDynamic(this, &UAT_JumpAndWaitLanding::OnLandedCallback);
	}

	Super::OnDestroy(AbilityEnded);
}

void UAT_JumpAndWaitLanding::OnLandedCallback(const FHitResult& Hit)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnComplete.Broadcast();
	}
}
