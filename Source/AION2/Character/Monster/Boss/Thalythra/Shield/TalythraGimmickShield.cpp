// Fill out your copyright notice in the Description page of Project Settings.


#include "TalythraGimmickShield.h"
#include "Components/SphereComponent.h"
#include "Character/Daeva/Daeva.h"

// Sets default values
ATalythraGimmickShield::ATalythraGimmickShield()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));

	RootComponent = Collision; 

	// Replication 설정 
	bReplicates = true;					// 이 Actor를 클라이언트에도 만들어지도록 설정
	SetReplicateMovement(true);			// 이 Actor의 위치/회전/속도를 클라이언트에 동기화 설정
}
// Called when the game starts or when spawne

void ATalythraGimmickShield::BeginPlay()
{
	Super::BeginPlay();
	
	
	Collision->OnComponentBeginOverlap.AddDynamic(this, &ATalythraGimmickShield::OnShieldBeginOverlapEvent);
	Collision->OnComponentEndOverlap.AddDynamic(this, &ATalythraGimmickShield::OnShieldEndOverlapEvent);
}

// Called every frame
void ATalythraGimmickShield::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void ATalythraGimmickShield::OnShieldBeginOverlapEvent(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority() == false)
		return;


	// 플레이어가 소환한 다른 쉴드 충돌 무시
	if (ATalythraGimmickShield* OtherProj = Cast<ATalythraGimmickShield>(OtherActor))
	{
		return; 
	}

	AAOCharacter* HitCharacter = Cast<AAOCharacter>(OtherActor);
	if (HitCharacter == nullptr)
		return; 

	ADaeva* pPlayer = Cast<ADaeva>(HitCharacter);
	if (pPlayer == nullptr)
		return; 


	// 플레이어
	if (HitCharacter->IsDead() == false && pPlayer != nullptr)
	{
		pPlayer->Set_HasSheildColor(OrbColor);

	}

	
	// 반드시 패턴이 끝나고 삭제해 주어야함.
	//Destroy();

}



void ATalythraGimmickShield::OnShieldEndOverlapEvent(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (HasAuthority() == false)
		return;


	// 플레이어가 소환한 다른 쉴드 충돌 무시
	if (ATalythraGimmickShield* OtherProj = Cast<ATalythraGimmickShield>(OtherActor))
	{
		return;
	}

	AAOCharacter* HitCharacter = Cast<AAOCharacter>(OtherActor);
	if (HitCharacter == nullptr)
		return;


	ADaeva* pPlayer = Cast<ADaeva>(HitCharacter);
	if (pPlayer == nullptr)
		return;


	// 플레이어
	if (HitCharacter->IsDead() == false && pPlayer != nullptr)
	{
		pPlayer->Set_HasSheildColor(EOrbColor::None);

	}
}

