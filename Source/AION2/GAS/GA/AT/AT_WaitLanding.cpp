#include "GAS/GA/AT/AT_WaitLanding.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UAT_WaitLanding* UAT_WaitLanding::CreateTask(UGameplayAbility* OwningAbility)
{
	UAT_WaitLanding* NewTask = NewAbilityTask<UAT_WaitLanding>(OwningAbility);
	return NewTask;
}

void UAT_WaitLanding::Activate()
{
	Super::Activate();

	ACharacter* Character = CastChecked<ACharacter>(GetAvatarActor());
	Character->MovementModeChangedDelegate.AddDynamic(this, &UAT_WaitLanding::OnMovementModeChanged);

	SetWaitingOnAvatar();
}

void UAT_WaitLanding::OnDestroy(bool AbilityEnded)
{
	if (ACharacter* Character = Cast<ACharacter>(GetAvatarActor()))
	{
		Character->MovementModeChangedDelegate.RemoveDynamic(this, &UAT_WaitLanding::OnMovementModeChanged);
	}

	Super::OnDestroy(AbilityEnded);
}

void UAT_WaitLanding::OnMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	if (Movement && Movement->IsMovingOnGround())
	{
		OnComplete.Broadcast();
		EndTask();
	}
}
