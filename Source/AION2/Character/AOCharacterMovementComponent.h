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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glide")
    float GlideGravityScale = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glide")
    float GlideForwardAcceleration = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Glide")
    float GlideMaxFallSpeed = 200.f;

protected:
    virtual void PhysCustom(float DeltaTime, int32 Iterations) override;

    void PhysGlide(float DeltaTime, int32 Iterations);
};
