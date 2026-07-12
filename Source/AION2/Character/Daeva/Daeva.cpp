#include "Character/Daeva/Daeva.h"
#include "UI/AOQuickSlotComponent.h"
#include "Player/AOPlayerState.h"
#include "Manager/AOPlayerManager.h"
#include "GAS/AOGameplayTags.h"
#include "Character/AOCharacterMovementComponent.h"
#include "Data/DA_AbilitySet.h"
#include "Physics/Collision.h"
#include "Player/AOPlayerController.h"
#include "InputCoreTypes.h"

#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Game/AOGameMode.h"
#include "Game/AODungeonGameMode.h"
#include "GameplayEffect.h"

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

#include "Character/Monster/AOMonsterBase.h"

#include "Network/PacketHeader.h"
#include "AION2.h"

#include "NiagaraComponent.h"

const float TargetTraceRadius = 3000.0f;


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
	OverheadStatusWidgetComponent->SetDrawSize(FVector2D(80.0f, 30.0f));
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

	QuickSlotComponent = CreateDefaultSubobject<UAOQuickSlotComponent>(TEXT("QuickSlotComponent"));


	// 선환 추가 
	PlayerOrb = CreateDefaultSubobject<USceneComponent>(TEXT("PlayerOrb"));
	PlayerOrb->SetupAttachment(GetCapsuleComponent());

	BlueOrb = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BlueOrb"));
	BlueOrb->SetupAttachment(PlayerOrb);
	BlueOrb->SetAutoActivate(false); // 처음엔 꺼두기 

	PurpleOrb = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PurpleOrb"));
	PurpleOrb->SetupAttachment(PlayerOrb);
	PurpleOrb->SetAutoActivate(false); // 처음엔 꺼두기 


	PlayerAoeField = CreateDefaultSubobject<USceneComponent>(TEXT("AoeIndicator"));
	PlayerAoeField->SetupAttachment(GetCapsuleComponent());

	AoeField = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FieldRange"));
	AoeField->SetupAttachment(PlayerAoeField);

	// 이거 false로 고치기.
	AoeField->SetVisibility(false, true);

}

void ADaeva::BeginPlay()
{
	Super::BeginPlay();

	TargetZoomDistance = SpringArm->TargetArmLength;
	GetWorldTimerManager().SetTimer(TargetSearchTimer, this, &ThisClass::SearchTarget, 0.25f, true);

	if (GetNetMode() != NM_DedicatedServer)
	{
		GetWorldTimerManager().SetTimer(OverheadWidgetRefreshTimer,	this,&ADaeva::RefreshOverheadWidgetIfVisible,0.5f,true);
	}
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

	RestorePlayerInfoFromPlayerState();

	// 선환 추가 
	SetGenericTeamId(FGenericTeamId(TEAM_PERCEPTION_DAEVA)); // 플레이어 팀

	//UE_LOG(LogTemp, Warning, TEXT("[%s] TeamID set: %d"),
	//	*GetName(), GetGenericTeamId().GetId());
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

	RestorePlayerInfoFromPlayerState();
	
	// LocalController일 때만 UI 만들도록 설정
	if (AAOPlayerController* AOController = Cast<AAOPlayerController>(GetController()))
	{
		if (AOController->IsLocalController())
		{
			AOController->HandlePawnASCReady();
		}
	}

	// LocalPlayer가 아닐 때도 OverheadStatusWidget은 Client에서 보여야 함 (다른 유저)
	BindOverheadStatusWidget();
}

void ADaeva::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADaeva, bWingVisible);
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

		EnhancedInputComponent->BindAction(LBAction, ETriggerEvent::Completed, this, &ADaeva::InputLBReleased);
		EnhancedInputComponent->BindAction(RBAction, ETriggerEvent::Completed, this, &ADaeva::InputRBReleased);

		// SuYeon: Released에 Bind되어있어야 UI도 키 입력 종료를 알 수 있음: KeyPressedEvent가 발생되기 위함.
		EnhancedInputComponent->BindAction(Key1Action, ETriggerEvent::Completed, this, &ADaeva::GASInputReleased, static_cast<int32>(EAbilityID::Key1));
		EnhancedInputComponent->BindAction(Key2Action, ETriggerEvent::Completed, this, &ADaeva::GASInputReleased, static_cast<int32>(EAbilityID::Key2));
		EnhancedInputComponent->BindAction(Key3Action, ETriggerEvent::Completed, this, &ADaeva::GASInputReleased, static_cast<int32>(EAbilityID::Key3));
		EnhancedInputComponent->BindAction(Key4Action, ETriggerEvent::Completed, this, &ADaeva::GASInputReleased, static_cast<int32>(EAbilityID::Key4));
		EnhancedInputComponent->BindAction(KeyQAction, ETriggerEvent::Completed, this, &ADaeva::GASInputReleased, static_cast<int32>(EAbilityID::KeyQ));
		EnhancedInputComponent->BindAction(KeyEAction, ETriggerEvent::Completed, this, &ADaeva::GASInputReleased, static_cast<int32>(EAbilityID::KeyE));


		EnhancedInputComponent->BindAction(KeyXAction, ETriggerEvent::Triggered, this, &ADaeva::SendItem, 0);
		EnhancedInputComponent->BindAction(KeyBAction, ETriggerEvent::Triggered, this, &ADaeva::SendItem, 1);

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

void ADaeva::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearGAS();

	if (OverheadStatusWidgetComponent)
	{
		if (UAOPlayerHUDWidget* StatusWidget =
			Cast<UAOPlayerHUDWidget>(OverheadStatusWidgetComponent->GetUserWidgetObject()))
		{
			StatusWidget->ClearBinding();
		}
	}

	Super::EndPlay(EndPlayReason);
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
	CheckTargetGroggy();

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

		if (!IsEnemy(HitActor))
		{
			continue;
		}

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
	//
	bIsDead = false;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->StopMovementImmediately();
	//

	if (!ASC)
	{
		return;
	}
	const UAOAttributeSet* AttributeSet = ASC->GetSet<UAOAttributeSet>();

	if (!AttributeSet)
	{
		return;
	}

	const float RespawnHealth = AttributeSet->GetMaxHealth();
	const float RespawnMana = AttributeSet->GetMaxMana();
	const float RespawnStamina = AttributeSet->GetMaxStamina();

	ASC->SetNumericAttributeBase(UAOAttributeSet::GetHealthAttribute(), AttributeSet->GetMaxHealth());
	ASC->SetNumericAttributeBase(UAOAttributeSet::GetManaAttribute(), AttributeSet->GetMaxMana());
	ASC->SetNumericAttributeBase(UAOAttributeSet::GetStaminaAttribute(), AttributeSet->GetMaxStamina());

	// 사망 태그 제거.
	ASC->RemoveLooseGameplayTag(STATE_DEAD);
	ASC->RemoveReplicatedLooseGameplayTag(STATE_DEAD);

	// New Pawn이므로 기본적으로 false이지만 명확하게 하기 위해 초기화.
	bIsDead = false;

	if (OverheadStatusWidgetComponent)
	{
		OverheadStatusWidgetComponent->SetVisibility(true);
	}

	// 같은 Pawn으로 부활하는 경우 OnRep_PlayerState가 다시 안 올 수 있으므로 직접 다시 함.
	BindOverheadStatusWidget();

	// 로컬 갱신.
	NotifyPlayerUIReady(); // 서버에서도 호출 될 수 있으므로, PlayerController에 Clinet RPC를 만들어서 HUD 다시 묶는 것이 좋다.

	if (HasAuthority())
	{
		if (AAOPlayerController* AOController = Cast<AAOPlayerController>(GetController()))
		{
			AOController->Client_RefreshPlayerHUD();
		}
	}


	if (HasAuthority())
	{
		Multicast_PlayMontage(EMontageID::Rebirth, 1.3f);
		Multicast_PlayWingMontage(EMontageID::Rebirth, 1.0f);
		SetWingVisibilityOnServer(true);
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &ThisClass::OnRebirthMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, GetMontageByID(EMontageID::Rebirth));
		}
	}
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

	FVector NewMoveInputDirection =	ForwardVector * Movement.Y + RightVector * Movement.X;

	if (NewMoveInputDirection.IsNearlyZero())
	{
		return;
	}

	bHasMoveInput = true;

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

	if (HasAuthority() && ManaRegenEffect)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ManaRegenEffect, 1.f, Context);

		if (SpecHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

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

	// Notify to the PlayerController that the PlayerCharacter is ready.
	NotifyPlayerUIReady();
}

void ADaeva::ClearGAS()
{
	// 클라이언트/서버 모두 해야 하는 delegate cleanup
	if (ASC && bMoveSpeedDelegateRegistered)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UAOAttributeSet::GetMoveSpeedAttribute()).Remove(MoveSpeedChangedDelegateHandle);

		MoveSpeedChangedDelegateHandle.Reset();
		bMoveSpeedDelegateRegistered = false;
	}

	if (ASC && HealthChangedDelegateHandle.IsValid())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UAOAttributeSet::GetHealthAttribute()).Remove(HealthChangedDelegateHandle);

		HealthChangedDelegateHandle.Reset();
	}

	
	// Suyeon: added Exception Handling (!ASC)
	// 서버 권한에서만 해야 하는 ability spec 제거: 
	// ClearAbility는 서버인지 체크해야 함=> HasAuthority() 
	if (HasAuthority() && ASC)
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
		if (LastPressedFeedbackAbilityID != InputId)
		{
			LastPressedFeedbackAbilityID = InputId;
		
			if (AAOPlayerController* PC = Cast<AAOPlayerController>(GetController()))
			{
				PC->PlaySkillPressedFeedback(InputId);
			}
		}

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
	// For UI: 마지막에 눌린 값을 초기화
	if (LastPressedFeedbackAbilityID == InputId)
	{
		LastPressedFeedbackAbilityID = INDEX_NONE;
	}

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

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DashStaminaRegenEffect, 1.0f, EffectContext);

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

	MoveSpeedChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate( 
		UAOAttributeSet::GetMoveSpeedAttribute()).AddUObject(this, &ADaeva::OnMoveSpeedChanged);

	bMoveSpeedDelegateRegistered = true;

	const float CurrentMoveSpeed = ASC->GetNumericAttribute(UAOAttributeSet::GetMoveSpeedAttribute());

	GetCharacterMovement()->MaxWalkSpeed = CurrentMoveSpeed;
}

void ADaeva::OnMoveSpeedChanged(const FOnAttributeChangeData& Data)
{
	if (!GetCharacterMovement())
	{
		return;
	}

	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
}

void ADaeva::OnAttackSucceeded(const FAttackData& AttackData,AActor* HitActor,const FHitResult& HitResult,bool& bDidShakeCamera)
{
	Super::OnAttackSucceeded(AttackData, HitActor, HitResult, bDidShakeCamera);

	PlayCameraShake(bDidShakeCamera);

	if (!HasAuthority())
	{
		return;
	}

	if (!AttackData.bRestoreManaOnHit)
	{
		return;
	}

	if (!HitManaRegenEffect)
	{
		return;
	}

	UAbilitySystemComponent* MyASC = GetAbilitySystemComponent();

	if (!MyASC)
	{
		return;
	}

	FGameplayEffectContextHandle ContextHandle = MyASC->MakeEffectContext();
	ContextHandle.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle =	MyASC->MakeOutgoingSpec(HitManaRegenEffect, 1.f, ContextHandle);

	if (!SpecHandle.IsValid())
	{
		return;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.HitManaRegen")),HitManaRegenAmount);

	const float BeforeMana =
		MyASC->GetNumericAttribute(UAOAttributeSet::GetManaAttribute());

	MyASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	const float AfterMana =
		MyASC->GetNumericAttribute(UAOAttributeSet::GetManaAttribute());
}

void ADaeva::TakeDamageAO(const FAttackData& AttackData, const FHitResult& HitResult, AAOCharacter* DamageCauser)
{
	if (ASC->HasMatchingGameplayTag(STATE_DASHING))
	{
		ASC->ExecuteGameplayCue(CUE_GHOSTTRAIL);
		return;
	}

	if (StateCombatApplyEffect)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(StateCombatApplyEffect, 1.f, Context);
		if (SpecHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
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

	// 활강 대시
	if (GetCharacterMovement()->MovementMode == MOVE_Custom &&
		GetCharacterMovement()->CustomMovementMode == static_cast<uint8>(EAOMovementMode::Glide))
	{
		GASInputPressed(static_cast<int32>(EAbilityID::GlideDash));
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

	EAbilityID AbilityID = EAbilityID::LB_1;

	// 두 태그가 잠깐 함께 존재해도 더 높은 콤보를 우선.
	if (ASC->HasMatchingGameplayTag(COMBO_AVAILABLE_LB3))
	{
		AbilityID = EAbilityID::LB_3;
	}
	else if (ASC->HasMatchingGameplayTag(COMBO_AVAILABLE_LB2))
	{
		AbilityID = EAbilityID::LB_2;
	}

	GASInputPressed(static_cast<int32>(AbilityID));
}

void ADaeva::InputRBPressed()
{
	if (IsDead())
	{
		return;
	}

	RequestStopSprint();

	EAbilityID AbilityID = EAbilityID::RB_1;

	// 두 태그가 잠깐 함께 존재해도 더 높은 콤보를 우선.
	if (ASC->HasMatchingGameplayTag(COMBO_AVAILABLE_RB3))
	{
		AbilityID = EAbilityID::RB_3;
	}
	else if (ASC->HasMatchingGameplayTag(COMBO_AVAILABLE_RB2))
	{
		AbilityID = EAbilityID::RB_2;
	}

	GASInputPressed(static_cast<int32>(AbilityID));
}

void ADaeva::InputMoveReleased()
{
	RequestStopSprint();
}

void ADaeva::InputLBReleased()
{
	if (ASC)
	{
		GASInputReleased(static_cast<int32>(EAbilityID::LB_1));
		GASInputReleased(static_cast<int32>(EAbilityID::LB_2));
		GASInputReleased(static_cast<int32>(EAbilityID::LB_3));
	}

	OnComboInputCompleted.Broadcast(
		static_cast<int32>(EAbilityID::LB_1)
	);
}

void ADaeva::InputRBReleased()
{
	if (ASC)
	{
		GASInputReleased(static_cast<int32>(EAbilityID::RB_1));
		GASInputReleased(static_cast<int32>(EAbilityID::RB_2));
		GASInputReleased(static_cast<int32>(EAbilityID::RB_3));
	}

	OnComboInputCompleted.Broadcast(
		static_cast<int32>(EAbilityID::RB_1)
	);
}

void ADaeva::InputXPressed()
{
	
}

void ADaeva::InputBPressed()
{
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

void ADaeva::OnRebirthMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (HasAuthority())
	{
		SetWingVisibilityOnServer(false);
	}
}

void ADaeva::OnRep_IsDead()
{
	//OverheadStatusWidgetComponent->DestroyComponent();
	//부활 후 다시 사용할 컴포넌트라서 숨기기만 하면 된다.
	if (OverheadStatusWidgetComponent)
	{
		OverheadStatusWidgetComponent->SetVisibility(!bIsDead);
	}
}

void ADaeva::HandleDeath(EDeathReason DeathReason)
{
	if (bIsDead)
	{
		return;
	}

	if (!HasAuthority())
	{
		return;
	}

	bIsDead = true;

	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Death] CharacterMovement is null: %s"), *GetName());
	}

	OnPlayerDead.Broadcast(this);

	if (ASC)
	{
		ASC->CancelAllAbilities();

		FGameplayTagContainer TagsToRemove;
		TagsToRemove.AddTag(STATE_COMBAT);
		ASC->RemoveActiveEffectsWithGrantedTags(TagsToRemove);

		ASC->SetLooseGameplayTagCount(STATE_COMBAT, 0);
		ASC->SetLooseGameplayTagCount(STATE_ATTACKING, 0);
		ASC->SetLooseGameplayTagCount(STATE_DASHING, 0);
		ASC->SetLooseGameplayTagCount(STATE_JUMPING, 0);
		ASC->SetLooseGameplayTagCount(STATE_GLIDING, 0);

		ASC->AddLooseGameplayTag(STATE_DEAD);
		ASC->AddReplicatedLooseGameplayTag(STATE_DEAD);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Death] ASC is null: %s"), *GetName());
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (PlayerController)
	{
		if (AAODungeonGameMode* DungeonGameMode = World->GetAuthGameMode<AAODungeonGameMode>())
		{
			const bool bIsFallDeath = (DeathReason == EDeathReason::Fall);
			DungeonGameMode->NotifyPlayerDied(PlayerController, bIsFallDeath);
		}
		else if (AAOGameMode* AOGameMode = World->GetAuthGameMode<AAOGameMode>())
		{
			AOGameMode->NotifyPlayerDied(PlayerController);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[Death] GameMode is null"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Death] PlayerController is null before detach: %s"), *GetName());
	}

	Multicast_PlayMontage(EMontageID::Die, 1.0f);
	Multicast_PlayWingMontage(EMontageID::Die, 1.0f);
	SetWingVisibilityOnServer(true);

	// 여기서는 제거하거나 주석 처리
	// DetachFromControllerPendingDestroy();
}

void ADaeva::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	
	if (Data.NewValue <= 0.0f && !bIsDead)
	{
		HandleDeath();
	}
}

void ADaeva::TestSetHealth(float NewHealth)
{
	if (ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Test] Setting Health to %.1f via Console Command"), NewHealth);
		ASC->SetNumericAttributeBase(UAOAttributeSet::GetHealthAttribute(), NewHealth);
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
		return;
	}

	FGameplayEffectContextHandle SprintContext = ASC->MakeEffectContext();
	SprintContext.AddSourceObject(this);

	FGameplayEffectSpecHandle SprintSpec = ASC->MakeOutgoingSpec(SprintEffect, 1.0f, SprintContext);

	if (!SprintSpec.IsValid())
	{
		return;
	}

	SprintEffectHandle =
		ASC->ApplyGameplayEffectSpecToSelf(*SprintSpec.Data.Get());

	FGameplayEffectContextHandle DrainContext = ASC->MakeEffectContext();
	DrainContext.AddSourceObject(this);

	FGameplayEffectSpecHandle DrainSpec = ASC->MakeOutgoingSpec(SprintDrainEffect, 1.0f, DrainContext);

	if (!DrainSpec.IsValid())
	{
		return;
	}

	SprintDrainEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*DrainSpec.Data.Get());
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

	const FGameplayTag SprintTag = FGameplayTag::RequestGameplayTag(FName("State.Sprint"));

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

void ADaeva::SetWingVisibilityOnServer(bool NewVisible)
{
	if (!HasAuthority())
	{
		return;
	}

	bWingVisible = NewVisible;
	OnRep_WingVisible();
	ForceNetUpdate();
}

void ADaeva::OnRep_WingVisible()
{
	SetWingVisibility(bWingVisible);
}

void ADaeva::RestorePlayerInfoFromPlayerState()
{
	AAOPlayerState* AOPlayerState = GetPlayerState<AAOPlayerState>();

	if (!AOPlayerState)
	{
		return;
	}

	// 1. HP Apply
	if (HasAuthority())
	{
		float InitialHP = AOPlayerState->GetInitialHP();
		if (InitialHP > 0.0f && ASC)
		{
			ASC->SetNumericAttributeBase(UAOAttributeSet::GetHealthAttribute(), InitialHP);
			UE_LOG(LogTemp, Log, TEXT("[Dungeon] Restored HP for %s on Server: %.1f"), *GetName(), InitialHP);
		}
	}

	// 2. Items Apply 
	if (!HasAuthority() && IsLocallyControlled())
	{
		UAOPlayerManager* PlayerManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAOPlayerManager>() : nullptr;
		UAOQuickSlotComponent* QuickSlotComp = GetQuickSlotComponent();
		if (PlayerManager && QuickSlotComp)
		{
			const TMap<int32, Protocol::ItemData>& Items = PlayerManager->GetMyItems();
			for (const auto& Pair : Items)
			{
				const Protocol::ItemData& Item = Pair.Value;
				QuickSlotComp->InitializeQuickSlot(Item.slotindex(), Item.itemtemplateid(), Item.iteminstancedid(), Item.count());
				UE_LOG(LogTemp, Log, TEXT("[Dungeon] Restored Item to QuickSlot[%d] from Local Subsystem: TemplateId=%d, Count=%d"), 
					Item.slotindex(), Item.itemtemplateid(), Item.count());
			}
		}
	}

}

void ADaeva::FellOutOfWorld(const UDamageType& DmgType)
{
	if (!HasAuthority()) return;
	if (bIsDead) return;
	HandleDeath();
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

		if (AAOMonsterBase* PreviousMonster = Cast<AAOMonsterBase>(PreviousTarget))
		{
			PreviousMonster->SetTargetWidgetVisible(false);
		}

		if (AAOMonsterBase* CurrentMonster = Cast<AAOMonsterBase>(CurrentTarget))
		{
			CurrentMonster->SetTargetWidgetVisible(true);
		}

		// Related to MonsterHUD.
		if (AAOPlayerController* PC = Cast<AAOPlayerController>(GetController()))
		{
			if (AAOMonsterBase* CurrentMonster = Cast<AAOMonsterBase>(CurrentTarget))
			{
				PC->ShowTargetMonsterHUD(CurrentMonster);
			}
			else
			{
				PC->HideTargetMonsterHUD();
			}
		}
	}
}

void ADaeva::CheckTargetGroggy()
{
	if (!ASC)
	{
		return;
	}

	if (!IsValid(CurrentTarget))
	{
		if (ASC->HasMatchingGameplayTag(COMBO_AVAILABLE_KEYE))
		{
			ASC->RemoveLooseGameplayTag(COMBO_AVAILABLE_KEYE);
		}
		return;
	}

	AAOMonsterBase* Monster = Cast<AAOMonsterBase>(CurrentTarget);
	if (!Monster)
	{
		return;
	}

	if (!ASC->HasMatchingGameplayTag(COMBO_AVAILABLE_KEYE) && Monster->IsGroggy())
	{
		ASC->AddLooseGameplayTag(COMBO_AVAILABLE_KEYE);
	}
	else if (ASC->HasMatchingGameplayTag(COMBO_AVAILABLE_KEYE) && !Monster->IsGroggy())
	{
		ASC->RemoveLooseGameplayTag(COMBO_AVAILABLE_KEYE);
	}
}

void ADaeva::BindOverheadStatusWidget()
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (!OverheadStatusWidgetComponent)
	{
		return;
	}

	AAOPlayerState* AOPlayerState = GetPlayerState<AAOPlayerState>();
	if (!AOPlayerState)
	{
		if (++PawnASCBindRetryCount <= PawnASCBindMaxRetryCount)
		{
			GetWorldTimerManager().SetTimerForNextTick(
				this,
				&ADaeva::BindOverheadStatusWidget
			);
		}

		return;
	}

	UAbilitySystemComponent* PlayerStateASC = AOPlayerState->GetAbilitySystemComponent();
	if (!PlayerStateASC)
	{
		if (++PawnASCBindRetryCount <= PawnASCBindMaxRetryCount)
		{
			GetWorldTimerManager().SetTimerForNextTick(
				this,
				&ADaeva::BindOverheadStatusWidget
			);
		}

		return;
	}

	UAOPlayerHUDWidget* StatusWidget =
		Cast<UAOPlayerHUDWidget>(OverheadStatusWidgetComponent->GetUserWidgetObject());

	if (!StatusWidget)
	{
		OverheadStatusWidgetComponent->InitWidget();

		StatusWidget =	Cast<UAOPlayerHUDWidget>(OverheadStatusWidgetComponent->GetUserWidgetObject());

		if (!StatusWidget)
		{
			if (++PawnASCBindRetryCount <= PawnASCBindMaxRetryCount)
			{
				GetWorldTimerManager().SetTimerForNextTick(
					this,
					&ADaeva::BindOverheadStatusWidget
				);
			}

			return;
		}
	}

	// 여기서부터 핵심.
	// 같은 ASC / 같은 Widget이어도 다시 바인딩하고 현재 Attribute 값을 다시 밀어준다.
	BoundOverheadStatusASC = PlayerStateASC;
	BoundOverheadStatusWidget = StatusWidget;
	PawnASCBindRetryCount = 0;

	StatusWidget->BindToPlayerState(AOPlayerState);
	StatusWidget->BroadcastInitialAttributes();

	OverheadStatusWidgetComponent->RequestRedraw();

	UE_LOG(	LogTemp,Warning,TEXT("[Overhead Bind/Refresh] %s | PS=%s | ASC=%s | Widget=%s"),*GetName(),	*GetNameSafe(AOPlayerState),*GetNameSafe(PlayerStateASC),*GetNameSafe(StatusWidget));
}

void ADaeva::NotifyPlayerUIReady()
{
	AAOPlayerState* AOPlayerState = GetPlayerState<AAOPlayerState>();
	if (!AOPlayerState || !ASC)
	{
		return;
	}

	bPlayerUIReady = true;

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

void ADaeva::SetMyId(uint64 Id)
{
	MyId = Id;
}

void ADaeva::SetMyClass(uint8 ClassType)
{
	//Type = static_cast<EDaevaClassType>(ClassType);
	///AAOPlayerState* AOPlayerState = GetPlayerState<AAOPlayerState>();
	//AOPlayerState->SetMyClass(static_cast<EDaevaClassType>(ClassType));
}

void ADaeva::SetMyName(FString InName)
{
	//AAOPlayerState* AOPlayerState = GetPlayerState<AAOPlayerState>();
	//AOPlayerState->SetMyName(InName);
}

void ADaeva::SendHp(float NewHp)
{
	Protocol::C_ChangeHpPacket HpPacket;
	HpPacket.set_playerid(MyId);
	HpPacket.set_hp(NewHp);
	SEND_PACKET(HpPacket, PKT_C_CHANGE_HP);
}

void ADaeva::SendItem(int32 SlotIndex)
{
	if (!bCanUseItem) return;
	bCanUseItem = false;
	GetWorldTimerManager().SetTimer(ItemCoolTimeHandler, this, &ThisClass::SetItemUse, ItemCoolTime, false);

	Protocol::C_UseItemPacket UseItemPkt;
	UseItemPkt.set_playerid(MyId);
	SEND_PACKET(UseItemPkt, PKT_C_USE_ITEM);
}

void ADaeva::SetItemUse()
{
	bCanUseItem = true;
}



void ADaeva::EatOrb(EOrbColor NewColor)
{
	if (NewColor == LastOrbColor)
	{
		// 같은 색 연속 -> 스택 증가 
		++OrbStack;
	}


	else
	{
		// 다른 색 -> 초기화 후 1로 시작. 
		OrbStack = 1;
		LastOrbColor = NewColor;

		switch (NewColor)
		{
		case EOrbColor::PURPLE:
		{
			Set_BlueOrb_RenderOnOff(false);
		}
		break;
		case EOrbColor::BLUE:
		{
			Set_PurpleOrb_RenderOnOff(false);
		}
		break;
		}
	}


	// 같은 2번 연속 먹었을 때만 효과 발동 
	if (OrbStack >= 2)
	{
		// 여기서 효과 발동

		switch (NewColor)
		{
		case EOrbColor::BLUE:
		{
			Set_BlueOrb_RenderOnOff(true);
		}
		break;
		case EOrbColor::PURPLE:
		{
			Set_PurpleOrb_RenderOnOff(true);
		}
		break;
		}


	}


}



void ADaeva::Set_AOE_RenderOnOff_Implementation(bool _bOnOff)
{
	AoeField->SetVisibility(_bOnOff, true);

}

void ADaeva::Set_BlueOrb_RenderOnOff_Implementation(bool _bOnOff)
{
	if (_bOnOff == true)
	{
		BlueOrb->Activate(_bOnOff);
	}

	else
	{
		BlueOrb->DeactivateImmediate();
	}

}



void ADaeva::Set_PurpleOrb_RenderOnOff_Implementation(bool _bOnOff)
{
	if (_bOnOff == true)
	{
		PurpleOrb->Activate(_bOnOff);
	}


	else
	{
		PurpleOrb->DeactivateImmediate();
	}

}


void ADaeva::Reset_OrbStackAndColor()
{
	OrbStack = 0;
	LastOrbColor = EOrbColor::None;


}

void ADaeva::RefreshOverheadWidgetIfVisible()
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (!OverheadStatusWidgetComponent)
	{
		return;
	}

	// StatusWidget이 있어도 내부 ASC 바인딩이 꼬였을 수 있으므로
	// 매번 다시 Bind 시도.
	BindOverheadStatusWidget();

	UAOPlayerHUDWidget* StatusWidget = Cast<UAOPlayerHUDWidget>(OverheadStatusWidgetComponent->GetUserWidgetObject());

	if (!StatusWidget)
	{
		return;
	}

	StatusWidget->BroadcastInitialAttributes();
	OverheadStatusWidgetComponent->RequestRedraw();
}
