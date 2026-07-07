#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AOCharacterMovementComponent.generated.h"

UENUM(BlueprintType)
enum class EAOMovementMode : uint8
{
	None = 0,
	Glide = 1
};

UCLASS()
class AION2_API UAOCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Glide|Dash")
    void StartGlideDash();

    virtual void UpdateFromCompressedFlags(uint8 Flags) override;
    virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

protected:
    virtual void PhysCustom(float DeltaTime, int32 Iterations) override;

    void PhysGlide(float DeltaTime, int32 Iterations);
    void BeginGlideDash(const FVector& Input);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glide")
    float GlideForwardSpeed = 1200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glide")
    float GlideTurnSpeed = 90.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glide")
    float GlideFallSpeed = 120.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glide")
    float GlideFastFallSpeed = 700.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glide")
    float GlideAccelerationInterpSpeed = 4.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glide")
    float GlideFallInterpSpeed = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glide")
    float GlideGravityScale = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glide")
    float GlideForwardAcceleration = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glide")
    float GlideMaxFallSpeed = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glide|Dash")
    float GlideDashSpeed = 2400.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glide|Dash")
    float GlideDashDuration = 0.5f;

public:
    UPROPERTY(Transient)
    bool bWantsGlideDash = false;

    UPROPERTY(Transient)
    bool bIsGlideDashing = false;

    UPROPERTY(Transient)
    float GlideDashRemainingTime = 0.f;

    UPROPERTY(Transient)
    FVector GlideDashDirection = FVector::ZeroVector;
};
