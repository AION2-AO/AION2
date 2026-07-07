#pragma once

#include "CoreMinimal.h"
#include "Character/AOCharacter.h"
#include "InputActionValue.h"
#include "GameplayTagContainer.h"
#include "GAS/AttributeSet/AOAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "GenericTeamAgentInterface.h"
#include "Types/AOTypes.h"
#include "Daeva.generated.h"

class USkeletalMeshComponent;
class UInputAction;
class UGameplayEffect;

class UAOUserWidgetBase;
class UWidgetComponent;
class UAOWidgetComponentBase;

class AAOPlayerState;
class UAbilitySystemComponent;
class UAOQuickSlotComponent;
class UAOPlayerHUDWidget;

class UDA_AbilitySet;

// 낙사 처리용.
UENUM(BlueprintType)
enum class EDeathReason : uint8
{
	Normal,
	Fall
};

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
	GlideDash,
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
	KeyE,
	GlideDash
};

/*
* UI: Notify Player UI Ready
* Not using anymore in the HUD Logic
* but leaving it in case someone is using it.
*/
DECLARE_MULTICAST_DELEGATE_ThreeParams(
	FOnPlayerUIReady,
	AAOPlayerState*,
	UAbilitySystemComponent*,
	ADaeva*
);

UCLASS()
class AION2_API ADaeva : public AAOCharacter , public IGenericTeamAgentInterface
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


	/* SeonHwan */
	virtual FGenericTeamId GetGenericTeamId() const override { return FGenericTeamId(TeamID); }
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override { TeamID = NewTeamID.GetId(); }

public:
	// SuYeon: Daeva should deliver own ability set.
	const UDA_AbilitySet* GetCombatAbilitySet() const { return CombatAbilitySet; }

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

	void InputXPressed();
	void InputBPressed();

protected:
	void OnCombatStateChanged(const FGameplayTag Tag, int32 NewCount);
	void OnRebirthMontageEnded(UAnimMontage* Montage, bool bInterrupted);

public:
	virtual void HandleDeath(EDeathReason DeathReason = EDeathReason::Normal);
	virtual void OnHealthChanged(const FOnAttributeChangeData& Data);

	UFUNCTION(Exec)
	void TestSetHealth(float NewHealth);

	// SeonHwan 추가 
	void EatOrb(EOrbColor NewColor);

protected:
	FDelegateHandle HealthChangedDelegateHandle;

protected:
	void StartSprint();
	void StopSprint();
	void OnStaminaChangedForSprint(const FOnAttributeChangeData& Data);
	void InputShiftReleased();
	void InputShiftPressed();

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Sprint")
	TSubclassOf<UGameplayEffect> SprintEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Sprint")
	TSubclassOf<UGameplayEffect> SprintDrainEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Mana")
	TSubclassOf<UGameplayEffect> ManaRegenEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Mana")
	TSubclassOf<UGameplayEffect> HitManaRegenEffect;

	// Seohwan ( aicontroller에서 적 및 동료 판별 기준 ) 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	uint8 TeamID = 1;

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

public:
	void SetWingVisibilityOnServer(bool NewVisible);
	UFUNCTION()
	void OnRep_WingVisible();

	//H.Y (PlayerState 복원).
public :
	void RestorePlayerInfoFromPlayerState();

	virtual void FellOutOfWorld(const UDamageType& DmgType) override; // 낙사 처리.

	//

private:
	void CreatePart(EDaevaPartType PartType, const TCHAR* ComponentName);
	void PlayCameraShake(bool& bDidShakeCamera);
	bool IsFrontOfCamera(AActor* Other);
	float CalcDistanceSquaredToScreenCenter(AActor* Other);
	void ChangeCurrentTargetInClient(AAOCharacter* NewTarget);

public:
	/*
	* SuYeon: Only Local Player Floats Head-up UI.
	* Public because it is called from player controller
	* after the playercontroller finds all of the Daeva's ASC condition is ready.
	*/
	void BindOverheadStatusWidget();

public:
	FORCEINLINE UAnimMontage* GetMontageByID(EMontageID Index) const { return Montages[Index]; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return Weapon; }
	FORCEINLINE USkeletalMeshComponent* GetSubWeaponMesh() const { return SubWeapon; }
	FORCEINLINE USkeletalMeshComponent* GetWingMesh() const { return Wing; }
	FORCEINLINE UAnimInstance* GetWingAnimInstance() const { return GetWingMesh()->GetAnimInstance(); }
	virtual TArray<USkeletalMeshComponent*> GetAllMeshes() override;
	FORCEINLINE UAOQuickSlotComponent* GetQuickSlotComponent() const { return QuickSlotComponent; }

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
	float MaxZoomDistance = 1500.f;

	UPROPERTY(EditDefaultsOnly, Category ="GAS|Mana")
	float HitManaRegenAmount = 5.f;

	float TargetZoomDistance;

public:
	/*
	* UI: Notify Player UI Ready
	* Not using anymore in the HUD Logic
	* but leaving it in case someone is using it.
	*/
	FOnPlayerUIReady OnPlayerUIReady;

	/*
	* UI: Notify Player UI Ready
	* Not using anymore in the HUD Logic
	* but leaving it in case someone is using it.
	*/
	bool IsPlayerUIReady() const;
	
	/*
	* UI: Notify Player UI Ready
	* Not using anymore in the HUD Logic
	* but leaving it in case someone is using it.
	*/
	void NotifyPlayerUIReady();

public:
	uint64 GetMy() { return MyId; }
	void SetMyId(uint64 Id);
	void SetMyClass(uint8 ClassType);
	void SetMyName(FString InName);

	void SendHp(float NewHp);
	void SendItem(int32 SlotIndex);
	void SetItemUse();
private:
	bool bPlayerUIReady = false;

	FTimerHandle ItemCoolTimeHandler;
	float ItemCoolTime = 5.0f;

	bool bCanUseItem = true;

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

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> KeyXAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> KeyBAction;

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

	UPROPERTY(ReplicatedUsing = OnRep_WingVisible)
	bool bWingVisible = false;

private:
	UPROPERTY(EditDefaultsOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UDA_AbilitySet> CombatAbilitySet;

	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> CombatAbilityHandles;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> DashStaminaRegenEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> StateCombatApplyEffect;

	bool bTagEventsRegistered = false;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UCameraShakeBase> CameraShakeClass;

	AAOCharacter* PreviousTarget = nullptr;
	FTimerHandle TargetSearchTimer;

private:
	UPROPERTY(VisibleAnywhere, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAOWidgetComponentBase> OverheadStatusWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QuickSlot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAOQuickSlotComponent> QuickSlotComponent;

protected:
	
	uint64 MyId = -1;

	// Seonhwan 여기서 데바의 색깔 구슬 카운트 하기  
private:
	UPROPERTY()
	EOrbColor LastOrbColor = EOrbColor::None;

	UPROPERTY()
	int8 OrbStack = 0;

	UPROPERTY(EditAnywhere, Category = "Orb", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> PlayerOrb;

	UPROPERTY(EditAnywhere, Category = "Orb", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UNiagaraComponent> BlueOrb;

	UPROPERTY(EditAnywhere, Category = "Orb", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UNiagaraComponent> PurpleOrb;


	UPROPERTY(EditAnywhere, Category = "AOE", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> PlayerAoeField;

	UPROPERTY(EditAnywhere, Category = "AOE", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> AoeField;


	UPROPERTY(EditAnywhere, Category = "OrbGimmickColor", meta = (AllowPrivateAccess = "true"))
	EOrbColor HasShieldColor;


public:
	UFUNCTION(NetMulticast, Unreliable)
	void Set_BlueOrb_RenderOnOff(bool _bOnOff);

	UFUNCTION(NetMulticast, Unreliable)
	void Set_PurpleOrb_RenderOnOff(bool _bOnOff);

	UFUNCTION(NetMulticast, Unreliable)
	void Set_AOE_RenderOnOff(bool _bOnOff);

	void Reset_OrbStackAndColor();
	void Set_HasSheildColor(EOrbColor _OrbShieldColor) { HasShieldColor = _OrbShieldColor; }

	EOrbColor Get_LastOrbColor() { return LastOrbColor; }
	EOrbColor Get_CurrentDaevaHasSheildColor() { return HasShieldColor; }

	int8 Get_OrbStack() { return OrbStack; }

private:
	/* SuYeon */
	/*
	* 이미 성공 처리된 상태(같은 Daeva의 ASC가 Bound됨)을 체크하기 위해 추가
	* 원래 WidgetcomponentBase에서 해줘야 하는데 일단 구현 성공부터 보기 위해 추가했음
	*/
	TWeakObjectPtr<UAbilitySystemComponent> BoundOverheadStatusASC;

	// 혹시 몰라서 WidgetComponent의 instnace도 비교하도록 추가. (방금 생성된 새로운 개체일 수 있음)
	TWeakObjectPtr<UAOPlayerHUDWidget> BoundOverheadStatusWidget;


private:
	// OverHeadWidget, BottomStatusHUD의 Pawn Ready Tick 재시도 횟수 Count.
	int32 PawnASCBindRetryCount = 0;

	// 일단 넉넉하게 180 => 3초로 잡기. 잘 되면 점점 줄여서 60을 목표로.
	int32 PawnASCBindMaxRetryCount = 180;

};
