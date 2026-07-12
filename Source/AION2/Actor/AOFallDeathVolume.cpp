#include "Actor/AOFallDeathVolume.h"

#include "Components/BoxComponent.h"
#include "Character/Daeva/Daeva.h"
#include "Game/AODungeonGameMode.h"
#include "GameFramework/PlayerController.h"

AAOFallDeathVolume::AAOFallDeathVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	SetRootComponent(Box);

	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionObjectType(ECC_WorldStatic);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Box->SetBoxExtent(FVector(5000.f, 5000.f, 200.f));
}

void AAOFallDeathVolume::BeginPlay()
{
	Super::BeginPlay();

	Box->OnComponentBeginOverlap.AddDynamic(this, &AAOFallDeathVolume::OnBoxBeginOverlap);
}

void AAOFallDeathVolume::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,	const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	ADaeva* Daeva = Cast<ADaeva>(OtherActor);
	if (!Daeva)
	{
		return;
	}

	if (Daeva->IsDead())
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(Daeva->GetController());
	if (!PC)
	{
		return;
	}

	AAODungeonGameMode* DungeonGM = GetWorld()->GetAuthGameMode<AAODungeonGameMode>();
	if (!DungeonGM)
	{
		return;
	}


	UE_LOG(LogTemp, Error, TEXT("[FallDeathVolume] Fall death detected: %s"), *Daeva->GetName());
	Daeva->HandleDeath(EDeathReason::Fall);
}