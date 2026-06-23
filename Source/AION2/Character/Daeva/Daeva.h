#pragma once

#include "CoreMinimal.h"
#include "Character/AOCharacter.h"
#include "InputActionValue.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpecHandle.h"
#include "Daeva.generated.h"

class USkeletalMeshComponent;
class UInputAction;
class UGameplayEffect;

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
	Dash,
	CombatDash,
	Glide,
	GlideLand,
	StopGlide,
	LB
};

UENUM(BlueprintType)
enum class EAbilityID : uint8
{
	Dash,
	Jump,
	Glide,
	StopGlide,
	LB_1,
	LB_2,
	LB_3
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
	virtual void UnPossessed() override;
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
	virtual void ClearGAS() override;
	void GASInputPressed(int32 InputId);
	void GASInputReleased(int32 InputId);

	virtual void ApplyDashStaminaRegenEffect();

private:
	void InputShiftPressed();
	void InputSpacePressed();
	void InputLBPressed();

protected:
	void OnCombatStateChanged(const FGameplayTag Tag, int32 NewCount);

private:
	void SetWeaponVisibility(bool NewVisible);
	void SetSubWeaponVisibility(bool NewVisible);
	void SetWingVisibility(bool NewVisible);

private:
	void CreatePart(EDaevaPartType PartType, const TCHAR* ComponentName);

public:
	FORCEINLINE UAnimMontage* GetMontageByID(EMontageID Index) const { return Montages[Index]; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return Weapon; }
	FORCEINLINE USkeletalMeshComponent* GetSubWeaponMesh() const { return SubWeapon; }
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
	float MaxZoomDistance = 1200.f;

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

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LBAction;

private:
	UPROPERTY(EditAnywhere, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	TMap<EMontageID, TObjectPtr<UAnimMontage>> Montages;

	UPROPERTY(EditAnywhere, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	TMap<EMontageID, TObjectPtr<UAnimMontage>> WingMontages;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TMap<EDaevaPartType, TObjectPtr<USkeletalMeshComponent>> Parts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> SubWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Wing;

private:
	UPROPERTY(EditDefaultsOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UDA_AbilitySet> CombatAbilitySet;

	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> CombatAbilityHandles;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> DashStaminaRegenEffect;

	bool bTagEventsRegistered = false;
};
