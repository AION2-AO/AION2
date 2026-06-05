#include "Animation/AOCharacterAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UAOCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    APawn* Pawn = TryGetPawnOwner();
    if (!Pawn)
    {
        return;
    }

    Speed = Pawn->GetVelocity().Length();

    ACharacter* Character = Cast<ACharacter>(Pawn);
    if (!Character)
    {
        return;
    }

    bIsInAir = Character->GetMovementComponent()->IsFalling() || Character->GetMovementComponent()->IsFlying();
}
