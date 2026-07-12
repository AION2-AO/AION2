#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/CombatInterface.h"
#include "SiliatorShuriken.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class AAOCharacter;

UCLASS()
class AION2_API ASiliatorShuriken : public AActor
{
	GENERATED_BODY()

public:
	ASiliatorShuriken();

	virtual void Tick(float DeltaTime) override;

	void SetTarget(AAOCharacter* InTarget, const FAttackData& InAttackData);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Destroyed() override;

	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	void StartMove();
	void MoveToTarget(float DeltaTime);
	void RotateShuriken(float DeltaTime);
	void ProcessTickDamage();
	void ApplyTickDamage(AAOCharacter* HitCharacter);
	void UpdateAOEPlane();

private:
	UFUNCTION()
	void OnRep_TargetCharacter();

private:
	bool bHasMoveStarted = false;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	FAttackData AttackData;

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> BoxCollision;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(ReplicatedUsing = "OnRep_TargetCharacter")
	TObjectPtr<AAOCharacter> TargetCharacter;

	UPROPERTY()
	TSet<TObjectPtr<AAOCharacter>> OverlappingCharacters;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Shuriken|Move")
	float MoveSpeed = 250.f;

	UPROPERTY(EditDefaultsOnly, Category = "Shuriken|Move")
	float RotationSpeed = 1080.f;

	UPROPERTY(EditDefaultsOnly, Category = "Shuriken|Damage")
	float DamageInterval = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Shuriken|Life")
	float LifeTime = 10.f;

	float DamageElapsedTime = 0.f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shuriken|AOE")
	TObjectPtr<UStaticMeshComponent> AOEPlaneComp;

	UPROPERTY(EditDefaultsOnly, Category = "Shuriken|AOE")
	float AOEPlaneWidth = 120.f;

	UPROPERTY(EditDefaultsOnly, Category = "Shuriken|AOE")
	float AOEPlaneZOffset = -80.f;

	UPROPERTY(EditDefaultsOnly, Category = "Shuriken|AOE")
	float PlaneBaseSizeX = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Shuriken|AOE")
	float PlaneBaseSizeY = 100.f;
};