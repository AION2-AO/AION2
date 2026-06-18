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

    const FVector Input = ConsumeInputVector().GetClampedToMaxSize(1.f);

    const float TurnInput = Input.Y;   // A/D
    const float DiveInput = Input.X;   // W/S, 프로젝트 축에 맞게 조정 필요

    // 좌우 조향
    if (!FMath::IsNearlyZero(TurnInput))
    {
        FRotator NewRotation = UpdatedComponent->GetComponentRotation();
        NewRotation.Yaw += TurnInput * GlideTurnSpeed * DeltaTime;

        MoveUpdatedComponent(
            FVector::ZeroVector,
            NewRotation,
            false
        );
    }

    const FVector Forward = UpdatedComponent->GetForwardVector();

    // 기본 전방 활강 속도 유지
    FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.f);
    FVector TargetHorizontalVelocity = Forward * GlideForwardSpeed;

    HorizontalVelocity = FMath::VInterpTo(
        HorizontalVelocity,
        TargetHorizontalVelocity,
        DeltaTime,
        GlideAccelerationInterpSpeed
    );

    Velocity.X = HorizontalVelocity.X;
    Velocity.Y = HorizontalVelocity.Y;

    // 기본 낙하 + W 입력 시 빠른 하강
    const float TargetFallSpeed =
        FMath::Lerp(GlideFallSpeed, GlideFastFallSpeed, FMath::Clamp(DiveInput, 0.f, 1.f));

    Velocity.Z = FMath::FInterpTo(
        Velocity.Z,
        -TargetFallSpeed,
        DeltaTime,
        GlideFallInterpSpeed
    );

    const FVector Delta = Velocity * DeltaTime;

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
