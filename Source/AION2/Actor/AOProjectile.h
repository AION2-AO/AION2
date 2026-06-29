#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/CombatInterface.h"
#include "AOProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class AAOCharacter;

UCLASS()
class AION2_API AAOProjectile : public AActor
{
    GENERATED_BODY()

public:
    AAOProjectile();

    void InitProjectile(const FAttackData& InAttackData, AAOCharacter* InDamageCauser, AAOCharacter* InTarget, const FVector& InDirection);

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnProjectileOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    TObjectPtr<USphereComponent> Collision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    bool bHoming = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    float ProjectileSpeed = 3000.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (EditCondition = "bHoming"))
    float HomingAcceleration = 10000.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    float LifeSeconds = 5.f;

    UPROPERTY()
    FAttackData AttackData;

    UPROPERTY()
    TObjectPtr<AAOCharacter> DamageCauser;

    UPROPERTY()
    TObjectPtr<AAOCharacter> Target;
};
