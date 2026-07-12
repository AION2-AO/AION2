// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/CombatInterface.h"
#include "LutalisElectricZone.generated.h"

class AAOCharacter;
class ADaeva;
class UMaterialInstanceDynamic;
class UNiagaraComponent;
class UNiagaraSystem;
class USceneComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class AION2_API ALutalisElectricZone : public AActor
{
	GENERATED_BODY()

public:
	ALutalisElectricZone();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Lutalis|ElectricZone")
	void InitZone(AAOCharacter* InDamageCauser, const FAttackData& InTickDamageData, float InRadius, float InSectorAngle);

	UFUNCTION(BlueprintCallable, Category = "Lutalis|ElectricZone")
	int32 StartRandomArcWarning(float InWarningDuration = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "Lutalis|ElectricZone")
	bool StartArcWarning(int32 ArcIndex, float InWarningDuration = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "Lutalis|ElectricZone")
	bool ActivatePendingArc();

	UFUNCTION(BlueprintCallable, Category = "Lutalis|ElectricZone")
	void FinishAllArcs();

	UFUNCTION(BlueprintCallable, Category = "Lutalis|ElectricZone")
	void ResetElectricZone();

	UFUNCTION(BlueprintCallable, Category = "Lutalis|ElectricZone")
	int32 SelectRandomUnusedArcIndex();

	UFUNCTION(BlueprintCallable, Category = "Lutalis|ElectricZone")
	bool GetArcIndicatorForwardYaw(int32 ArcIndex, float& OutYaw);

protected:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartArcWarning(int32 ArcIndex, float InWarningDuration);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastActivateArc(int32 ArcIndex);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFinishAllArcs();

protected:
	void RefreshArcReferenceCache();
	void EnsureArcStateArrays();
	void CreateArcMaterialInstance();

	void StartArcWarningInternal(int32 ArcIndex, float InWarningDuration);
	void ActivateArcInternal(int32 ArcIndex);
	void FinishArcInternal(int32 ArcIndex);

	int32 PickRandomUnusedArcIndex() const;

	void UpdateWarningFill();
	void UpdateActiveDamage(float DeltaTime);

	void SetArcFillAmount(int32 ArcIndex, float FillAmount);
	void SetArcWarningVisible(int32 ArcIndex, bool bVisible);
	void SetArcActiveVisible(int32 ArcIndex, bool bVisible);

	void ApplyDamageTick(int32 ArcIndex);
	void BuildDamageHitResult(ADaeva* Target, FHitResult& OutHitResult) const;

	bool IsInsideElectricZone(AAOCharacter* Target, int32 ArcIndex) const;
	bool IsInsideAnyActiveElectricZone(AAOCharacter* Target) const;

	bool IsValidArcIndex(int32 ArcIndex) const;
	int32 GetArcCount() const;

	bool IsAnyArcWarningInProgress() const;
	bool IsAnyArcActive() const;

	void ResolveDamageCauser();
	void DrawDebugZone() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ElectricZone")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "ElectricZone|ArcIndicator")
	TObjectPtr<AActor> ArcIndicator1;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "ElectricZone|ArcIndicator")
	TObjectPtr<AActor> ArcIndicator2;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "ElectricZone|ArcIndicator")
	TObjectPtr<AActor> ArcIndicator3;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "ElectricZone|ArcIndicator")
	TObjectPtr<AActor> ArcIndicator4;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> ArcIndicatorActors;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> ArcIndicatorMeshes;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "ElectricZone")
	TObjectPtr<AActor> ElectricZoneNS1;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "ElectricZone")
	TObjectPtr<AActor> ElectricZoneNS2;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "ElectricZone")
	TObjectPtr<AActor> ElectricZoneNS3;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "ElectricZone")
	TObjectPtr<AActor> ElectricZoneNS4;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> ElectricZoneNS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ElectricZone|Niagara")
	TObjectPtr<UNiagaraSystem> ElectricZoneNiagaraSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ElectricZone")
	float Radius = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ElectricZone")
	float SectorAngle = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ElectricZone")
	float WarningDuration = 1.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ElectricZone")
	float ActiveDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ElectricZone")
	float DamageInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ElectricZone")
	float FirstDamageDelay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ElectricZone")
	float GroundTolerance = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ElectricZone|Damage")
	FAttackData TickDamageData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ElectricZone|Material")
	FName FillAmountParameterName = TEXT("FillInnerArc");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ElectricZone|Material")
	float WarningStartFillAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ElectricZone|Material")
	float WarningEndFillAmount = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ElectricZone|Debug")
	bool bDrawDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ElectricZone|Debug", meta = (EditCondition = "bDrawDebug"))
	float DebugDrawDuration = 0.1f;

private:
	UPROPERTY()
	TObjectPtr<AAOCharacter> DamageCauser;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> ArcMIDs;

	TArray<bool> bArcUsed;
	TArray<bool> bArcWarnings;
	TArray<bool> bArcWarningFilled;
	TArray<bool> bArcActives;
	TArray<bool> bArcFirstDamageApplied;

	TArray<float> ArcWarningStartTimes;
	TArray<float> ArcWarningDurations;
	TArray<float> ArcActiveElapsedTimes;
	TArray<float> ArcDamageElapsedTimes;

	int32 PendingArcIndex = INDEX_NONE;
};
