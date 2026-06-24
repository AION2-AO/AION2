#include "Character/Daeva/Daeva.h"
#include "Player/AOPlayerState.h"
#include "GAS/AOGameplayTags.h"
#include "Character/AOCharacterMovementComponent.h"
#include "Data/DA_AbilitySet.h"
#include "Physics/Collision.h"
#include "Player/AOPlayerController.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"

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

	TargetZoomDistance = SpringArm->TargetArmLength;
}

void ADaeva::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//ValidateTarget();
	//if (!CurrentTarget)
	//{
	//	SearchTarget();
	//	//GetWorldTimerManager().SetTimer(TargetSearchTimer, this, &ThisClass::SearchTarget, 1.0f, true);
	//}

	Tick_Camera(DeltaTime);
}

void ADaeva::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitGAS();
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
	}
}

void ADaeva::Tick_Camera(float DeltaTime)
{
	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, TargetZoomDistance, DeltaTime, 10.f);
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

bool ADaeva::SearchTarget()
{
	TArray<FHitResult> OutHitResults;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(TargetingTrace), false, this);

	FVector SweepStart = GetActorLocation();
	FVector SweepEnd = SweepStart;
	bool bHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, SweepStart, SweepEnd, FQuat::Identity, CCHANNEL_TARGETING, FCollisionShape::MakeSphere(TargetTraceRadius), Params);

	if (!bHitDetected)
	{
		return false;
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
		return false;
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

	return true;
}

void ADaeva::Move(const FInputActionValue& Value)
{
	FVector2D Movement = Value.Get<FVector2D>();

	FRotator Rotation = GetControlRotation();
	FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

	FVector ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	FVector RightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	FVector NewMoveInputDirection = ForwardVector * Movement.Y + RightVector * Movement.X;
	if (NewMoveInputDirection.IsNearlyZero())
	{
		return;
	}

	CurrentMoveInputDirection = NewMoveInputDirection.GetSafeNormal();
	AddMovementInput(CurrentMoveInputDirection);
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

void ADaeva::OnCombatStateChanged(const FGameplayTag Tag, int32 NewCount)
{
	const bool bIsCombat = NewCount > 0;

	SetWeaponVisibility(bIsCombat);
	SetSubWeaponVisibility(bIsCombat);
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

void ADaeva::ValidateTarget()
{
	if (!IsValid(CurrentTarget))
	{
		CurrentTarget = nullptr;
		return;
	}

	//if (CurrentTarget->IsDead())
	//{
	//	CurrentTarget = nullptr;
	//	return;
	//}

	if (FVector::DistSquared(GetActorLocation(), CurrentTarget->GetActorLocation()) > FMath::Square(TargetTraceRadius))
	{
		CurrentTarget = nullptr;
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
