// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA/AT/Monster/AT_Monster_MoveToLocation.h"

#include "GameFramework/Actor.h"

UAT_Monster_MoveToLocation::UAT_Monster_MoveToLocation()
{
	bTickingTask = true;
	bSimulatedTask = false;
}

UAT_Monster_MoveToLocation* UAT_Monster_MoveToLocation::MoveToLocation(
	UGameplayAbility* OwningAbility,
	FVector InTargetLocation,
	float InDuration,
	bool bInSweepMovement,
	bool bInSnapToTargetOnEnd
)
{
	UAT_Monster_MoveToLocation* Task = NewAbilityTask<UAT_Monster_MoveToLocation>(OwningAbility);
	Task->TargetLocation = InTargetLocation;
	Task->Duration = InDuration;
	Task->bSweepMovement = bInSweepMovement;
	Task->bSnapToTargetOnEnd = bInSnapToTargetOnEnd;
	return Task;
}

void UAT_Monster_MoveToLocation::Activate()
{
	Super::Activate();

	AActor* AvatarActor = GetAvatarActor();
	if (!IsValid(AvatarActor))
	{
		EndTask();
		return;
	}

	StartLocation = AvatarActor->GetActorLocation();
	ElapsedTime = 0.0f;
	bMoveStarted = true;

	if (Duration <= KINDA_SMALL_NUMBER)
	{
		AvatarActor->SetActorLocation(TargetLocation, bSweepMovement);
		FinishMove();
	}
}

void UAT_Monster_MoveToLocation::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (bFinished || !bMoveStarted)
	{
		return;
	}

	AActor* AvatarActor = GetAvatarActor();
	if (!IsValid(AvatarActor))
	{
		EndTask();
		return;
	}

	ElapsedTime += DeltaTime;

	const float Alpha = FMath::Clamp(ElapsedTime / FMath::Max(Duration, KINDA_SMALL_NUMBER), 0.0f, 1.0f);
	const FVector NewLocation = FMath::Lerp(StartLocation, TargetLocation, Alpha);

	AvatarActor->SetActorLocation(NewLocation, bSweepMovement);

	if (Alpha >= 1.0f)
	{
		FinishMove();
	}
}

void UAT_Monster_MoveToLocation::OnDestroy(bool bInOwnerFinished)
{
	if (bInOwnerFinished && bSnapToTargetOnEnd && bMoveStarted && !bFinished)
	{
		if (AActor* AvatarActor = GetAvatarActor())
		{
			AvatarActor->SetActorLocation(TargetLocation, bSweepMovement);
		}
	}

	Super::OnDestroy(bInOwnerFinished);
}

void UAT_Monster_MoveToLocation::FinishMove()
{
	if (bFinished)
	{
		return;
	}

	bFinished = true;

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnFinished.Broadcast();
	}

	EndTask();
}
