#include "Character/AOCharacterMovementComponent.h"

#include "GameFramework/Character.h"

void UAOCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
    if (CustomMovementMode == static_cast<uint8>(EAOMovementMode::Glide))
    {
        PhysGlide(DeltaTime, Iterations);
        return;
    }

    Super::PhysCustom(DeltaTime, Iterations);
}

void UAOCharacterMovementComponent::PhysGlide(float DeltaTime, int32 Iterations)
{
    if (!CharacterOwner || !UpdatedComponent)
    {
        return;
    }

    FVector Accel = GetPendingInputVector().GetClampedToMaxSize(1.f) * GlideForwardAcceleration;

    Velocity += Accel * DeltaTime;
    Velocity.Z += GetGravityZ() * GlideGravityScale * DeltaTime;
    Velocity.Z = FMath::Max(Velocity.Z, -GlideMaxFallSpeed);

    FVector Delta = Velocity * DeltaTime;

    FHitResult Hit;
    SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

    if (Hit.IsValidBlockingHit())
    {
        if (IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit))
        {
            SetMovementMode(MOVE_Walking);
            return;
        }

        SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);
    }
}
