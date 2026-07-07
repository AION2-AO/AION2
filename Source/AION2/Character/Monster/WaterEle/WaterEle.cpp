// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Monster/WaterEle/WaterEle.h"
#include "Components/SphereComponent.h"
#include "GAS/AttributeSet/AOAttributeSet.h"
#include "Character/AOCharacter.h"
#include "GAS/AOGameplayTags.h"

AWaterEle::AWaterEle(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	NearDetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("NearDetectionShpere"));
	NearDetectionSphere->SetupAttachment(RootComponent);
	NearDetectionSphere->SetSphereRadius(300.f);

	NearDetectionSphere->SetCollisionProfileName(TEXT("Trigger"));
}

void AWaterEle::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AWaterEle::BeginPlay()
{
	Super::BeginPlay();
	NearDetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AWaterEle::InNearSphereBeginOverlap);
	NearDetectionSphere->OnComponentEndOverlap.AddDynamic(this, &AWaterEle::OnNearSphereEndOverlap);
}

void AWaterEle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWaterEle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{

}

void AWaterEle::InNearSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority() == false) return;

	if (OtherActor && OtherActor != this && Cast<AAOCharacter>(OtherActor))
	{
		if (ASC = GetAbilitySystemComponent())
		{
			ASC->AddLooseGameplayTag(STATE_MONSTER_WE_CANNON);
		}
	}
}

void AWaterEle::OnNearSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}
void AWaterEle::WaterCannon()
{

}

void AWaterEle::InitAttributeSet()
{
	if (AttributeSet)
	{
		AttributeSet->InitHealth(700.f);
		AttributeSet->InitMaxHealth(700.f);
	}
}
