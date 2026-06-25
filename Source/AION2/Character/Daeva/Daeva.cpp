#include "Character/Daeva/Daeva.h"
#include "Player/AOPlayerState.h"
#include "GAS/AOGameplayTags.h"
#include "Character/AOCharacterMovementComponent.h"
#include "Data/DA_AbilitySet.h"
#include "Physics/Collision.h"
#include "Player/AOPlayerController.h"

#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "GAS/AttributeSet/AOAttributeSet.h"

#include "AION2.h"

const float TargetTraceRadius = 2000.0f;

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
}

void ADaeva::BeginPlay()
{
	Super::BeginPlay();

	LastLoc = GetActorLocation();
	LastRot = GetActorRotation();
	bWasMovingLastSend = false;

	TargetZoomDistance = SpringArm->TargetArmLength;
	GetWorldTimerManager().SetTimer(TargetSearchTimer, this, &ThisClass::SearchTarget, 1.0f, true);
}

void ADaeva::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Tick_Camera(DeltaTime);
	Tick_Combat(DeltaTime);

	//if (!IsLocallyControlled())
	//{
	//	FVector NewLocation = FMath::VInterpTo(GetActorLocation(), TargetLoc, DeltaTime, 10.f);
	//	FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 10.f);

	//	SetActorLocation(NewLocation);
	//	SetActorRotation(NewRotation);

	//	GetCharacterMovement()->Velocity = TargetVel;
	//}
}

void ADaeva::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitGAS();

	//if (!IsLocallyControlled()) return;
	//{
	//	UE_LOG(LogTemp, Log, TEXT(" ADaeva::BeginPlay() - SetTimer"));
	//	GetWorldTimerManager().SetTimer(SendMoveHandle, this, &ADaeva::SendMovePacket, SendMoveTimer, true);
	//}
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
		EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &ADaeva::GASInputPressed, static_cast<int32>(EAbilityID::Dash));
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
	}


}

void ADaeva::Tick_Camera(float DeltaTime)
{
	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, TargetZoomDistance, DeltaTime, 10.f);
}

void ADaeva::Tick_Combat(float DeltaTime)
{
	if (!HasAuthority() && IsLocallyControlled())
	{
		SearchTarget();
		if (IsValid(CurrentTarget) && PreviousTarget != CurrentTarget)
		{
			Server_SetCurrentTarget(CurrentTarget);
			UE_LOG(LogTemp, Log, TEXT("%s"), *GetNameSafe(CurrentTarget));
		}
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

void ADaeva::SearchTarget()
{
	PreviousTarget = CurrentTarget;

	TArray<FHitResult> OutHitResults;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(TargetingTrace), false, this);

	FVector SweepStart = GetActorLocation();
	FVector SweepEnd = SweepStart;
	bool bHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, SweepStart, SweepEnd, FQuat::Identity, CCHANNEL_TARGETING, FCollisionShape::MakeSphere(TargetTraceRadius), Params);

	if (!bHitDetected)
	{
		CurrentTarget = nullptr;
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
		CurrentTarget = nullptr;
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

	CurrentTarget = Candidates[0].Target;
}

void ADaeva::Move(const FInputActionValue& Value)
{
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

void ADaeva::TakeDamageAO(const FAttackData& AttackData, AAOCharacter* DamageCauser)
{
	// Todo: 회피 & 리턴 처리

	Super::TakeDamageAO(AttackData, DamageCauser);

	bool bDidShakeCamera = false;
	PlayCameraShake(bDidShakeCamera);
}

void ADaeva::InputShiftPressed()
{
	if (IsSprinting())
	{
		return;
	}

	GASInputPressed(static_cast<int32>(EAbilityID::Dash));

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
		GASInputPressed(static_cast<int32>(EAbilityID::Glide));
		return;
	}

	GASInputPressed(static_cast<int32>(EAbilityID::Jump));
}

void ADaeva::InputLBPressed()
{
	GASInputReleased(static_cast<int32>(EAbilityID::Dash));

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
	bHasMoveInput = false;
	RequestStopSprint();
}

void ADaeva::OnCombatStateChanged(const FGameplayTag Tag, int32 NewCount)
{
	const bool bIsCombat = NewCount > 0;

	SetWeaponVisibility(bIsCombat);
	SetSubWeaponVisibility(bIsCombat);
}

void ADaeva::StartSprint()
{
	if (!ASC || !HasAuthority())
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

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Sprint] Sprint applied / HandleValid=%d / Speed=%.1f"),
		SprintEffectHandle.IsValid(),
		ASC->GetNumericAttribute(UAOAttributeSet::GetMoveSpeedAttribute())
	);

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

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Sprint] Drain applied / HandleValid=%d"),
		SprintDrainEffectHandle.IsValid()
	);
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

	UE_LOG(LogTemp, Warning, TEXT("[Sprint] Stopped"));
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

void ADaeva::SendMovePacket()
{
	bool bCurrentMovement = HasMovement();

	bool bShouldSend = false;

	if (bCurrentMovement)
	{
		bShouldSend = true;

		bWasMovingLastSend = true;
	}

	else
	{
		// ���� ����
		if (bWasMovingLastSend)
		{
			// ��Ŷ ������
			bShouldSend = true;
			bWasMovingLastSend = false;
		}
		else
		{
			// ��� ���� ����
			return;
		}
	}

	if (bShouldSend) 
	{
		FVector CurrLoc = GetActorLocation();
		FRotator CurrRot = GetActorRotation();

		Protocol::C_MovePacket MovePacket;
		MovePacket.set_playerid(MyId);

		Protocol::Vector3* Location = MovePacket.mutable_playerlocation();
		Location->set_x(CurrLoc.X);
		Location->set_y(CurrLoc.Y);
		Location->set_z(CurrLoc.Z);

		FVector CurrVelocity = GetCharacterMovement()->Velocity;

		Protocol::Vector3* Velocity = MovePacket.mutable_playervelocity();
		Velocity->set_x(CurrVelocity.X);
		Velocity->set_y(CurrVelocity.Y);
		Velocity->set_z(CurrVelocity.Z);

		Protocol::Rotator3* Rotation = MovePacket.mutable_playerrotation();
		Rotation->set_pitch(CurrRot.Pitch);
		Rotation->set_yaw(CurrRot.Yaw);
		Rotation->set_roll(CurrRot.Roll);

		SEND_PACKET(MovePacket, PKT_C_MOVE);

		LastLoc = CurrLoc;
		LastRot = CurrRot;
	}
}

bool ADaeva::HasMovement()
{
	FVector CurrentLoc = GetActorLocation();
	FRotator CurrentRot = GetActorRotation();

	// ĳ���� ��ġ�� ���� �Ÿ� �̻� ����������
	float Distance = FVector::DistSquared(CurrentLoc, LastLoc);

	// ĳ���� ������ ���� ���� ����������
	float YawDiff = FRotator::NormalizeAxis(CurrentRot.Yaw - LastRot.Yaw);
	bool bRotated = FMath::Abs(YawDiff) >= 10.0f;

	// ĳ���͹����Ʈ�� ��ȭ�� �־�����
	bool bIsMoving = GetCharacterMovement()->Velocity.SizeSquared() > 0.f;

	return (Distance > 25.f) || bRotated || bIsMoving;
}

bool ADaeva::IsCurrentMoving()
{
	if (!GetCharacterMovement()) return false;
	bool bHasVelocity = GetCharacterMovement()->Velocity.SizeSquared() > 100.f;

	bool bHasInput = !GetPendingMovementInputVector().IsNearlyZero();

	return bHasVelocity || bHasInput;
}

void ADaeva::ReceiveMovePacket(FVector& NewLoc, FRotator& NewRot, FVector& NewVel)
{
	TargetLoc = NewLoc;
	TargetRot = NewRot;
	TargetVel = NewVel;
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
