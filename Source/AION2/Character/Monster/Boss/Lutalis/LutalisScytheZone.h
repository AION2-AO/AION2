// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/CombatInterface.h"
#include "LutalisScytheZone.generated.h"

class AAOCharacter;
class USceneComponent;
class UBoxComponent;
class USkeletalMeshComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UAnimSequenceBase;

UENUM()
enum class ELutalisScytheZoneVisualState : uint8
{
	Hidden,
	Warning,
	Scythe
};

UCLASS(Blueprintable)
class AION2_API ALutalisScytheZone : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALutalisScytheZone();

	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitZone(AAOCharacter* InDamageCauser, const FAttackData& InDamageData, float InLength, float InWidth);
	void StartWarning(float InWarningDuration);
	void ActivateSweep();
	void BeginDamageWindow();
	void EndDamageWindow();

protected:
	UFUNCTION()
	void OnRep_VisualState();

	UFUNCTION()
	void OnRep_ZoneDimensions();

	void StartWarningInternal(float InWarningDuration);
	void ActivateSweepInternal();
	void ApplyZoneDimensions();
	void UpdateWarning();
	void ApplyDamageToOverlaps();
	void AlignScytheToWarningRotation();
	void DrawDamageBoxDebug() const;
	void RefreshTickEnabled();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> WarningRectMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> ScytheMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> ScytheSkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> DamageBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName FillParameterName = TEXT("FillAmount");

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimSequenceBase> ScytheAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ScytheHeight = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRotator ScytheRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DamageHeight = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ExtraLifeAfterAnimation = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FallbackLifeTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bDrawDamageBoxDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug", meta = (ClampMin = "0.0"))
	float DamageBoxDebugDrawDuration = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug", meta = (ClampMin = "0.0"))
	float DamageBoxDebugThickness = 4.0f;

private:
	UPROPERTY()
	TObjectPtr<AAOCharacter> DamageCauser;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> WarningMID;

	FAttackData DamageData;

	UPROPERTY(ReplicatedUsing = OnRep_ZoneDimensions)
	float Length = 1800.f;

	UPROPERTY(ReplicatedUsing = OnRep_ZoneDimensions)
	float Width = 500.f;

	float WarningDuration = 1.f;
	float WarningElapsed = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_VisualState)
	ELutalisScytheZoneVisualState VisualState = ELutalisScytheZoneVisualState::Hidden;

	UPROPERTY(Replicated)
	float ReplicatedWarningDuration = 1.f;

	bool bWarning = false;
	bool bDamageWindowActive = false;

	UPROPERTY()
	TSet<TObjectPtr<AActor>> DamagedActors;
};
