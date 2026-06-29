// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/AODunzeonEntrance.h"
#include "Blueprint/UserWidget.h"
#include "Character/Daeva/Daeva.h"
#include "Components/BoxComponent.h"
#include "Player/AOPlayerController.h"
#include "Game/AOGameInstance.h"
#include "Manager/AOUIManager.h"
#include "Character/ServerCharacter/MMODaeva.h"

// Sets default values
AAODunzeonEntrance::AAODunzeonEntrance()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBoxComponent"));

	if (TriggerBox)
	{
		TriggerBox->SetBoxExtent(FVector(40.f, 30.f, 50.f));
		TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	}

	EffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EffectComponent"));
	EffectComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AAODunzeonEntrance::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AAODunzeonEntrance::OnOverlapBegin);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AAODunzeonEntrance::OnOverlapEnd);

	RootComponent = TriggerBox; // Root 지정은 생성자에서 하는 것이 안정적임

	if (DunzeonEffect && EffectComponent)
	{
		EffectComponent->SetAsset(DunzeonEffect);
		EffectComponent->Activate();
	}
	UIManager = GetGameInstance()->GetSubsystem<UAOUIManager>();
}

// Called every frame
void AAODunzeonEntrance::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAODunzeonEntrance::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<ADaeva>(OtherActor))
	{
		UE_LOG(LogTemp, Log, TEXT("Dunzeon OnOverlapBegin"));
		PC = Cast<AAOPlayerController>(GetWorld()->GetFirstPlayerController());
		if (PC)
		{
			DunzeonWaitingRoomWidget = UIManager->ShowWidget(DunzeonWaitingRoomClass, EUILayer::PopUp);
			EnableInput(PC);

			if (InputComponent && DunzeonWaitingRoomWidget)
			{
				OverlappedPlayer = OtherActor;
				InputComponent->BindKey(EKeys::F, IE_Pressed, this, &AAODunzeonEntrance::EnterDunzeonWaitingRoom);
			}
		}
	}
}

void AAODunzeonEntrance::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<ADaeva>(OtherActor))
	{
		if (PC)
		{
			DisableInput(PC);
			UIManager->HideWidget(DunzeonWaitingRoomWidget);
		}
	}
}

void AAODunzeonEntrance::EnterDunzeon()
{


}

void AAODunzeonEntrance::EnterDunzeonWaitingRoom()
{
	UIManager->HideWidget(DunzeonWaitingRoomWidget);

	DunzeonRoomWidget = UIManager->ShowWidget(DunzeonRoomClass, EUILayer::System);
	if (PC)
	{
		PC->SetShowMouseCursor(true);
		AMMODaeva* Owner = Cast<AMMODaeva>( PC->GetPawn());
		if (Owner)
		{
			Owner->SendDungeonWait();
		}
	}
}


