// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/CombatInterface.h"
#include "Components/TimelineComponent.h"
#include "WaveCircle.generated.h"

UCLASS()
class AION2_API AWaveCircle : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AWaveCircle();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override; 

    void PlayRingPulse();

    UFUNCTION()
    void UpdateRingScale(float Alpha);

    UFUNCTION()
    void OnRingPulseFinished(); 



    UFUNCTION()
    void CheckRingHit();




private:
    UPROPERTY(EditAnywhere, Category = "AttackData", meta = (AllowPrivateAccess = "true"))
    FAttackData AttackData;

    // 도넛 링 매쉬 ( 스케일 대상 )
    UPROPERTY(EditAnywhere, Category = "WaveCircle", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> RingMesh; 


    // 커졌을 때 최대 스케일
    UPROPERTY(EditAnywhere, Category = "WaveCircle|Pulse", meta = (AllowPrivateAccess = "true"))
    float MaxRingScale = 10.f; 

    // 원래(기준) 스케일
    UPROPERTY(EditAnywhere, Category = "WaveCircle|Pulse", meta = (AllowPrivateAccess = "true"))
    float BaseRingScale = 1.f;

    // 0->1->0 형태의 왕복 커브 ( Ease In/Out 권장 )
    UPROPERTY(EditAnywhere, Category = "WaveCircle|Pulse", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCurveFloat> RingPulseCurve; 

    // 0->1->0 으로 진행되는 PlayRate 조절 
    UPROPERTY(EditAnywhere, Category = "WaveCircle|Pulse", meta = (AllowPrivateAccess = "true"))
    float PulsePlayRate = 0.5f;

    // 펄스 끝나면 자동 소멸
    UPROPERTY(EditAnywhere, Category = "WaveCircle|Pulse", meta = (AllowPrivateAccess = "true"))
    bool DestroyOnFinished = true; 

    FTimeline RingPulseTimeline; 

    UPROPERTY(EditAnywhere, Category = "WaveCircle|Pulse", meta = (AllowPrivateAccess = "true"))
    float BaseMeshRadius; 


    UPROPERTY(EditAnywhere, Category = "WaveCircle|Pulse", meta = (AllowPrivateAccess = "true"))
    float BaseRingRadius = 175.f; // 175.f


    UPROPERTY(EditAnywhere, Category = "WaveCircle|Pulse", meta = (AllowPrivateAccess = "true"))
    float RingCollisionHeight = 100.f; // 175.f


    TArray<TObjectPtr<AActor>> AlreadyHit;
};



//UFUNCTION()
 //void OnWaveRingOverlapEvent(
 //    UPrimitiveComponent* OverlappedComp,
 //    AActor* OtherActor,
 //    UPrimitiveComponent* OtherComp,
 //    int32 OtherBodyIndex,
 //    bool bFromSweep,
 //    const FHitResult& SweepResult
 //);


 //UFUNCTION()
 //void OnProjecHit(
 //    UPrimitiveComponent* HitComp,
 //    AActor* OtherActor,
 //    UPrimitiveComponent* OtherComp,
 //    FVector NormalImpulse,
 //    const FHitResult& Hit);