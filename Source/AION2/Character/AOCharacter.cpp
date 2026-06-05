#include "Character/AOCharacter.h"

AAOCharacter::AAOCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAOCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAOCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAOCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

