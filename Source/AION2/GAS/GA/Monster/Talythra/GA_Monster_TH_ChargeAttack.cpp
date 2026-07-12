// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Monster_TH_ChargeAttack.h"
#include "Components/CapsuleComponent.h"
#include "Character/Monster/AOMonsterBase.h"
#include "Physics/Collision.h"
#include "Engine/OverlapResult.h"
#include "Character/Daeva/Daeva.h"
#include "Character/AOCharacter.h"

void UGA_Monster_TH_ChargeAttack::OnHitCheckBegin(FGameplayEventData Payload)
{
    if (HasAuthority(&CurrentActivationInfo) == false)
        return;


    AAOMonsterBase* Monster = CastChecked<AAOMonsterBase>(GetAvatarActorFromActorInfo());

    if (UCapsuleComponent* Capsule = Monster->GetCapsuleComponent())
    {
        Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    }


    HitActors.Reset();


    if (ChargeHitTickTask)
    {
        ChargeHitTickTask->EndTask();
        ChargeHitTickTask = nullptr;
    }

    PrevLocation = GetAvatarActorFromActorInfo()->GetActorLocation(); 

    ChargeHitTickTask = UAT_Monster_ChargeHitTick::CreateChargeHitTick(this);
    ChargeHitTickTask->OnTick.AddDynamic(this, &UGA_Monster_TH_ChargeAttack::PerformChargeHitCheck);
    ChargeHitTickTask->ReadyForActivation();

}

void UGA_Monster_TH_ChargeAttack::OnHitCheckEnd(FGameplayEventData Payload)
{
    if (HasAuthority(&CurrentActivationInfo) == false)
        return; 


    AAOMonsterBase* Monster = CastChecked<AAOMonsterBase>(GetAvatarActorFromActorInfo());

    if (UCapsuleComponent* Capsule = Monster->GetCapsuleComponent())
    {
        Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    }

    if(ChargeHitTickTask)
    {
        ChargeHitTickTask->EndTask(); 
        ChargeHitTickTask = nullptr; 
    }

    HitActors.Reset(); 
}


void UGA_Monster_TH_ChargeAttack::PerformChargeHitCheck()
{
    // 서버 권한에서만
    if (HasAuthority(&CurrentActivationInfo) == false)
        return;

    AAOMonsterBase* Monster = Cast<AAOMonsterBase>(GetAvatarActorFromActorInfo());
    if (Monster == nullptr)
        return;

    UCapsuleComponent* Capsule = Monster->GetCapsuleComponent();
    if (Capsule == nullptr)
        return;

    // 검사 위치/회전
    const FVector  Location = Monster->GetActorLocation();
    const FQuat    Rotation = Monster->GetActorQuat();
    const float    HalfHeight = Capsule->GetScaledCapsuleHalfHeight();

    // 검사 모양 — 확장된 반지름 사용
    const FCollisionShape Shape =
        FCollisionShape::MakeCapsule(ChargeHitRadius, HalfHeight);

    // 쿼리 옵션
    FCollisionQueryParams Params(SCENE_QUERY_STAT(AttackTrace), false);
    Params.bTraceComplex = false;
    Params.AddIgnoredActor(Monster);

    // 판정 실행
    TArray<FHitResult> Hits;
    const FVector Start = PrevLocation;   // 지난 틱 위치를 멤버로 저장해둬야 함
    const FVector End = Location;

    PrevLocation = Location; // 다음 틱을 위해 갱신 

    const bool bHit = GetWorld()->SweepMultiByChannel(
        Hits, Start, End, Rotation, CCHANNEL_ATTACK, Shape, Params);

//#if ENABLE_DRAW_DEBUG
//    // 스윕 시작/끝 캡슐
//    const FColor SweepColor = bHit ? FColor::Green : FColor::Red;
//    DrawDebugCapsule(GetWorld(), Start, HalfHeight, ChargeHitRadius, Rotation,
//        SweepColor, false, 1.0f, 0, 1.0f);
//    DrawDebugCapsule(GetWorld(), End, HalfHeight, ChargeHitRadius, Rotation,
//        SweepColor, false, 1.0f, 0, 1.0f);
//    // 이동 경로 선
//    DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 1.0f, 0, 1.0f);
//#endif

    for (const FHitResult& Hit : Hits)
    {
        AActor* Target = Hit.GetActor();
        ADaeva* pDaeva = Cast<ADaeva>(Target);
        if (pDaeva == nullptr) 
            continue;

        if (IsVaildTarget(pDaeva) == false) 
            continue;

        if (HitActors.Contains(Target)) 
            continue;

        HitActors.Add(Target);

  
       

        bool camerashake = true; 
        Monster->OnAttackSucceeded(AttackData, pDaeva, Hit, camerashake);  // 진짜 HitResult
    }
}