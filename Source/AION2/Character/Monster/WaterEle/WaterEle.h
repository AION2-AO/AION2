// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Monster/AOMonsterBase.h"
#include "WaterEle.generated.h"

/**
 * 
 */
UCLASS()
class AION2_API AWaterEle : public AAOMonsterBase
{
	GENERATED_BODY()
public:
	AWaterEle(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region Attribute Setting 
	virtual void InitAttributeSet() override;
#pragma endregion 

	UFUNCTION()
	void InNearSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnNearSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 물대포 발사
	void WaterCannon();

	// 투사체 발사 전에 타겟 플레이어를 바라보도록 하는 함수
	void TurnToTarget();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USphereComponent> NearDetectionSphere;
};
