// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ServerCharacter/MMODaeva.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "Player/AOPlayerController.h"
#include "UI/AOChattingWidget.h"
#include "UI/AOMainHUDWidget.h"
#include "Manager/AOUIManager.h"
#include "UI/Mail/MainMailWidget.h"
#include "AION2.h"
#include "GAS/AOGameplayTags.h"
#include "GAS/AttributeSet/AOAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Character/AOCharacterMovementComponent.h"

void AMMODaeva::BeginPlay()
{
	Super::BeginPlay();
	LastLoc = GetActorLocation();
	LastRot = GetActorRotation();
	TargetLoc = LastLoc;
	TargetRot = LastRot;
	bWasMovingLastSend = false;
}

void AMMODaeva::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsLocallyControlled())
	{
		FVector NewLocation = FMath::VInterpTo(GetActorLocation(), TargetLoc, DeltaTime, 10.f);
		FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 10.f);

		SetActorLocation(NewLocation);
		SetActorRotation(NewRotation);

		GetCharacterMovement()->Velocity = TargetVel;
	}
}

void AMMODaeva::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!IsLocallyControlled()) return;
	{
		UE_LOG(LogTemp, Log, TEXT(" AMMODaeva::PossessedBy() - SetTimer"));
		GetWorldTimerManager().SetTimer(SendMoveHandle, this, &AMMODaeva::SendMovePacket, SendMoveTimer, true);
	}
}

void AMMODaeva::SendMovePacket()
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
		if (bWasMovingLastSend)
		{
			bShouldSend = true;
			bWasMovingLastSend = false;
		}
		else
		{
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

bool AMMODaeva::HasMovement()
{
	FVector CurrentLoc = GetActorLocation();
	FRotator CurrentRot = GetActorRotation();

	float Distance = FVector::DistSquared(CurrentLoc, LastLoc);

	float YawDiff = FRotator::NormalizeAxis(CurrentRot.Yaw - LastRot.Yaw);
	bool bRotated = FMath::Abs(YawDiff) >= 10.0f;

	bool bIsMoving = GetCharacterMovement()->Velocity.SizeSquared() > 0.f;

	return (Distance > 25.f) || bRotated || bIsMoving;
}

void AMMODaeva::InputMoveReleased()
{
	bHasMoveInput = false;
	RequestStopSprint();
}

void AMMODaeva::OnChatActivateTriggered()
{
	AAOPlayerController* PlayerController = Cast<AAOPlayerController>(GetController());
	if (PlayerController)
	{
		auto HUD = PlayerController->GetMainHUD();
		if (HUD->ChattingWidget)
		{
			HUD->ChattingWidget->ActivateChatInput();
		}
	}
}

void AMMODaeva::OnMailActivateTriggerd()
{
	ToggleMailWidget();
}

void AMMODaeva::ToggleMailWidget()
{
	UAOUIManager* UIManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAOUIManager>() : nullptr;
	if (UIManager)
	{
		UMainMailWidget* MainMailWidget = UIManager->GetWidget<UMainMailWidget>();
		if (MainMailWidget && MainMailWidget->IsInViewport())
		{
			AAOPlayerController* PC = Cast<AAOPlayerController>(GetController());
			if (PC)
			{
				PC->ToggleMailWidget();
			}
			return;
		}
	}

	Protocol::C_MailListPacket ReqList;
	ReqList.set_playerid(MyId);
	SEND_PACKET(ReqList, PKT_C_MAIL_LIST);
}

bool AMMODaeva::IsCurrentMoving()
{
	if (!GetCharacterMovement()) return false;
	bool bHasVelocity = GetCharacterMovement()->Velocity.SizeSquared() > 100.f;

	bool bHasInput = !GetPendingMovementInputVector().IsNearlyZero();

	return bHasVelocity || bHasInput;
}

void AMMODaeva::SendDungeonWait()
{
	UE_LOG(LogTemp, Log, TEXT("DunzeonWaitingRoom Enter"));

	Protocol::C_DungeonWaitingRoomEnterPacket EnterWaitPacket;
	SEND_PACKET(EnterWaitPacket, PKT_C_DUNGEON_ENTER_WAITING_ROOM);
}

void AMMODaeva::SetHp(int32 Hp)
{

}

void AMMODaeva::ReceiveMovePacket(FVector& NewLoc, FRotator& NewRot, FVector& NewVel)
{
	TargetLoc = NewLoc;
	TargetRot = NewRot;
	TargetVel = NewVel;
}


void AMMODaeva::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	ACharacter::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMMODaeva::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMMODaeva::Look);
		EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &AMMODaeva::Zoom);
		EnhancedInputComponent->BindAction(ChatActivateAction, ETriggerEvent::Triggered, this, &AMMODaeva::OnChatActivateTriggered);
		EnhancedInputComponent->BindAction(MailAction, ETriggerEvent::Started, this, &AMMODaeva::OnMailActivateTriggerd);

		EnhancedInputComponent->BindAction(
			MoveAction,
			ETriggerEvent::Completed,
			this,
			&AMMODaeva::InputMoveReleased
		);

		EnhancedInputComponent->BindAction(KeyXAction, ETriggerEvent::Triggered, this, &AMMODaeva::SendItem, 0);
		EnhancedInputComponent->BindAction(KeyBAction, ETriggerEvent::Triggered, this, &AMMODaeva::SendItem, 1);

		EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AMMODaeva::MMOInputShiftPressed);
	}
}


void AMMODaeva::PlayMontageWithSection(EMontageID MontageID, float PlayRate, FName SectionName)
{
	if (!GetMesh() || !GetMontageByID(MontageID))
	{
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (IsLocallyControlled())
		{
			AnimInstance->RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
		}
		else
		{
			AnimInstance->RootMotionMode = ERootMotionMode::IgnoreRootMotion;
		}

		AnimInstance->Montage_Play(GetMontageByID(MontageID), PlayRate);
		if (SectionName != NAME_None)
		{
			AnimInstance->Montage_JumpToSection(SectionName, GetMontageByID(MontageID));
		}
	}
}

bool AMMODaeva::CanDash() const
{
	if (IsDead())
	{
		return false;
	}

	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (!MovementComp)
	{
		return false;
	}

	// Check if falling, flying, or gliding (Custom movement mode)
	if (MovementComp->IsFalling() ||
		MovementComp->IsFlying() ||
		(MovementComp->MovementMode == MOVE_Custom &&
			MovementComp->CustomMovementMode == static_cast<uint8>(EAOMovementMode::Glide)))
	{
		return false;
	}

	// Check if already dashing
	if (ASC && ASC->HasMatchingGameplayTag(STATE_DASHING))
	{
		return false;
	}

	// Cooldown check
	if (GetWorld()->GetTimeSeconds() - LastDashTime < DashCooldown)
	{
		return false;
	}

	// Stamina check
	if (ASC)
	{
		const UAOAttributeSet* AttributeSet = ASC->GetSet<UAOAttributeSet>();
		if (AttributeSet && AttributeSet->GetStamina() < 20.0f)
		{
			return false;
		}
	}

	return true;
}

void AMMODaeva::PlayDash()
{
	EMontageID SelectedMontageID = EMontageID::Dash;
	float MontagePlayRate = 1.0f;

	UAnimMontage* DashMontage = GetMontageByID(SelectedMontageID);
	if (!DashMontage)
	{
		return;
	}

	bool bForward = HasMoveInput();
	FName SectionName = bForward ? FName("Forward") : FName("Back");

	PlayMontageWithSection(SelectedMontageID, MontagePlayRate, SectionName);

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AMMODaeva::OnDashMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, DashMontage);
	}

	if (ASC)
	{
		ASC->AddLooseGameplayTag(STATE_DASHING);

		const UAOAttributeSet* AttributeSet = ASC->GetSet<UAOAttributeSet>();
		if (AttributeSet)
		{
			float CurrentStamina = AttributeSet->GetStamina();
			ASC->SetNumericAttributeBase(UAOAttributeSet::GetStaminaAttribute(), FMath::Max(0.0f, CurrentStamina - 20.0f));
		}
	}

	LastDashTime = GetWorld()->GetTimeSeconds();
}

void AMMODaeva::OnDashMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UE_LOG(LogTemp, Warning, TEXT("OnDashMontageEnded: Montage %s ended, bInterrupted = %s"),
		Montage ? *Montage->GetName() : TEXT("None"),
		bInterrupted ? TEXT("True") : TEXT("False"));

	if (ASC)
	{
		ASC->RemoveLooseGameplayTag(STATE_DASHING);
	}
}

void AMMODaeva::MMOInputShiftPressed()
{
	if (IsDead())
	{
		return;
	}

	// 대시 실행
	if (CanDash())
	{
		PlayDash();

		FVector CurrLoc = GetActorLocation();
		FRotator CurrRot = GetActorRotation();

		Protocol::C_DashPacket DashPacket;
		DashPacket.set_playerid(MyId);
		Protocol::Vector3* Location = DashPacket.mutable_playerlocation();
		Location->set_x(CurrLoc.X);
		Location->set_y(CurrLoc.Y);
		Location->set_z(CurrLoc.Z);

		FVector CurrVelocity = GetCharacterMovement()->Velocity;

		Protocol::Vector3* Velocity = DashPacket.mutable_playervelocity();
		Velocity->set_x(CurrVelocity.X);
		Velocity->set_y(CurrVelocity.Y);
		Velocity->set_z(CurrVelocity.Z);

		Protocol::Rotator3* Rotation = DashPacket.mutable_playerrotation();
		Rotation->set_pitch(CurrRot.Pitch);
		Rotation->set_yaw(CurrRot.Yaw);
		Rotation->set_roll(CurrRot.Roll);


		SEND_PACKET(DashPacket, PKT_C_DASH);


		LastLoc = CurrLoc;
		LastRot = CurrRot;

	}
}

void AMMODaeva::ReceiveDashPacket(FVector& NewLoc, FRotator& NewRot, FVector& NewVel)
{
	TargetLoc = NewLoc;
	TargetRot = NewRot;
	TargetVel = NewVel;

	EMontageID SelectedMontageID = EMontageID::Dash;
	float MontagePlayRate = 1.0f;

	UAnimMontage* DashMontage = GetMontageByID(SelectedMontageID);
	if (!DashMontage)
	{
		return;
	}

	FName SectionName = TargetVel.SizeSquared() > 10000.f ? FName("Forward") : FName("Back");

	PlayMontageWithSection(SelectedMontageID, MontagePlayRate, SectionName);

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AMMODaeva::OnDashMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, DashMontage);
	}
}