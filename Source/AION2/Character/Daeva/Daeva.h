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

UENUM(BlueprintType)
enum class EMontageID : uint8
{
	Dash UMETA(DisplayName = "Dash"),
	Glide UMETA(DisplayName = "Glide"),
	GlideLand UMETA(DisplayName = "GlideLand"),
	StopGlide UMETA(DisplayName = "StopGlide")
};

UENUM(BlueprintType)
enum class EAbilityInputID : uint8
{
	Dash UMETA(DisplayName = "Dash"),
	Jump UMETA(DisplayName = "Jump"),
	Glide UMETA(DisplayName = "Glide"),
	StopGlide UMETA(DisplayName = "StopGlide")
};

UCLASS()
class AION2_API ADaeva : public AAOCharacter
{
	GENERATED_BODY()

public:
	ADaeva(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayWingMontage(EMontageID MontageID, float PlayRate);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetWingVisibility(bool NewVisible);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void Tick_Camera(float DeltaTime);

protected:
	virtual void Move(const FInputActionValue& Value);
	virtual void Look(const FInputActionValue& Value);
	virtual void Zoom(const FInputActionValue& Value);

protected:
	virtual void InitGAS() override;
	void GASInputPressed(int32 InputId);
	void GASInputReleased(int32 InputId);

private:
	void InputShiftPressed();
	void InputSpacePressed();

private:
	void SetWingVisibility(bool NewVisible);

private:
	void CreatePart(EDaevaPartType PartType, const TCHAR* ComponentName);

public:
	FORCEINLINE UAnimMontage* GetMontageByAbilityInputID(EMontageID Index) const { return Montages[Index]; }
	FORCEINLINE USkeletalMeshComponent* GetWingMesh() const { return Wing; }
	FORCEINLINE UAnimInstance* GetWingAnimInstance() const { return GetWingMesh()->GetAnimInstance(); }

private:
	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	FVector CurrentMoveInputDirection;

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float ZoomSpeed = 100.f;

	UPROPERTY(EditAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float MinZoomDistance = 100.f;

	UPROPERTY(EditAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float MaxZoomDistance = 1000.f;

	float TargetZoomDistance;

private:
	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ZoomAction;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ShiftAction;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SpaceAction;

private:
	UPROPERTY(EditAnywhere, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	TMap<EMontageID, TObjectPtr<UAnimMontage>> Montages;

	UPROPERTY(EditAnywhere, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	TMap<EMontageID, TObjectPtr<UAnimMontage>> WingMontages;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TMap<EDaevaPartType, TObjectPtr<USkeletalMeshComponent>> Parts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Wing;
};
