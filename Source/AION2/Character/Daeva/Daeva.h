#pragma once

#include "CoreMinimal.h"
#include "Character/AOCharacter.h"
#include "InputActionValue.h"
#include "Daeva.generated.h"

class USkeletalMeshComponent;
class UInputAction;

UENUM(BlueprintType)
enum class EDaevaPartType : uint8
{
	Helmet,
	Hair,
	Head,
	Shoulder,
	Cape,
	Body,
	Glove,
	Pants,
	Boots
};

UCLASS()
class AION2_API ADaeva : public AAOCharacter
{
	GENERATED_BODY()

public:
	ADaeva();

private:
	void CreatePart(EDaevaPartType PartType, const TCHAR* ComponentName);

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void Move(const FInputActionValue& Value);
	virtual void Look(const FInputActionValue& Value);

private:
	bool bHasCurrentMoveInput = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	FVector CurrentMoveInputDirection;

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> Camera;

private:
	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TMap<EDaevaPartType, TObjectPtr<USkeletalMeshComponent>> Parts;
};
