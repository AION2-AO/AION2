// Fill out your copyright notice in the Description page of Project Settings.

#include "TalythraCollectibleOrb.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Character/AOCharacter.h"
#include "Character/Monster/Boss/Thalythra/Talythra.h"
#include "GAS/AttributeSet/AOAttributeSet.h"
#include "Character/Daeva/Daeva.h"


// Sets default values
ATalythraCollectibleOrb::ATalythraCollectibleOrb()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));

	RootComponent = Collision;


	// 네트워크 보간 설정
	ProjectileMovement->bInterpMovement = true;          // 이동 보간 설정 
	ProjectileMovement->bInterpRotation = true;          // 회전 보간 설정
	ProjectileMovement->InterpLocationTime = 0.05f;      // Min Net Update(30Hz) 기준 안전값
	ProjectileMovement->InterpRotationTime = 0.025f;     // 회전 보간 시간 설정
	ProjectileMovement->InterpLocationMaxLagDistance = 300.f; // 일정이상 거리 멀어지면 보간 포기하고 스냅하는 설정 


	// Replication 설정 
	bReplicates = true;					// 이 Actor를 클라이언트에도 만들어지도록 설정
	SetReplicateMovement(true);			// 이 Actor의 위치/회전/속도를 클라이언트에 동기화 설정
	SetNetUpdateFrequency(100.f);		// 초당 최대 100번까지 클라에게 패킷을 보내라.
	SetMinNetUpdateFrequency(60.f);     // 변화가 없거나 우선순위가 낮아도 초당 최소 30번은 체크 보장해라. 




}

// Called when the game starts or when spawned
void ATalythraCollectibleOrb::BeginPlay()
{
	Super::BeginPlay();


	Collision->OnComponentBeginOverlap.AddDynamic(this, &ATalythraCollectibleOrb::OnProjectileOverlapEvent);
}

// Called every frame
void ATalythraCollectibleOrb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void ATalythraCollectibleOrb::OnProjectileOverlapEvent(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority() == false)
		return;

	if (Cast<ATalythraCollectibleOrb>(OtherActor))
		return;  // 투사체끼리 무시

	AAOCharacter* HitCharacter = Cast<AAOCharacter>(OtherActor);
	if (HitCharacter == nullptr)
		return;  // 캐릭터 아니면 무시 (Destroy 안 함)

	// 보스
	if (HitCharacter == GetInstigator())
	{
		if (ATalythra* pTalythra = Cast<ATalythra>(HitCharacter))
		{
			float AttackPower = pTalythra->GetAttributeSet()->GetAttackPower() + 2.f;
			pTalythra->GetAttributeSet()->SetAttackPower(AttackPower);
		}
	}
	// 플레이어
	else if (ADaeva* pPlayer = Cast<ADaeva>(HitCharacter))
	{
		if (!pPlayer->IsDead())
		{
			if (ATalythra* pTalythra = Cast<ATalythra>(GetInstigator()))
			{
				pTalythra->Add_OrbHittedDaeva(pPlayer);
				pPlayer->EatOrb(OrbColor);
			}
		}
	}

	Destroy();
}




void ATalythraCollectibleOrb::InitVelocityAndDirection(const FVector Direction)
{
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity =
			Direction.GetSafeNormal() * ProjectileMovement->InitialSpeed;
	}
}