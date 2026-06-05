#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AOCharacter.generated.h"

UCLASS()
class AION2_API AAOCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAOCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
