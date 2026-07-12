// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Monster/AOMonsterBase.h"
#include "Lutalis.generated.h"

class ALutalisElectricZone;
class ALutalisScytheZone;

/**
 * 
 */

UCLASS()
class AION2_API ALutalis : public AAOMonsterBase
{
	GENERATED_BODY()
	
public:

	ALutalis(const FObjectInitializer& ObjectInitializer);

protected:

	virtual void BeginPlay() override;
	virtual void InitAttributeSet() override;
	virtual void EndGroggy() override;

// ElectricZone
public:

	UFUNCTION(BlueprintCallable, Category = "Lutalis|ElectricZone")
	bool BeginElectricRangeWarning(float WarningDuration);

	UFUNCTION(BlueprintCallable, Category = "Lutalis|ElectricZone")
	bool ActivateElectricRangeDamage();

	UFUNCTION(BlueprintCallable, Category = "Lutalis|ElectricZone")
	void ConsumeElectricZoneRequestTag();

	UFUNCTION(BlueprintCallable, Category = "Lutalis|ElectricZone")
	bool PrepareElectricZoneCast();

	UFUNCTION(BlueprintCallable, Category = "Lutalis|ElectricZone")
	bool TickRotateToPreparedElectricZoneYaw(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Lutalis|ElectricZone")
	bool BeginPreparedElectricRangeWarning(float WarningDuration);

	UFUNCTION(BlueprintCallable, Category = "Lutalis|ElectricZone")
	void ResetPreparedElectricZoneCast();

private:
	bool ResolveElectricZoneActor();

private:

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Lutalis|ElectricZone", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ALutalisElectricZone> ElectricZoneActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lutalis|ElectricZone", meta = (AllowPrivateAccess = "true"))
	float ElectricZoneRadius = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lutalis|ElectricZone", meta = (AllowPrivateAccess = "true"))
	float ElectricZoneSectorAngle = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lutalis|ElectricZone", meta = (AllowPrivateAccess = "true"))
	FAttackData ElectricZoneTickDamageData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lutalis|ElectricZone", meta = (AllowPrivateAccess = "true"))
	float ElectricZoneRotateSpeed = 720.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lutalis|ElectricZone", meta = (AllowPrivateAccess = "true"))
	float ElectricZoneRotateTolerance = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lutalis|ElectricZone", meta = (AllowPrivateAccess = "true"))
	float ElectricZoneFacingYawOffset = 180.f;

	int32 PreparedElectricZoneArcIndex = INDEX_NONE;
	float PreparedElectricZoneTargetYaw = 0.f;

// Shythe
public:

	UFUNCTION(BlueprintCallable, Category = "Lutalis|Scythe")
	bool BeginScytheWarning(float WarningDuration);

	UFUNCTION(BlueprintCallable, Category = "Lutalis|Scythe")
	bool ActivateScytheSweep();

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lutalis|Scythe", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ALutalisScytheZone> ScytheZoneClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lutalis|Scythe", meta = (AllowPrivateAccess = "true"))
	float ScytheZoneLength = 1800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lutalis|Scythe", meta = (AllowPrivateAccess = "true"))
	float ScytheZoneWidth = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lutalis|Scythe", meta = (AllowPrivateAccess = "true"))
	FAttackData ScytheDamageData;

	UPROPERTY()
	TArray<TObjectPtr<ALutalisScytheZone>> ActiveScytheZones;
};
