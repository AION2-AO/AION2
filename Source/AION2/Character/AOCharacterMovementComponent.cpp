#include "Character/AOCharacterMovementComponent.h"

#include "GameFramework/Character.h"

class FSavedMove_AOCharacter : public FSavedMove_Character
{
public:
    using Super = FSavedMove_Character;

    uint8 bSavedWantsGlideDash : 1;

    virtual void Clear() override
    {
        Super::Clear();
        bSavedWantsGlideDash = false;
    }

    virtual uint8 GetCompressedFlags() const override
    {
        uint8 Result = Super::GetCompressedFlags();

        if (bSavedWantsGlideDash)
        {
            Result |= FLAG_Custom_0;
        }

        return Result;
    }

    virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override
    {
        const FSavedMove_AOCharacter* NewAOMove =
            static_cast<const FSavedMove_AOCharacter*>(NewMove.Get());

        if (bSavedWantsGlideDash != NewAOMove->bSavedWantsGlideDash)
        {
            return false;
        }

        return Super::CanCombineWith(NewMove, Character, MaxDelta);
    }

    virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override
    {
        Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

        if (const UAOCharacterMovementComponent* MoveComp = Cast<UAOCharacterMovementComponent>(Character->GetCharacterMovement()))
        {
            bSavedWantsGlideDash = MoveComp->bWantsGlideDash;
        }
    }

    virtual void PrepMoveFor(ACharacter* Character) override
    {
        Super::PrepMoveFor(Character);

        if (UAOCharacterMovementComponent* MoveComp =
            Cast<UAOCharacterMovementComponent>(Character->GetCharacterMovement()))
        {
            MoveComp->bWantsGlideDash = bSavedWantsGlideDash;
        }
    }
};

class FNetworkPredictionData_Client_AOCharacter : public FNetworkPredictionData_Client_Character
{
public:
    using Super = FNetworkPredictionData_Client_Character;

    FNetworkPredictionData_Client_AOCharacter(const UCharacterMovementComponent& ClientMovement)
        : Super(ClientMovement)
    {
    }

    virtual FSavedMovePtr AllocateNewMove() override
    {
        return FSavedMovePtr(new FSavedMove_AOCharacter());
    }
};

void UAOCharacterMovementComponent::StartGlideDash()
{
    if (MovementMode != MOVE_Custom || CustomMovementMode != static_cast<uint8>(EAOMovementMode::Glide))
    {
        return;
    }

    if (bIsGlideDashing)
    {
        return;
    }

    bWantsGlideDash = true;
}

void UAOCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
    Super::UpdateFromCompressedFlags(Flags);

    bWantsGlideDash = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

FNetworkPredictionData_Client* UAOCharacterMovementComponent::GetPredictionData_Client() const
{
    check(PawnOwner != nullptr);

    if (!ClientPredictionData)
    {
        UAOCharacterMovementComponent* MutableThis =
            const_cast<UAOCharacterMovementComponent*>(this);

        MutableThis->ClientPredictionData =
            new FNetworkPredictionData_Client_AOCharacter(*this);

        MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
        MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
    }

    return ClientPredictionData;
}

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

    if (bWantsGlideDash && !bIsGlideDashing)
    {
        BeginGlideDash(Input);
    }

    if (bIsGlideDashing)
    {
        GlideDashRemainingTime -= DeltaTime;

        Velocity.X = GlideDashDirection.X * GlideDashSpeed;
        Velocity.Y = GlideDashDirection.Y * GlideDashSpeed;
        Velocity.Z = FMath::FInterpTo(Velocity.Z, -GlideFallSpeed, DeltaTime, GlideFallInterpSpeed);

        if (GlideDashRemainingTime <= 0.f)
        {
            bIsGlideDashing = false;
        }
    }
    else
    {
        const float TurnInput = Input.Y;   // A/D
        const float DiveInput = Input.X;   // W/S, 프로젝트 축에 맞게 조정 필요

        // 좌우 조향
        if (!FMath::IsNearlyZero(TurnInput))
        {
            FRotator NewRotation = UpdatedComponent->GetComponentRotation();
            NewRotation.Yaw += TurnInput * GlideTurnSpeed * DeltaTime;
            MoveUpdatedComponent(FVector::ZeroVector, NewRotation, false);
        }

        const FVector Forward = UpdatedComponent->GetForwardVector();

        // 기본 전방 활강 속도 유지
        FVector HorizontalVelocity = FVector(Velocity.X, Velocity.Y, 0.f);
        FVector TargetHorizontalVelocity = Forward * GlideForwardSpeed;

        HorizontalVelocity = FMath::VInterpTo(HorizontalVelocity, TargetHorizontalVelocity, DeltaTime, GlideAccelerationInterpSpeed);

        Velocity.X = HorizontalVelocity.X;
        Velocity.Y = HorizontalVelocity.Y;

        // 기본 낙하 + W 입력 시 빠른 하강
        const float TargetFallSpeed = FMath::Lerp(GlideFallSpeed, GlideFastFallSpeed, FMath::Clamp(DiveInput, 0.f, 1.f));

        Velocity.Z = FMath::FInterpTo(Velocity.Z, -TargetFallSpeed, DeltaTime, GlideFallInterpSpeed);
    }

    const FVector Delta = Velocity * DeltaTime;

    FHitResult Hit;
    SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

    if (Hit.IsValidBlockingHit())
    {
        if (IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit))
        {
            bWantsGlideDash = false;
            bIsGlideDashing = false;
            SetMovementMode(MOVE_Walking);
            return;
        }

        SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);
    }
}

void UAOCharacterMovementComponent::BeginGlideDash(const FVector& Input)
{
    bWantsGlideDash = false;
    bIsGlideDashing = true;
    GlideDashRemainingTime = GlideDashDuration;

    FVector DashDir = UpdatedComponent->GetForwardVector();

    if (!Input.IsNearlyZero())
    {
        DashDir = UpdatedComponent->GetForwardVector() * Input.X + UpdatedComponent->GetRightVector() * Input.Y;
        DashDir.Z = 0.f;
        DashDir = DashDir.GetSafeNormal();
    }

    if (DashDir.IsNearlyZero())
    {
        DashDir = UpdatedComponent->GetForwardVector();
    }

    GlideDashDirection = DashDir;
}
