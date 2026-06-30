// Fill out your copyright notice in the Description page of Project Settings.


#include "AOStoreEntrance.h"
#include "Character/Daeva/Daeva.h"
#include "Manager/AOUIManager.h"
#include  "Components/BoxComponent.h"
#include "Player/AOPlayerController.h"

// Sets default values
AAOStoreEntrance::AAOStoreEntrance()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBoxComponent"));

	if (TriggerBox)
	{
		TriggerBox->SetBoxExtent(FVector(40.f, 30.f, 50.f));
		TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	}
}

// Called when the game starts or when spawnedd
void AAOStoreEntrance::BeginPlay()
{
	Super::BeginPlay();
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AAOStoreEntrance::OnOverlapBegin);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AAOStoreEntrance::OnOverlapEnd);

	RootComponent = TriggerBox;
	UIManager = GetGameInstance()->GetSubsystem<UAOUIManager>();
}

// Called every frame
void AAOStoreEntrance::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAOStoreEntrance::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
                                      class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                      const FHitResult& SweepResult)
{
	if (Cast<ADaeva>(OtherActor))
	{
		UE_LOG(LogTemp, Log, TEXT("Store OnOverlapBegin"));
		PC = Cast<AAOPlayerController>(GetWorld()->GetFirstPlayerController());
		if (PC)
		{
			StorePopUpWidget = UIManager->ShowWidget(StorePopUpClass, EUILayer::PopUp);
			EnableInput(PC);

			if (InputComponent)
			{
				if (StorePopUpWidget)
					InputComponent->BindKey(EKeys::F, IE_Pressed, this, &AAOStoreEntrance::EnterStore);
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("No StorePopUpClass"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("No InputComponent"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No Player Controller"));
		}
	}
}

void AAOStoreEntrance::OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
                                    class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (StorePopUpWidget)
	{
		UIManager->HideWidget(StorePopUpWidget);
	}
	if (StoreWidget)
	{
		UIManager->HideWidget(StoreWidget);
	}

	PC->SetShowMouseCursor(false);
	DisableInput(PC);
}

void AAOStoreEntrance::EnterStore()
{
	UIManager->HideWidget(StorePopUpWidget);

	StoreWidget = UIManager->ShowWidget(StoreWidgetClass, EUILayer::System);
	if (PC)
	{
		PC->CurrentMouseCursor = EMouseCursor::Default;
		PC->SetShowMouseCursor(true);
	}
}
