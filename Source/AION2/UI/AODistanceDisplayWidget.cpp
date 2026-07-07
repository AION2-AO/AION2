// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AODistanceDisplayWidget.h"
#include "Components/TextBlock.h"

void UAODistanceDisplayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Calculate every 0.1f
	DistanceUpdateElapsed += InDeltaTime;
	if (DistanceUpdateElapsed < DistanceUpdateInterval)
	{
		return;
	}

	// Initialize the tick.
	DistanceUpdateElapsed = 0.f;

	// Exception Handling.
	if (!TB_DistanceFromPlayer)
	{
		return;
	}


	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APawn* LocalPawn = PC ? PC->GetPawn() : nullptr;
	if (!IsValid(LocalPawn) || !IsValid(BoundAbilitySystemActor))
	{
		return;
	}

	// Calculate the distance.
	const float DistanceCm = FVector::Dist(
		LocalPawn->GetActorLocation(),
		BoundAbilitySystemActor->GetActorLocation()
	);

	// centimeter => meter
	const int32 DistanceM = FMath::RoundToInt(DistanceCm / 100.f);

	TB_DistanceFromPlayer->SetText(
		FText::FromString(FString::Printf(TEXT("%dM"), DistanceM)));
}
