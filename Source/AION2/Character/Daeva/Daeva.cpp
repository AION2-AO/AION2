#include "Character/Daeva/Daeva.h"
#include "Player/AOPlayerState.h"
#include "GAS/AOGameplayTags.h"
#include "Character/AOCharacterMovementComponent.h"
#include "Data/DA_AbilitySet.h"
#include "Physics/Collision.h"
#include "Player/AOPlayerController.h"

#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Game/AODungeonGameMode.h"

#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "GAS/AttributeSet/AOAttributeSet.h"


#include "UI/AOWidgetComponentBase.h"
#include "UI/AOPlayerHUDWidget.h"
#include "Components/WidgetComponent.h"

const float TargetTraceRadius = 3500.0f;

ADaeva::ADaeva(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->JumpZVelocity = 520.0f;

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -90.0f), FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetVisibility(false);
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 500.0f;
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 75.0f));
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraLag = true;
	SpringArm->bEnableCameraRotationLag = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	CreatePart(EDaevaPartType::Helmet, TEXT("HelmetPart"));
	CreatePart(EDaevaPartType::Hair, TEXT("HairPart"));
	CreatePart(EDaevaPartType::Head, TEXT("HeadPart"));
	CreatePart(EDaevaPartType::Shoulder, TEXT("ShoulderPart"));
	CreatePart(EDaevaPartType::Cape, TEXT("CapePart"));
	CreatePart(EDaevaPartType::Body, TEXT("BodyPart"));
	CreatePart(EDaevaPartType::Glove, TEXT("GlovePart"));
	CreatePart(EDaevaPartType::Pants, TEXT("PantsPart"));
	CreatePart(EDaevaPartType::Boots, TEXT("BootsPart"));

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon"));
	Weapon->SetVisibility(false);

	SubWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SubWeapon"));
	SubWeapon->SetVisibility(false);

	Wing = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Wing"));
	Wing->SetupAttachment(GetMesh(), TEXT("Wing_Root"));
	Wing->SetVisibility(false);

	// Head-up UI
	OverheadStatusWidgetComponent = CreateDefaultSubobject<UAOWidgetComponentBase>(TEXT("OverheadStatusWidget"));
	OverheadStatusWidgetComponent->SetupAttachment(RootComponent);
	OverheadStatusWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	OverheadStatusWidgetComponent->SetBlendMode(EWidgetBlendMode::Transparent);
	OverheadStatusWidgetComponent->SetDrawSize(FVector2D(80.0f, 14.0f));
	OverheadStatusWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 130.0f));
	OverheadStatusWidgetComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 180.0f));
	OverheadStatusWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FClassFinder<UUserWidget>
		WidgetClass(
			TEXT("/Game/UI/Ingame/WBP_PlayaerStatus_Head.WBP_PlayaerStatus_Head_C"));

	if (WidgetClass.Succeeded())
	{
		OverheadStatusWidgetComponent->SetWidgetClass(
			WidgetClass.Class);
	}
}

void ADaeva::BeginPlay()
{
	Super::BeginPlay();

	TargetZoomDistance = SpringArm->TargetArmLength;
	GetWorldTimerManager().SetTimer(TargetSearchTimer, this, &ThisClass::SearchTarget, 0.25f, true);
}

void ADaeva::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Tick_Camera(DeltaTime);
}

void ADaeva::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitGAS();

	// LocalController일 때만 UI 만들도록 설정
	if (AAOPlayerController* AOController = Cast<AAOPlayerController>(NewController))
	{
		if (AOController->IsLocalController())
		{
			AOController->HandlePawnASCReady();
		}
	}

	BindOverheadStatusWidget();
}

void ADaeva::UnPossessed()
{
	ClearGAS();

	Super::UnPossessed();
}

void ADaeva::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitGAS();

	// LocalController일 때만 UI 만들도록 설정
	if (AAOPlayerController* AOController = Cast<AAOPlayerController>(GetController()))
	{
		if (AOController->IsLocalController())
		{
			AOController->HandlePawnASCReady();
		}
	}

	BindOverheadStatusWidget();
}

void ADaeva::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ADaeva::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADaeva::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADaeva::Look);
		EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ADaeva::Zoom);
		//EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &ADaeva::GASInputPressed, static_cast<int32>(EAbilityID::Dash));
		EnhancedInputComponent->BindAction(SpaceAction, ETriggerEvent::Started, this, &ADaeva::InputSpacePressed);
		EnhancedInputComponent->BindAction(LBAction, ETriggerEvent::Triggered, this, &ADaeva::InputLBPressed);
		EnhancedInputComponent->BindAction(RBAction, ETriggerEvent::Triggered, this, &ADaeva::InputRBPressed);
		EnhancedInputComponent->BindAction(Key1Action, ETriggerEvent::Triggered, this, &ADaeva::GASInputPressed, static_cast<int32>(EAbilityID::Key1));
		EnhancedInputComponent->BindAction(Key2Action, ETriggerEvent::Triggered, this, &ADaeva::GASInputPressed, static_cast<int32>(EAbilityID::Key2));
		EnhancedInputComponent->BindAction(Key3Action, ETriggerEvent::Triggered, this, &ADaeva::GASInputPressed, static_cast<int32>(EAbilityID::Key3));
		EnhancedInputComponent->BindAction(Key4Action, ETriggerEvent::Triggered, this, &ADaeva::GASInputPressed, static_cast<int32>(EAbilityID::Key4));
		EnhancedInputComponent->BindAction(KeyQAction, ETriggerEvent::Triggered, this, &ADaeva::GASInputPressed, static_cast<int32>(EAbilityID::KeyQ));
		EnhancedInputComponent->BindAction(KeyEAction, ETriggerEvent::Triggered, this, &ADaeva::GASInputPressed, static_cast<int32>(EAbilityID::KeyE));

		/*EnhancedInputComponent->BindAction(
			ShiftAction,
			ETriggerEvent::Completed,
			this,
			&ADaeva::InputShiftReleased
		);*/

		EnhancedInputComponent->BindAction(
			MoveAction,
			ETriggerEvent::Completed,
			this,
			&ADaeva::InputMoveReleased
		);

		EnhancedInputComponent->BindAction(
			ShiftAction,
			ETriggerEvent::Started,
			this,
			&ADaeva::InputShiftPressed
		);
	}
}

void ADaeva::Tick_Camera(float DeltaTime)
{
	if (IsLocallyControlled())
	{
		SpringArm->TargetArmLength =
			FMath::FInterpTo(SpringArm->TargetArmLength, TargetZoomDistance, DeltaTime, 10.f);
	}
}

void ADaeva::Multicast_PlayMontage_Implementation(EMontageID MontageID, float PlayRate)
{
	if (!GetMesh() || !Montages[MontageID])
	{
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Play(Montages[MontageID], PlayRate);
	}
}

void ADaeva::Multicast_PlayWingMontage_Implementation(EMontageID MontageID, float PlayRate)
{
	if (!Wing || !WingMontages[MontageID])
	{
		return;
	}

	if (UAnimInstance* WingAnimInstance = Wing->GetAnimInstance())
	{
		WingAnimInstance->Montage_Play(WingMontages[MontageID], PlayRate);
	}
}

void ADaeva::Multicast_SetWingVisibility_Implementation(bool NewVisible)
{
	SetWingVisibility(NewVisible);
}

void ADaeva::Client_PlayCameraShake_Implementation()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !CameraShakeClass)
	{
		return;
	}

	PC->ClientStartCameraShake(CameraShakeClass);
}

void ADaeva::Server_SetCurrentTarget_Implementation(AAOCharacter* NewTarget)
{
	SetCurrentTarget(NewTarget);
}

bool ADaeva::HasMoveInput()
{
	return GetCharacterMovement()->GetCurrentAcceleration().SizeSquared() > 0.0f;
}

void ADaeva::SearchTarget()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	PreviousTarget = CurrentTarget;

	TArray<FHitResult> OutHitResults;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(TargetingTrace), false, this);

	FVector SweepStart = GetActorLocation();
	FVector SweepEnd = SweepStart;
	bool bHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, SweepStart, SweepEnd, FQuat::Identity, CCHANNEL_TARGETING, FCollisionShape::MakeSphere(TargetTraceRadius), Params);

	if (!bHitDetected)
	{
		ChangeCurrentTargetInClient(nullptr);
		return;
	}

	struct FTargetCandidate
	{
		TObjectPtr<AAOCharacter> Target;
		float ScreenDistanceSquared;
		float WorldDistanceSquared;
	};

	TArray<FTargetCandidate> Candidates;
	for (const FHitResult& HitResult : OutHitResults)
	{
		AAOCharacter* HitActor = Cast<AAOCharacter>(HitResult.GetActor());
		if (!IsValid(HitActor))
		{
			continue;
		}

		if (HitActor->IsDead())
		{
			continue;
		}

		//if (!IsEnemy(HitActor))
		//{
		//	continue;
		//}

		if (!IsFrontOfCamera(HitActor))
		{
			continue;
		}

		Candidates.Emplace(HitActor, CalcDistanceSquaredToScreenCenter(HitActor), FVector::DistSquared(GetActorLocation(), HitActor->GetActorLocation()));
	}

	if (Candidates.IsEmpty())
	{
		ChangeCurrentTargetInClient(nullptr);
		return;
	}

	Candidates.Sort(
		[](const FTargetCandidate& A, const FTargetCandidate& B)
		{
			if (!FMath::IsNearlyEqual(A.ScreenDistanceSquared, B.ScreenDistanceSquared, 100.f))
			{
				return A.ScreenDistanceSquared < B.ScreenDistanceSquared;
			}

			return A.WorldDistanceSquared < B.WorldDistanceSquared;
		}
	);

	ChangeCurrentTargetInClient(Candidates[0].Target);
}

void ADaeva::TeleportBackToTarget()
{
	if (!IsValid(CurrentTarget))
	{
		return;
	}

	FVector TargetLocation = CurrentTarget->GetActorLocation();
	FVector BehindLocation = TargetLocation - CurrentTarget->GetActorForwardVector() * 200.f;
	BehindLocation.Z = GetActorLocation().Z;

	FVector Direction = TargetLocation - BehindLocation;
	Direction.Z = 0.f;

	const FRotator LookAtRot = Direction.Rotation();

	TeleportTo(BehindLocation, LookAtRot);
	SetCameraByLookAt(LookAtRot);
}

FRotator ADaeva::GetLookAtToTarget()
{
	if (!IsValid(CurrentTarget))
	{
		return FRotator::ZeroRotator;
	}

	FVector TargetLocation = CurrentTarget->GetActorLocation();
	FVector Direction = TargetLocation - GetActorLocation();
	Direction.Z = 0.f;
	return Direction.Rotation();
}

void ADaeva::SetCameraByLookAt(const FRotator& LookAtRot)
{
	if (IsLocallyControlled())
	{
		if (AController* Controller = GetController())
		{
			SpringArm->bEnableCameraLag = false;
			SpringArm->bEnableCameraRotationLag = false;

			Controller->SetControlRotation(LookAtRot);

			GetWorldTimerManager().SetTimerForNextTick(
				[this]()
				{
					SpringArm->bEnableCameraLag = true;
					SpringArm->bEnableCameraRotationLag = true;
				}
			);
		}
	}
}

void ADaeva::ResetForDungeonRespawn()
{
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("ASC is null"));
		return;
	}

	const UAOAttributeSet* AttributeSet = ASC->GetSet<UAOAttributeSet>();

	if (!AttributeSet)
	{
		UE_LOG(LogTemp, Error, TEXT("AttributeSet is null"));
		return;
	}

	const float RespawnHealth = AttributeSet->GetMaxHealth();
	const float RespawnMana = AttributeSet->GetMana();
	const float RespawnStamina = AttributeSet->GetStamina();

	ASC->SetNumericAttributeBase(UAOAttributeSet::GetHealthAttribute(), RespawnHealth);
	ASC->SetNumericAttributeBase(UAOAttributeSet::GetManaAttribute(), RespawnMana);
	ASC->SetNumericAttributeBase(UAOAttributeSet::GetStaminaAttribute(), RespawnStamina);

	// 사망 태그 제거.
	const FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName("State.Dead"));
	ASC->RemoveLooseGameplayTag(DeadTag);

	// New Pawn이므로 기본적으로 false이지만 명확하게 하기 위해 초기화.
	bIsDead = false;
}

void ADaeva::Move(const FInputActionValue& Value)
{
	if (IsDead())
	{
		return;
	}


	FVector2D Movement = Value.Get<FVector2D>();

	FRotator Rotation = GetControlRotation();
	FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

	FVector ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	FVector RightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	FVector NewMoveInputDirection =
		ForwardVector * Movement.Y +
		RightVector * Movement.X;

	if (NewMoveInputDirection.IsNearlyZero())
	{
		return;
	}

	bHasMoveInput = true;

	CurrentMoveInputDirection = NewMoveInputDirection.GetSafeNormal();
	AddMovementInput(CurrentMoveInputDirection);

	
	/*if (bSprintInputHeld)
	{
		RequestStartSprint();
	}*/
}

void ADaeva::Look(const FInputActionValue& Value)
{
	FVector2D RotationValue = Value.Get<FVector2D>();

	AddControllerYawInput(RotationValue.X);
	AddControllerPitchInput(-RotationValue.Y);
}

void ADaeva::Zoom(const FInputActionValue& Value)
{
	const float AxisValue = Value.Get<float>();

	TargetZoomDistance -= AxisValue * ZoomSpeed;
	TargetZoomDistance = FMath::Clamp(TargetZoomDistance, MinZoomDistance, MaxZoomDistance);
}

void ADaeva::InitGAS()
{
	AAOPlayerState* GASPS = GetPlayerState<AAOPlayerState>();
	if (!GASPS)
	{
		return;
	}

	ASC = GASPS->GetAbilitySystemComponent();
	ASC->InitAbilityActorInfo(GASPS, this);

	if (!HealthChangedDelegateHandle.IsValid())
	{
		HealthChangedDelegateHandle =
			ASC->GetGameplayAttributeValueChangeDelegate(
				UAOAttributeSet::GetHealthAttribute()
			).AddUObject(this, &ADaeva::OnHealthChanged);
	}


	if (!ASC->HasMatchingGameplayTag(TEAM_DAEVA))
	{
		ASC->AddLooseGameplayTag(TEAM_DAEVA);
	}

	if (!SprintStaminaChangedDelegateHandle.IsValid())
	{
		SprintStaminaChangedDelegateHandle =
			ASC->GetGameplayAttributeValueChangeDelegate(
				UAOAttributeSet::GetStaminaAttribute()
			).AddUObject(this, &ADaeva::OnStaminaChangedForSprint);
	}

	BindMoveSpeedAttribute();

	if (!bTagEventsRegistered)
	{
		ASC->RegisterGameplayTagEvent(STATE_COMBAT, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ADaeva::OnCombatStateChanged);

		bTagEventsRegistered = true;
	}

	if (HasAuthority())
	{
		GASPS->GiveCommonAbilities();
		CombatAbilitySet->GiveToASC(ASC, CombatAbilityHandles);
		ApplyDashStaminaRegenEffect();
	}

	// UI ?앹꽦 諛?Bind.
	NotifyPlayerUIReady();
}

void ADaeva::ClearGAS()
{
	if (ASC && bMoveSpeedDelegateRegistered)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(
			UAOAttributeSet::GetMoveSpeedAttribute()
		).Remove(MoveSpeedChangedDelegateHandle);

		MoveSpeedChangedDelegateHandle.Reset();
		bMoveSpeedDelegateRegistered = false;
	}

	if (ASC && HealthChangedDelegateHandle.IsValid())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(
			UAOAttributeSet::GetHealthAttribute()
		).Remove(HealthChangedDelegateHandle);

		HealthChangedDelegateHandle.Reset();
	}

	if (HasAuthority())
	{
		for (FGameplayAbilitySpecHandle Handle : CombatAbilityHandles)
		{
			ASC->ClearAbility(Handle);
		}

		CombatAbilityHandles.Empty();
	}
}

void ADaeva::GASInputPressed(int32 InputId)
{
	if (FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromInputID(InputId))
	{
		Spec->InputPressed = true;
		if (Spec->IsActive())
		{
			ASC->AbilitySpecInputPressed(*Spec);
		}
		else
		{
			ASC->TryActivateAbility(Spec->Handle);
		}
	}
}

void ADaeva::GASInputReleased(int32 InputId)
{
	if (FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromInputID(InputId))
	{
		Spec->InputPressed = false;
		if (Spec->IsActive())
		{
			ASC->AbilitySpecInputReleased(*Spec);
		}
	}
}

void ADaeva::ApplyDashStaminaRegenEffect()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!ASC || !DashStaminaRegenEffect)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle =
		ASC->MakeOutgoingSpec(DashStaminaRegenEffect, 1.0f, EffectContext);

	if (SpecHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void ADaeva::BindMoveSpeedAttribute()
{
	if (!ASC || bMoveSpeedDelegateRegistered)
	{
		return;
	}

	MoveSpeedChangedDelegateHandle =
		ASC->GetGameplayAttributeValueChangeDelegate(
			UAOAttributeSet::GetMoveSpeedAttribute()
		).AddUObject(this, &ADaeva::OnMoveSpeedChanged);

	bMoveSpeedDelegateRegistered = true;

	const float CurrentMoveSpeed =
		ASC->GetNumericAttribute(UAOAttributeSet::GetMoveSpeedAttribute());

	GetCharacterMovement()->MaxWalkSpeed = CurrentMoveSpeed;

	UE_LOG(LogTemp, Log, TEXT("[MoveSpeed] Initial Apply: %.1f"), CurrentMoveSpeed);
}

void ADaeva::OnMoveSpeedChanged(const FOnAttributeChangeData& Data)
{
	if (!GetCharacterMovement())
	{
		return;
	}

	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[MoveSpeed] %.1f -> %.1f"),
		Data.OldValue,
		Data.NewValue
	);
}

void ADaeva::OnAttackSucceeded(const FAttackData& AttackData, AActor* HitActor, const FHitResult& HitResult, bool& bDidShakeCamera)
{
	Super::OnAttackSucceeded(AttackData, HitActor, HitResult, bDidShakeCamera);

	PlayCameraShake(bDidShakeCamera);
}

void ADaeva::TakeDamageAO(const FAttackData& AttackData, const FHitResult& HitResult, AAOCharacter* DamageCauser)
{
	if (ASC->HasMatchingGameplayTag(STATE_DASHING))
	{
		ASC->ExecuteGameplayCue(CUE_GHOSTTRAIL);
		return;
	}
	
	Super::TakeDamageAO(AttackData, HitResult, DamageCauser);

	bool bDidShakeCamera = false;
	PlayCameraShake(bDidShakeCamera);
}

void ADaeva::InputShiftPressed()
{
	if (IsDead())
	{
		return;
	}

	// 대시는 전투, 비전투 모두 가능.
	GASInputPressed(static_cast<int32>(EAbilityID::Dash));

	// 전투 중과 활강 중에는 Sprint 금지
	if (ASC->HasMatchingGameplayTag(STATE_COMBAT) || ASC->HasMatchingGameplayTag(STATE_GLIDING))
	{
		return;
	}

	if (bHasMoveInput)
	{
		RequestStartSprint();
	}
}

void ADaeva::InputSpacePressed()
{
	if (GetCharacterMovement()->MovementMode == MOVE_Custom &&
		GetCharacterMovement()->CustomMovementMode == static_cast<uint8>(EAOMovementMode::Glide))
	{
		GASInputPressed(static_cast<int32>(EAbilityID::StopGlide));
		return;
	}

	if (GetCharacterMovement()->IsFalling())
	{
		RequestStopSprint();

		GASInputPressed(static_cast<int32>(EAbilityID::Glide));
		return;
	}

	GASInputPressed(static_cast<int32>(EAbilityID::Jump));
}

void ADaeva::InputLBPressed()
{
	if (IsDead())
	{
		return;
	}

	RequestStopSprint();

	if (ASC->HasMatchingGameplayTag(COMBO_AVAILABLE_LB2))
	{
		GASInputPressed(static_cast<int32>(EAbilityID::LB_2));
	}
	else if (ASC->HasMatchingGameplayTag(COMBO_AVAILABLE_LB3))
	{
		GASInputPressed(static_cast<int32>(EAbilityID::LB_3));
	}
	else
	{
		GASInputPressed(static_cast<int32>(EAbilityID::LB_1));
	}
}

void ADaeva::InputRBPressed()
{
	if (IsDead())
	{
		return;
	}

	RequestStopSprint();

	if (ASC->HasMatchingGameplayTag(COMBO_AVAILABLE_RB2))
	{
		GASInputPressed(static_cast<int32>(EAbilityID::RB_2));
	}
	else if (ASC->HasMatchingGameplayTag(COMBO_AVAILABLE_RB3))
	{
		GASInputPressed(static_cast<int32>(EAbilityID::RB_3));
	}
	else
	{
		GASInputPressed(static_cast<int32>(EAbilityID::RB_1));
	}
}

void ADaeva::InputMoveReleased()
{
	RequestStopSprint();
}

void ADaeva::OnCombatStateChanged(const FGameplayTag Tag, int32 NewCount)
{
	const bool bIsCombat = NewCount > 0;

	SetWeaponVisibility(bIsCombat);
	SetSubWeaponVisibility(bIsCombat);

	// 전투 진입 순간 기준 Sprint 강제 종료.
	if (bIsCombat)
	{
		RequestStopSprint();
	}
}

void ADaeva::HandleDeath()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	UE_LOG(LogTemp, Warning, TEXT("[Death] %s Died"), *GetName());

	// 죽기 전에 Controller를 먼저 확보해야 한다.
	APlayerController* PlayerController =
		Cast<APlayerController>(GetController());

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (ASC)
	{
		ASC->CancelAllAbilities();

		const FGameplayTag DeadTag =
			FGameplayTag::RequestGameplayTag(FName("State.Dead"));

		ASC->AddLooseGameplayTag(DeadTag);
	}

	if (HasAuthority())
	{
		// GameMode에 먼저 사망 사실 전달
		if (PlayerController)
		{
			if (AAODungeonGameMode* DungeonGameMode =
				GetWorld()->GetAuthGameMode<AAODungeonGameMode>())
			{
				UE_LOG(
					LogTemp,
					Warning,
					TEXT("[Death] Notify Dungeon GameMode: %s"),
					*PlayerController->GetName()
				);

				DungeonGameMode->NotifyPlayerDied(PlayerController);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[Death] DungeonGameMode is null"));
			}
		}
		else
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT("[Death] PlayerController is null before detach: %s"),
				*GetName()
			);
		}

		// 사망 애니메이션은 Controller가 붙어 있어도 재생 가능
		Multicast_PlayMontage(EMontageID::Die, 1.0f);
		Multicast_PlayWingMontage(EMontageID::Die, 1.0f);
		Multicast_SetWingVisibility(true);

		// 여기서는 제거하거나 주석 처리
		// DetachFromControllerPendingDestroy();
	}
}

void ADaeva::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Health] %s : %.1f -> %.1f"),
		*GetName(),
		Data.OldValue,
		Data.NewValue
	);

	if (Data.NewValue <= 0.0f && !bIsDead)
	{
		HandleDeath();
	}
}

void ADaeva::StartSprint()
{
	if (!ASC || !HasAuthority())
	{
		return;
	}

	//서버 권한 기준으로 최종 차단.
	if (ASC->HasMatchingGameplayTag(STATE_COMBAT))
	{
		return;
	}


	if (SprintEffectHandle.IsValid())
	{
		return;
	}

	const UAOAttributeSet* AttributeSet = ASC->GetSet<UAOAttributeSet>();
	if (!AttributeSet || AttributeSet->GetStamina() <= 0.0f)
	{
		return;
	}

	if (!SprintEffect || !SprintDrainEffect)
	{
		UE_LOG(LogTemp, Error, TEXT("[Sprint] Sprint GE reference missing"));
		return;
	}

	FGameplayEffectContextHandle SprintContext = ASC->MakeEffectContext();
	SprintContext.AddSourceObject(this);

	FGameplayEffectSpecHandle SprintSpec =
		ASC->MakeOutgoingSpec(SprintEffect, 1.0f, SprintContext);

	if (!SprintSpec.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[Sprint] SprintSpec invalid"));
		return;
	}

	SprintEffectHandle =
		ASC->ApplyGameplayEffectSpecToSelf(*SprintSpec.Data.Get());

	FGameplayEffectContextHandle DrainContext = ASC->MakeEffectContext();
	DrainContext.AddSourceObject(this);

	FGameplayEffectSpecHandle DrainSpec =
		ASC->MakeOutgoingSpec(SprintDrainEffect, 1.0f, DrainContext);

	if (!DrainSpec.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[Sprint] DrainSpec invalid"));
		return;
	}

	SprintDrainEffectHandle =
		ASC->ApplyGameplayEffectSpecToSelf(*DrainSpec.Data.Get());
}

void ADaeva::StopSprint()
{
	if (!ASC || !HasAuthority())
	{
		return;
	}

	if (SprintEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(SprintEffectHandle);
		SprintEffectHandle.Invalidate();
	}

	if (SprintDrainEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(SprintDrainEffectHandle);
		SprintDrainEffectHandle.Invalidate();
	}
}

bool ADaeva::IsSprinting() const
{
	if (!ASC)
	{
		return false;
	}

	const FGameplayTag SprintTag =
		FGameplayTag::RequestGameplayTag(FName("State.Sprint"));

	return ASC->HasMatchingGameplayTag(SprintTag);
}

void ADaeva::OnStaminaChangedForSprint(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue > 0.0f)
	{
		return;
	}

	StopSprint();
}

void ADaeva::InputShiftReleased()
{
	//bSprintInputHeld = false;
	RequestStopSprint();
}

void ADaeva::ServerStartSprint_Implementation()
{
	StartSprint();
}

void ADaeva::ServerStopSprint_Implementation()
{
	StopSprint();
}

void ADaeva::RequestStartSprint()
{
	if (HasAuthority())
	{
		StartSprint();
	}
	else
	{
		ServerStartSprint();
	}
}

void ADaeva::RequestStopSprint()
{
	if (HasAuthority())
	{
		StopSprint();
	}
	else
	{
		ServerStopSprint();
	}
}

void ADaeva::SetWeaponVisibility(bool NewVisible)
{
	if (Weapon)
	{
		Weapon->SetVisibility(NewVisible);
	}
}

void ADaeva::SetSubWeaponVisibility(bool NewVisible)
{
	if (SubWeapon)
	{
		SubWeapon->SetVisibility(NewVisible);
	}
}

void ADaeva::SetWingVisibility(bool NewVisible)
{
	if (Wing)
	{
		Wing->SetVisibility(NewVisible);
	}

	if (Parts[EDaevaPartType::Cape])
	{
		Parts[EDaevaPartType::Cape]->SetVisibility(!NewVisible);
	}
}

void ADaeva::CreatePart(EDaevaPartType PartType, const TCHAR* ComponentName)
{
	USkeletalMeshComponent* PartMesh = CreateDefaultSubobject<USkeletalMeshComponent>(ComponentName);

	PartMesh->SetupAttachment(GetMesh());
	PartMesh->SetLeaderPoseComponent(GetMesh());

	Parts.Add(PartType, PartMesh);
}

void ADaeva::PlayCameraShake(bool& bDidShakeCamera)
{
	if (!bDidShakeCamera)
	{
		Client_PlayCameraShake();

		bDidShakeCamera = true;
	}
}

bool ADaeva::IsFrontOfCamera(AActor* Other)
{
	const FVector CameraLocation = Camera->GetComponentLocation();
	const FVector CameraForward = Camera->GetForwardVector();
	const FVector ToTarget = (Other->GetActorLocation() - CameraLocation).GetSafeNormal();
	const float Dot = FVector::DotProduct(CameraForward, ToTarget);
	return Dot > 0.0f;
}

float ADaeva::CalcDistanceSquaredToScreenCenter(AActor* Other)
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	FVector2D ScreenPosition;
	PC->ProjectWorldLocationToScreen(Other->GetActorLocation(), ScreenPosition);

	int32 ViewportX, ViewportY;
	PC->GetViewportSize(ViewportX, ViewportY);

	FVector2D ScreenCenter(ViewportX * 0.5f, ViewportY * 0.5f);

	return FVector2D::DistSquared(ScreenPosition, ScreenCenter);
}

void ADaeva::ChangeCurrentTargetInClient(AAOCharacter* NewTarget)
{
	CurrentTarget = NewTarget;
	if (PreviousTarget != CurrentTarget)
	{
		Server_SetCurrentTarget(CurrentTarget);
	}
}

void ADaeva::BindOverheadStatusWidget()
{

	if (GetNetMode() == NM_DedicatedServer || !OverheadStatusWidgetComponent)
	{
		return;
	}

	AAOPlayerState* AOPlayerState = GetPlayerState<AAOPlayerState>();
	if (!AOPlayerState)
	{
		return;
	}

	if (UAOPlayerHUDWidget* StatusWidget = Cast<UAOPlayerHUDWidget>(OverheadStatusWidgetComponent->GetUserWidgetObject()))
	{
		StatusWidget->BindToPlayerState(AOPlayerState);
	}
}


bool ADaeva::IsPlayerUIReady() const
{
	return bPlayerUIReady && ASC != nullptr && GetPlayerState<AAOPlayerState>() != nullptr;
}

void ADaeva::NotifyPlayerUIReady()
{
	AAOPlayerState* AOPlayerState = GetPlayerState<AAOPlayerState>();
	if (!AOPlayerState || !ASC)
	{
		return;
	}

	bPlayerUIReady = true;

	BindOverheadStatusWidget();

	OnPlayerUIReady.Broadcast(AOPlayerState, ASC, this);
}

TArray<USkeletalMeshComponent*> ADaeva::GetAllMeshes()
{
	TArray<USkeletalMeshComponent*> Meshes;

	for (const auto& Pair : Parts)
	{
		if (USkeletalMeshComponent* PartMesh = Pair.Value)
		{
			Meshes.Add(PartMesh);
		}
	}

	if (Weapon)
	{
		Meshes.Add(Weapon);
	}

	if (SubWeapon)
	{
		Meshes.Add(SubWeapon);
	}

	return Meshes;
}
