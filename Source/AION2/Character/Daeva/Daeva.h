#pragma once

#include "CoreMinimal.h"
#include "Character/AOCharacter.h"
#include "InputActionValue.h"
#include "GameplayTagContainer.h"
#include "GAS/AttributeSet/AOAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "Daeva.generated.h"

class USkeletalMeshComponent;
class UInputAction;
class UGameplayEffect;

class UAOUserWidgetBase;
class UWidgetComponent;
class UAOWidgetComponentBase;
class USceneComponent;
class UMaterialInterface;

class AAOPlayerState;
class UAbilitySystemComponent;

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
	LB,
	RB,
	Key1,
	Key2,
	Key3,
	Key4,
	KeyQ,
	KeyE,
	Die,
	Rebirth
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
	LB_3,
	RB_1,
	RB_2,
	RB_3,
	Key1,
	Key2,
	Key3,
	Key4,
	KeyQ,
	KeyE
};

// UI: Player ASC�� �غ�Ǹ� bind
DECLARE_MULTICAST_DELEGATE_ThreeParams(
	FOnPlayerUIReady,
	AAOPlayerState*,
	UAbilitySystemComponent*,
	ADaeva*
);

UCLASS()
class AION2_API ADaeva : public AAOCharacter
{
	GENERATED_BODY()

public:
	ADaeva(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	void Tick_Camera(float DeltaTime);

public:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMontage(EMontageID MontageID, float PlayRate);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayWingMontage(EMontageID MontageID, float PlayRate);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetWingVisibility(bool NewVisible);

	UFUNCTION(Server, Reliable)
	void Server_SetCurrentTarget(AAOCharacter* NewTarget);

	UFUNCTION(Client, Unreliable)
	void Client_PlayCameraShake();

public:
	bool HasMoveInput();
	virtual void SearchTarget() override;
	virtual void TeleportBackToTarget() override;
	FRotator GetLookAtToTarget();
	void SetCameraByLookAt(const FRotator& LookAtRot);
	void ResetForDungeonRespawn();

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

protected:
	void BindMoveSpeedAttribute();
	void OnMoveSpeedChanged(const FOnAttributeChangeData& Data);

	FDelegateHandle MoveSpeedChangedDelegateHandle;
	bool bMoveSpeedDelegateRegistered = false;

protected:
	virtual void OnAttackSucceeded(const FAttackData& AttackData, AActor* HitActor, const FHitResult& HitResult, bool& bDidShakeCamera) override;
	virtual void TakeDamageAO(const FAttackData& AttackData, const FHitResult& HitResult, AAOCharacter* DamageCauser) override;

private:
	void InputSpacePressed();
	void InputLBPressed();
	void InputRBPressed();
	void InputMoveReleased();

protected:
	void OnCombatStateChanged(const FGameplayTag Tag, int32 NewCount);

public:
	virtual void HandleDeath();
	virtual void OnHealthChanged(const FOnAttributeChangeData& Data);

protected:
	FDelegateHandle HealthChangedDelegateHandle;

protected:
	void StartSprint();
	void StopSprint();
	void OnStaminaChangedForSprint(const FOnAttributeChangeData& Data);
	void InputShiftReleased();
	void InputShiftPressed();

	UPROPERTY(EditDefaultsOnly, Category = "Sprint")
	TSubclassOf<UGameplayEffect> SprintEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Sprint")
	TSubclassOf<UGameplayEffect> SprintDrainEffect;


	UFUNCTION(Server, Reliable)
	void ServerStartSprint();

	UFUNCTION(Server, Reliable)
	void ServerStopSprint();

	void RequestStartSprint();
	void RequestStopSprint();


	FActiveGameplayEffectHandle SprintEffectHandle;
	FActiveGameplayEffectHandle SprintDrainEffectHandle;

	FDelegateHandle SprintStaminaChangedDelegateHandle;

	//bool bSprintInputHeld = false;
	bool IsSprinting() const;
	bool bHasMoveInput = false;

private:
	void SetWeaponVisibility(bool NewVisible);
	void SetSubWeaponVisibility(bool NewVisible);
	void SetWingVisibility(bool NewVisible);

private:
	void CreatePart(EDaevaPartType PartType, const TCHAR* ComponentName);
	void PlayCameraShake(bool& bDidShakeCamera);
	bool IsFrontOfCamera(AActor* Other);
	float CalcDistanceSquaredToScreenCenter(AActor* Other);
	void ChangeCurrentTargetInClient(AAOCharacter* NewTarget);

private:
	// UI ����. Local Player�� ���� Head-up UI�� �߰��Ѵ�.
	void BindOverheadStatusWidget();

public:
	FORCEINLINE UAnimMontage* GetMontageByID(EMontageID Index) const { return Montages[Index]; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return Weapon; }
	FORCEINLINE USkeletalMeshComponent* GetSubWeaponMesh() const { return SubWeapon; }
	FORCEINLINE USkeletalMeshComponent* GetWingMesh() const { return Wing; }
	FORCEINLINE UAnimInstance* GetWingAnimInstance() const { return GetWingMesh()->GetAnimInstance(); }
	virtual TArray<USkeletalMeshComponent*> GetAllMeshes() override;

private:
	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	FVector CurrentMoveInputDirection;

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> Camera;

	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float ZoomSpeed = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float MinZoomDistance = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float MaxZoomDistance = 1200.f;

	float TargetZoomDistance;

public:
	// UI: Player ASC�� �غ�Ǹ� UI Bind.
	FOnPlayerUIReady OnPlayerUIReady;

	bool IsPlayerUIReady() const;
	void NotifyPlayerUIReady();

private:
	bool bPlayerUIReady = false;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ZoomAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ShiftAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SpaceAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LBAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> RBAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> Key1Action;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> Key2Action;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> Key3Action;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> Key4Action;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> KeyQAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> KeyEAction;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	TMap<EMontageID, TObjectPtr<UAnimMontage>> Montages;

	UPROPERTY(EditDefaultsOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	TMap<EMontageID, TObjectPtr<UAnimMontage>> WingMontages;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TMap<EDaevaPartType, TObjectPtr<USkeletalMeshComponent>> Parts;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Weapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> SubWeapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Wing;

private:
	UPROPERTY(EditDefaultsOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UDA_AbilitySet> CombatAbilitySet;

	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> CombatAbilityHandles;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> DashStaminaRegenEffect;

	bool bTagEventsRegistered = false;

	UPROPERTY(EditDefaultsOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UCameraShakeBase> CameraShakeClass;

private:
	AAOCharacter* PreviousTarget = nullptr;
	FTimerHandle TargetSearchTimer;

private:
	UPROPERTY(VisibleAnywhere, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAOWidgetComponentBase> OverheadStatusWidgetComponent;

	UPROPERTY(VisibleAnywhere, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> BillboardComponent;

	TObjectPtr<UMaterialInterface> WidgetMaterial;
};
