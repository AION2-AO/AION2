#include "GAS/GA/AT/AT_RotateToTarget.h"
#include "Character/AOCharacter.h"

UAT_RotateToTarget::UAT_RotateToTarget()
{
    bTickingTask = true;
}

UAT_RotateToTarget* UAT_RotateToTarget::RotateToTarget(UGameplayAbility* OwningAbility, float InAvailableRange, float InInterpSpeed)
{
    UAT_RotateToTarget* Task = NewAbilityTask<UAT_RotateToTarget>(OwningAbility);
    Task->AvailableRange = InAvailableRange;
    Task->InterpSpeed = InInterpSpeed;
    return Task;
}

void UAT_RotateToTarget::Activate()
{
    Super::Activate();
}

void UAT_RotateToTarget::TickTask(float DeltaTime)
{
    Super::TickTask(DeltaTime);

    AAOCharacter* Character = Cast<AAOCharacter>(GetAvatarActor());
    if (!Character)
    {
        return;
    }

    AAOCharacter* Target = Character->GetCurrentTarget();
    if (!IsValid(Target))
    {
        return;
    }

    const FVector MyLoc = Character->GetActorLocation();
    const FVector TargetLoc = Target->GetActorLocation();
    if (FVector::Distance(MyLoc, TargetLoc) > AvailableRange)
    {
        return;
    }

    FVector Direction = TargetLoc - MyLoc;
    Direction.Z = 0.f;
    if (Direction.IsNearlyZero())
    {
        return;
    }

    const FRotator TargetRotation = Direction.Rotation();
    const FRotator NewRotation = FMath::RInterpTo(Character->GetActorRotation(), TargetRotation, DeltaTime, InterpSpeed);

    Character->SetActorRotation(NewRotation);
}

void UAT_RotateToTarget::OnDestroy(bool bInOwnerFinished)
{
    Super::OnDestroy(bInOwnerFinished);
}
